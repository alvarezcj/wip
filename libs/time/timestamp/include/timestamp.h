#pragma once

#include <chrono>
#include <string>
#include <ctime>

namespace wip {
namespace time {
namespace timestamp {

/**
 * @brief Represents a specific point in time with comprehensive functionality
 */
class Timestamp {
public:
    /**
     * @brief Default constructor - creates timestamp for current time
     */
    Timestamp();
    
    /**
     * @brief Constructor from system clock time point
     * @param time_point System clock time point
     */
    explicit Timestamp(const std::chrono::system_clock::time_point& time_point);
    
    /**
     * @brief Constructor from Unix timestamp in seconds
     * @param unix_timestamp Unix timestamp in seconds
     */
    explicit Timestamp(std::time_t unix_timestamp);
    
    /**
     * @brief Constructor from date and time components
     * @param year Year (e.g., 2023)
     * @param month Month (1-12)
     * @param day Day (1-31)
     * @param hour Hour (0-23)
     * @param minute Minute (0-59)
     * @param second Second (0-59)
     * @param millisecond Millisecond (0-999)
     */
    Timestamp(int year, int month, int day, 
              int hour = 0, int minute = 0, int second = 0, int millisecond = 0);
    
    /**
     * @brief Constructor from ISO 8601 string
     * @param iso_string ISO 8601 formatted string (e.g., "2023-12-25T15:30:45Z")
     */
    explicit Timestamp(const std::string& iso_string);
    
    /**
     * @brief Copy constructor
     */
    Timestamp(const Timestamp& other) = default;
    
    /**
     * @brief Assignment operator
     */
    Timestamp& operator=(const Timestamp& other) = default;
    
    /**
     * @brief Move constructor
     */
    Timestamp(Timestamp&& other) noexcept = default;
    
    /**
     * @brief Move assignment operator
     */
    Timestamp& operator=(Timestamp&& other) noexcept = default;
    
    /**
     * @brief Destructor
     */
    ~Timestamp() = default;
    
    // Static factory methods
    
    /**
     * @brief Create timestamp for current time
     */
    static Timestamp now();
    
    /**
     * @brief Create timestamp for Unix epoch (1970-01-01 00:00:00 UTC)
     */
    static Timestamp epoch();
    
    /**
     * @brief Create timestamp from Unix timestamp in seconds
     */
    static Timestamp from_unix(std::time_t unix_timestamp);
    
    /**
     * @brief Create timestamp from Unix timestamp in milliseconds
     */
    static Timestamp from_unix_ms(std::int64_t unix_timestamp_ms);
    
    /**
     * @brief Create timestamp from Unix timestamp in microseconds
     */
    static Timestamp from_unix_us(std::int64_t unix_timestamp_us);
    
    /**
     * @brief Create timestamp from Unix timestamp in nanoseconds
     */
    static Timestamp from_unix_ns(std::int64_t unix_timestamp_ns);
    
    /**
     * @brief Create timestamp from ISO 8601 string
     */
    static Timestamp from_iso_string(const std::string& iso_string);
    
    // Getters
    
    /**
     * @brief Get the underlying time point
     */
    const std::chrono::system_clock::time_point& time_point() const;
    
    /**
     * @brief Get Unix timestamp in seconds
     */
    std::time_t unix_timestamp() const;
    
    /**
     * @brief Get Unix timestamp in milliseconds
     */
    std::int64_t unix_timestamp_ms() const;
    
    /**
     * @brief Get Unix timestamp in microseconds
     */
    std::int64_t unix_timestamp_us() const;
    
    /**
     * @brief Get Unix timestamp in nanoseconds
     */
    std::int64_t unix_timestamp_ns() const;
    
    /**
     * @brief Get year component
     */
    int year() const;
    
    /**
     * @brief Get month component (1-12)
     */
    int month() const;
    
    /**
     * @brief Get day component (1-31)
     */
    int day() const;
    
    /**
     * @brief Get hour component (0-23)
     */
    int hour() const;
    
    /**
     * @brief Get minute component (0-59)
     */
    int minute() const;
    
    /**
     * @brief Get second component (0-59)
     */
    int second() const;
    
    /**
     * @brief Get millisecond component (0-999)
     */
    int millisecond() const;
    
    /**
     * @brief Get microsecond component (0-999999)
     */
    int microsecond() const;
    
    /**
     * @brief Get nanosecond component (0-999999999)
     */
    int nanosecond() const;
    
    /**
     * @brief Get day of week (0=Sunday, 6=Saturday)
     */
    int day_of_week() const;
    
    /**
     * @brief Get day of year (1-366)
     */
    int day_of_year() const;
    
    /**
     * @brief Get week of year (1-53)
     */
    int week_of_year() const;
    
    // Formatting
    
    /**
     * @brief Format as ISO 8601 string
     * @param include_milliseconds Include milliseconds in output
     * @return ISO 8601 formatted string
     */
    std::string to_iso_string(bool include_milliseconds = true) const;
    
    /**
     * @brief Format as RFC 2822 string
     * @return RFC 2822 formatted string
     */
    std::string to_rfc2822_string() const;
    
    /**
     * @brief Format as Unix timestamp string
     * @return Unix timestamp as string
     */
    std::string to_unix_string() const;
    
    /**
     * @brief Format as readable string
     * @return Human-readable formatted string
     */
    std::string to_readable_string() const;
    
    /**
     * @brief Format with custom format string
     * @param format Format string (strftime compatible)
     * @return Formatted string
     */
    std::string format(const std::string& format_string) const;
    
    // Arithmetic operations
    
    /**
     * @brief Add duration to timestamp
     * @param duration Duration to add
     * @return New timestamp
     */
    template<typename Rep, typename Period>
    Timestamp operator+(const std::chrono::duration<Rep, Period>& duration) const;
    
    /**
     * @brief Subtract duration from timestamp
     * @param duration Duration to subtract
     * @return New timestamp
     */
    template<typename Rep, typename Period>
    Timestamp operator-(const std::chrono::duration<Rep, Period>& duration) const;
    
    /**
     * @brief Calculate duration between timestamps
     * @param other Other timestamp
     * @return Duration between timestamps
     */
    std::chrono::nanoseconds operator-(const Timestamp& other) const;
    
    /**
     * @brief Add duration to this timestamp
     * @param duration Duration to add
     * @return Reference to this timestamp
     */
    template<typename Rep, typename Period>
    Timestamp& operator+=(const std::chrono::duration<Rep, Period>& duration);
    
    /**
     * @brief Subtract duration from this timestamp
     * @param duration Duration to subtract
     * @return Reference to this timestamp
     */
    template<typename Rep, typename Period>
    Timestamp& operator-=(const std::chrono::duration<Rep, Period>& duration);
    
    // Convenience addition methods
    
    /**
     * @brief Add years to timestamp
     * @param years Number of years to add
     * @return New timestamp
     */
    Timestamp add_years(int years) const;
    
    /**
     * @brief Add months to timestamp
     * @param months Number of months to add
     * @return New timestamp
     */
    Timestamp add_months(int months) const;
    
    /**
     * @brief Add days to timestamp
     * @param days Number of days to add
     * @return New timestamp
     */
    Timestamp add_days(int days) const;
    
    /**
     * @brief Add hours to timestamp
     * @param hours Number of hours to add
     * @return New timestamp
     */
    Timestamp add_hours(int hours) const;
    
    /**
     * @brief Add minutes to timestamp
     * @param minutes Number of minutes to add
     * @return New timestamp
     */
    Timestamp add_minutes(int minutes) const;
    
    /**
     * @brief Add seconds to timestamp
     * @param seconds Number of seconds to add
     * @return New timestamp
     */
    Timestamp add_seconds(int seconds) const;
    
    /**
     * @brief Add milliseconds to timestamp
     * @param milliseconds Number of milliseconds to add
     * @return New timestamp
     */
    Timestamp add_milliseconds(int milliseconds) const;
    
    // Comparison operators
    
    bool operator==(const Timestamp& other) const;
    bool operator!=(const Timestamp& other) const;
    bool operator<(const Timestamp& other) const;
    bool operator<=(const Timestamp& other) const;
    bool operator>(const Timestamp& other) const;
    bool operator>=(const Timestamp& other) const;
    
    // Utility methods
    
    /**
     * @brief Check if this timestamp is before another
     * @param other Other timestamp
     * @return True if this timestamp is before other
     */
    bool is_before(const Timestamp& other) const;
    
    /**
     * @brief Check if this timestamp is after another
     * @param other Other timestamp
     * @return True if this timestamp is after other
     */
    bool is_after(const Timestamp& other) const;
    
    /**
     * @brief Check if this timestamp is between two others (inclusive)
     * @param start Start timestamp
     * @param end End timestamp
     * @return True if this timestamp is between start and end
     */
    bool is_between(const Timestamp& start, const Timestamp& end) const;
    
    /**
     * @brief Check if the date is a leap year
     * @return True if leap year
     */
    bool is_leap_year() const;
    
    /**
     * @brief Check if the date is a weekend (Saturday or Sunday)
     * @return True if weekend
     */
    bool is_weekend() const;
    
    /**
     * @brief Check if the date is a weekday (Monday through Friday)
     * @return True if weekday
     */
    bool is_weekday() const;
    
    /**
     * @brief Get the start of the day (00:00:00.000)
     * @return Timestamp for start of day
     */
    Timestamp start_of_day() const;
    
    /**
     * @brief Get the end of the day (23:59:59.999)
     * @return Timestamp for end of day
     */
    Timestamp end_of_day() const;
    
    /**
     * @brief Get the start of the week (Sunday 00:00:00.000)
     * @return Timestamp for start of week
     */
    Timestamp start_of_week() const;
    
    /**
     * @brief Get the end of the week (Saturday 23:59:59.999)
     * @return Timestamp for end of week
     */
    Timestamp end_of_week() const;
    
    /**
     * @brief Get the start of the month (1st day 00:00:00.000)
     * @return Timestamp for start of month
     */
    Timestamp start_of_month() const;
    
    /**
     * @brief Get the end of the month (last day 23:59:59.999)
     * @return Timestamp for end of month
     */
    Timestamp end_of_month() const;
    
    /**
     * @brief Get the start of the year (January 1st 00:00:00.000)
     * @return Timestamp for start of year
     */
    Timestamp start_of_year() const;
    
    /**
     * @brief Get the end of the year (December 31st 23:59:59.999)
     * @return Timestamp for end of year
     */
    Timestamp end_of_year() const;
    
    /**
     * @brief Get a string representation of the timestamp
     * @return String representation
     */
    std::string to_string() const;

private:
    std::chrono::system_clock::time_point time_point_;
    
    /**
     * @brief Get tm structure for the timestamp
     * @return tm structure
     */
    std::tm to_tm() const;
};

// Template method implementations
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