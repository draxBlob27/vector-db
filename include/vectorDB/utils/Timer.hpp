#ifndef TIMER_HPP
#define TIMER_HPP
#include <chrono>

class Timer {
private:
    using millisec = std::chrono::duration<double, std::ratio<1, 1000>>;
    using Clock = std::chrono::high_resolution_clock;

    std::chrono::time_point<Clock> m_beg{Clock::now()};

public:
    void reset() {
        m_beg = Clock::now();
    }

    double elapsed() const {
        return std::chrono::duration_cast<millisec>(Clock::now() - m_beg).count();
    }
};
#endif //TIMER_HPP