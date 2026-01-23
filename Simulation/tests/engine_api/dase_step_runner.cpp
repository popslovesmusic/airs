// Minimal CLI-backed step runner for non-SID engines.
// Usage: dase_step_runner.exe <input.json> <output.json>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

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
    std::filesystem::path repo_root = std::filesystem::current_path().parent_path();
    std::filesystem::path cli = repo_root / "Simulation" / "dase_cli" / "dase_cli.exe";
    if (!std::filesystem::exists(cli)) {
        std::cerr << "missing cli: " << cli << "\n";
        return 1;
    }
    auto tmp_out = std::filesystem::temp_directory_path() / "dase_step_runner_out.txt";
    std::string cmd = "powershell -NoLogo -NoProfile -Command \"Get-Content -Raw '" + input.string() + "' | & '" + cli.string() + "' | Set-Content -Encoding ASCII '" + tmp_out.string() + "'\"";
    int rc = std::system(cmd.c_str());
    if (rc != 0) {
        return rc;
    }
    std::ifstream out(tmp_out, std::ios::binary);
    if (!out.is_open()) return 3;
    stdout_out.assign(std::istreambuf_iterator<char>(out), {});
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << "usage: dase_step_runner <input.json> <output.json>\n";
        return 1;
    }
    std::filesystem::path input_path = argv[1];
    std::filesystem::path output_path = argv[2];

    // Ensure input exists.
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

    // Hash captured stdout as proxy for state.
    std::string hash = fnv1a_64(stdout_capture);

    // Emit minimal output JSON.
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
