/** SID Stability - C++ port from sids/sid_stability.py */
#pragma once
#include "sid_diagram.hpp"
#include <deque>
namespace sid {
constexpr int MAX_LOOP_HISTORY = 100;
struct StabilityResult {
    bool stable;
    std::string message;
};
class StabilityAnalyzer {
private:
    std::deque<double> history_;
public:
    bool check_convergence(double value, double epsilon = 1e-6) {
        history_.push_back(value);
        if (history_.size() > MAX_LOOP_HISTORY) history_.pop_front();
        if (history_.size() < 2) return false;
        double prev = history_[history_.size() - 2];
        return std::abs(value - prev) < epsilon;
    }
};
}
