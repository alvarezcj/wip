#include "wip_string.h"

#include <algorithm>
#include <cctype>
#include <charconv>
#include <random>
#include <sstream>
#include <regex>

namespace wip::utils::string {

std::string to_upper(std::string_view input) {
    std::string result{input};
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return result;
}

std::string to_lower(std::string_view input) {
    std::string result{input};
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::string capitalize(std::string_view input) {
    if (input.empty()) {
        return "";
    }
    
    std::string result{input};
    result[0] = std::toupper(static_cast<unsigned char>(result[0]));
    std::transform(result.begin() + 1, result.end(), result.begin() + 1,
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::string title_case(std::string_view input) {
    std::string result{input};
    bool capitalize_next = true;
    
    for (char& c : result) {
        if (std::isalpha(static_cast<unsigned char>(c))) {
            c = capitalize_next ? std::toupper(static_cast<unsigned char>(c)) 
                                : std::tolower(static_cast<unsigned char>(c));
            capitalize_next = false;
        } else {
            capitalize_next = true;
        }
    }
    
    return result;
}

std::string trim(std::string_view input, std::string_view chars) {
    const auto start = input.find_first_not_of(chars);
    if (start == std::string_view::npos) {
        return "";
    }
    
    const auto end = input.find_last_not_of(chars);
    return std::string{input.substr(start, end - start + 1)};
}

std::string trim_left(std::string_view input, std::string_view chars) {
    const auto start = input.find_first_not_of(chars);
    if (start == std::string_view::npos) {
        return "";
    }
    
    return std::string{input.substr(start)};
}

std::string trim_right(std::string_view input, std::string_view chars) {
    const auto end = input.find_last_not_of(chars);
    if (end == std::string_view::npos) {
        return "";
    }
    
    return std::string{input.substr(0, end + 1)};
}

std::vector<std::string> split(std::string_view data, char delimiter, size_t max_splits) {
    std::vector<std::string> result;
    
    if (data.empty()) {
        result.emplace_back("");
        return result;
    }
    
    std::string item;
    std::stringstream ss{std::string{data}};
    
    size_t splits = 0;
    while (std::getline(ss, item, delimiter)) {
        if (max_splits > 0 && splits >= max_splits) {
            // Add the rest as the last element
            std::string remainder;
            if (std::getline(ss, remainder, '\0')) {
                item += delimiter + remainder;
            }
            result.push_back(item);
            break;
        }
        result.push_back(item);
        ++splits;
    }
    
    return result;
}

std::vector<std::string> split(std::string_view data, std::string_view delimiter, size_t max_splits) {
    std::vector<std::string> result;
    if (delimiter.empty()) {
        result.emplace_back(data);
        return result;
    }
    
    size_t start = 0;
    size_t splits = 0;
    
    while (start < data.length()) {
        size_t end = data.find(delimiter, start);
        
        if (end == std::string_view::npos || (max_splits > 0 && splits >= max_splits)) {
            result.emplace_back(data.substr(start));
            break;
        }
        
        result.emplace_back(data.substr(start, end - start));
        start = end + delimiter.length();
        ++splits;
    }
    
    return result;
}

std::string join(const std::vector<std::string>& parts, std::string_view delimiter) {
    if (parts.empty()) {
        return "";
    }
    
    if (parts.size() == 1) {
        return parts[0];
    }
    
    std::ostringstream oss;
    oss << parts[0];
    
    for (size_t i = 1; i < parts.size(); ++i) {
        oss << delimiter << parts[i];
    }
    
    return oss.str();
}

bool starts_with(std::string_view text, std::string_view prefix) {
    return text.length() >= prefix.length() && 
           text.substr(0, prefix.length()) == prefix;
}

bool ends_with(std::string_view text, std::string_view suffix) {
    return text.length() >= suffix.length() && 
           text.substr(text.length() - suffix.length()) == suffix;
}

bool contains(std::string_view text, std::string_view substring) {
    return text.find(substring) != std::string_view::npos;
}

std::string replace_all(std::string_view text, std::string_view from, std::string_view to) {
    if (from.empty()) {
        return std::string{text};
    }
    
    std::string result{text};
    size_t pos = 0;
    
    while ((pos = result.find(from, pos)) != std::string::npos) {
        result.replace(pos, from.length(), to);
        pos += to.length();
    }
    
    return result;
}

std::string replace_first(std::string_view text, std::string_view from, std::string_view to) {
    std::string result{text};
    size_t pos = result.find(from);
    
    if (pos != std::string::npos) {
        result.replace(pos, from.length(), to);
    }
    
    return result;
}

std::string pad_left(std::string_view text, size_t length, char pad_char) {
    if (text.length() >= length) {
        return std::string{text};
    }
    
    std::string result(length - text.length(), pad_char);
    result += text;
    return result;
}

std::string pad_right(std::string_view text, size_t length, char pad_char) {
    if (text.length() >= length) {
        return std::string{text};
    }
    
    std::string result{text};
    result.append(length - text.length(), pad_char);
    return result;
}

std::string center(std::string_view text, size_t length, char pad_char) {
    if (text.length() >= length) {
        return std::string{text};
    }
    
    size_t total_padding = length - text.length();
    size_t left_padding = total_padding / 2;
    size_t right_padding = total_padding - left_padding;
    
    std::string result(left_padding, pad_char);
    result += text;
    result.append(right_padding, pad_char);
    
    return result;
}

std::string reverse(std::string_view text) {
    std::string result{text};
    std::reverse(result.begin(), result.end());
    return result;
}

bool is_palindrome(std::string_view text, bool ignore_case, bool ignore_spaces) {
    std::string processed{text};
    
    if (ignore_spaces) {
        processed.erase(std::remove_if(processed.begin(), processed.end(),
                                     [](unsigned char c) { return std::isspace(c); }),
                       processed.end());
    }
    
    if (ignore_case) {
        std::transform(processed.begin(), processed.end(), processed.begin(),
                      [](unsigned char c) { return std::tolower(c); });
    }
    
    return processed == reverse(processed);
}

size_t count_occurrences(std::string_view text, std::string_view substring, bool overlap) {
    if (substring.empty()) {
        return 0;
    }
    
    size_t count = 0;
    size_t pos = 0;
    
    while ((pos = text.find(substring, pos)) != std::string_view::npos) {
        ++count;
        pos += overlap ? 1 : substring.length();
    }
    
    return count;
}

bool is_alpha(std::string_view text) {
    if (text.empty()) {
        return false;
    }
    
    return std::all_of(text.begin(), text.end(),
                      [](unsigned char c) { return std::isalpha(c); });
}

bool is_numeric(std::string_view text) {
    if (text.empty()) {
        return false;
    }
    
    return std::all_of(text.begin(), text.end(),
                      [](unsigned char c) { return std::isdigit(c); });
}

bool is_alphanumeric(std::string_view text) {
    if (text.empty()) {
        return false;
    }
    
    return std::all_of(text.begin(), text.end(),
                      [](unsigned char c) { return std::isalnum(c); });
}

std::optional<int> to_int(std::string_view text) {
    int result;
    auto [ptr, ec] = std::from_chars(text.data(), text.data() + text.size(), result);
    
    if (ec == std::errc{} && ptr == text.data() + text.size()) {
        return result;
    }
    
    return std::nullopt;
}

std::optional<double> to_double(std::string_view text) {
    try {
        size_t pos;
        double result = std::stod(std::string{text}, &pos);
        
        if (pos == text.length()) {
            return result;
        }
    }
    catch (const std::exception&) {
        // Conversion failed
    }
    
    return std::nullopt;
}

std::string substitute(std::string_view template_str, const std::unordered_map<std::string, std::string>& variables) {
    std::string result{template_str};
    
    for (const auto& [key, value] : variables) {
        std::string placeholder = "${" + key + "}";
        result = replace_all(result, placeholder, value);
    }
    
    return result;
}

std::string random_string(size_t length, std::string_view charset) {
    if (charset.empty() || length == 0) {
        return "";
    }
    
    static thread_local std::random_device rd;
    static thread_local std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, static_cast<int>(charset.size() - 1));
    
    std::string result;
    result.reserve(length);
    
    for (size_t i = 0; i < length; ++i) {
        result += charset[dis(gen)];
    }
    
    return result;
}

size_t levenshtein_distance(std::string_view str1, std::string_view str2) {
    const size_t len1 = str1.length();
    const size_t len2 = str2.length();
    
    if (len1 == 0) return len2;
    if (len2 == 0) return len1;
    
    std::vector<std::vector<size_t>> matrix(len1 + 1, std::vector<size_t>(len2 + 1));
    
    // Initialize first row and column
    for (size_t i = 0; i <= len1; ++i) {
        matrix[i][0] = i;
    }
    for (size_t j = 0; j <= len2; ++j) {
        matrix[0][j] = j;
    }
    
    // Fill the matrix
    for (size_t i = 1; i <= len1; ++i) {
        for (size_t j = 1; j <= len2; ++j) {
            size_t cost = (str1[i - 1] == str2[j - 1]) ? 0 : 1;
            
            matrix[i][j] = std::min({
                matrix[i - 1][j] + 1,      // deletion
                matrix[i][j - 1] + 1,      // insertion
                matrix[i - 1][j - 1] + cost // substitution
            });
        }
    }
    
    return matrix[len1][len2];
}

double similarity(std::string_view str1, std::string_view str2) {
    if (str1.empty() && str2.empty()) {
        return 1.0;
    }
    
    size_t max_len = std::max(str1.length(), str2.length());
    if (max_len == 0) {
        return 1.0;
    }
    
    size_t distance = levenshtein_distance(str1, str2);
    return 1.0 - static_cast<double>(distance) / static_cast<double>(max_len);
}

}  // namespace wip::utils::string
