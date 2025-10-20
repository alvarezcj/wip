#pragma once

#include <chrono>
#include <string>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace wip {
namespace time {
namespace utilities {

/**
 * @brief Time unit enumeration for duration conversions
 */
enum class TimeUnit {
    Nanoseconds,
    Microseconds,
    Milliseconds,
    Seconds,
    Minutes,
    Hours,
    Days,
    Weeks
};

/**
 * @brief Date format enumeration for string conversions
 */
enum class DateFormat {
    ISO8601,        // 2023-12-25T15:30:45Z
    US,             // 12/25/2023 3:30:45 PM
    European,       // 25/12/2023 15:30:45
    RFC2822,        // Mon, 25 Dec 2023 15:30:45 +0000
    UnixTimestamp,  // 1703520645
    Readable        // Monday, December 25, 2023 at 3:30:45 PM
};

/**
 * @brief Get current system time as steady clock time point
 */
std::chrono::steady_clock::time_point now_steady();

/**
 * @brief Get current system time as system clock time point
 */
std::chrono::system_clock::time_point now_system();

/**
 * @brief Get current time as high resolution clock time point
 */
std::chrono::high_resolution_clock::time_point now_high_res();

/**
 * @brief Get current Unix timestamp in seconds
 */
std::time_t unix_timestamp();

/**
 * @brief Get current Unix timestamp in milliseconds
 */
std::int64_t unix_timestamp_ms();

/**
 * @brief Get current Unix timestamp in microseconds
 */
std::int64_t unix_timestamp_us();

/**
 * @brief Get current Unix timestamp in nanoseconds
 */
std::int64_t unix_timestamp_ns();

/**
 * @brief Convert duration to specified time unit
 * @param duration Duration to convert
 * @param unit Target time unit
 * @return Duration value in the specified unit
 */
template<typename Rep, typename Period>
double to_time_unit(const std::chrono::duration<Rep, Period>& duration, TimeUnit unit);

/**
 * @brief Create duration from value and time unit
 * @param value Numeric value
 * @param unit Time unit
 * @return Duration in nanoseconds
 */
std::chrono::nanoseconds from_time_unit(double value, TimeUnit unit);

/**
 * @brief Format time point as string
 * @param time_point Time point to format
 * @param format Desired date format
 * @param timezone_offset Timezone offset in hours (default: 0 for UTC)
 * @return Formatted time string
 */
std::string format_time(const std::chrono::system_clock::time_point& time_point, 
                       DateFormat format = DateFormat::ISO8601,
                       int timezone_offset = 0);

/**
 * @brief Parse time string into time point
 * @param time_string Time string to parse
 * @param format Expected format of the string
 * @return Parsed time point, or epoch if parsing fails
 */
std::chrono::system_clock::time_point parse_time(const std::string& time_string, 
                                                DateFormat format = DateFormat::ISO8601);

/**
 * @brief Check if a year is a leap year
 * @param year Year to check
 * @return True if leap year, false otherwise
 */
bool is_leap_year(int year);

/**
 * @brief Get number of days in a month
 * @param year Year
 * @param month Month (1-12)
 * @return Number of days in the month
 */
int days_in_month(int year, int month);

/**
 * @brief Get day of week (0=Sunday, 6=Saturday)
 * @param year Year
 * @param month Month (1-12)
 * @param day Day (1-31)
 * @return Day of week
 */
int day_of_week(int year, int month, int day);

/**
 * @brief Get day of year (1-366)
 * @param year Year
 * @param month Month (1-12)
 * @param day Day (1-31)
 * @return Day of year
 */
int day_of_year(int year, int month, int day);

/**
 * @brief Get week number of year (1-53)
 * @param year Year
 * @param month Month (1-12)
 * @param day Day (1-31)
 * @return Week number
 */
int week_of_year(int year, int month, int day);

/**
 * @brief Add business days to a date (excluding weekends)
 * @param time_point Starting time point
 * @param business_days Number of business days to add
 * @return New time point
 */
std::chrono::system_clock::time_point add_business_days(
    const std::chrono::system_clock::time_point& time_point, 
    int business_days);

/**
 * @brief Calculate business days between two dates
 * @param start Start time point
 * @param end End time point
 * @return Number of business days
 */
int business_days_between(const std::chrono::system_clock::time_point& start,
                         const std::chrono::system_clock::time_point& end);

/**
 * @brief Sleep for specified duration
 * @param duration Duration to sleep
 */
template<typename Rep, typename Period>
void sleep_for(const std::chrono::duration<Rep, Period>& duration);

/**
 * @brief Sleep until specified time point
 * @param time_point Time point to sleep until
 */
void sleep_until(const std::chrono::steady_clock::time_point& time_point);

/**
 * @brief Measure execution time of a function
 * @param func Function to measure
 * @return Execution time in nanoseconds
 */
template<typename Func>
std::chrono::nanoseconds measure_execution_time(Func&& func);

/**
 * @brief Simple stopwatch class for timing operations
 */
class Stopwatch {
public:
    /**
     * @brief Start the stopwatch
     */
    void start();
    
    /**
     * @brief Stop the stopwatch
     */
    void stop();
    
    /**
     * @brief Reset the stopwatch
     */
    void reset();
    
    /**
     * @brief Get elapsed time
     * @return Elapsed time in nanoseconds
     */
    std::chrono::nanoseconds elapsed() const;
    
    /**
     * @brief Get elapsed time in specified unit
     * @param unit Target time unit
     * @return Elapsed time in the specified unit
     */
    double elapsed(TimeUnit unit) const;
    
    /**
     * @brief Check if stopwatch is running
     * @return True if running, false otherwise
     */
    bool is_running() const;

private:
    std::chrono::high_resolution_clock::time_point start_time_;
    std::chrono::high_resolution_clock::time_point end_time_;
    bool running_ = false;
    bool has_result_ = false;
};

/**
 * @brief RAII timer for automatic time measurement
 */
class ScopedTimer {
public:
    /**
     * @brief Constructor starts timing
     * @param name Name for this timer
     */
    explicit ScopedTimer(const std::string& name = "Timer");
    
    /**
     * @brief Destructor prints elapsed time
     */
    ~ScopedTimer();
    
    /**
     * @brief Get elapsed time so far
     * @return Elapsed time in nanoseconds
     */
    std::chrono::nanoseconds elapsed() const;

private:
    std::string name_;
    std::chrono::high_resolution_clock::time_point start_time_;
};

} // namespace utilities
} // namespace time
} // namespace wip

// Template implementations
#include "time_utilities_impl.h"