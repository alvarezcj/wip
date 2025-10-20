#include "time_utilities.h"
#include <thread>
#include <iostream>
#include <cmath>

namespace wip {
namespace time {
namespace utilities {

std::chrono::steady_clock::time_point now_steady() {
    return std::chrono::steady_clock::now();
}

std::chrono::system_clock::time_point now_system() {
    return std::chrono::system_clock::now();
}

std::chrono::high_resolution_clock::time_point now_high_res() {
    return std::chrono::high_resolution_clock::now();
}

std::time_t unix_timestamp() {
    return std::chrono::system_clock::to_time_t(now_system());
}

std::int64_t unix_timestamp_ms() {
    auto now = now_system();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return ms.count();
}

std::int64_t unix_timestamp_us() {
    auto now = now_system();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch());
    return us.count();
}

std::int64_t unix_timestamp_ns() {
    auto now = now_system();
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch());
    return ns.count();
}

std::chrono::nanoseconds from_time_unit(double value, TimeUnit unit) {
    switch (unit) {
        case TimeUnit::Nanoseconds:
            return std::chrono::nanoseconds(static_cast<std::int64_t>(value));
        case TimeUnit::Microseconds:
            return std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::duration<double, std::micro>(value));
        case TimeUnit::Milliseconds:
            return std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::duration<double, std::milli>(value));
        case TimeUnit::Seconds:
            return std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::duration<double>(value));
        case TimeUnit::Minutes:
            return std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::duration<double, std::ratio<60>>(value));
        case TimeUnit::Hours:
            return std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::duration<double, std::ratio<3600>>(value));
        case TimeUnit::Days:
            return std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::duration<double, std::ratio<86400>>(value));
        case TimeUnit::Weeks:
            return std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::duration<double, std::ratio<604800>>(value));
        default:
            return std::chrono::nanoseconds::zero();
    }
}

std::string format_time(const std::chrono::system_clock::time_point& time_point, 
                       DateFormat format, int timezone_offset) {
    auto time_t = std::chrono::system_clock::to_time_t(time_point);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        time_point.time_since_epoch()) % 1000;
    
    // Apply timezone offset
    time_t += timezone_offset * 3600;
    
    std::tm* tm = std::gmtime(&time_t);
    std::ostringstream oss;
    
    switch (format) {
        case DateFormat::ISO8601:
            oss << std::put_time(tm, "%Y-%m-%dT%H:%M:%S");
            if (ms.count() > 0) {
                oss << "." << std::setfill('0') << std::setw(3) << ms.count();
            }
            if (timezone_offset == 0) {
                oss << "Z";
            } else {
                oss << (timezone_offset >= 0 ? "+" : "") 
                    << std::setfill('0') << std::setw(2) << timezone_offset 
                    << ":00";
            }
            break;
            
        case DateFormat::US:
            oss << std::put_time(tm, "%m/%d/%Y %I:%M:%S %p");
            break;
            
        case DateFormat::European:
            oss << std::put_time(tm, "%d/%m/%Y %H:%M:%S");
            break;
            
        case DateFormat::RFC2822:
            oss << std::put_time(tm, "%a, %d %b %Y %H:%M:%S");
            if (timezone_offset == 0) {
                oss << " +0000";
            } else {
                oss << " " << (timezone_offset >= 0 ? "+" : "") 
                    << std::setfill('0') << std::setw(2) << std::abs(timezone_offset) 
                    << "00";
            }
            break;
            
        case DateFormat::UnixTimestamp:
            oss << std::chrono::system_clock::to_time_t(time_point);
            break;
            
        case DateFormat::Readable:
            oss << std::put_time(tm, "%A, %B %d, %Y at %I:%M:%S %p");
            break;
    }
    
    return oss.str();
}

std::chrono::system_clock::time_point parse_time(const std::string& time_string, 
                                                DateFormat format) {
    std::tm tm = {};
    std::istringstream ss(time_string);
    
    switch (format) {
        case DateFormat::ISO8601:
            ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
            break;
        case DateFormat::US:
            ss >> std::get_time(&tm, "%m/%d/%Y %I:%M:%S %p");
            break;
        case DateFormat::European:
            ss >> std::get_time(&tm, "%d/%m/%Y %H:%M:%S");
            break;
        case DateFormat::RFC2822:
            ss >> std::get_time(&tm, "%a, %d %b %Y %H:%M:%S");
            break;
        case DateFormat::UnixTimestamp: {
            std::time_t timestamp = std::stoll(time_string);
            return std::chrono::system_clock::from_time_t(timestamp);
        }
        case DateFormat::Readable:
            ss >> std::get_time(&tm, "%A, %B %d, %Y at %I:%M:%S %p");
            break;
    }
    
    if (ss.fail()) {
        return std::chrono::system_clock::time_point{};
    }
    
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

bool is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int days_in_month(int year, int month) {
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    
    if (month < 1 || month > 12) return 0;
    
    if (month == 2 && is_leap_year(year)) {
        return 29;
    }
    
    return days[month - 1];
}

int day_of_week(int year, int month, int day) {
    // Zeller's congruence algorithm
    if (month < 3) {
        month += 12;
        year--;
    }
    
    int k = year % 100;
    int j = year / 100;
    
    int h = (day + ((13 * (month + 1)) / 5) + k + (k / 4) + (j / 4) - 2 * j) % 7;
    
    // Convert to Sunday = 0, Monday = 1, ..., Saturday = 6
    return (h + 5) % 7;
}

int day_of_year(int year, int month, int day) {
    int total_days = day;
    
    for (int m = 1; m < month; ++m) {
        total_days += days_in_month(year, m);
    }
    
    return total_days;
}

int week_of_year(int year, int month, int day) {
    int day_of_year_val = day_of_year(year, month, day);
    int jan1_day_of_week = day_of_week(year, 1, 1);
    
    // Adjust for the fact that week 1 might start in the previous year
    int week = (day_of_year_val + jan1_day_of_week - 1) / 7;
    
    if (jan1_day_of_week <= 4) { // Thursday or earlier
        week++;
    }
    
    return std::max(1, week);
}

std::chrono::system_clock::time_point add_business_days(
    const std::chrono::system_clock::time_point& time_point, 
    int business_days) {
    
    auto current = time_point;
    int days_added = 0;
    
    while (days_added < business_days) {
        current += std::chrono::hours(24);
        
        auto time_t = std::chrono::system_clock::to_time_t(current);
        std::tm* tm = std::gmtime(&time_t);
        int weekday = tm->tm_wday; // 0 = Sunday, 6 = Saturday
        
        // Skip weekends
        if (weekday != 0 && weekday != 6) {
            days_added++;
        }
    }
    
    return current;
}

int business_days_between(const std::chrono::system_clock::time_point& start,
                         const std::chrono::system_clock::time_point& end) {
    if (start >= end) return 0;
    
    auto current = start;
    int business_days = 0;
    
    while (current < end) {
        current += std::chrono::hours(24);
        
        auto time_t = std::chrono::system_clock::to_time_t(current);
        std::tm* tm = std::gmtime(&time_t);
        int weekday = tm->tm_wday;
        
        // Count business days only
        if (weekday != 0 && weekday != 6 && current <= end) {
            business_days++;
        }
    }
    
    return business_days;
}

void sleep_until(const std::chrono::steady_clock::time_point& time_point) {
    std::this_thread::sleep_until(time_point);
}

// Stopwatch implementation
void Stopwatch::start() {
    start_time_ = std::chrono::high_resolution_clock::now();
    running_ = true;
    has_result_ = false;
}

void Stopwatch::stop() {
    if (running_) {
        end_time_ = std::chrono::high_resolution_clock::now();
        running_ = false;
        has_result_ = true;
    }
}

void Stopwatch::reset() {
    running_ = false;
    has_result_ = false;
}

std::chrono::nanoseconds Stopwatch::elapsed() const {
    if (running_) {
        auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(now - start_time_);
    } else if (has_result_) {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(end_time_ - start_time_);
    } else {
        return std::chrono::nanoseconds::zero();
    }
}

double Stopwatch::elapsed(TimeUnit unit) const {
    return to_time_unit(elapsed(), unit);
}

bool Stopwatch::is_running() const {
    return running_;
}

// ScopedTimer implementation
ScopedTimer::ScopedTimer(const std::string& name) 
    : name_(name), start_time_(std::chrono::high_resolution_clock::now()) {
}

ScopedTimer::~ScopedTimer() {
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time - start_time_);
    
    std::cout << "[" << name_ << "] Elapsed time: " 
              << to_time_unit(duration, TimeUnit::Milliseconds) << " ms" << std::endl;
}

std::chrono::nanoseconds ScopedTimer::elapsed() const {
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::nanoseconds>(now - start_time_);
}

} // namespace utilities
} // namespace time
} // namespace wip