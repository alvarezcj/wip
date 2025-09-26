#include <iostream>
#include <iomanip>
#include <vector>
#include <unordered_map>
#include <unordered_set>

#include "file.h"
#include "wip_string.h"
#include "json_serializer.h"
#include "uid.h"
#include "args.h"
#include "dice.h"

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

void demo_cli_args() {
    std::cout << "\n--- CLI Args Parser Demo ---\n\n";

    // Create a parser with program info
    wip::cli::args::ArgumentParser parser("demo", "Demo CLI application for argument parsing", "1.0.0");

    // Add flags
    parser.add_flag({"-v", "--verbose"}, "verbose")
          .description("Enable verbose output");
    parser.add_flag({"-h", "--help"}, "help")
          .description("Show help message");
    
    // Add options
    parser.add_option({"-f", "--file"}, "file")
          .description("Input file path")
          .required();
    
    parser.add_option({"-o", "--output"}, "output")
          .description("Output directory")
          .default_value(std::string("./output"));
    
    parser.add_option({"-c", "--count"}, "count")
          .description("Number of iterations")
          .default_value(1);

    // Demo parsing with simulated command line (excluding program name)
    std::vector<std::string> test_args = {
        "--verbose",
        "--file", "input.txt",
        "--count", "5"
    };

    std::cout << "Parsing command: demo ";
    for (size_t i = 0; i < test_args.size(); ++i) {
        if (i > 0) std::cout << " ";
        std::cout << test_args[i];
    }
    std::cout << "\n\n";

    try {
        auto result = parser.parse(test_args);
        
        if (!result) {
            std::cout << "Parsing failed!\n";
            return;
        }
        
        std::cout << "Parsing successful!\n";
        std::cout << "Parsed results:\n";
        
        // Check flags
        if (result->get_bool("verbose").value_or(false)) {
            std::cout << "  - Verbose mode: enabled\n";
        }
        
        // Check options
        auto file = result->get_string("file");
        if (file) {
            std::cout << "  - Input file: " << *file << "\n";
        }
        
        auto output = result->get_string("output");
        if (output) {
            std::cout << "  - Output directory: " << *output << "\n";
        }
        
        auto count = result->get_int("count");
        if (count) {
            std::cout << "  - Count: " << *count << "\n";
        }
        
    } catch (const std::exception& e) {
        std::cout << "Parsing error: " << e.what() << "\n";
    }

    // Demo help generation
    std::cout << "\nGenerated help:\n";
    std::cout << parser.help() << "\n";
}

void demonstrate_dice_library() {
    using namespace wip::game::dice;
    
    std::cout << "\n=== Dice Library Demonstration ===\n" << std::endl;
    
    // Basic dice rolling
    std::cout << "1. Basic Dice Rolling:" << std::endl;
    auto d6 = Die<int>::d6();
    auto d20 = Die<int>::d20();
    
    std::cout << "d6 rolls: ";
    for (int i = 0; i < 5; ++i) {
        std::cout << d6.roll() << " ";
    }
    std::cout << std::endl;
    
    std::cout << "d20 rolls: ";
    for (int i = 0; i < 5; ++i) {
        std::cout << d20.roll() << " ";
    }
    std::cout << std::endl;
    
    // Advantage and disadvantage
    std::cout << "\n2. Advantage & Disadvantage:" << std::endl;
    std::cout << "d20 advantage: ";
    for (int i = 0; i < 3; ++i) {
        std::cout << d20.advantage() << " ";
    }
    std::cout << std::endl;
    
    std::cout << "d20 disadvantage: ";
    for (int i = 0; i < 3; ++i) {
        std::cout << d20.disadvantage() << " ";
    }
    std::cout << std::endl;
    
    // Modifiers
    std::cout << "\n3. Dice Modifiers:" << std::endl;
    auto modified_d6 = Die<int>::d6();
    modified_d6.add_modifier(Modifier(ModifierType::Add, 3, "magic bonus"));
    
    auto result = modified_d6.roll_detailed();
    std::cout << "d6 + 3: rolled " << (result.total() - 3) << " + 3 = " << result.total() << std::endl;
    
    // Keep highest modifier
    auto ability_die = Die<int>::d6();
    ability_die.add_modifier(Modifier(ModifierType::KeepHighest, 3, "keep highest 3"));
    
    auto ability_result = ability_die.roll_detailed(4);
    std::cout << "4d6 keep highest 3: ";
    std::cout << "rolled [";
    for (size_t i = 0; i < ability_result.individual_rolls.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << ability_result.individual_rolls[i].value;
    }
    std::cout << "] -> kept [";
    for (size_t i = 0; i < ability_result.kept_values.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << ability_result.kept_values[i];
    }
    std::cout << "] = " << ability_result.total() << std::endl;
    
    // Custom dice
    std::cout << "\n4. Custom Dice:" << std::endl;
    auto fudge = Die<int>::fudge();
    std::cout << "Fudge die rolls: ";
    for (int i = 0; i < 10; ++i) {
        int roll = fudge.roll();
        if (roll == -1) std::cout << "[-] ";
        else if (roll == 0) std::cout << "[0] ";
        else std::cout << "[+] ";
    }
    std::cout << std::endl;
    
    // Custom weighted die
    std::vector<int> weighted_faces = {1, 2, 2, 3, 3, 3, 4, 5, 6};
    Die<int> weighted(weighted_faces);
    std::cout << "Weighted die (favors 3): ";
    for (int i = 0; i < 10; ++i) {
        std::cout << weighted.roll() << " ";
    }
    std::cout << std::endl;
    
    // Exploding dice
    std::cout << "\n5. Exploding Dice:" << std::endl;
    auto exploding_d6 = Die<int>::d6();
    std::cout << "Exploding d6 results: ";
    for (int i = 0; i < 5; ++i) {
        int result = exploding_d6.exploding();
        std::cout << result;
        if (result > 6) std::cout << "(!)";
        std::cout << " ";
    }
    std::cout << std::endl;
    
    // Dice sets
    std::cout << "\n6. Dice Sets:" << std::endl;
    DiceSet<int> damage_dice;
    damage_dice.add_dice(Die<int>::d8(), 2);  // 2d8
    damage_dice.add_dice(Die<int>::d6(), 1);  // +1d6
    
    auto damage_result = damage_dice.roll_detailed();
    std::cout << "Damage roll (2d8 + 1d6): ";
    for (int value : damage_result.kept_values) {
        std::cout << value << " ";
    }
    std::cout << "= " << damage_result.total() << std::endl;
    
    // D&D mechanics
    std::cout << "\n7. D&D Mechanics:" << std::endl;
    
    // Ability scores
    std::cout << "Character ability scores:" << std::endl;
    auto stats = gaming::dnd::generate_character_stats();
    for (const auto& stat : stats) {
        std::cout << "  " << std::setw(12) << stat.first << ": " << std::setw(2) << stat.second;
        int modifier = (stat.second - 10) / 2;
        if (modifier >= 0) std::cout << " (+" << modifier << ")";
        else std::cout << " (" << modifier << ")";
        std::cout << std::endl;
    }
    
    // Attack rolls
    std::cout << "\nAttack sequence:" << std::endl;
    for (int i = 0; i < 3; ++i) {
        int attack = gaming::dnd::attack_roll(5);  // +5 attack bonus
        std::cout << "  Attack roll: " << std::setw(2) << attack;
        if (attack >= 20) std::cout << " (CRITICAL HIT!)";
        else if (attack >= 15) std::cout << " (Hit)";
        else std::cout << " (Miss)";
        std::cout << std::endl;
    }
    
    // Death saving throws
    std::cout << "\nDeath saving throws:" << std::endl;
    int successes = 0, failures = 0;
    for (int i = 0; i < 5 && successes < 3 && failures < 3; ++i) {
        bool success = gaming::dnd::death_save();
        std::cout << "  Roll " << (i + 1) << ": ";
        if (success) {
            std::cout << "SUCCESS";
            successes++;
        } else {
            std::cout << "FAILURE";
            failures++;
        }
        std::cout << " (S:" << successes << " F:" << failures << ")" << std::endl;
    }
    
    if (successes >= 3) std::cout << "  -> Character STABILIZES!" << std::endl;
    else if (failures >= 3) std::cout << "  -> Character DIES!" << std::endl;
    
    // Dice expressions
    std::cout << "\n8. Dice Expressions:" << std::endl;
    std::vector<std::string> expressions = {
        "d20", "3d6", "2d8+5", "4d6kh3", "1d100"
    };
    
    for (const std::string& expr : expressions) {
        if (DiceExpression::is_valid(expr)) {
            auto expr_result = DiceExpression::evaluate(expr);
            std::cout << "  " << std::setw(8) << expr << " -> " << std::setw(3) 
                     << expr_result.total() << " (" << expr_result.breakdown << ")" << std::endl;
        } else {
            std::cout << "  " << std::setw(8) << expr << " -> INVALID" << std::endl;
        }
    }
    
    // Gaming utilities
    std::cout << "\n9. Gaming Utilities:" << std::endl;
    
    // Percentile rolls
    std::cout << "Percentile rolls: ";
    for (int i = 0; i < 5; ++i) {
        std::cout << std::setw(3) << gaming::utils::percentile() << "% ";
    }
    std::cout << std::endl;
    
    // Random table
    std::map<int, std::string> encounter_table = {
        {20, "Nothing happens"},
        {40, "Friendly NPC"},
        {60, "Treasure found"},
        {80, "Minor monster"},
        {95, "Major monster"},
        {100, "Boss encounter"}
    };
    
    std::cout << "Random encounters: ";
    for (int i = 0; i < 3; ++i) {
        std::string encounter = gaming::utils::random_table_entry(encounter_table);
        std::cout << "[" << encounter << "] ";
    }
    std::cout << std::endl;
    
    // Dice pools
    std::cout << "Dice pool (5d6, target 4+): ";
    for (int i = 0; i < 3; ++i) {
        int successes = gaming::utils::dice_pool_successes(5, 6, 4);
        std::cout << successes << " successes  ";
    }
    std::cout << std::endl;
    
    // Global convenience functions
    std::cout << "\n10. Quick Roll Functions:" << std::endl;
    std::cout << "Quick d20: " << wip::game::dice::d20() << std::endl;
    std::cout << "Quick 3d6: " << roll_dice(3, 6) << std::endl;
    std::cout << "Notation roll '2d8+3': " << roll("2d8+3") << std::endl;
    
    std::cout << "\nDice library demonstration complete!" << std::endl;
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
        
        // Demonstrate CLI args parsing
        demo_cli_args();
        
        // Demonstrate dice library
        demonstrate_dice_library();
        
        std::cout << "\nPlayground demonstration completed successfully!" << std::endl;
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}