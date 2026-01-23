// Shared helpers for harness tests (determinism, metrics paths).
#include "harness_test_util.hpp"

#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <regex>
#include <cstdlib>
#ifdef _WIN32
#include <Windows.h>
#include <process.h>
#endif

namespace {

std::string sanitize_token(const std::string& in) {
    std::string out;
    out.reserve(in.size());
    for (char c : in) {
        if (std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-') {
            out.push_back(c);
        } else {
            out.push_back('_');
        }
    }
    return out;
}

std::string iso_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    gmtime_s(&tm, &t);
#else
    gmtime_r(&t, &tm);
#endif
    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H-%M-%SZ", &tm);
    return std::string(buf);
}

}  // namespace

namespace harness {

std::mt19937 seeded_rng(uint64_t seed) {
    return std::mt19937{static_cast<std::mt19937::result_type>(seed)};
}

std::string hash_bytes(const std::vector<uint8_t>& data) {
    // Lightweight hash for determinism checks.
    std::size_t h = 1469598103934665603ull;
    for (auto b : data) {
        h ^= static_cast<std::size_t>(b);
        h *= 1099511628211ull;
    }
    std::ostringstream oss;
    oss << std::hex << h;
    return oss.str();
}

std::filesystem::path project_root() {
#ifdef AIRS_ROOT_PATH
    return std::filesystem::path(AIRS_ROOT_PATH);
#else
    return std::filesystem::current_path().parent_path();
#endif
}

std::filesystem::path artifacts_root() {
#ifdef ARTIFACTS_ROOT_PATH
    return std::filesystem::path(ARTIFACTS_ROOT_PATH);
#else
    return project_root() / "artifacts";
#endif
}

std::filesystem::path metrics_root() {
#ifdef METRICS_ROOT_PATH
    return std::filesystem::path(METRICS_ROOT_PATH);
#else
    return project_root() / "metrics";
#endif
}

std::filesystem::path engine_metrics_dir(const std::string& engine_family) {
    auto dir = metrics_root() / sanitize_token(engine_family);
    ensure_directory(dir);
    return dir;
}

void ensure_directory(const std::filesystem::path& path) {
    std::error_code ec;
    if (!std::filesystem::exists(path)) {
        std::filesystem::create_directories(path, ec);
    }
}

void write_metrics_json(const std::string& engine_family,
                        const std::string& test_case,
                        const std::map<std::string, double>& numeric_metrics,
                        const std::map<std::string, std::string>& string_metrics) {
    auto dir = engine_metrics_dir(engine_family);
    auto filename = sanitize_token(test_case) + "_" + iso_timestamp() + ".json";
    auto path = dir / filename;

    std::ofstream out(path, std::ios::trunc);
    if (!out.is_open()) {
        return;
    }

    out << "{\n";
    out << "  \"engine\": \"" << engine_family << "\",\n";
    out << "  \"test_case\": \"" << test_case << "\",\n";
    out << "  \"timestamp\": \"" << iso_timestamp() << "\",\n";
    out << "  \"metrics\": {\n";

    bool first = true;
    for (const auto& [k, v] : numeric_metrics) {
        if (!first) out << ",\n";
        out << "    \"" << k << "\": " << v;
        first = false;
    }
    for (const auto& [k, v] : string_metrics) {
        if (!first) out << ",\n";
        out << "    \"" << k << "\": \"" << v << "\"";
        first = false;
    }
    out << "\n  }\n";
    out << "}\n";
}

StepRunOutput run_step_runner(const std::filesystem::path& runner,
                              const std::filesystem::path& input_jsonl,
                              const std::filesystem::path& output_json) {
    ensure_directory(output_json.parent_path());
    auto rpath = std::filesystem::weakly_canonical(runner);
    auto ipath = std::filesystem::weakly_canonical(input_jsonl);
    auto opath = std::filesystem::weakly_canonical(output_json);

    int rc = 0;
#ifdef _WIN32
    // Avoid cmd.exe quoting issues by spawning the runner directly.
    auto r = rpath.wstring();
    auto i = ipath.wstring();
    auto o = opath.wstring();
    rc = _wspawnl(_P_WAIT,
                  r.c_str(),
                  r.c_str(),
                  i.c_str(),
                  o.c_str(),
                  nullptr);
#else
    std::string cmd = "\"" + rpath.string() + "\" \"" + ipath.string() + "\" \"" + opath.string() + "\"";
    rc = std::system(cmd.c_str());
#endif

    if (rc != 0) {
        std::cerr << "runner failed rc=" << rc;
#ifdef _WIN32
        std::cerr << " last_error=" << GetLastError();
#endif
        std::cerr << " cmd=" << rpath.string() << " " << ipath.string() << " " << opath.string() << "\n";
        return {};
    }
    std::ifstream in(output_json);
    if (!in.is_open()) {
        std::cerr << "output missing: " << output_json << "\n";
        return {};
    }
    std::string content((std::istreambuf_iterator<char>(in)), {});
    StepRunOutput out{};
    // Extract hash
    auto pos = content.find("\"hash\"");
    if (pos != std::string::npos) {
        pos = content.find(':', pos);
        if (pos != std::string::npos) {
            pos = content.find('"', pos);
            if (pos != std::string::npos) {
                auto end = content.find('"', pos + 1);
                if (end != std::string::npos) {
                    out.hash = content.substr(pos + 1, end - pos - 1);
                }
            }
        }
    }
    // Extract metrics as simple key:value pairs
    auto mpos = content.find("\"metrics\"");
    if (mpos != std::string::npos) {
        mpos = content.find('{', mpos);
        auto mend = content.find('}', mpos);
        if (mpos != std::string::npos && mend != std::string::npos && mend > mpos) {
            std::string block = content.substr(mpos, mend - mpos);
            std::regex kv_re("\"([^\"]+)\"\\s*:\\s*([-+0-9eE\\.]+)");
            std::smatch match;
            std::string::const_iterator search_start(block.cbegin());
            while (std::regex_search(search_start, block.cend(), match, kv_re)) {
                if (match.size() == 3) {
                    const std::string key = match[1].str();
                    const double val = std::strtod(match[2].str().c_str(), nullptr);
                    out.metrics[key] = val;
                }
                search_start = match.suffix().first;
            }
        }
    }
    return out;
}

}  // namespace harness
