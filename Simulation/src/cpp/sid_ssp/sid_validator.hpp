/** SID Validator - Simplified C++ port */
#pragma once
#include "sid_diagram.hpp"
#include <string>
#include <vector>
namespace sid {
struct ValidationError {
    std::string category;
    std::string severity;
    std::string message;
};
class DiagramValidator {
public:
    std::vector<ValidationError> validate(const Diagram& diagram) {
        std::vector<ValidationError> errors;
        if (diagram.has_cycle()) {
            errors.push_back({"cycle", "error", "Diagram contains cycle"});
        }
        return errors;
    }
};
}
