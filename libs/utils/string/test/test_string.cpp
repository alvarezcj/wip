#include "wip_string.h"

#include <gtest/gtest.h>

class StringTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code here if needed
    }
    
    void TearDown() override {
        // Cleanup code here if needed
    }
};

// Test case conversion functions
TEST_F(StringTest, ToUpper_WhenValidString_ReturnsUppercase) {
    EXPECT_EQ(wip::utils::string::to_upper("hello"), "HELLO");
    EXPECT_EQ(wip::utils::string::to_upper("Hello World"), "HELLO WORLD");
    EXPECT_EQ(wip::utils::string::to_upper("123abc"), "123ABC");
}

TEST_F(StringTest, ToUpper_WhenEmptyString_ReturnsEmpty) {
    EXPECT_EQ(wip::utils::string::to_upper(""), "");
}

TEST_F(StringTest, ToLower_WhenValidString_ReturnsLowercase) {
    EXPECT_EQ(wip::utils::string::to_lower("HELLO"), "hello");
    EXPECT_EQ(wip::utils::string::to_lower("Hello World"), "hello world");
    EXPECT_EQ(wip::utils::string::to_lower("123ABC"), "123abc");
}

TEST_F(StringTest, ToLower_WhenEmptyString_ReturnsEmpty) {
    EXPECT_EQ(wip::utils::string::to_lower(""), "");
}

TEST_F(StringTest, Capitalize_WhenValidString_ReturnsCapitalized) {
    EXPECT_EQ(wip::utils::string::capitalize("hello"), "Hello");
    EXPECT_EQ(wip::utils::string::capitalize("HELLO"), "Hello");
    EXPECT_EQ(wip::utils::string::capitalize("hELLO wORLD"), "Hello world");
}

TEST_F(StringTest, Capitalize_WhenEmptyString_ReturnsEmpty) {
    EXPECT_EQ(wip::utils::string::capitalize(""), "");
}

TEST_F(StringTest, TitleCase_WhenValidString_ReturnsTitleCase) {
    EXPECT_EQ(wip::utils::string::title_case("hello world"), "Hello World");
    EXPECT_EQ(wip::utils::string::title_case("the quick brown fox"), "The Quick Brown Fox");
    EXPECT_EQ(wip::utils::string::title_case("hello-world_test"), "Hello-World_Test");
}

// Test trimming functions
TEST_F(StringTest, Trim_WhenWhitespaceAroundString_RemovesWhitespace) {
    EXPECT_EQ(wip::utils::string::trim("  hello  "), "hello");
    EXPECT_EQ(wip::utils::string::trim("\t\nhello\r\n"), "hello");
    EXPECT_EQ(wip::utils::string::trim("   "), "");
}

TEST_F(StringTest, Trim_WhenCustomChars_RemovesCustomChars) {
    EXPECT_EQ(wip::utils::string::trim("xxhelloxx", "x"), "hello");
    EXPECT_EQ(wip::utils::string::trim("abchelloabc", "abc"), "hello");
}

TEST_F(StringTest, TrimLeft_WhenWhitespaceAtStart_RemovesLeftWhitespace) {
    EXPECT_EQ(wip::utils::string::trim_left("  hello  "), "hello  ");
    EXPECT_EQ(wip::utils::string::trim_left("xxhelloxx", "x"), "helloxx");
}

TEST_F(StringTest, TrimRight_WhenWhitespaceAtEnd_RemovesRightWhitespace) {
    EXPECT_EQ(wip::utils::string::trim_right("  hello  "), "  hello");
    EXPECT_EQ(wip::utils::string::trim_right("xxhelloxx", "x"), "xxhello");
}

// Test split functions
TEST_F(StringTest, Split_WhenCharDelimiter_ReturnsSplitStrings) {
    auto result = wip::utils::string::split("a,b,c", ',');
    std::vector<std::string> expected = {"a", "b", "c"};
    EXPECT_EQ(result, expected);
}

TEST_F(StringTest, Split_WhenStringDelimiter_ReturnsSplitStrings) {
    auto result = wip::utils::string::split("hello::world::test", "::");
    std::vector<std::string> expected = {"hello", "world", "test"};
    EXPECT_EQ(result, expected);
}

TEST_F(StringTest, Split_WhenMaxSplits_LimitsNumberOfSplits) {
    auto result = wip::utils::string::split("a,b,c,d", ',', 2);
    std::vector<std::string> expected = {"a", "b", "c,d"};
    EXPECT_EQ(result, expected);
}

TEST_F(StringTest, Split_WhenEmptyString_ReturnsEmptyVector) {
    auto result = wip::utils::string::split("", ',');
    std::vector<std::string> expected = {""};
    EXPECT_EQ(result, expected);
}

// Test join function
TEST_F(StringTest, Join_WhenValidParts_ReturnsJoinedString) {
    std::vector<std::string> parts = {"hello", "world", "test"};
    EXPECT_EQ(wip::utils::string::join(parts, ", "), "hello, world, test");
    EXPECT_EQ(wip::utils::string::join(parts, "::"), "hello::world::test");
}

TEST_F(StringTest, Join_WhenEmptyVector_ReturnsEmptyString) {
    std::vector<std::string> empty_parts;
    EXPECT_EQ(wip::utils::string::join(empty_parts, ","), "");
}

TEST_F(StringTest, Join_WhenSingleElement_ReturnsElement) {
    std::vector<std::string> single_part = {"hello"};
    EXPECT_EQ(wip::utils::string::join(single_part, ","), "hello");
}

// Test prefix/suffix checking
TEST_F(StringTest, StartsWith_WhenStringStartsWithPrefix_ReturnsTrue) {
    EXPECT_TRUE(wip::utils::string::starts_with("hello world", "hello"));
    EXPECT_TRUE(wip::utils::string::starts_with("test", "test"));
    EXPECT_FALSE(wip::utils::string::starts_with("hello", "world"));
    EXPECT_FALSE(wip::utils::string::starts_with("hi", "hello"));
}

TEST_F(StringTest, EndsWith_WhenStringEndsWithSuffix_ReturnsTrue) {
    EXPECT_TRUE(wip::utils::string::ends_with("hello world", "world"));
    EXPECT_TRUE(wip::utils::string::ends_with("test", "test"));
    EXPECT_FALSE(wip::utils::string::ends_with("hello", "world"));
    EXPECT_FALSE(wip::utils::string::ends_with("hi", "hello"));
}

TEST_F(StringTest, Contains_WhenStringContainsSubstring_ReturnsTrue) {
    EXPECT_TRUE(wip::utils::string::contains("hello world", "lo wo"));
    EXPECT_TRUE(wip::utils::string::contains("test", "test"));
    EXPECT_FALSE(wip::utils::string::contains("hello", "world"));
}

// Test replacement functions
TEST_F(StringTest, ReplaceAll_WhenSubstringExists_ReplacesAllOccurrences) {
    EXPECT_EQ(wip::utils::string::replace_all("hello world hello", "hello", "hi"), "hi world hi");
    EXPECT_EQ(wip::utils::string::replace_all("aaa", "aa", "b"), "ba");
}

TEST_F(StringTest, ReplaceAll_WhenSubstringNotFound_ReturnsOriginal) {
    EXPECT_EQ(wip::utils::string::replace_all("hello world", "xyz", "abc"), "hello world");
}

TEST_F(StringTest, ReplaceFirst_WhenSubstringExists_ReplacesFirstOccurrence) {
    EXPECT_EQ(wip::utils::string::replace_first("hello world hello", "hello", "hi"), "hi world hello");
}

// Test padding functions
TEST_F(StringTest, PadLeft_WhenLengthGreaterThanString_AddsLeftPadding) {
    EXPECT_EQ(wip::utils::string::pad_left("hello", 10), "     hello");
    EXPECT_EQ(wip::utils::string::pad_left("hi", 5, 'x'), "xxxhi");
}

TEST_F(StringTest, PadLeft_WhenLengthLessOrEqual_ReturnsOriginal) {
    EXPECT_EQ(wip::utils::string::pad_left("hello", 3), "hello");
    EXPECT_EQ(wip::utils::string::pad_left("hello", 5), "hello");
}

TEST_F(StringTest, PadRight_WhenLengthGreaterThanString_AddsRightPadding) {
    EXPECT_EQ(wip::utils::string::pad_right("hello", 10), "hello     ");
    EXPECT_EQ(wip::utils::string::pad_right("hi", 5, 'x'), "hixxx");
}

TEST_F(StringTest, Center_WhenLengthGreaterThanString_CentersString) {
    EXPECT_EQ(wip::utils::string::center("hi", 6), "  hi  ");
    EXPECT_EQ(wip::utils::string::center("hello", 9, 'x'), "xxhelloxx");
}

// Test reverse and palindrome
TEST_F(StringTest, Reverse_WhenValidString_ReturnsReversedString) {
    EXPECT_EQ(wip::utils::string::reverse("hello"), "olleh");
    EXPECT_EQ(wip::utils::string::reverse(""), "");
    EXPECT_EQ(wip::utils::string::reverse("a"), "a");
}

TEST_F(StringTest, IsPalindrome_WhenPalindrome_ReturnsTrue) {
    EXPECT_TRUE(wip::utils::string::is_palindrome("racecar"));
    EXPECT_TRUE(wip::utils::string::is_palindrome("A man a plan a canal Panama", true, true));
    EXPECT_TRUE(wip::utils::string::is_palindrome("Madam", true, false));
    EXPECT_FALSE(wip::utils::string::is_palindrome("hello"));
}

// Test counting and validation
TEST_F(StringTest, CountOccurrences_WhenSubstringExists_ReturnsCorrectCount) {
    EXPECT_EQ(wip::utils::string::count_occurrences("hello hello world", "hello"), 2);
    EXPECT_EQ(wip::utils::string::count_occurrences("aaa", "aa", false), 1);
    EXPECT_EQ(wip::utils::string::count_occurrences("aaa", "aa", true), 2);
    EXPECT_EQ(wip::utils::string::count_occurrences("hello", "xyz"), 0);
}

TEST_F(StringTest, IsAlpha_WhenOnlyLetters_ReturnsTrue) {
    EXPECT_TRUE(wip::utils::string::is_alpha("hello"));
    EXPECT_TRUE(wip::utils::string::is_alpha("ABC"));
    EXPECT_FALSE(wip::utils::string::is_alpha("hello123"));
    EXPECT_FALSE(wip::utils::string::is_alpha("hello world"));
    EXPECT_FALSE(wip::utils::string::is_alpha(""));
}

TEST_F(StringTest, IsNumeric_WhenOnlyDigits_ReturnsTrue) {
    EXPECT_TRUE(wip::utils::string::is_numeric("123"));
    EXPECT_TRUE(wip::utils::string::is_numeric("0"));
    EXPECT_FALSE(wip::utils::string::is_numeric("123abc"));
    EXPECT_FALSE(wip::utils::string::is_numeric("12.3"));
    EXPECT_FALSE(wip::utils::string::is_numeric(""));
}

TEST_F(StringTest, IsAlphanumeric_WhenOnlyLettersAndDigits_ReturnsTrue) {
    EXPECT_TRUE(wip::utils::string::is_alphanumeric("hello123"));
    EXPECT_TRUE(wip::utils::string::is_alphanumeric("ABC123"));
    EXPECT_FALSE(wip::utils::string::is_alphanumeric("hello world"));
    EXPECT_FALSE(wip::utils::string::is_alphanumeric("hello-123"));
    EXPECT_FALSE(wip::utils::string::is_alphanumeric(""));
}

// Test conversion functions
TEST_F(StringTest, ToInt_WhenValidInteger_ReturnsInteger) {
    EXPECT_EQ(wip::utils::string::to_int("123"), 123);
    EXPECT_EQ(wip::utils::string::to_int("-456"), -456);
    EXPECT_EQ(wip::utils::string::to_int("0"), 0);
    EXPECT_FALSE(wip::utils::string::to_int("123abc").has_value());
    EXPECT_FALSE(wip::utils::string::to_int("").has_value());
    EXPECT_FALSE(wip::utils::string::to_int("12.3").has_value());
}

TEST_F(StringTest, ToDouble_WhenValidDouble_ReturnsDouble) {
    EXPECT_EQ(wip::utils::string::to_double("123.45"), 123.45);
    EXPECT_EQ(wip::utils::string::to_double("-456.78"), -456.78);
    EXPECT_EQ(wip::utils::string::to_double("0"), 0.0);
    EXPECT_EQ(wip::utils::string::to_double("123"), 123.0);
    EXPECT_FALSE(wip::utils::string::to_double("123abc").has_value());
    EXPECT_FALSE(wip::utils::string::to_double("").has_value());
}

// Test template substitution
TEST_F(StringTest, Substitute_WhenValidTemplate_ReplacesVariables) {
    std::unordered_map<std::string, std::string> vars = {
        {"name", "John"},
        {"age", "30"}
    };
    
    EXPECT_EQ(wip::utils::string::substitute("Hello ${name}, you are ${age} years old", vars),
              "Hello John, you are 30 years old");
    EXPECT_EQ(wip::utils::string::substitute("No variables here", vars),
              "No variables here");
    EXPECT_EQ(wip::utils::string::substitute("${unknown} variable", vars),
              "${unknown} variable");
}

// Test random string generation
TEST_F(StringTest, RandomString_WhenValidLength_ReturnsStringOfCorrectLength) {
    auto result = wip::utils::string::random_string(10);
    EXPECT_EQ(result.length(), 10);
    
    auto custom_result = wip::utils::string::random_string(5, "ABC");
    EXPECT_EQ(custom_result.length(), 5);
    for (char c : custom_result) {
        EXPECT_TRUE(c == 'A' || c == 'B' || c == 'C');
    }
}

TEST_F(StringTest, RandomString_WhenZeroLength_ReturnsEmptyString) {
    EXPECT_EQ(wip::utils::string::random_string(0), "");
}

// Test Levenshtein distance and similarity
TEST_F(StringTest, LevenshteinDistance_WhenValidStrings_ReturnsCorrectDistance) {
    EXPECT_EQ(wip::utils::string::levenshtein_distance("hello", "hello"), 0);
    EXPECT_EQ(wip::utils::string::levenshtein_distance("hello", "hallo"), 1);
    EXPECT_EQ(wip::utils::string::levenshtein_distance("hello", ""), 5);
    EXPECT_EQ(wip::utils::string::levenshtein_distance("", "world"), 5);
    EXPECT_EQ(wip::utils::string::levenshtein_distance("kitten", "sitting"), 3);
}

TEST_F(StringTest, Similarity_WhenValidStrings_ReturnsCorrectSimilarity) {
    EXPECT_DOUBLE_EQ(wip::utils::string::similarity("hello", "hello"), 1.0);
    EXPECT_DOUBLE_EQ(wip::utils::string::similarity("hello", "hallo"), 0.8);
    EXPECT_DOUBLE_EQ(wip::utils::string::similarity("", ""), 1.0);
    
    auto sim = wip::utils::string::similarity("hello", "world");
    EXPECT_GE(sim, 0.0);
    EXPECT_LE(sim, 1.0);
}
