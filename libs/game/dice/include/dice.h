#pragma once

#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <optional>
#include <algorithm>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <map>
#include <type_traits>

// Include our RNG library
#include "rng.h"

namespace wip {
namespace game {
namespace dice {

// ==================== Enums and Constants ====================

enum class RollType {
    Normal,
    Advantage,      // Roll twice, take higher
    Disadvantage    // Roll twice, take lower
};

enum class ModifierType {
    Add,
    Subtract,
    Multiply,
    Divide,
    RerollOn,       // Reroll if value equals X
    ExplodingOn,    // Roll again and add if value equals X
    KeepHighest,    // Keep N highest dice
    KeepLowest,     // Keep N lowest dice
    DropHighest,    // Drop N highest dice
    DropLowest      // Drop N lowest dice
};

// ==================== Utility Classes ====================

/**
 * @brief Represents a modifier applied to dice rolls
 */
struct Modifier {
    ModifierType type;
    int value;
    std::string description;
    
    Modifier(ModifierType t, int v, const std::string& desc = "")
        : type(t), value(v), description(desc) {}
};

/**
 * @brief Result of a single die roll with history
 */
struct SingleRollResult {
    int value;
    bool was_rerolled = false;
    bool was_exploded = false;
    bool was_dropped = false;
    std::vector<int> explosion_chain;  // For exploding dice
    
    SingleRollResult(int v) : value(v) {}
};

/**
 * @brief Complete result of a dice roll operation
 */
class RollResult {
public:
    std::vector<SingleRollResult> individual_rolls;
    std::vector<int> kept_values;
    std::vector<int> dropped_values;
    int final_total;
    std::string expression;
    std::string breakdown;
    
    RollResult() : final_total(0) {}
    
    // Accessors
    int total() const { return final_total; }
    const std::vector<int>& values() const { return kept_values; }
    size_t count() const { return kept_values.size(); }
    
    // Statistical methods
    int min_value() const {
        return kept_values.empty() ? 0 : *std::min_element(kept_values.begin(), kept_values.end());
    }
    
    int max_value() const {
        return kept_values.empty() ? 0 : *std::max_element(kept_values.begin(), kept_values.end());
    }
    
    double average() const {
        if (kept_values.empty()) return 0.0;
        return static_cast<double>(final_total) / kept_values.size();
    }
    
    std::string to_string() const;
};

// ==================== Die Class Template ====================

/**
 * @brief Template class representing a single die
 * @tparam T The type for die face values (typically int, but could be string for custom dice)
 */
template<typename T = int>
class Die {
public:
    /**
     * @brief Construct a standard die
     * @param sides Number of sides (default: 6)
     * @param rng Optional RNG generator (uses global if not provided)
     */
    explicit Die(int sides = 6, std::shared_ptr<wip::utils::rng::RandomGenerator> rng = nullptr);
    
    /**
     * @brief Construct a custom die with specific face values
     * @param faces Vector of face values
     * @param rng Optional RNG generator
     */
    explicit Die(const std::vector<T>& faces, std::shared_ptr<wip::utils::rng::RandomGenerator> rng = nullptr);
    
    // Copy and move constructors
    Die(const Die& other) = default;
    Die(Die&& other) = default;
    Die& operator=(const Die& other) = default;
    Die& operator=(Die&& other) = default;
    
    // ==================== Basic Rolling ====================
    
    /**
     * @brief Roll the die once
     * @return Single roll result
     */
    T roll();
    
    /**
     * @brief Roll the die multiple times
     * @param count Number of rolls
     * @return Vector of roll results
     */
    std::vector<T> roll(int count);
    
    /**
     * @brief Roll with advantage/disadvantage
     * @param type Type of roll (Normal, Advantage, Disadvantage)
     * @return Roll result
     */
    T roll(RollType type);
    
    // ==================== Modifiers ====================
    
    /**
     * @brief Add a modifier to this die
     * @param modifier Modifier to add
     */
    void add_modifier(const Modifier& modifier);
    
    /**
     * @brief Clear all modifiers
     */
    void clear_modifiers();
    
    /**
     * @brief Roll with detailed result including modifier effects
     * @return Detailed roll result
     */
    RollResult roll_detailed();
    
    /**
     * @brief Roll multiple dice with detailed results
     * @param count Number of dice to roll
     * @return Combined detailed result
     */
    RollResult roll_detailed(int count);
    
    // ==================== Convenience Methods ====================
    
    /**
     * @brief Roll with advantage (roll twice, keep higher)
     * @return Higher of two rolls
     */
    T advantage();
    
    /**
     * @brief Roll with disadvantage (roll twice, keep lower)
     * @return Lower of two rolls
     */
    T disadvantage();
    
    /**
     * @brief Exploding dice - keep rolling and adding when max is rolled
     * @param explode_on Value that triggers explosion (default: max face)
     * @return Total of all exploded rolls
     */
    T exploding(std::optional<T> explode_on = std::nullopt);
    
    /**
     * @brief Reroll on specific value
     * @param reroll_on Value that triggers reroll
     * @param max_rerolls Maximum number of rerolls to prevent infinite loops
     * @return Final roll value
     */
    T reroll_on(T reroll_on, int max_rerolls = 10);
    
    // ==================== Information ====================
    
    /**
     * @brief Get number of sides
     */
    int sides() const { return static_cast<int>(faces_.size()); }
    
    /**
     * @brief Get all face values
     */
    const std::vector<T>& faces() const { return faces_; }
    
    /**
     * @brief Get minimum possible value
     */
    T min_value() const { return *std::min_element(faces_.begin(), faces_.end()); }
    
    /**
     * @brief Get maximum possible value
     */
    T max_value() const { return *std::max_element(faces_.begin(), faces_.end()); }
    
    /**
     * @brief Get expected value (average)
     */
    double expected_value() const;
    
    /**
     * @brief Check if this is a standard numeric die
     */
    bool is_standard() const;
    
    /**
     * @brief Get string representation
     */
    std::string to_string() const;
    
    // ==================== Static Factory Methods ====================
    
    static Die d4() { return Die(4); }
    static Die d6() { return Die(6); }
    static Die d8() { return Die(8); }
    static Die d10() { return Die(10); }
    static Die d12() { return Die(12); }
    static Die d20() { return Die(20); }
    static Die d100() { return Die(100); }
    
    /**
     * @brief Create a coin (2-sided die with 0 and 1)
     */
    static Die coin() { return Die(std::vector<T>{0, 1}); }
    
    /**
     * @brief Create a Fudge/FATE die (-1, 0, +1)
     */
    static Die fudge() { return Die(std::vector<T>{-1, 0, 1}); }

private:
    std::vector<T> faces_;
    std::vector<Modifier> modifiers_;
    std::shared_ptr<wip::utils::rng::RandomGenerator> rng_;
    
    void apply_modifiers(std::vector<SingleRollResult>& rolls, RollResult& result);
};

// ==================== DiceSet Class Template ====================

/**
 * @brief Collection of dice that can be rolled together
 */
template<typename T = int>
class DiceSet {
public:
    DiceSet() = default;
    
    /**
     * @brief Add a die to the set
     */
    void add_die(const Die<T>& die);
    
    /**
     * @brief Add multiple identical dice
     */
    void add_dice(const Die<T>& die, int count);
    
    /**
     * @brief Remove all dice from the set
     */
    void clear();
    
    /**
     * @brief Roll all dice in the set
     * @return Vector of results, one per die
     */
    std::vector<T> roll();
    
    /**
     * @brief Roll all dice with detailed results
     * @return Combined roll result
     */
    RollResult roll_detailed();
    
    /**
     * @brief Get total of rolling all dice
     * @return Sum of all dice rolls
     */
    T roll_total();
    
    /**
     * @brief Get number of dice in set
     */
    size_t size() const { return dice_.size(); }
    
    /**
     * @brief Check if set is empty
     */
    bool empty() const { return dice_.empty(); }
    
    /**
     * @brief Get string representation
     */
    std::string to_string() const;
    
    // ==================== Common Gaming Dice Sets ====================
    
    /**
     * @brief Create a standard D&D ability score set (4d6, drop lowest)
     */
    static DiceSet ability_score();

private:
    std::vector<Die<T>> dice_;
};

// ==================== Dice Expression Parser ====================

class DiceExpression {
public:
    static RollResult evaluate(const std::string& expression);
    static bool is_valid(const std::string& expression);
    static std::string help_text();

private:
    struct ParsedExpression {
        int count = 1;
        int sides = 6;
        std::vector<Modifier> modifiers;
        int constant = 0;
    };
    
    static ParsedExpression parse(const std::string& expression);
    static RollResult evaluate_parsed(const ParsedExpression& parsed);
};

// ==================== Gaming Utilities ====================

namespace gaming {

namespace dnd {
    int ability_score();
    int d20_advantage();
    int d20_disadvantage();
    int attack_roll(int modifier = 0);
    int damage_roll(const std::string& damage_dice);
    bool death_save();
    std::map<std::string, int> generate_character_stats();
}

namespace utils {
    int percentile();
    
    template<typename T>
    T random_table_entry(const std::map<int, T>& table);
    
    int dice_pool_successes(int pool_size, int die_size, int target_number);
    int exploding_dice_pool(int pool_size, int die_size, int explode_on);
}

} // namespace gaming

// ==================== Global Convenience Functions ====================

int d4();
int d6();
int d8();
int d10();
int d12();
int d20();
int d100();

int roll_dice(int count, int sides, int modifier = 0);
int roll(const std::string& notation);
RollResult roll_detailed(const std::string& notation);

// ==================== Template Implementations ====================

// Template implementations are included here to avoid linker issues
template<typename T>
Die<T>::Die(int sides, std::shared_ptr<wip::utils::rng::RandomGenerator> rng) 
    : rng_(rng) {
    if (sides <= 0) {
        throw std::invalid_argument("Die must have at least 1 side");
    }
    
    faces_.resize(sides);
    if constexpr (std::is_arithmetic_v<T>) {
        for (int i = 0; i < sides; ++i) {
            faces_[i] = static_cast<T>(i + 1);
        }
    } else {
        throw std::invalid_argument("Non-arithmetic types require explicit face values");
    }
    
    if (!rng_) {
        rng_ = std::make_shared<wip::utils::rng::RandomGenerator>(wip::utils::rng::global());
    }
}

template<typename T>
Die<T>::Die(const std::vector<T>& faces, std::shared_ptr<wip::utils::rng::RandomGenerator> rng)
    : faces_(faces), rng_(rng) {
    if (faces_.empty()) {
        throw std::invalid_argument("Die must have at least one face");
    }
    
    if (!rng_) {
        rng_ = std::make_shared<wip::utils::rng::RandomGenerator>(wip::utils::rng::global());
    }
}

template<typename T>
T Die<T>::roll() {
    int index = rng_->uniform_int(0, static_cast<int>(faces_.size()) - 1);
    return faces_[index];
}

template<typename T>
std::vector<T> Die<T>::roll(int count) {
    if (count < 0) {
        throw std::invalid_argument("Roll count cannot be negative");
    }
    
    std::vector<T> results;
    results.reserve(count);
    for (int i = 0; i < count; ++i) {
        results.push_back(roll());
    }
    return results;
}

template<typename T>
T Die<T>::roll(RollType type) {
    switch (type) {
        case RollType::Advantage:
            return advantage();
        case RollType::Disadvantage:
            return disadvantage();
        default:
            return roll();
    }
}

template<typename T>
void Die<T>::add_modifier(const Modifier& modifier) {
    modifiers_.push_back(modifier);
}

template<typename T>
void Die<T>::clear_modifiers() {
    modifiers_.clear();
}

template<typename T>
RollResult Die<T>::roll_detailed() {
    return roll_detailed(1);
}

template<typename T>
RollResult Die<T>::roll_detailed(int count) {
    RollResult result;
    result.individual_rolls.reserve(count);
    
    for (int i = 0; i < count; ++i) {
        T roll_value = roll();
        if constexpr (std::is_arithmetic_v<T>) {
            result.individual_rolls.emplace_back(static_cast<int>(roll_value));
        }
    }
    
    // Apply modifiers (this will calculate final_total)
    apply_modifiers(result.individual_rolls, result);
    
    // Generate breakdown string
    std::ostringstream breakdown;
    breakdown << count << "d" << sides();
    if (!modifiers_.empty()) {
        breakdown << " (with modifiers)";
    }
    breakdown << " = " << result.final_total;
    result.breakdown = breakdown.str();
    
    return result;
}

template<typename T>
T Die<T>::advantage() {
    T roll1 = roll();
    T roll2 = roll();
    return std::max(roll1, roll2);
}

template<typename T>
T Die<T>::disadvantage() {
    T roll1 = roll();
    T roll2 = roll();
    return std::min(roll1, roll2);
}

template<typename T>
T Die<T>::exploding(std::optional<T> explode_on) {
    if (!explode_on) {
        explode_on = max_value();
    }
    
    T total = roll();
    T current = total;
    
    // Prevent infinite loops
    int explosion_count = 0;
    const int max_explosions = 100;
    
    while (current == *explode_on && explosion_count < max_explosions) {
        current = roll();
        total = static_cast<T>(static_cast<int>(total) + static_cast<int>(current));
        ++explosion_count;
    }
    
    return total;
}

template<typename T>
T Die<T>::reroll_on(T reroll_on, int max_rerolls) {
    T result = roll();
    int rerolls = 0;
    
    while (result == reroll_on && rerolls < max_rerolls) {
        result = roll();
        ++rerolls;
    }
    
    return result;
}

template<typename T>
double Die<T>::expected_value() const {
    if constexpr (std::is_arithmetic_v<T>) {
        double sum = 0.0;
        for (const auto& face : faces_) {
            sum += static_cast<double>(face);
        }
        return sum / faces_.size();
    }
    return 0.0;  // Cannot calculate for non-arithmetic types
}

template<typename T>
bool Die<T>::is_standard() const {
    if constexpr (std::is_arithmetic_v<T>) {
        if (faces_.size() == 0) return false;
        
        // Check if faces are consecutive integers starting from 1
        for (size_t i = 0; i < faces_.size(); ++i) {
            if (static_cast<int>(faces_[i]) != static_cast<int>(i + 1)) {
                return false;
            }
        }
        return true;
    }
    return false;
}

template<typename T>
std::string Die<T>::to_string() const {
    if (is_standard()) {
        return "d" + std::to_string(faces_.size());
    } else {
        std::ostringstream oss;
        oss << "d{";
        for (size_t i = 0; i < faces_.size(); ++i) {
            if (i > 0) oss << ",";
            if constexpr (std::is_arithmetic_v<T>) {
                oss << faces_[i];
            } else {
                oss << "?";  // Cannot convert non-arithmetic to string easily
            }
        }
        oss << "}";
        return oss.str();
    }
}

template<typename T>
void Die<T>::apply_modifiers(std::vector<SingleRollResult>& rolls, RollResult& result) {
    // Initially, all rolls are kept
    for (const auto& roll : rolls) {
        if constexpr (std::is_arithmetic_v<T>) {
            result.kept_values.push_back(roll.value);
        }
    }
    
    // Apply modifiers in order
    for (const auto& modifier : modifiers_) {
        switch (modifier.type) {
            case ModifierType::KeepHighest:
                if (modifier.value < static_cast<int>(result.kept_values.size())) {
                    std::sort(result.kept_values.rbegin(), result.kept_values.rend());
                    result.dropped_values.insert(result.dropped_values.end(),
                        result.kept_values.begin() + modifier.value, result.kept_values.end());
                    result.kept_values.resize(modifier.value);
                }
                break;
                
            case ModifierType::KeepLowest:
                if (modifier.value < static_cast<int>(result.kept_values.size())) {
                    std::sort(result.kept_values.begin(), result.kept_values.end());
                    result.dropped_values.insert(result.dropped_values.end(),
                        result.kept_values.begin() + modifier.value, result.kept_values.end());
                    result.kept_values.resize(modifier.value);
                }
                break;
                
            case ModifierType::DropHighest:
                if (modifier.value < static_cast<int>(result.kept_values.size())) {
                    std::sort(result.kept_values.rbegin(), result.kept_values.rend());
                    result.dropped_values.insert(result.dropped_values.end(),
                        result.kept_values.begin(), result.kept_values.begin() + modifier.value);
                    result.kept_values.erase(result.kept_values.begin(), result.kept_values.begin() + modifier.value);
                }
                break;
                
            case ModifierType::DropLowest:
                if (modifier.value < static_cast<int>(result.kept_values.size())) {
                    std::sort(result.kept_values.begin(), result.kept_values.end());
                    result.dropped_values.insert(result.dropped_values.end(),
                        result.kept_values.begin(), result.kept_values.begin() + modifier.value);
                    result.kept_values.erase(result.kept_values.begin(), result.kept_values.begin() + modifier.value);
                }
                break;
                
            // For Add/Subtract/Multiply/Divide, we'll apply to the final total later
            case ModifierType::Add:
            case ModifierType::Subtract:
            case ModifierType::Multiply:
            case ModifierType::Divide:
                // These are handled after calculating the base total
                break;
                
            default:
                // Other modifiers would require more complex logic
                break;
        }
    }
    
    // Calculate base total from kept values
    int base_total = std::accumulate(result.kept_values.begin(), result.kept_values.end(), 0);
    
    // Apply arithmetic modifiers to the total
    result.final_total = base_total;
    for (const auto& modifier : modifiers_) {
        switch (modifier.type) {
            case ModifierType::Add:
                result.final_total += modifier.value;
                break;
                
            case ModifierType::Subtract:
                result.final_total -= modifier.value;
                break;
                
            case ModifierType::Multiply:
                result.final_total *= modifier.value;
                break;
                
            case ModifierType::Divide:
                if (modifier.value != 0) {
                    result.final_total /= modifier.value;
                }
                break;
                
            default:
                // Non-arithmetic modifiers already handled above
                break;
        }
    }
}

// DiceSet template implementations
template<typename T>
void DiceSet<T>::add_die(const Die<T>& die) {
    dice_.push_back(die);
}

template<typename T>
void DiceSet<T>::add_dice(const Die<T>& die, int count) {
    for (int i = 0; i < count; ++i) {
        dice_.push_back(die);
    }
}

template<typename T>
void DiceSet<T>::clear() {
    dice_.clear();
}

template<typename T>
std::vector<T> DiceSet<T>::roll() {
    std::vector<T> results;
    results.reserve(dice_.size());
    for (auto& die : dice_) {
        results.push_back(die.roll());
    }
    return results;
}

template<typename T>
RollResult DiceSet<T>::roll_detailed() {
    RollResult combined_result;
    
    // Roll all dice and collect individual results
    for (auto& die : dice_) {
        T roll_value = die.roll();
        if constexpr (std::is_arithmetic_v<T>) {
            combined_result.individual_rolls.emplace_back(static_cast<int>(roll_value));
            combined_result.kept_values.push_back(static_cast<int>(roll_value));
        }
    }
    
    // For ability score sets (4 dice), apply drop-lowest logic
    if (dice_.size() == 4) {
        std::sort(combined_result.kept_values.begin(), combined_result.kept_values.end());
        combined_result.dropped_values.push_back(combined_result.kept_values.front());
        combined_result.kept_values.erase(combined_result.kept_values.begin());
    }
    
    // Calculate final total
    combined_result.final_total = std::accumulate(combined_result.kept_values.begin(), 
                                                 combined_result.kept_values.end(), 0);
    
    // Generate breakdown
    std::ostringstream breakdown;
    breakdown << dice_.size() << " dice total = " << combined_result.final_total;
    combined_result.breakdown = breakdown.str();
    
    return combined_result;
}

template<typename T>
T DiceSet<T>::roll_total() {
    T total{};
    for (auto& die : dice_) {
        if constexpr (std::is_arithmetic_v<T>) {
            total = static_cast<T>(static_cast<int>(total) + static_cast<int>(die.roll()));
        }
    }
    return total;
}

template<typename T>
std::string DiceSet<T>::to_string() const {
    if (dice_.empty()) {
        return "Empty DiceSet";
    }
    
    std::ostringstream oss;
    oss << "DiceSet[" << dice_.size() << " dice]";
    return oss.str();
}

template<typename T>
DiceSet<T> DiceSet<T>::ability_score() {
    DiceSet<T> set;
    // For ability scores, we want 4 regular d6 dice, then apply drop-lowest as a set
    Die<T> d6(6);
    set.add_dice(d6, 4);
    return set;
}

// Gaming utilities template implementations
namespace gaming {
namespace utils {

template<typename T>
T random_table_entry(const std::map<int, T>& table) {
    if (table.empty()) {
        throw std::invalid_argument("Cannot select from empty table");
    }
    
    auto& rng = wip::utils::rng::global();
    int max_weight = table.rbegin()->first;
    int roll = rng.uniform_int(1, max_weight);
    
    auto it = table.lower_bound(roll);
    if (it == table.end()) {
        --it;  // Take the last entry if roll is beyond all weights
    }
    
    return it->second;
}

} // namespace utils
} // namespace gaming

} // namespace dice
} // namespace game
} // namespace wip