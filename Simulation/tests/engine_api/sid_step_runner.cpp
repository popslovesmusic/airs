// Minimal CLI-backed step runner for SID engines.
// Usage: sid_step_runner.exe <input.json> <output.json>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace {

std::string fnv1a_64(const std::string& data) {
    std::uint64_t h = 1469598103934665603ull;
    for (unsigned char c : data) {
        h ^= static_cast<std::uint64_t>(c);
        h *= 1099511628211ull;
    }
    std::ostringstream oss;
    oss << std::hex << h;
    return oss.str();
}

int run_cli(const std::filesystem::path& input, std::string& stdout_out) {
    std::filesystem::path cli = std::filesystem::path("Simulation") / "dase_cli" / "sid_cli.exe";
    if (!std::filesystem::exists(cli)) {
        std::cerr << "missing cli: " << cli << "\n";
        return 1;
    }
    std::string cmd = "type \"" + input.string() + "\" | \"" + cli.string() + "\"";
    FILE* pipe = _popen(cmd.c_str(), "r");
    if (!pipe) return 2;
    char buffer[4096];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        stdout_out.append(buffer);
    }
    int rc = _pclose(pipe);
    return rc;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "usage: sid_step_runner <input.json> <output.json>\n";
        return 1;
    }
    std::filesystem::path input_path = argv[1];
    std::filesystem::path output_path = argv[2];

    if (!std::filesystem::exists(input_path)) {
        std::cerr << "input missing\n";
        return 1;
    }

    std::string stdout_capture;
    int rc = run_cli(input_path, stdout_capture);
    if (rc != 0) {
        std::cerr << "cli failed: " << rc << "\n";
        return rc;
    }

    std::string hash = fnv1a_64(stdout_capture);

    std::ofstream out(output_path, std::ios::trunc);
    if (!out.is_open()) {
        std::cerr << "cannot open output\n";
        return 1;
    }
    out << "{\n";
    out << "  \"status\": \"ok\",\n";
    out << "  \"hash\": \"" << hash << "\",\n";
    out << "  \"metrics\": {}\n";
    out << "}\n";
    return 0;
}
