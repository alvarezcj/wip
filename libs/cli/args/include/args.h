#pragma once

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <functional>
#include <variant>
#include <memory>
#include <iostream>
#include <sstream>

namespace wip {
namespace cli {
namespace args {

// Result type for parsing operations
template<typename T>
using Result = std::optional<T>;

// Argument value types
using ArgumentValue = std::variant<bool, int, double, std::string, std::vector<std::string>>;

// Validation function type
using Validator = std::function<bool(const ArgumentValue&)>;

// Conversion function type
template<typename T>
using Converter = std::function<std::optional<T>(const std::string&)>;

/**
 * @brief Argument specification for the parser
 */
struct ArgumentSpec {
    std::string name;                    // Argument name (used for access)
    std::vector<std::string> flags;      // Command-line flags (e.g., {"-v", "--verbose"})
    std::string description;             // Help description
    ArgumentValue default_value;         // Default value
    bool required = false;               // Whether argument is required
    bool is_flag = false;               // Boolean flag vs value argument
    Validator validator;                // Optional validation function
    std::string env_var;                // Environment variable fallback
    std::vector<std::string> choices;   // Valid choices (for validation)
    bool multiple = false;              // Allow multiple values
    std::string metavar;                // Placeholder in help (e.g., FILE, COUNT)
};

/**
 * @brief Subcommand specification
 */
class SubCommand;

/**
 * @brief Parse results containing all argument values
 */
class ParseResult {
public:
    ParseResult() = default;
    ParseResult(const ParseResult&) = delete;
    ParseResult& operator=(const ParseResult&) = delete;
    ParseResult(ParseResult&&) = default;
    ParseResult& operator=(ParseResult&&) = default;
    
    // Get argument value as specific type
    template<typename T>
    std::optional<T> get(const std::string& name) const;
    
    // Get argument value as string (most common)
    std::optional<std::string> get_string(const std::string& name) const;
    
    // Get argument value as integer
    std::optional<int> get_int(const std::string& name) const;
    
    // Get argument value as double
    std::optional<double> get_double(const std::string& name) const;
    
    // Get argument value as boolean
    std::optional<bool> get_bool(const std::string& name) const;
    
    // Get argument value as vector of strings
    std::optional<std::vector<std::string>> get_strings(const std::string& name) const;
    
    // Check if argument was provided
    bool has(const std::string& name) const;
    
    // Get all argument names
    std::vector<std::string> get_names() const;
    
    // Get the selected subcommand name (if any)
    std::optional<std::string> get_subcommand() const;
    
    // Get the parse result for a subcommand
    std::optional<ParseResult> get_subcommand_result() const;

private:
    friend class ArgumentParser;
    friend class SubCommand;
    
    std::map<std::string, ArgumentValue> values_;
    std::string subcommand_;
    std::unique_ptr<ParseResult> subcommand_result_;
};

/**
 * @brief Fluent argument builder for type-safe argument specification
 */
class ArgumentBuilder {
public:
    ArgumentBuilder(ArgumentSpec& spec) : spec_(spec) {}
    
    // Set description
    ArgumentBuilder& description(const std::string& desc);
    
    // Set default value
    template<typename T>
    ArgumentBuilder& default_value(const T& value){
        spec_.default_value = value;
        return *this;
    }
    
    // Mark as required
    ArgumentBuilder& required(bool req = true);
    
    // Set environment variable fallback
    ArgumentBuilder& env(const std::string& env_var);
    
    // Set valid choices
    ArgumentBuilder& choices(const std::vector<std::string>& choices);
    
    // Allow multiple values
    ArgumentBuilder& multiple(bool mult = true);
    
    // Set metavar for help
    ArgumentBuilder& metavar(const std::string& metavar);
    
    // Set custom validator
    ArgumentBuilder& validator(Validator val);
    
private:
    ArgumentSpec& spec_;
};

/**
 * @brief Subcommand for implementing command hierarchies
 */
class SubCommand {
public:
    SubCommand(const std::string& name, const std::string& description = "");
    
    // Add flag argument (boolean)
    ArgumentBuilder add_flag(const std::vector<std::string>& flags, const std::string& name);
    
    // Add option argument (with value)
    ArgumentBuilder add_option(const std::vector<std::string>& flags, const std::string& name);
    
    // Add nested subcommand
    SubCommand& add_subcommand(const std::string& name, const std::string& description = "");
    
    // Generate help text
    std::string help() const;
    
    // Parse arguments for this subcommand
    Result<ParseResult> parse(const std::vector<std::string>& args) const;

private:
    friend class ArgumentParser;
    
    std::string name_;
    std::string description_;
    std::vector<ArgumentSpec> arguments_;
    std::map<std::string, std::unique_ptr<SubCommand>> subcommands_;
    
    // Helper methods
    std::optional<ArgumentSpec*> find_argument_by_flag(const std::string& flag) const;
    ArgumentValue parse_value(const ArgumentSpec& spec, const std::string& value) const;
    bool validate_argument(const ArgumentSpec& spec, const ArgumentValue& value) const;
    std::string generate_usage() const;
};

/**
 * @brief Main argument parser class
 */
class ArgumentParser {
public:
    ArgumentParser(const std::string& program_name, const std::string& description = "", const std::string& version = "");
    
    // Add flag argument (boolean)
    ArgumentBuilder add_flag(const std::vector<std::string>& flags, const std::string& name);
    
    // Add option argument (with value)
    ArgumentBuilder add_option(const std::vector<std::string>& flags, const std::string& name);
    
    // Add subcommand
    SubCommand& add_subcommand(const std::string& name, const std::string& description = "");
    
    // Parse arguments from command line
    Result<ParseResult> parse(int argc, char* argv[]) const;
    
    // Parse arguments from vector
    Result<ParseResult> parse(const std::vector<std::string>& args) const;
    
    // Generate help text
    std::string help() const;
    
    // Generate usage string
    std::string usage() const;
    
    // Set custom help flag
    ArgumentParser& help_flags(const std::vector<std::string>& flags);
    
    // Set custom version flag
    ArgumentParser& version_flags(const std::vector<std::string>& flags);
    
    // Enable/disable automatic help
    ArgumentParser& auto_help(bool enable = true);
    
    // Enable/disable automatic version
    ArgumentParser& auto_version(bool enable = true);

private:
    std::string program_name_;
    std::string description_;
    std::string version_;
    std::vector<ArgumentSpec> arguments_;
    std::map<std::string, std::unique_ptr<SubCommand>> subcommands_;
    
    // Configuration
    std::vector<std::string> help_flags_ = {"-h", "--help"};
    std::vector<std::string> version_flags_ = {"-V", "--version"};
    bool auto_help_ = true;
    bool auto_version_ = true;
    
    // Helper methods
    std::optional<ArgumentSpec*> find_argument_by_flag(const std::string& flag) const;
    ArgumentValue parse_value(const ArgumentSpec& spec, const std::string& value) const;
    bool validate_argument(const ArgumentSpec& spec, const ArgumentValue& value) const;
    ArgumentValue get_default_or_env_value(const ArgumentSpec& spec) const;
    bool is_help_flag(const std::string& flag) const;
    bool is_version_flag(const std::string& flag) const;
    std::string generate_usage() const;
    std::vector<std::string> tokenize_arguments(int argc, char* argv[]) const;
};

// Utility functions for type conversions
namespace converters {

// Convert string to int
std::optional<int> to_int(const std::string& str);

// Convert string to double
std::optional<double> to_double(const std::string& str);

// Convert string to bool
std::optional<bool> to_bool(const std::string& str);

// Split string into vector
std::vector<std::string> split(const std::string& str, char delimiter = ',');

} // namespace converters

// Common validators
namespace validators {

// Validate file exists
bool file_exists(const ArgumentValue& value);

// Validate directory exists
bool directory_exists(const ArgumentValue& value);

// Validate positive integer
bool positive_int(const ArgumentValue& value);

// Validate range (for numeric values)
template<typename T>
Validator range(T min, T max);

// Validate one of choices
Validator one_of(const std::vector<std::string>& choices);

} // namespace validators

} // namespace args
} // namespace cli
} // namespace wip