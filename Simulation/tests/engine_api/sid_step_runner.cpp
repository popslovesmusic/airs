// Minimal CLI-backed step runner for SID engines.
// Usage: sid_step_runner.exe <input.json> <output.json>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <regex>
#include <cctype>
#include <cstdlib>
#ifdef _WIN32
#include <windows.h>
#endif

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

std::string normalize_stdout(const std::string& raw) {
    std::stringstream ss(raw);
    std::string line;
    std::string normalized;
    while (std::getline(ss, line)) {
        if (line.find("\"status\":\"success\"") == std::string::npos) continue;
        line = std::regex_replace(line, std::regex("\"execution_time_ms\"\\s*:\\s*[^,}]+,?"), "");
        line = std::regex_replace(line, std::regex("\"engine_id\"\\s*:\\s*\"[^\"]*\""), "\"engine_id\":\"eng\"");
        normalized += line;
        normalized.push_back('\n');
    }
    return normalized;
}

double extract_metric(const std::string& raw, const std::string& key) {
    const std::string pattern = "\"" + key + "\"";
    auto pos = raw.rfind(pattern);
    if (pos == std::string::npos) return 0.0;
    pos = raw.find(':', pos);
    if (pos == std::string::npos) return 0.0;
    ++pos;
    while (pos < raw.size() && std::isspace(static_cast<unsigned char>(raw[pos]))) ++pos;
    const char* start = raw.c_str() + pos;
    char* endptr = nullptr;
    double val = std::strtod(start, &endptr);
    if (endptr == start) return 0.0;
    return val;
}

int run_cli(const std::filesystem::path& exe_path,
            const std::filesystem::path& input,
            std::string& stdout_out) {
    auto exe_dir = exe_path.parent_path();                // .../build/Debug
    auto repo_root = exe_dir.parent_path().parent_path(); // .../airs
    std::filesystem::path cli = repo_root / "Simulation" / "dase_cli" / "dase_cli.exe";
    if (!std::filesystem::exists(cli)) {
        std::cerr << "missing cli: " << cli << "\n";
        return 1;
    }

#ifndef _WIN32
    std::cerr << "step runner currently supports Windows only\n";
    return 1;
#else
    std::ifstream in(input, std::ios::binary);
    if (!in.is_open()) {
        std::cerr << "cannot open input: " << input << "\n";
        return 1;
    }
    std::string payload((std::istreambuf_iterator<char>(in)), {});

    SECURITY_ATTRIBUTES sa{};
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = nullptr;
    sa.bInheritHandle = TRUE;

    HANDLE child_stdin_read = nullptr;
    HANDLE child_stdin_write = nullptr;
    HANDLE child_stdout_read = nullptr;
    HANDLE child_stdout_write = nullptr;

    if (!CreatePipe(&child_stdin_read, &child_stdin_write, &sa, 0)) {
        std::cerr << "CreatePipe stdin failed\n";
        return 1;
    }
    if (!CreatePipe(&child_stdout_read, &child_stdout_write, &sa, 0)) {
        CloseHandle(child_stdin_read);
        CloseHandle(child_stdin_write);
        std::cerr << "CreatePipe stdout failed\n";
        return 1;
    }
    SetHandleInformation(child_stdin_write, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(child_stdout_read, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFOW si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = child_stdin_read;
    si.hStdOutput = child_stdout_write;
    si.hStdError = child_stdout_write;

    PROCESS_INFORMATION pi{};

    std::wstring cli_w = cli.wstring();
    std::wstring cmd_line = L"\"" + cli_w + L"\"";

    BOOL ok = CreateProcessW(
        cli_w.c_str(),
        cmd_line.data(),
        nullptr,
        nullptr,
        TRUE,
        CREATE_NO_WINDOW,
        nullptr,
        nullptr,
        &si,
        &pi);

    CloseHandle(child_stdin_read);
    CloseHandle(child_stdout_write);

    if (!ok) {
        std::cerr << "CreateProcess failed: " << GetLastError() << "\n";
        CloseHandle(child_stdin_write);
        CloseHandle(child_stdout_read);
        return 1;
    }

    DWORD written = 0;
    if (!payload.empty()) {
        if (!WriteFile(child_stdin_write, payload.data(), static_cast<DWORD>(payload.size()), &written, nullptr)) {
            std::cerr << "WriteFile stdin failed: " << GetLastError() << "\n";
        }
    }
    CloseHandle(child_stdin_write);

    constexpr DWORD kBufSize = 4096;
    char buffer[kBufSize];
    DWORD read = 0;
    while (ReadFile(child_stdout_read, buffer, kBufSize, &read, nullptr) && read > 0) {
        stdout_out.append(buffer, buffer + read);
    }
    CloseHandle(child_stdout_read);

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exit_code = 0;
    GetExitCodeProcess(pi.hProcess, &exit_code);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return static_cast<int>(exit_code);
#endif
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
    std::filesystem::path exe_path = std::filesystem::absolute(argv[0]);
    int rc = run_cli(exe_path, input_path, stdout_capture);
    if (rc != 0) {
        std::cerr << "cli failed: " << rc << "\n";
        return rc;
    }

    std::string normalized = normalize_stdout(stdout_capture);
    std::string hash = fnv1a_64(normalized);
    double active_nodes = extract_metric(stdout_capture, "active_nodes");
    double total_mass = extract_metric(stdout_capture, "total_mass");

    std::ofstream out(output_path, std::ios::trunc);
    if (!out.is_open()) {
        std::cerr << "cannot open output\n";
        return 1;
    }
    out << "{\n";
    out << "  \"status\": \"ok\",\n";
    out << "  \"hash\": \"" << hash << "\",\n";
    out << "  \"metrics\": {\"active_nodes\": " << active_nodes << ", \"total_mass\": " << total_mass << "}\n";
    out << "}\n";
    return 0;
}
