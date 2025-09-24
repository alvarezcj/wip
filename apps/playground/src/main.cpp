#include <iostream>
#include <iomanip>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "file.h"
#include "wip_string.h"
#include "json_serializer.h"
#include "uid.h"

void demonstrate_json_serializer() {
    std::cout << "\n=== JSON Serializer Demonstration ===\n" << std::endl;
    
    // String serialization
    std::cout << "1. String Serialization:" << std::endl;
    wip::serialization::JsonSerializer<std::string> string_serializer;
    std::string text = "Hello, JSON Serializer!";
    auto json_text = string_serializer.serialize(text);
    std::cout << "Serialized: " << json_text << std::endl;
    auto deserialized_text = string_serializer.deserialize(json_text);
    std::cout << "Deserialized: " << (deserialized_text ? deserialized_text.value() : "failed") << std::endl;
    
    // Integer serialization
    std::cout << "\n2. Integer Serialization:" << std::endl;
    wip::serialization::JsonSerializer<int> int_serializer;
    int number = 42;
    auto json_number = int_serializer.serialize(number);
    std::cout << "Serialized: " << json_number << std::endl;
    auto deserialized_number = int_serializer.deserialize(json_number);
    std::cout << "Deserialized: " << (deserialized_number ? std::to_string(deserialized_number.value()) : "failed") << std::endl;
    
    // Double serialization
    std::cout << "\n3. Double Serialization:" << std::endl;
    wip::serialization::JsonSerializer<double> double_serializer;
    double pi = 3.14159;
    auto json_pi = double_serializer.serialize(pi);
    std::cout << "Serialized: " << json_pi << std::endl;
    auto deserialized_pi = double_serializer.deserialize(json_pi);
    std::cout << "Deserialized: " << (deserialized_pi ? std::to_string(deserialized_pi.value()) : "failed") << std::endl;
    
    // Boolean serialization
    std::cout << "\n4. Boolean Serialization:" << std::endl;
    wip::serialization::JsonSerializer<bool> bool_serializer;
    bool flag = true;
    auto json_flag = bool_serializer.serialize(flag);
    std::cout << "Serialized: " << json_flag << std::endl;
    auto deserialized_flag = bool_serializer.deserialize(json_flag);
    std::cout << "Deserialized: " << (deserialized_flag ? (deserialized_flag.value() ? "true" : "false") : "failed") << std::endl;
}

void demonstrate_uid() {
    std::cout << "\n=== UID (UUID) Demonstration ===\n" << std::endl;
    
    // Basic UID generation
    std::cout << "1. Basic UID Generation:" << std::endl;
    wip::utils::uid::UID uid1;
    wip::utils::uid::UID uid2 = wip::utils::uid::UID::generate();
    
    std::cout << "UID 1: " << uid1.to_string() << std::endl;
    std::cout << "UID 2: " << uid2.to_string() << std::endl;
    std::cout << "Are they equal? " << (uid1 == uid2 ? "Yes" : "No") << std::endl;
    
    // String round-trip
    std::cout << "\n2. String Round-trip Conversion:" << std::endl;
    std::string uid_string = uid1.to_string();
    wip::utils::uid::UID uid_from_string(uid_string);
    std::cout << "Original: " << uid_string << std::endl;
    std::cout << "From String: " << uid_from_string.to_string() << std::endl;
    std::cout << "Are they equal? " << (uid1 == uid_from_string ? "Yes" : "No") << std::endl;
    
    // Null UID
    std::cout << "\n3. Null UID:" << std::endl;
    wip::utils::uid::UID null_uid = wip::utils::uid::UID::null();
    std::cout << "Null UID: " << null_uid.to_string() << std::endl;
    std::cout << "Is null? " << (null_uid.is_null() ? "Yes" : "No") << std::endl;
    std::cout << "Is generated UID null? " << (uid1.is_null() ? "Yes" : "No") << std::endl;
    
    // Rule of 7 demonstration
    std::cout << "\n4. Rule of 7 (Copy/Move operations):" << std::endl;
    wip::utils::uid::UID original = wip::utils::uid::UID::generate();
    std::cout << "Original: " << original.to_string() << std::endl;
    
    // Copy constructor
    wip::utils::uid::UID copied(original);
    std::cout << "Copied: " << copied.to_string() << std::endl;
    std::cout << "Copy works? " << (original == copied ? "Yes" : "No") << std::endl;
    
    // Copy assignment
    wip::utils::uid::UID assigned;
    assigned = original;
    std::cout << "Assigned: " << assigned.to_string() << std::endl;
    std::cout << "Assignment works? " << (original == assigned ? "Yes" : "No") << std::endl;
    
    // Move constructor
    std::string original_str = original.to_string();
    wip::utils::uid::UID moved(std::move(original));
    std::cout << "Moved: " << moved.to_string() << std::endl;
    std::cout << "Move works? " << (moved.to_string() == original_str ? "Yes" : "No") << std::endl;
    
    // Hash function demonstration
    std::cout << "\n5. Hash Function (for containers):" << std::endl;
    std::unordered_set<wip::utils::uid::UID> uid_set;
    std::unordered_map<wip::utils::uid::UID, std::string> uid_map;
    
    for (int i = 0; i < 5; ++i) {
        wip::utils::uid::UID uid = wip::utils::uid::UID::generate();
        uid_set.insert(uid);
        uid_map[uid] = "Value " + std::to_string(i + 1);
    }
    
    std::cout << "Created " << uid_set.size() << " unique UIDs in set" << std::endl;
    std::cout << "Created " << uid_map.size() << " entries in map:" << std::endl;
    
    for (const auto& [uid, value] : uid_map) {
        std::cout << "  " << uid.to_string() << " -> " << value << std::endl;
    }
    
    // Uniqueness test
    std::cout << "\n6. Uniqueness Test (1000 UIDs):" << std::endl;
    std::unordered_set<std::string> unique_uids;
    const int test_count = 1000;
    
    for (int i = 0; i < test_count; ++i) {
        wip::utils::uid::UID uid = wip::utils::uid::UID::generate();
        unique_uids.insert(uid.to_string());
    }
    
    std::cout << "Generated " << test_count << " UIDs" << std::endl;
    std::cout << "Unique UIDs: " << unique_uids.size() << std::endl;
    std::cout << "All unique? " << (unique_uids.size() == test_count ? "Yes" : "No") << std::endl;
}

void demonstrate_string_utilities() {
    std::cout << "\n=== String Utilities Demonstration ===\n" << std::endl;
    
    // Case conversion
    std::cout << "1. Case Conversion:" << std::endl;
    std::string sample = "Hello World! This is a Test.";
    std::cout << "Original: " << sample << std::endl;
    std::cout << "Upper: " << wip::utils::string::to_upper(sample) << std::endl;
    std::cout << "Lower: " << wip::utils::string::to_lower(sample) << std::endl;
    std::cout << "Capitalize: " << wip::utils::string::capitalize(sample) << std::endl;
    std::cout << "Title Case: " << wip::utils::string::title_case(sample) << std::endl;
    
    // Trimming
    std::cout << "\n2. Trimming Operations:" << std::endl;
    std::string messy = "   Hello World!   ";
    std::cout << "Original: '" << messy << "'" << std::endl;
    std::cout << "Trimmed: '" << wip::utils::string::trim(messy) << "'" << std::endl;
    std::cout << "Left Trim: '" << wip::utils::string::trim_left(messy) << "'" << std::endl;
    std::cout << "Right Trim: '" << wip::utils::string::trim_right(messy) << "'" << std::endl;
    
    // Splitting and joining
    std::cout << "\n3. Split and Join:" << std::endl;
    std::string csv_data = "apple,banana,cherry,date";
    auto fruits = wip::utils::string::split(csv_data, ',');
    std::cout << "CSV: " << csv_data << std::endl;
    std::cout << "Split fruits:" << std::endl;
    for (const auto& fruit : fruits) {
        std::cout << "  - " << fruit << std::endl;
    }
    std::cout << "Joined with ' | ': " << wip::utils::string::join(fruits, " | ") << std::endl;
    
    // String checking
    std::cout << "\n4. String Analysis:" << std::endl;
    std::string url = "https://example.com/api/users";
    std::cout << "URL: " << url << std::endl;
    std::cout << "Starts with 'https://': " << (wip::utils::string::starts_with(url, "https://") ? "Yes" : "No") << std::endl;
    std::cout << "Ends with '/users': " << (wip::utils::string::ends_with(url, "/users") ? "Yes" : "No") << std::endl;
    std::cout << "Contains 'api': " << (wip::utils::string::contains(url, "api") ? "Yes" : "No") << std::endl;
    
    // Replacement
    std::cout << "\n5. String Replacement:" << std::endl;
    std::string text = "The quick brown fox jumps over the lazy dog. The fox is quick.";
    std::cout << "Original: " << text << std::endl;
    std::cout << "Replace 'fox' with 'cat': " << wip::utils::string::replace_all(text, "fox", "cat") << std::endl;
    std::cout << "Replace first 'the' with 'a': " << wip::utils::string::replace_first(text, "the", "a") << std::endl;
    
    // Padding and centering
    std::cout << "\n6. Padding and Centering:" << std::endl;
    std::string title = "REPORT";
    std::cout << "Original: '" << title << "'" << std::endl;
    std::cout << "Pad Left (10): '" << wip::utils::string::pad_left(title, 10) << "'" << std::endl;
    std::cout << "Pad Right (10): '" << wip::utils::string::pad_right(title, 10) << "'" << std::endl;
    std::cout << "Center (15): '" << wip::utils::string::center(title, 15, '=') << "'" << std::endl;
    
    // Palindrome checking
    std::cout << "\n7. Palindrome Detection:" << std::endl;
    std::vector<std::string> test_strings = {"racecar", "A man a plan a canal Panama", "hello", "Madam"};
    for (const auto& test_str : test_strings) {
        bool is_palindrome = wip::utils::string::is_palindrome(test_str, true, true);
        std::cout << "'" << test_str << "' is " << (is_palindrome ? "" : "not ") << "a palindrome" << std::endl;
    }
    
    // String validation
    std::cout << "\n8. String Validation:" << std::endl;
    std::vector<std::string> validation_tests = {"Hello", "12345", "Hello123", "Hello World"};
    for (const auto& test_str : validation_tests) {
        std::cout << "'" << test_str << "': ";
        std::cout << "Alpha=" << (wip::utils::string::is_alpha(test_str) ? "Y" : "N") << ", ";
        std::cout << "Numeric=" << (wip::utils::string::is_numeric(test_str) ? "Y" : "N") << ", ";
        std::cout << "Alphanumeric=" << (wip::utils::string::is_alphanumeric(test_str) ? "Y" : "N") << std::endl;
    }
    
    // Safe conversion
    std::cout << "\n9. Safe Type Conversion:" << std::endl;
    std::vector<std::string> numbers = {"123", "45.67", "invalid", "0", "-789"};
    for (const auto& num_str : numbers) {
        auto int_val = wip::utils::string::to_int(num_str);
        auto double_val = wip::utils::string::to_double(num_str);
        std::cout << "'" << num_str << "' -> ";
        std::cout << "int: " << (int_val ? std::to_string(*int_val) : "invalid") << ", ";
        std::cout << "double: " << (double_val ? std::to_string(*double_val) : "invalid") << std::endl;
    }
    
    // Template substitution
    std::cout << "\n10. Template Substitution:" << std::endl;
    std::string template_str = "Hello ${name}! You have ${count} new messages.";
    std::unordered_map<std::string, std::string> variables = {
        {"name", "Alice"},
        {"count", "3"}
    };
    std::cout << "Template: " << template_str << std::endl;
    std::cout << "Result: " << wip::utils::string::substitute(template_str, variables) << std::endl;
    
    // String similarity
    std::cout << "\n11. String Similarity:" << std::endl;
    std::vector<std::pair<std::string, std::string>> similarity_tests = {
        {"hello", "hello"},
        {"hello", "hallo"},
        {"kitten", "sitting"},
        {"completely", "different"}
    };
    for (const auto& [str1, str2] : similarity_tests) {
        double sim = wip::utils::string::similarity(str1, str2);
        size_t distance = wip::utils::string::levenshtein_distance(str1, str2);
        std::cout << "'" << str1 << "' vs '" << str2 << "': ";
        std::cout << "similarity=" << std::fixed << std::setprecision(2) << sim;
        std::cout << ", distance=" << distance << std::endl;
    }
    
    // Random string generation
    std::cout << "\n12. Random String Generation:" << std::endl;
    std::cout << "Random 10-char string: " << wip::utils::string::random_string(10) << std::endl;
    std::cout << "Random 8-char hex: " << wip::utils::string::random_string(8, "0123456789ABCDEF") << std::endl;
    std::cout << "Random 6-char password: " << wip::utils::string::random_string(6, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*") << std::endl;
}

int main() {
    try {
        std::cout << "Starting WIP Playground Application..." << std::endl;
        
        // Demonstrate string utilities
        demonstrate_string_utilities();
        
        // Demonstrate JSON serializer
        demonstrate_json_serializer();
        
        // Demonstrate UID functionality
        demonstrate_uid();
        
        std::cout << "\nPlayground demonstration completed successfully!" << std::endl;
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}