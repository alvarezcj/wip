#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <optional>
#include <unordered_map>

namespace wip::utils::string {

/**
 * @brief Converts a string to uppercase
 * @param input The input string
 * @return The uppercase version of the input string
 */
std::string to_upper(std::string_view input);

/**
 * @brief Converts a string to lowercase
 * @param input The input string
 * @return The lowercase version of the input string
 */
std::string to_lower(std::string_view input);

/**
 * @brief Converts the first character to uppercase and the rest to lowercase
 * @param input The input string
 * @return The capitalized version of the input string
 */
std::string capitalize(std::string_view input);

/**
 * @brief Capitalizes the first letter of each word
 * @param input The input string
 * @return The title case version of the input string
 */
std::string title_case(std::string_view input);

/**
 * @brief Removes whitespace from the beginning and end of a string
 * @param input The input string
 * @param chars Characters to trim (default: whitespace)
 * @return The trimmed string
 */
std::string trim(std::string_view input, std::string_view chars = " \t\n\r\f\v");

/**
 * @brief Removes whitespace from the beginning of a string
 * @param input The input string
 * @param chars Characters to trim (default: whitespace)
 * @return The left-trimmed string
 */
std::string trim_left(std::string_view input, std::string_view chars = " \t\n\r\f\v");

/**
 * @brief Removes whitespace from the end of a string
 * @param input The input string
 * @param chars Characters to trim (default: whitespace)
 * @return The right-trimmed string
 */
std::string trim_right(std::string_view input, std::string_view chars = " \t\n\r\f\v");

/**
 * @brief Splits a string by delimiter
 * @param data The string to split
 * @param delimiter The character to split by
 * @param max_splits Maximum number of splits (0 = unlimited)
 * @return Vector of string parts
 */
std::vector<std::string> split(std::string_view data, char delimiter, size_t max_splits = 0);

/**
 * @brief Splits a string by delimiter string
 * @param data The string to split
 * @param delimiter The string to split by
 * @param max_splits Maximum number of splits (0 = unlimited)
 * @return Vector of string parts
 */
std::vector<std::string> split(std::string_view data, std::string_view delimiter, size_t max_splits = 0);

/**
 * @brief Joins a vector of strings with a delimiter
 * @param parts The strings to join
 * @param delimiter The delimiter to use
 * @return The joined string
 */
std::string join(const std::vector<std::string>& parts, std::string_view delimiter);

/**
 * @brief Checks if a string starts with a prefix
 * @param text The text to check
 * @param prefix The prefix to look for
 * @return true if text starts with prefix
 */
bool starts_with(std::string_view text, std::string_view prefix);

/**
 * @brief Checks if a string ends with a suffix
 * @param text The text to check
 * @param suffix The suffix to look for
 * @return true if text ends with suffix
 */
bool ends_with(std::string_view text, std::string_view suffix);

/**
 * @brief Checks if a string contains a substring
 * @param text The text to search in
 * @param substring The substring to search for
 * @return true if text contains substring
 */
bool contains(std::string_view text, std::string_view substring);

/**
 * @brief Replaces all occurrences of a substring with another string
 * @param text The text to modify
 * @param from The substring to replace
 * @param to The replacement string
 * @return The modified string
 */
std::string replace_all(std::string_view text, std::string_view from, std::string_view to);

/**
 * @brief Replaces the first occurrence of a substring with another string
 * @param text The text to modify
 * @param from The substring to replace
 * @param to The replacement string
 * @return The modified string
 */
std::string replace_first(std::string_view text, std::string_view from, std::string_view to);

/**
 * @brief Pads a string to a specified length with a character on the left
 * @param text The text to pad
 * @param length The desired total length
 * @param pad_char The character to pad with
 * @return The padded string
 */
std::string pad_left(std::string_view text, size_t length, char pad_char = ' ');

/**
 * @brief Pads a string to a specified length with a character on the right
 * @param text The text to pad
 * @param length The desired total length
 * @param pad_char The character to pad with
 * @return The padded string
 */
std::string pad_right(std::string_view text, size_t length, char pad_char = ' ');

/**
 * @brief Centers a string in a field of specified length
 * @param text The text to center
 * @param length The desired total length
 * @param pad_char The character to pad with
 * @return The centered string
 */
std::string center(std::string_view text, size_t length, char pad_char = ' ');

/**
 * @brief Reverses a string
 * @param text The text to reverse
 * @return The reversed string
 */
std::string reverse(std::string_view text);

/**
 * @brief Checks if a string is a palindrome (reads the same forwards and backwards)
 * @param text The text to check
 * @param ignore_case Whether to ignore case differences
 * @param ignore_spaces Whether to ignore whitespace
 * @return true if text is a palindrome
 */
bool is_palindrome(std::string_view text, bool ignore_case = true, bool ignore_spaces = true);

/**
 * @brief Counts the number of occurrences of a substring
 * @param text The text to search in
 * @param substring The substring to count
 * @param overlap Whether to count overlapping occurrences
 * @return The number of occurrences
 */
size_t count_occurrences(std::string_view text, std::string_view substring, bool overlap = false);

/**
 * @brief Checks if a string contains only alphabetic characters
 * @param text The text to check
 * @return true if text contains only letters
 */
bool is_alpha(std::string_view text);

/**
 * @brief Checks if a string contains only numeric characters
 * @param text The text to check
 * @return true if text contains only digits
 */
bool is_numeric(std::string_view text);

/**
 * @brief Checks if a string contains only alphanumeric characters
 * @param text The text to check
 * @return true if text contains only letters and digits
 */
bool is_alphanumeric(std::string_view text);

/**
 * @brief Converts a string to an integer safely
 * @param text The text to convert
 * @return The integer value, or nullopt if conversion fails
 */
std::optional<int> to_int(std::string_view text);

/**
 * @brief Converts a string to a double safely
 * @param text The text to convert
 * @return The double value, or nullopt if conversion fails
 */
std::optional<double> to_double(std::string_view text);

/**
 * @brief Performs simple template substitution
 * @param template_str The template string with ${key} placeholders
 * @param variables Map of variable names to values
 * @return The string with substitutions made
 */
std::string substitute(std::string_view template_str, const std::unordered_map<std::string, std::string>& variables);

/**
 * @brief Generates a random string of specified length
 * @param length The desired length
 * @param charset The character set to choose from (default: alphanumeric)
 * @return A random string
 */
std::string random_string(size_t length, std::string_view charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");

/**
 * @brief Calculates the Levenshtein distance between two strings
 * @param str1 First string
 * @param str2 Second string
 * @return The edit distance between the strings
 */
size_t levenshtein_distance(std::string_view str1, std::string_view str2);

/**
 * @brief Calculates similarity between two strings as a percentage
 * @param str1 First string
 * @param str2 Second string
 * @return Similarity as a value between 0.0 and 1.0
 */
double similarity(std::string_view str1, std::string_view str2);

}  // namespace wip::utils::string
