#include "time_utilities.h"
#include <gtest/gtest.h>
#include <thread>
#include <chrono>

using namespace wip::time::utilities;

class TimeUtilitiesTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(TimeUtilitiesTest, NowFunctions) {
    auto system_now = now_system();
    auto steady_now = now_steady();
    auto high_res_now = now_high_res();
    
    // These should all represent approximately the same moment
    // We can't test exact equality due to time passing, but they should be recent
    EXPECT_TRUE(system_now.time_since_epoch().count() > 0);
    EXPECT_TRUE(steady_now.time_since_epoch().count() > 0);
    EXPECT_TRUE(high_res_now.time_since_epoch().count() > 0);
}

TEST_F(TimeUtilitiesTest, UnixTimestamp) {
    auto unix_sec = unix_timestamp();
    auto unix_ms = unix_timestamp_ms();
    auto unix_us = unix_timestamp_us();
    auto unix_ns = unix_timestamp_ns();
    
    // Unix timestamps should be positive and reasonable
    EXPECT_GT(unix_sec, 1000000000);  // Should be after year 2001
    EXPECT_GT(unix_ms, unix_sec * 1000);
    EXPECT_GT(unix_us, unix_ms * 1000);
    EXPECT_GT(unix_ns, unix_us * 1000);
}

TEST_F(TimeUtilitiesTest, FormatTime) {
    std::time_t test_time = 1609459200; // 2021-01-01 00:00:00 UTC
    auto test_point = std::chrono::system_clock::from_time_t(test_time);
    
    auto iso_str = format_time(test_point, DateFormat::ISO8601);
    auto rfc_str = format_time(test_point, DateFormat::RFC2822);
    auto readable_str = format_time(test_point, DateFormat::Readable);
    
    EXPECT_FALSE(iso_str.empty());
    EXPECT_FALSE(rfc_str.empty());
    EXPECT_FALSE(readable_str.empty());
    
    // ISO should contain expected patterns
    EXPECT_TRUE(iso_str.find("2021") != std::string::npos);
    EXPECT_TRUE(iso_str.find("T") != std::string::npos || iso_str.find(" ") != std::string::npos);
}

TEST_F(TimeUtilitiesTest, ParseTime) {
    std::string iso_string = "2021-01-01T12:30:45";
    auto parsed_time = parse_time(iso_string, DateFormat::ISO8601);
    
    // Check if parsing was successful by comparing to epoch
    auto epoch = std::chrono::system_clock::time_point{};
    EXPECT_GT(parsed_time, epoch);
    
    // Test round-trip
    auto formatted = format_time(parsed_time, DateFormat::ISO8601);
    EXPECT_TRUE(formatted.find("2021") != std::string::npos);
    // Note: time formatting might differ due to timezone handling
}

TEST_F(TimeUtilitiesTest, BusinessDays) {
    // Test with a known Monday (2021-01-04)
    std::time_t monday = 1609747200;
    auto monday_point = std::chrono::system_clock::from_time_t(monday);
    
    // Add business days
    auto friday = add_business_days(monday_point, 4);
    EXPECT_GT(friday, monday_point);
    
    // Calculate business days between
    auto days_between = business_days_between(monday_point, friday);
    EXPECT_EQ(days_between, 4);
}

TEST_F(TimeUtilitiesTest, Stopwatch) {
    Stopwatch stopwatch;
    
    EXPECT_FALSE(stopwatch.is_running());
    EXPECT_EQ(stopwatch.elapsed(TimeUnit::Seconds), 0.0);
    
    stopwatch.start();
    EXPECT_TRUE(stopwatch.is_running());
    
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    auto elapsed = stopwatch.elapsed(TimeUnit::Seconds);
    EXPECT_GT(elapsed, 0.0);
    EXPECT_LT(elapsed, 1.0); // Should be much less than a second
    
    stopwatch.stop();
    EXPECT_FALSE(stopwatch.is_running());
    
    auto stopped_elapsed = stopwatch.elapsed(TimeUnit::Seconds);
    EXPECT_GE(stopped_elapsed, elapsed);
}

TEST_F(TimeUtilitiesTest, ScopedTimer) {
    // Simple test that ScopedTimer can be constructed and destructed
    {
        ScopedTimer timer("Test Timer");
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        auto elapsed = timer.elapsed();
        EXPECT_GT(elapsed.count(), 0);
    }
}

TEST_F(TimeUtilitiesTest, TimeConversions) {
    auto seconds = to_time_unit(std::chrono::minutes(2), TimeUnit::Seconds);
    EXPECT_EQ(seconds, 120);
    
    auto milliseconds = to_time_unit(std::chrono::seconds(1), TimeUnit::Milliseconds);
    EXPECT_EQ(milliseconds, 1000);
    
    auto microseconds = to_time_unit(std::chrono::milliseconds(1), TimeUnit::Microseconds);
    EXPECT_EQ(microseconds, 1000);
    
    auto nanoseconds = to_time_unit(std::chrono::microseconds(1), TimeUnit::Nanoseconds);
    EXPECT_EQ(nanoseconds, 1000);
}

TEST_F(TimeUtilitiesTest, SleepFunctions) {
    auto start = now_steady();
    
    sleep_for(std::chrono::milliseconds(10));
    
    auto end = now_steady();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_GE(elapsed.count(), 8);  // Allow some margin for timing precision
    EXPECT_LE(elapsed.count(), 50); // But not too much
}