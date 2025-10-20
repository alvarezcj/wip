#include "timestamp.h"
#include <sstream>
#include <iomanip>
#include <cmath>

#ifndef _WIN32
#define _GNU_SOURCE
#include <time.h>
#endif

namespace wip {
namespace time {
namespace timestamp {

// Constructors
Timestamp::Timestamp() : time_point_(std::chrono::system_clock::now()) {
}

Timestamp::Timestamp(const std::chrono::system_clock::time_point& time_point) 
    : time_point_(time_point) {
}

Timestamp::Timestamp(std::time_t unix_timestamp) 
    : time_point_(std::chrono::system_clock::from_time_t(unix_timestamp)) {
}

Timestamp::Timestamp(int year, int month, int day, int hour, int minute, int second, int millisecond) {
    std::tm tm = {};
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = minute;
    tm.tm_sec = second;
    
    // Use timegm if available, otherwise use mktime and adjust
#ifdef _WIN32
    time_point_ = std::chrono::system_clock::from_time_t(_mkgmtime(&tm));
#else
    time_point_ = std::chrono::system_clock::from_time_t(timegm(&tm));
#endif
    time_point_ += std::chrono::milliseconds(millisecond);
}

Timestamp::Timestamp(const std::string& iso_string) {
    // Parse ISO 8601 string (simplified version)
    std::tm tm = {};
    std::istringstream ss(iso_string);
    
    // Try to parse with format YYYY-MM-DDTHH:MM:SS
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
    
    if (ss.fail()) {
        // Fallback to current time if parsing fails
        time_point_ = std::chrono::system_clock::now();
    } else {
#ifdef _WIN32
        time_point_ = std::chrono::system_clock::from_time_t(_mkgmtime(&tm));
#else
        time_point_ = std::chrono::system_clock::from_time_t(timegm(&tm));
#endif
    }
}

// Static factory methods
Timestamp Timestamp::now() {
    return Timestamp();
}

Timestamp Timestamp::epoch() {
    return Timestamp(std::chrono::system_clock::time_point{});
}

Timestamp Timestamp::from_unix(std::time_t unix_timestamp) {
    return Timestamp(unix_timestamp);
}

Timestamp Timestamp::from_unix_ms(std::int64_t unix_timestamp_ms) {
    auto time_point = std::chrono::system_clock::time_point{} + std::chrono::milliseconds(unix_timestamp_ms);
    return Timestamp(time_point);
}

Timestamp Timestamp::from_unix_us(std::int64_t unix_timestamp_us) {
    auto ms = unix_timestamp_us / 1000;
    auto us = unix_timestamp_us % 1000;
    auto timestamp = Timestamp(ms);
    timestamp.time_point_ += std::chrono::microseconds(us);
    return timestamp;
}

Timestamp Timestamp::from_unix_ns(std::int64_t unix_timestamp_ns) {
    auto ms = unix_timestamp_ns / 1000000;
    auto ns = unix_timestamp_ns % 1000000;
    auto timestamp = Timestamp(ms);
    timestamp.time_point_ += std::chrono::nanoseconds(ns);
    return timestamp;
}

Timestamp Timestamp::from_iso_string(const std::string& iso_string) {
    return Timestamp(iso_string);
}

// Getters
const std::chrono::system_clock::time_point& Timestamp::time_point() const {
    return time_point_;
}

std::time_t Timestamp::unix_timestamp() const {
    return std::chrono::system_clock::to_time_t(time_point_);
}

std::int64_t Timestamp::unix_timestamp_ms() const {
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_point_.time_since_epoch());
    return ms.count();
}

std::int64_t Timestamp::unix_timestamp_us() const {
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(time_point_.time_since_epoch());
    return us.count();
}

std::int64_t Timestamp::unix_timestamp_ns() const {
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(time_point_.time_since_epoch());
    return ns.count();
}

std::tm Timestamp::to_tm() const {
    auto time_t = std::chrono::system_clock::to_time_t(time_point_);
    std::tm tm;
#ifdef _WIN32
    gmtime_s(&tm, &time_t);
#else
    gmtime_r(&time_t, &tm);
#endif
    return tm;
}

int Timestamp::year() const {
    return to_tm().tm_year + 1900;
}

int Timestamp::month() const {
    return to_tm().tm_mon + 1;
}

int Timestamp::day() const {
    return to_tm().tm_mday;
}

int Timestamp::hour() const {
    return to_tm().tm_hour;
}

int Timestamp::minute() const {
    return to_tm().tm_min;
}

int Timestamp::second() const {
    return to_tm().tm_sec;
}

int Timestamp::millisecond() const {
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_point_.time_since_epoch());
    return ms.count() % 1000;
}

int Timestamp::microsecond() const {
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(time_point_.time_since_epoch());
    return us.count() % 1000000;
}

int Timestamp::nanosecond() const {
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(time_point_.time_since_epoch());
    return ns.count() % 1000000000;
}

int Timestamp::day_of_week() const {
    return to_tm().tm_wday;
}

int Timestamp::day_of_year() const {
    return to_tm().tm_yday + 1;
}

int Timestamp::week_of_year() const {
    auto tm = to_tm();
    int day_of_year = tm.tm_yday + 1;
    int jan1_day_of_week = (tm.tm_wday - tm.tm_yday) % 7;
    if (jan1_day_of_week < 0) jan1_day_of_week += 7;
    
    int week = (day_of_year + jan1_day_of_week - 1) / 7;
    if (jan1_day_of_week <= 4) {
        week++;
    }
    
    return std::max(1, week);
}

// Formatting
std::string Timestamp::to_iso_string(bool include_milliseconds) const {
    auto time_t = std::chrono::system_clock::to_time_t(time_point_);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(time_point_.time_since_epoch()) % 1000;
    
    std::tm tm;
#ifdef _WIN32
    gmtime_s(&tm, &time_t);
#else
    gmtime_r(&time_t, &tm);
#endif
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");
    
    if (include_milliseconds && ms.count() > 0) {
        oss << "." << std::setfill('0') << std::setw(3) << ms.count();
    }
    
    oss << "Z";
    return oss.str();
}

std::string Timestamp::to_rfc2822_string() const {
    auto time_t = std::chrono::system_clock::to_time_t(time_point_);
    std::tm tm;
#ifdef _WIN32
    gmtime_s(&tm, &time_t);
#else
    gmtime_r(&time_t, &tm);
#endif
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%a, %d %b %Y %H:%M:%S +0000");
    return oss.str();
}

std::string Timestamp::to_unix_string() const {
    return std::to_string(unix_timestamp());
}

std::string Timestamp::to_readable_string() const {
    auto time_t = std::chrono::system_clock::to_time_t(time_point_);
    std::tm tm = to_tm();
    
    std::ostringstream oss;
    oss << std::put_time(&tm, "%A, %B %d, %Y at %I:%M:%S %p");
    return oss.str();
}

std::string Timestamp::format(const std::string& format_string) const {
    auto tm = to_tm();
    std::ostringstream oss;
    oss << std::put_time(&tm, format_string.c_str());
    return oss.str();
}

// Arithmetic operations
std::chrono::nanoseconds Timestamp::operator-(const Timestamp& other) const {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(time_point_ - other.time_point_);
}

// Convenience addition methods
Timestamp Timestamp::add_years(int years) const {
    auto tm = to_tm();
    tm.tm_year += years;
    return Timestamp(std::chrono::system_clock::from_time_t(std::mktime(&tm)));
}

Timestamp Timestamp::add_months(int months) const {
    auto tm = to_tm();
    tm.tm_mon += months;
    
    // Handle month overflow/underflow
    while (tm.tm_mon >= 12) {
        tm.tm_mon -= 12;
        tm.tm_year++;
    }
    while (tm.tm_mon < 0) {
        tm.tm_mon += 12;
        tm.tm_year--;
    }
    
    return Timestamp(std::chrono::system_clock::from_time_t(std::mktime(&tm)));
}

Timestamp Timestamp::add_days(int days) const {
    return *this + std::chrono::hours(24 * days);
}

Timestamp Timestamp::add_hours(int hours) const {
    return *this + std::chrono::hours(hours);
}

Timestamp Timestamp::add_minutes(int minutes) const {
    return *this + std::chrono::minutes(minutes);
}

Timestamp Timestamp::add_seconds(int seconds) const {
    return *this + std::chrono::seconds(seconds);
}

Timestamp Timestamp::add_milliseconds(int milliseconds) const {
    return *this + std::chrono::milliseconds(milliseconds);
}

// Comparison operators
bool Timestamp::operator==(const Timestamp& other) const {
    return time_point_ == other.time_point_;
}

bool Timestamp::operator!=(const Timestamp& other) const {
    return time_point_ != other.time_point_;
}

bool Timestamp::operator<(const Timestamp& other) const {
    return time_point_ < other.time_point_;
}

bool Timestamp::operator<=(const Timestamp& other) const {
    return time_point_ <= other.time_point_;
}

bool Timestamp::operator>(const Timestamp& other) const {
    return time_point_ > other.time_point_;
}

bool Timestamp::operator>=(const Timestamp& other) const {
    return time_point_ >= other.time_point_;
}

// Utility methods
bool Timestamp::is_before(const Timestamp& other) const {
    return *this < other;
}

bool Timestamp::is_after(const Timestamp& other) const {
    return *this > other;
}

bool Timestamp::is_between(const Timestamp& start, const Timestamp& end) const {
    return *this >= start && *this <= end;
}

bool Timestamp::is_leap_year() const {
    int y = year();
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

bool Timestamp::is_weekend() const {
    int dow = day_of_week();
    return dow == 0 || dow == 6; // Sunday or Saturday
}

bool Timestamp::is_weekday() const {
    return !is_weekend();
}

Timestamp Timestamp::start_of_day() const {
    return Timestamp(year(), month(), day(), 0, 0, 0, 0);
}

Timestamp Timestamp::end_of_day() const {
    return Timestamp(year(), month(), day(), 23, 59, 59, 999);
}

Timestamp Timestamp::start_of_week() const {
    int days_since_sunday = day_of_week();
    return start_of_day().add_days(-days_since_sunday);
}

Timestamp Timestamp::end_of_week() const {
    int days_until_saturday = 6 - day_of_week();
    return end_of_day().add_days(days_until_saturday);
}

Timestamp Timestamp::start_of_month() const {
    return Timestamp(year(), month(), 1, 0, 0, 0, 0);
}

Timestamp Timestamp::end_of_month() const {
    // Get the last day of the month
    int days_in_month = 31;
    int m = month();
    int y = year();
    
    if (m == 2) {
        days_in_month = is_leap_year() ? 29 : 28;
    } else if (m == 4 || m == 6 || m == 9 || m == 11) {
        days_in_month = 30;
    }
    
    return Timestamp(y, m, days_in_month, 23, 59, 59, 999);
}

Timestamp Timestamp::start_of_year() const {
    return Timestamp(year(), 1, 1, 0, 0, 0, 0);
}

Timestamp Timestamp::end_of_year() const {
    return Timestamp(year(), 12, 31, 23, 59, 59, 999);
}

std::string Timestamp::to_string() const {
    return to_iso_string();
}

} // namespace timestamp
} // namespace time
} // namespace wip