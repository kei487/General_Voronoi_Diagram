#pragma once

#include <chrono>
#include <string>
#include <functional>

namespace gvd_topo {

class Timer {
public:
    using Clock = std::chrono::high_resolution_clock;
    Timer() : start_(Clock::now()) {}
    void reset() { start_ = Clock::now(); }
    double ms() const {
        return std::chrono::duration<double, std::milli>(Clock::now() - start_).count();
    }
private:
    Clock::time_point start_{};
};

class ScopeTimer {
public:
    ScopeTimer(const std::string& label, const std::function<void(const std::string&, double)>& cb)
        : label_(label), cb_(cb) {}
    ~ScopeTimer() { if (cb_) cb_(label_, timer_.ms()); }
private:
    std::string label_;
    std::function<void(const std::string&, double)> cb_;
    Timer timer_{};
};

} // namespace gvd_topo
