#pragma once

#include <thread>

namespace wip {
namespace time {
namespace utilities {

template<typename Rep, typename Period>
double to_time_unit(const std::chrono::duration<Rep, Period>& duration, TimeUnit unit) {
    switch (unit) {
        case TimeUnit::Nanoseconds:
            return std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();
        case TimeUnit::Microseconds:
            return std::chrono::duration_cast<std::chrono::duration<double, std::micro>>(duration).count();
        case TimeUnit::Milliseconds:
            return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(duration).count();
        case TimeUnit::Seconds:
            return std::chrono::duration_cast<std::chrono::duration<double>>(duration).count();
        case TimeUnit::Minutes:
            return std::chrono::duration_cast<std::chrono::duration<double, std::ratio<60>>>(duration).count();
        case TimeUnit::Hours:
            return std::chrono::duration_cast<std::chrono::duration<double, std::ratio<3600>>>(duration).count();
        case TimeUnit::Days:
            return std::chrono::duration_cast<std::chrono::duration<double, std::ratio<86400>>>(duration).count();
        case TimeUnit::Weeks:
            return std::chrono::duration_cast<std::chrono::duration<double, std::ratio<604800>>>(duration).count();
        default:
            return 0.0;
    }
}

template<typename Rep, typename Period>
void sleep_for(const std::chrono::duration<Rep, Period>& duration) {
    std::this_thread::sleep_for(duration);
}

template<typename Func>
std::chrono::nanoseconds measure_execution_time(Func&& func) {
    auto start = std::chrono::high_resolution_clock::now();
    func();
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
}

} // namespace utilities
} // namespace time
} // namespace wip