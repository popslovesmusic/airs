// Shared helpers for harness tests (determinism, metrics paths).
#pragma once

#include <filesystem>
#include <map>
#include <random>
#include <string>
#include <vector>
namespace harness {

std::mt19937 seeded_rng(uint64_t seed = 1337);
std::string hash_bytes(const std::vector<uint8_t>& data);

std::filesystem::path project_root();
std::filesystem::path artifacts_root();
std::filesystem::path metrics_root();
std::filesystem::path engine_metrics_dir(const std::string& engine_family);

void ensure_directory(const std::filesystem::path& path);

// Minimal JSON metrics writer (writes doubles and strings).
void write_metrics_json(const std::string& engine_family,
                        const std::string& test_case,
                        const std::map<std::string, double>& numeric_metrics,
                        const std::map<std::string, std::string>& string_metrics = {});

struct StepRunOutput {
    std::string hash;
    std::map<std::string, double> metrics;
};

// Run a step runner and return hash + metrics parsed from its output JSON.
StepRunOutput run_step_runner(const std::filesystem::path& runner,
                              const std::filesystem::path& input_jsonl,
                              const std::filesystem::path& output_json);

}  // namespace harness
