/** SID CRF - Simplified C++ port */
#pragma once
#include "sid_diagram.hpp"
#include <functional>
#include <string>
namespace sid {
using PredicateFn = std::function<bool(const Diagram&)>;
struct ConflictResolution {
    std::string action;
    bool success;
    std::string message;
};
}
