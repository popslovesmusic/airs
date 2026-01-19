/** SID Package Validator - Simplified C++ port */
#pragma once
#include <string>
#include <vector>
namespace sid {
struct PackageValidationResult {
    std::vector<std::string> errors;
    std::vector<std::string> warnings;
};
}
