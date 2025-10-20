#include "timestamp.h"
#include <gtest/gtest.h>
#include <chrono>
#include <thread>

using namespace wip::time::timestamp;

class TimestampTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(TimestampTest, DefaultConstructor) {
    Timestamp ts;
    
    // Should be approximately now
    auto now = std::chrono::system_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::seconds>(
        now - ts.time_point()).count();
    
    EXPECT_LT(std::abs(diff), 2); // Within 2 seconds
}

TEST_F(TimestampTest, UnixConstructors) {
    std::time_t unix_time = 1609459200; // 2021-01-01 00:00:00 UTC
    Timestamp ts(unix_time);
    
    EXPECT_EQ(ts.unix_timestamp(), unix_time);
    EXPECT_EQ(ts.year(), 2021);
    EXPECT_EQ(ts.month(), 1);
    EXPECT_EQ(ts.day(), 1);
    
    // Test millisecond constructor
    std::int64_t unix_ms = 1609459200000;
    Timestamp ts_ms = Timestamp::from_unix_ms(unix_ms);
    EXPECT_EQ(ts_ms.unix_timestamp_ms(), unix_ms);
}

TEST_F(TimestampTest, DateTimeConstructor) {
    Timestamp ts(2023, 6, 15, 14, 30, 45, 123);
    
    EXPECT_EQ(ts.year(), 2023);
    EXPECT_EQ(ts.month(), 6);
    EXPECT_EQ(ts.day(), 15);
    EXPECT_EQ(ts.hour(), 14);
    EXPECT_EQ(ts.minute(), 30);
    EXPECT_EQ(ts.second(), 45);
    EXPECT_EQ(ts.millisecond(), 123);
}

TEST_F(TimestampTest, StaticFactories) {
    auto now_ts = Timestamp::now();
    auto epoch_ts = Timestamp::epoch();
    
    EXPECT_GT(now_ts.unix_timestamp(), 1000000000);
    EXPECT_EQ(epoch_ts.unix_timestamp(), 0);
    
    auto from_unix = Timestamp::from_unix(1609459200);
    EXPECT_EQ(from_unix.unix_timestamp(), 1609459200);
    
    auto from_unix_ms = Timestamp::from_unix_ms(1609459200000);
    EXPECT_EQ(from_unix_ms.unix_timestamp_ms(), 1609459200000);
}

TEST_F(TimestampTest, ISOStringParsing) {
    auto ts = Timestamp::from_iso_string("2021-01-01T12:30:45");
    
    EXPECT_EQ(ts.year(), 2021);
    EXPECT_EQ(ts.month(), 1);
    EXPECT_EQ(ts.day(), 1);
    EXPECT_EQ(ts.hour(), 12);
    EXPECT_EQ(ts.minute(), 30);
    EXPECT_EQ(ts.second(), 45);
}

TEST_F(TimestampTest, Formatting) {
    Timestamp ts(2023, 6, 15, 14, 30, 45, 123);
    
    auto iso_str = ts.to_iso_string();
    auto iso_with_ms = ts.to_iso_string(true);
    auto rfc_str = ts.to_rfc2822_string();
    auto unix_str = ts.to_unix_string();
    auto readable_str = ts.to_readable_string();
    
    EXPECT_FALSE(iso_str.empty());
    EXPECT_FALSE(iso_with_ms.empty());
    EXPECT_FALSE(rfc_str.empty());
    EXPECT_FALSE(unix_str.empty());
    EXPECT_FALSE(readable_str.empty());
    
    EXPECT_TRUE(iso_str.find("2023") != std::string::npos);
    EXPECT_TRUE(iso_str.find("T") != std::string::npos);
    EXPECT_TRUE(iso_with_ms.find(".123") != std::string::npos);
}

TEST_F(TimestampTest, Arithmetic) {
    Timestamp ts1(2023, 1, 1, 12, 0, 0);
    Timestamp ts2(2023, 1, 1, 13, 0, 0);
    
    auto diff = ts2 - ts1;
    auto hours = std::chrono::duration_cast<std::chrono::hours>(diff);
    EXPECT_EQ(hours.count(), 1);
    
    auto ts3 = ts1 + std::chrono::hours(2);
    EXPECT_EQ(ts3.hour(), 14);
    
    auto ts4 = ts2 - std::chrono::minutes(30);
    EXPECT_EQ(ts4.hour(), 12);
    EXPECT_EQ(ts4.minute(), 30);
}

TEST_F(TimestampTest, ConvenienceAddMethods) {
    Timestamp ts(2023, 1, 15, 12, 30, 45);
    
    auto plus_years = ts.add_years(1);
    EXPECT_EQ(plus_years.year(), 2024);
    
    auto plus_months = ts.add_months(2);
    EXPECT_EQ(plus_months.month(), 3);
    
    auto plus_days = ts.add_days(10);
    EXPECT_EQ(plus_days.day(), 25);
    
    auto plus_hours = ts.add_hours(3);
    EXPECT_EQ(plus_hours.hour(), 15);
    
    auto plus_minutes = ts.add_minutes(15);
    EXPECT_EQ(plus_minutes.minute(), 45);
    
    auto plus_seconds = ts.add_seconds(30);
    EXPECT_EQ(plus_seconds.second(), 15);
}

TEST_F(TimestampTest, Comparisons) {
    Timestamp ts1(2023, 1, 1, 12, 0, 0);
    Timestamp ts2(2023, 1, 1, 13, 0, 0);
    Timestamp ts3(2023, 1, 1, 12, 0, 0);
    
    EXPECT_TRUE(ts1 < ts2);
    EXPECT_TRUE(ts1 <= ts2);
    EXPECT_TRUE(ts2 > ts1);
    EXPECT_TRUE(ts2 >= ts1);
    EXPECT_TRUE(ts1 == ts3);
    EXPECT_TRUE(ts1 != ts2);
    
    EXPECT_TRUE(ts1.is_before(ts2));
    EXPECT_TRUE(ts2.is_after(ts1));
    EXPECT_TRUE(ts1.is_between(ts1, ts2));
    EXPECT_TRUE(ts3.is_between(ts1, ts2));
}

TEST_F(TimestampTest, UtilityMethods) {
    // Test leap year
    Timestamp leap_year(2024, 1, 1);
    Timestamp non_leap_year(2023, 1, 1);
    
    EXPECT_TRUE(leap_year.is_leap_year());
    EXPECT_FALSE(non_leap_year.is_leap_year());
    
    // Test weekend/weekday (2023-01-01 was a Sunday)
    Timestamp sunday(2023, 1, 1);
    Timestamp monday(2023, 1, 2);
    
    EXPECT_TRUE(sunday.is_weekend());
    EXPECT_FALSE(sunday.is_weekday());
    EXPECT_FALSE(monday.is_weekend());
    EXPECT_TRUE(monday.is_weekday());
}

TEST_F(TimestampTest, PeriodMethods) {
    Timestamp ts(2023, 6, 15, 14, 30, 45, 123);
    
    auto start_of_day = ts.start_of_day();
    EXPECT_EQ(start_of_day.hour(), 0);
    EXPECT_EQ(start_of_day.minute(), 0);
    EXPECT_EQ(start_of_day.second(), 0);
    EXPECT_EQ(start_of_day.millisecond(), 0);
    
    auto end_of_day = ts.end_of_day();
    EXPECT_EQ(end_of_day.hour(), 23);
    EXPECT_EQ(end_of_day.minute(), 59);
    EXPECT_EQ(end_of_day.second(), 59);
    EXPECT_EQ(end_of_day.millisecond(), 999);
    
    auto start_of_month = ts.start_of_month();
    EXPECT_EQ(start_of_month.day(), 1);
    EXPECT_EQ(start_of_month.hour(), 0);
    
    auto end_of_month = ts.end_of_month();
    EXPECT_EQ(end_of_month.day(), 30); // June has 30 days
    EXPECT_EQ(end_of_month.hour(), 23);
    
    auto start_of_year = ts.start_of_year();
    EXPECT_EQ(start_of_year.month(), 1);
    EXPECT_EQ(start_of_year.day(), 1);
    
    auto end_of_year = ts.end_of_year();
    EXPECT_EQ(end_of_year.month(), 12);
    EXPECT_EQ(end_of_year.day(), 31);
}

TEST_F(TimestampTest, WeekCalculations) {
    Timestamp ts(2023, 6, 15); // This should be a Thursday
    
    EXPECT_EQ(ts.day_of_week(), 4); // Thursday is day 4 (0=Sunday)
    
    auto start_of_week = ts.start_of_week();
    EXPECT_EQ(start_of_week.day_of_week(), 0); // Should be Sunday
    
    auto end_of_week = ts.end_of_week();
    EXPECT_EQ(end_of_week.day_of_week(), 6); // Should be Saturday
    
    auto week_number = ts.week_of_year();
    EXPECT_GT(week_number, 0);
    EXPECT_LE(week_number, 53);
}

TEST_F(TimestampTest, DayOfYear) {
    Timestamp new_year(2023, 1, 1);
    EXPECT_EQ(new_year.day_of_year(), 1);
    
    Timestamp mid_year(2023, 7, 1);
    EXPECT_GT(mid_year.day_of_year(), 180);
    EXPECT_LT(mid_year.day_of_year(), 190);
}