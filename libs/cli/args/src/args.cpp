#include "args.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>

namespace wip {
namespace cli {
namespace args {

// ==================== ParseResult Implementation ====================

template<typename T>
std::optional<T> ParseResult::get(const std::string& name) const {
    auto it = values_.find(name);
    if (it == values_.end()) {
        return std::nullopt;
    }
    
    try {
        return std::get<T>(it->second);
    } catch (const std::bad_variant_access&) {
        return std::nullopt;
    }
}

// Explicit template instantiations
template std::optional<bool> ParseResult::get<bool>(const std::string& name) const;
template std::optional<int> ParseResult::get<int>(const std::string& name) const;
template std::optional<double> ParseResult::get<double>(const std::string& name) const;
template std::optional<std::string> ParseResult::get<std::string>(const std::string& name) const;
template std::optional<std::vector<std::string>> ParseResult::get<std::vector<std::string>>(const std::string& name) const;

std::optional<std::string> ParseResult::get_string(const std::string& name) const {
    return get<std::string>(name);
}

std::optional<int> ParseResult::get_int(const std::string& name) const {
    return get<int>(name);
}

std::optional<double> ParseResult::get_double(const std::string& name) const {
    return get<double>(name);
}

std::optional<bool> ParseResult::get_bool(const std::string& name) const {
    return get<bool>(name);
}

std::optional<std::vector<std::string>> ParseResult::get_strings(const std::string& name) const {
    return get<std::vector<std::string>>(name);
}

bool ParseResult::has(const std::string& name) const {
    return values_.find(name) != values_.end();
}

std::vector<std::string> ParseResult::get_names() const {
    std::vector<std::string> names;
    for (const auto& pair : values_) {
        names.push_back(pair.first);
    }
    return names;
}

std::optional<std::string> ParseResult::get_subcommand() const {
    return subcommand_.empty() ? std::nullopt : std::make_optional(subcommand_);
}

std::optional<ParseResult> ParseResult::get_subcommand_result() const {
    if (subcommand_result_) {
        // Create a new ParseResult and move the data
        ParseResult result;
        result.values_ = subcommand_result_->values_;
        result.subcommand_ = subcommand_result_->subcommand_;
        // Note: we can't copy the nested unique_ptr, so subcommand nesting is limited to one level
        return result;
    }
    return std::nullopt;
}

// ==================== ArgumentBuilder Implementation ====================

ArgumentBuilder& ArgumentBuilder::description(const std::string& desc) {
    spec_.description = desc;
    return *this;
}

// Explicit template instantiations
template ArgumentBuilder& ArgumentBuilder::default_value<bool>(const bool& value);
template ArgumentBuilder& ArgumentBuilder::default_value<int>(const int& value);
template ArgumentBuilder& ArgumentBuilder::default_value<double>(const double& value);
template ArgumentBuilder& ArgumentBuilder::default_value<std::string>(const std::string& value);
template ArgumentBuilder& ArgumentBuilder::default_value<std::vector<std::string>>(const std::vector<std::string>& value);

ArgumentBuilder& ArgumentBuilder::required(bool req) {
    spec_.required = req;
    return *this;
}

ArgumentBuilder& ArgumentBuilder::env(const std::string& env_var) {
    spec_.env_var = env_var;
    return *this;
}

ArgumentBuilder& ArgumentBuilder::choices(const std::vector<std::string>& choices) {
    spec_.choices = choices;
    return *this;
}

ArgumentBuilder& ArgumentBuilder::multiple(bool mult) {
    spec_.multiple = mult;
    return *this;
}

ArgumentBuilder& ArgumentBuilder::metavar(const std::string& metavar) {
    spec_.metavar = metavar;
    return *this;
}

ArgumentBuilder& ArgumentBuilder::validator(Validator val) {
    spec_.validator = val;
    return *this;
}

// ==================== SubCommand Implementation ====================

SubCommand::SubCommand(const std::string& name, const std::string& description)
    : name_(name), description_(description) {}

ArgumentBuilder SubCommand::add_flag(const std::vector<std::string>& flags, const std::string& name) {
    ArgumentSpec spec;
    spec.name = name;
    spec.flags = flags;
    spec.is_flag = true;
    spec.default_value = false;
    
    arguments_.push_back(spec);
    return ArgumentBuilder(arguments_.back());
}

ArgumentBuilder SubCommand::add_option(const std::vector<std::string>& flags, const std::string& name) {
    ArgumentSpec spec;
    spec.name = name;
    spec.flags = flags;
    spec.is_flag = false;
    spec.default_value = std::string{};
    
    arguments_.push_back(spec);
    return ArgumentBuilder(arguments_.back());
}

SubCommand& SubCommand::add_subcommand(const std::string& name, const std::string& description) {
    auto subcmd = std::make_unique<SubCommand>(name, description);
    auto& ref = *subcmd;
    subcommands_[name] = std::move(subcmd);
    return ref;
}

std::string SubCommand::help() const {
    std::ostringstream oss;
    
    if (!description_.empty()) {
        oss << description_ << "\n\n";
    }
    
    oss << "Usage: " << generate_usage() << "\n\n";
    
    // Options and flags
    std::vector<const ArgumentSpec*> option_args;
    for (const auto& arg : arguments_) {
        option_args.push_back(&arg);
    }
    
    if (!option_args.empty()) {
        oss << "Options:\n";
        for (const auto* arg : option_args) {
            oss << "  ";
            for (size_t i = 0; i < arg->flags.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << arg->flags[i];
            }
            
            if (!arg->is_flag) {
                oss << " " << (arg->metavar.empty() ? "VALUE" : arg->metavar);
            }
            
            if (!arg->description.empty()) {
                oss << "\t" << arg->description;
            }
            
            oss << "\n";
        }
        oss << "\n";
    }
    
    // Subcommands
    if (!subcommands_.empty()) {
        oss << "Subcommands:\n";
        for (const auto& [name, subcmd] : subcommands_) {
            oss << "  " << name;
            if (!subcmd->description_.empty()) {
                oss << "\t" << subcmd->description_;
            }
            oss << "\n";
        }
    }
    
    return oss.str();
}

Result<ParseResult> SubCommand::parse(const std::vector<std::string>& args) const {
    ParseResult result;
    size_t arg_index = 0;
    
    // Initialize with default values
    for (const auto& spec : arguments_) {
        if (std::holds_alternative<bool>(spec.default_value)) {
            result.values_[spec.name] = std::get<bool>(spec.default_value);
        } else if (std::holds_alternative<int>(spec.default_value)) {
            result.values_[spec.name] = std::get<int>(spec.default_value);
        } else if (std::holds_alternative<double>(spec.default_value)) {
            result.values_[spec.name] = std::get<double>(spec.default_value);
        } else if (std::holds_alternative<std::string>(spec.default_value)) {
            result.values_[spec.name] = std::get<std::string>(spec.default_value);
        } else if (std::holds_alternative<std::vector<std::string>>(spec.default_value)) {
            result.values_[spec.name] = std::get<std::vector<std::string>>(spec.default_value);
        }
    }
    
    while (arg_index < args.size()) {
        const std::string& arg = args[arg_index];
        
        // Check for subcommands first
        if (subcommands_.find(arg) != subcommands_.end()) {
            result.subcommand_ = arg;
            std::vector<std::string> sub_args(args.begin() + arg_index + 1, args.end());
            auto sub_result = subcommands_.at(arg)->parse(sub_args);
            if (!sub_result) {
                return std::nullopt;
            }
            result.subcommand_result_ = std::make_unique<ParseResult>(std::move(sub_result.value()));
            break;
        }
        
        // Handle flags and options
        if (arg.front() == '-') {
            auto arg_spec = find_argument_by_flag(arg);
            if (!arg_spec) {
                std::cerr << "Unknown argument: " << arg << std::endl;
                return std::nullopt;
            }
            
            if ((*arg_spec)->is_flag) {
                result.values_[(*arg_spec)->name] = true;
                arg_index++;
            } else {
                // Need a value
                if (arg_index + 1 >= args.size()) {
                    std::cerr << "Argument " << arg << " requires a value" << std::endl;
                    return std::nullopt;
                }
                
                auto value = parse_value(**arg_spec, args[arg_index + 1]);
                if (!validate_argument(**arg_spec, value)) {
                    return std::nullopt;
                }
                
                result.values_[(*arg_spec)->name] = value;
                arg_index += 2;
            }
        } else {
            std::cerr << "Unexpected positional argument: " << arg << std::endl;
            return std::nullopt;
        }
    }
    
    // Check required arguments
    for (const auto& spec : arguments_) {
        if (spec.required && result.values_.find(spec.name) == result.values_.end()) {
            std::cerr << "Required argument missing: " << spec.name << std::endl;
            return std::nullopt;
        }
    }
    
    return result;
}

std::optional<ArgumentSpec*> SubCommand::find_argument_by_flag(const std::string& flag) const {
    for (auto& arg : const_cast<SubCommand*>(this)->arguments_) {
        if (std::find(arg.flags.begin(), arg.flags.end(), flag) != arg.flags.end()) {
            return &arg;
        }
    }
    return std::nullopt;
}

ArgumentValue SubCommand::parse_value(const ArgumentSpec& spec, const std::string& value) const {
    if (spec.is_flag) {
        return true;
    }
    
    if (spec.multiple) {
        return converters::split(value);
    }
    
    // Try to infer type from default value
    if (std::holds_alternative<int>(spec.default_value)) {
        auto int_val = converters::to_int(value);
        return int_val ? int_val.value() : 0;
    } else if (std::holds_alternative<double>(spec.default_value)) {
        auto double_val = converters::to_double(value);
        return double_val ? double_val.value() : 0.0;
    } else if (std::holds_alternative<bool>(spec.default_value)) {
        auto bool_val = converters::to_bool(value);
        return bool_val ? bool_val.value() : false;
    }
    
    return value; // Default to string
}

bool SubCommand::validate_argument(const ArgumentSpec& spec, const ArgumentValue& value) const {
    // Check choices
    if (!spec.choices.empty()) {
        std::string str_value;
        if (std::holds_alternative<std::string>(value)) {
            str_value = std::get<std::string>(value);
        } else {
            return false;
        }
        
        if (std::find(spec.choices.begin(), spec.choices.end(), str_value) == spec.choices.end()) {
            std::cerr << "Invalid choice for " << spec.name << ": " << str_value << std::endl;
            return false;
        }
    }
    
    // Run custom validator
    if (spec.validator && !spec.validator(value)) {
        std::cerr << "Validation failed for argument: " << spec.name << std::endl;
        return false;
    }
    
    return true;
}

std::string SubCommand::generate_usage() const {
    std::ostringstream oss;
    oss << name_;
    
    // Add options
    for (const auto& arg : arguments_) {
        oss << " [";
        if (!arg.flags.empty()) {
            oss << arg.flags[0];
        }
        if (!arg.is_flag) {
            oss << " " << (arg.metavar.empty() ? "VALUE" : arg.metavar);
        }
        oss << "]";
    }
    
    // Add subcommands
    if (!subcommands_.empty()) {
        oss << " [subcommand]";
    }
    
    return oss.str();
}

// ==================== ArgumentParser Implementation ====================

ArgumentParser::ArgumentParser(const std::string& program_name, const std::string& description, const std::string& version)
    : program_name_(program_name), description_(description), version_(version) {}

ArgumentBuilder ArgumentParser::add_flag(const std::vector<std::string>& flags, const std::string& name) {
    ArgumentSpec spec;
    spec.name = name;
    spec.flags = flags;
    spec.is_flag = true;
    spec.default_value = false;
    
    arguments_.push_back(spec);
    return ArgumentBuilder(arguments_.back());
}

ArgumentBuilder ArgumentParser::add_option(const std::vector<std::string>& flags, const std::string& name) {
    ArgumentSpec spec;
    spec.name = name;
    spec.flags = flags;
    spec.is_flag = false;
    spec.default_value = std::string{};
    
    arguments_.push_back(spec);
    return ArgumentBuilder(arguments_.back());
}

SubCommand& ArgumentParser::add_subcommand(const std::string& name, const std::string& description) {
    auto subcmd = std::make_unique<SubCommand>(name, description);
    auto& ref = *subcmd;
    subcommands_[name] = std::move(subcmd);
    return ref;
}

Result<ParseResult> ArgumentParser::parse(int argc, char* argv[]) const {
    auto args = tokenize_arguments(argc, argv);
    return parse(args);
}

Result<ParseResult> ArgumentParser::parse(const std::vector<std::string>& args) const {
    // Check for help
    if (auto_help_) {
        for (const auto& arg : args) {
            if (is_help_flag(arg)) {
                std::cout << help() << std::endl;
                std::exit(0);
            }
        }
    }
    
    // Check for version
    if (auto_version_ && !version_.empty()) {
        for (const auto& arg : args) {
            if (is_version_flag(arg)) {
                std::cout << program_name_ << " " << version_ << std::endl;
                std::exit(0);
            }
        }
    }
    
    ParseResult result;
    size_t arg_index = 0;
    
    // Initialize with default values and environment variables
    for (const auto& spec : arguments_) {
        ArgumentValue value = get_default_or_env_value(spec);
        
        // Only add to result if there's a meaningful default value or environment variable
        bool should_add = false;
        
        if (std::holds_alternative<bool>(value)) {
            should_add = true;  // bool defaults are always meaningful
        } else if (std::holds_alternative<int>(value)) {
            should_add = (std::get<int>(value) != 0);  // non-zero int is meaningful
        } else if (std::holds_alternative<double>(value)) {
            should_add = (std::get<double>(value) != 0.0);  // non-zero double is meaningful
        } else if (std::holds_alternative<std::string>(value)) {
            should_add = !std::get<std::string>(value).empty();  // non-empty string is meaningful
        } else if (std::holds_alternative<std::vector<std::string>>(value)) {
            should_add = !std::get<std::vector<std::string>>(value).empty();  // non-empty vector is meaningful
        }
        
        // Also add if there's an environment variable set (even if default was empty)
        if (!should_add && !spec.env_var.empty() && std::getenv(spec.env_var.c_str()) != nullptr) {
            should_add = true;
        }
        
        if (should_add) {
            if (std::holds_alternative<bool>(value)) {
                result.values_[spec.name] = std::get<bool>(value);
            } else if (std::holds_alternative<int>(value)) {
                result.values_[spec.name] = std::get<int>(value);
            } else if (std::holds_alternative<double>(value)) {
                result.values_[spec.name] = std::get<double>(value);
            } else if (std::holds_alternative<std::string>(value)) {
                result.values_[spec.name] = std::get<std::string>(value);
            } else if (std::holds_alternative<std::vector<std::string>>(value)) {
                result.values_[spec.name] = std::get<std::vector<std::string>>(value);
            }
        }
    }
    
    while (arg_index < args.size()) {
        const std::string& arg = args[arg_index];
        
        // Check for subcommands first
        if (subcommands_.find(arg) != subcommands_.end()) {
            result.subcommand_ = arg;
            std::vector<std::string> sub_args(args.begin() + arg_index + 1, args.end());
            auto sub_result = subcommands_.at(arg)->parse(sub_args);
            if (!sub_result) {
                return std::nullopt;
            }
            result.subcommand_result_ = std::make_unique<ParseResult>(std::move(sub_result.value()));
            break;
        }
        
        // Handle flags and options
        if (arg.front() == '-') {
            auto arg_spec = find_argument_by_flag(arg);
            if (!arg_spec) {
                std::cerr << "Unknown argument: " << arg << std::endl;
                return std::nullopt;
            }
            
            if ((*arg_spec)->is_flag) {
                result.values_[(*arg_spec)->name] = true;
                arg_index++;
            } else {
                // Need a value
                if (arg_index + 1 >= args.size()) {
                    std::cerr << "Argument " << arg << " requires a value" << std::endl;
                    return std::nullopt;
                }
                
                auto value = parse_value(**arg_spec, args[arg_index + 1]);
                if (!validate_argument(**arg_spec, value)) {
                    return std::nullopt;
                }
                
                result.values_[(*arg_spec)->name] = value;
                arg_index += 2;
            }
        } else {
            // Positional arguments are not supported in this implementation
            std::cerr << "Unexpected positional argument: " << arg << std::endl;
            return std::nullopt;
        }
    }
    
    // Check required arguments
    for (const auto& spec : arguments_) {
        if (spec.required && result.values_.find(spec.name) == result.values_.end()) {
            std::cerr << "Required argument missing: " << spec.name << std::endl;
            return std::nullopt;
        }
    }
    
    return result;
}

std::string ArgumentParser::help() const {
    std::ostringstream oss;
    
    if (!description_.empty()) {
        oss << description_ << "\n\n";
    }
    
    oss << "Usage: " << usage() << "\n\n";
    
    // Options and flags
    std::vector<const ArgumentSpec*> option_args;
    for (const auto& arg : arguments_) {
        option_args.push_back(&arg);
    }
    
    if (!option_args.empty() || auto_help_ || auto_version_) {
        oss << "Options:\n";
        
        for (const auto* arg : option_args) {
            oss << "  ";
            for (size_t i = 0; i < arg->flags.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << arg->flags[i];
            }
            
            if (!arg->is_flag) {
                oss << " " << (arg->metavar.empty() ? "VALUE" : arg->metavar);
            }
            
            if (!arg->description.empty()) {
                oss << "\t" << arg->description;
            }
            
            oss << "\n";
        }
        
        if (auto_help_) {
            oss << "  ";
            for (size_t i = 0; i < help_flags_.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << help_flags_[i];
            }
            oss << "\tShow this help message and exit\n";
        }
        
        if (auto_version_ && !version_.empty()) {
            oss << "  ";
            for (size_t i = 0; i < version_flags_.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << version_flags_[i];
            }
            oss << "\tShow version and exit\n";
        }
        
        oss << "\n";
    }
    
    // Subcommands
    if (!subcommands_.empty()) {
        oss << "Subcommands:\n";
        for (const auto& [name, subcmd] : subcommands_) {
            oss << "  " << name;
            if (!subcmd->description_.empty()) {
                oss << "\t" << subcmd->description_;
            }
            oss << "\n";
        }
    }
    
    return oss.str();
}

std::string ArgumentParser::usage() const {
    return generate_usage();
}

ArgumentParser& ArgumentParser::help_flags(const std::vector<std::string>& flags) {
    help_flags_ = flags;
    return *this;
}

ArgumentParser& ArgumentParser::version_flags(const std::vector<std::string>& flags) {
    version_flags_ = flags;
    return *this;
}

ArgumentParser& ArgumentParser::auto_help(bool enable) {
    auto_help_ = enable;
    return *this;
}

ArgumentParser& ArgumentParser::auto_version(bool enable) {
    auto_version_ = enable;
    return *this;
}

std::optional<ArgumentSpec*> ArgumentParser::find_argument_by_flag(const std::string& flag) const {
    for (auto& arg : const_cast<ArgumentParser*>(this)->arguments_) {
        if (std::find(arg.flags.begin(), arg.flags.end(), flag) != arg.flags.end()) {
            return &arg;
        }
    }
    return std::nullopt;
}

ArgumentValue ArgumentParser::parse_value(const ArgumentSpec& spec, const std::string& value) const {
    if (spec.is_flag) {
        return true;
    }
    
    if (spec.multiple) {
        return converters::split(value);
    }
    
    // Try to infer type from default value
    if (std::holds_alternative<int>(spec.default_value)) {
        auto int_val = converters::to_int(value);
        return int_val ? int_val.value() : 0;
    } else if (std::holds_alternative<double>(spec.default_value)) {
        auto double_val = converters::to_double(value);
        return double_val ? double_val.value() : 0.0;
    } else if (std::holds_alternative<bool>(spec.default_value)) {
        auto bool_val = converters::to_bool(value);
        return bool_val ? bool_val.value() : false;
    }
    
    return value; // Default to string
}

bool ArgumentParser::validate_argument(const ArgumentSpec& spec, const ArgumentValue& value) const {
    // Check choices
    if (!spec.choices.empty()) {
        std::string str_value;
        if (std::holds_alternative<std::string>(value)) {
            str_value = std::get<std::string>(value);
        } else {
            return false;
        }
        
        if (std::find(spec.choices.begin(), spec.choices.end(), str_value) == spec.choices.end()) {
            std::cerr << "Invalid choice for " << spec.name << ": " << str_value << std::endl;
            return false;
        }
    }
    
    // Run custom validator
    if (spec.validator && !spec.validator(value)) {
        std::cerr << "Validation failed for argument: " << spec.name << std::endl;
        return false;
    }
    
    return true;
}

ArgumentValue ArgumentParser::get_default_or_env_value(const ArgumentSpec& spec) const {
    // Check environment variable first
    if (!spec.env_var.empty()) {
        const char* env_value = std::getenv(spec.env_var.c_str());
        if (env_value) {
            return parse_value(spec, std::string(env_value));
        }
    }
    
    // Return default value
    return spec.default_value;
}

bool ArgumentParser::is_help_flag(const std::string& flag) const {
    return std::find(help_flags_.begin(), help_flags_.end(), flag) != help_flags_.end();
}

bool ArgumentParser::is_version_flag(const std::string& flag) const {
    return std::find(version_flags_.begin(), version_flags_.end(), flag) != version_flags_.end();
}

std::string ArgumentParser::generate_usage() const {
    std::ostringstream oss;
    oss << program_name_;
    
    // Add options
    for (const auto& arg : arguments_) {
            oss << " [";
            if (!arg.flags.empty()) {
                oss << arg.flags[0];
            }
            if (!arg.is_flag) {
                oss << " " << (arg.metavar.empty() ? "VALUE" : arg.metavar);
            }
            oss << "]";
    }
    
    // Add subcommands
    if (!subcommands_.empty()) {
        oss << " [subcommand]";
    }
    
    return oss.str();
}

std::vector<std::string> ArgumentParser::tokenize_arguments(int argc, char* argv[]) const {
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) { // Skip program name
        args.emplace_back(argv[i]);
    }
    return args;
}

// ==================== Utility Functions ====================

namespace converters {

std::optional<int> to_int(const std::string& str) {
    try {
        size_t pos;
        int value = std::stoi(str, &pos);
        return pos == str.length() ? std::make_optional(value) : std::nullopt;
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<double> to_double(const std::string& str) {
    try {
        size_t pos;
        double value = std::stod(str, &pos);
        return pos == str.length() ? std::make_optional(value) : std::nullopt;
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<bool> to_bool(const std::string& str) {
    std::string lower_str = str;
    std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(), ::tolower);
    
    if (lower_str == "true" || lower_str == "1" || lower_str == "yes" || lower_str == "on") {
        return true;
    } else if (lower_str == "false" || lower_str == "0" || lower_str == "no" || lower_str == "off") {
        return false;
    }
    
    return std::nullopt;
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;
    
    while (std::getline(iss, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

} // namespace converters

namespace validators {

bool file_exists(const ArgumentValue& value) {
    if (!std::holds_alternative<std::string>(value)) {
        return false;
    }
    
    const std::string& path = std::get<std::string>(value);
    return std::filesystem::exists(path) && std::filesystem::is_regular_file(path);
}

bool directory_exists(const ArgumentValue& value) {
    if (!std::holds_alternative<std::string>(value)) {
        return false;
    }
    
    const std::string& path = std::get<std::string>(value);
    return std::filesystem::exists(path) && std::filesystem::is_directory(path);
}

bool positive_int(const ArgumentValue& value) {
    if (!std::holds_alternative<int>(value)) {
        return false;
    }
    
    return std::get<int>(value) > 0;
}

template<typename T>
Validator range(T min, T max) {
    return [min, max](const ArgumentValue& value) -> bool {
        if (std::holds_alternative<T>(value)) {
            T val = std::get<T>(value);
            return val >= min && val <= max;
        }
        return false;
    };
}

// Explicit template instantiations
template Validator range<int>(int min, int max);
template Validator range<double>(double min, double max);

Validator one_of(const std::vector<std::string>& choices) {
    return [choices](const ArgumentValue& value) -> bool {
        if (!std::holds_alternative<std::string>(value)) {
            return false;
        }
        
        const std::string& str_value = std::get<std::string>(value);
        return std::find(choices.begin(), choices.end(), str_value) != choices.end();
    };
}

} // namespace validators

} // namespace args
} // namespace cli
} // namespace wip