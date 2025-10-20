#pragma once

namespace wip {
namespace time {
namespace timestamp {

template<typename Rep, typename Period>
Timestamp Timestamp::operator+(const std::chrono::duration<Rep, Period>& duration) const {
    return Timestamp(time_point_ + duration);
}

template<typename Rep, typename Period>
Timestamp Timestamp::operator-(const std::chrono::duration<Rep, Period>& duration) const {
    return Timestamp(time_point_ - duration);
}

template<typename Rep, typename Period>
Timestamp& Timestamp::operator+=(const std::chrono::duration<Rep, Period>& duration) {
    time_point_ += duration;
    return *this;
}

template<typename Rep, typename Period>
Timestamp& Timestamp::operator-=(const std::chrono::duration<Rep, Period>& duration) {
    time_point_ -= duration;
    return *this;
}

} // namespace timestamp
} // namespace time
} // namespace wip