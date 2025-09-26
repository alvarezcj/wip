#include "dice.h"
#include <regex>
#include <cctype>

namespace wip {
namespace game {
namespace dice {

// ==================== RollResult Implementation ====================

std::string RollResult::to_string() const {
    std::stringstream ss;
    ss << "Expression: " << expression << "\n";
    ss << "Individual rolls: [";
    for (size_t i = 0; i < individual_rolls.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << individual_rolls[i].value;
        if (individual_rolls[i].was_dropped) ss << "(dropped)";
        if (individual_rolls[i].was_rerolled) ss << "(rerolled)";
        if (individual_rolls[i].was_exploded) ss << "(exploded)";
    }
    ss << "]\n";
    ss << "Kept values: [";
    for (size_t i = 0; i < kept_values.size(); ++i) {
        if (i > 0) ss << ", ";
        ss << kept_values[i];
    }
    ss << "]\n";
    ss << "Total: " << final_total;
    return ss.str();
}

// ==================== DiceExpression Implementation ====================

RollResult DiceExpression::evaluate(const std::string& expression) {
    try {
        ParsedExpression parsed = parse(expression);
        return evaluate_parsed(parsed);
    } catch (const std::exception& e) {
        RollResult result;
        result.expression = expression;
        result.breakdown = "Error: " + std::string(e.what());
        return result;
    }
}

bool DiceExpression::is_valid(const std::string& expression) {
    try {
        parse(expression);
        return true;
    } catch (...) {
        return false;
    }
}

std::string DiceExpression::help_text() {
    return R"(Dice Notation Help:
Basic Format: [count]d[sides][modifiers][+/-constant]

Examples:
  d20         - Single 20-sided die
  3d6         - Three 6-sided dice
  2d8+3       - Two 8-sided dice plus 3
  4d6kh3      - Four 6-sided dice, keep highest 3
  2d20adv     - Two d20s with advantage (keep higher)
  1d20dis     - One d20 with disadvantage (roll twice, keep lower)

Modifiers:
  kh[n]  - Keep highest n dice
  kl[n]  - Keep lowest n dice  
  dh[n]  - Drop highest n dice
  dl[n]  - Drop lowest n dice
  r[n]   - Reroll on value n
  e[n]   - Exploding dice on value n
  adv    - Advantage (roll twice, keep higher)
  dis    - Disadvantage (roll twice, keep lower)
)";
}

DiceExpression::ParsedExpression DiceExpression::parse(const std::string& expression) {
    ParsedExpression result;
    
    // Remove spaces and convert to lowercase
    std::string clean_expr;
    for (char c : expression) {
        if (!std::isspace(c)) {
            clean_expr += std::tolower(c);
        }
    }
    
    // Basic regex for dice notation
    std::regex dice_regex(R"((\d*)d(\d+)((?:kh|kl|dh|dl|r|e)\d+|adv|dis)*([+\-]\d+)?)");
    std::smatch match;
    
    if (std::regex_match(clean_expr, match, dice_regex)) {
        // Parse count
        if (match[1].length() > 0) {
            result.count = std::stoi(match[1]);
        }
        
        // Parse sides
        result.sides = std::stoi(match[2]);
        
        // Parse modifiers
        std::string modifiers_str = match[3];
        if (!modifiers_str.empty()) {
            std::regex modifier_regex(R"(kh(\d+)|kl(\d+)|dh(\d+)|dl(\d+)|r(\d+)|e(\d+)|adv|dis)");
            std::sregex_iterator iter(modifiers_str.begin(), modifiers_str.end(), modifier_regex);
            std::sregex_iterator end;
            
            while (iter != end) {
                std::smatch mod_match = *iter;
                if (mod_match[0] == "adv") {
                    // Handle advantage separately
                } else if (mod_match[0] == "dis") {
                    // Handle disadvantage separately  
                } else if (mod_match[1].matched) {
                    result.modifiers.emplace_back(ModifierType::KeepHighest, std::stoi(mod_match[1]));
                } else if (mod_match[2].matched) {
                    result.modifiers.emplace_back(ModifierType::KeepLowest, std::stoi(mod_match[2]));
                } else if (mod_match[3].matched) {
                    result.modifiers.emplace_back(ModifierType::DropHighest, std::stoi(mod_match[3]));
                } else if (mod_match[4].matched) {
                    result.modifiers.emplace_back(ModifierType::DropLowest, std::stoi(mod_match[4]));
                } else if (mod_match[5].matched) {
                    result.modifiers.emplace_back(ModifierType::RerollOn, std::stoi(mod_match[5]));
                } else if (mod_match[6].matched) {
                    result.modifiers.emplace_back(ModifierType::ExplodingOn, std::stoi(mod_match[6]));
                }
                ++iter;
            }
        }
        
        // Parse constant
        if (match[4].length() > 0) {
            result.constant = std::stoi(match[4]);
        }
    } else {
        throw std::invalid_argument("Invalid dice expression: " + expression);
    }
    
    return result;
}

RollResult DiceExpression::evaluate_parsed(const ParsedExpression& parsed) {
    Die<int> die(parsed.sides);
    
    // Add modifiers to die
    for (const auto& mod : parsed.modifiers) {
        die.add_modifier(mod);
    }
    
    // Roll the dice
    RollResult result = die.roll_detailed(parsed.count);
    
    // Apply constant modifier
    if (parsed.constant != 0) {
        result.final_total += parsed.constant;
        result.breakdown += (parsed.constant >= 0 ? " + " : " - ") + std::to_string(std::abs(parsed.constant));
    }
    
    return result;
}

// ==================== Gaming Utilities Implementation ====================

namespace gaming {
namespace dnd {

int ability_score() {
    Die<int> d6 = Die<int>::d6();
    d6.add_modifier(Modifier(ModifierType::DropLowest, 1, "drop lowest"));
    auto result = d6.roll_detailed(4);
    return result.total();
}

int d20_advantage() {
    return Die<int>::d20().advantage();
}

int d20_disadvantage() {
    return Die<int>::d20().disadvantage();
}

int attack_roll(int modifier) {
    return Die<int>::d20().roll() + modifier;
}

int damage_roll(const std::string& damage_dice) {
    return DiceExpression::evaluate(damage_dice).total();
}

bool death_save() {
    return Die<int>::d20().roll() >= 10;
}

std::map<std::string, int> generate_character_stats() {
    std::map<std::string, int> stats;
    std::vector<std::string> abilities = {"Strength", "Dexterity", "Constitution", "Intelligence", "Wisdom", "Charisma"};
    
    for (const auto& ability : abilities) {
        stats[ability] = ability_score();
    }
    
    return stats;
}

} // namespace dnd

namespace utils {

int percentile() {
    return Die<int>::d100().roll();
}

int dice_pool_successes(int pool_size, int die_size, int target_number) {
    int successes = 0;
    Die<int> die(die_size);
    
    for (int i = 0; i < pool_size; ++i) {
        if (die.roll() >= target_number) {
            successes++;
        }
    }
    
    return successes;
}

int exploding_dice_pool(int pool_size, int die_size, int explode_on) {
    int total = 0;
    Die<int> die(die_size);
    
    for (int i = 0; i < pool_size; ++i) {
        total += die.exploding(explode_on);
    }
    
    return total;
}

} // namespace utils
} // namespace gaming

// ==================== Global Convenience Functions ====================

int d4() { return Die<int>::d4().roll(); }
int d6() { return Die<int>::d6().roll(); }
int d8() { return Die<int>::d8().roll(); }
int d10() { return Die<int>::d10().roll(); }
int d12() { return Die<int>::d12().roll(); }
int d20() { return Die<int>::d20().roll(); }
int d100() { return Die<int>::d100().roll(); }

int roll_dice(int count, int sides, int modifier) {
    Die<int> die(sides);
    int total = 0;
    for (int i = 0; i < count; ++i) {
        total += die.roll();
    }
    return total + modifier;
}

int roll(const std::string& notation) {
    return DiceExpression::evaluate(notation).total();
}

RollResult roll_detailed(const std::string& notation) {
    return DiceExpression::evaluate(notation);
}

} // namespace dice
} // namespace game
} // namespace wip