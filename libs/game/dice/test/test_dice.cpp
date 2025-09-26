#include <gtest/gtest.h>
#include "dice.h"
#include <set>
#include <map>
#include <algorithm>
#include <cmath>

using namespace wip::game::dice;

class DiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Use seeded RNG for reproducible tests
        auto rng = std::make_shared<wip::utils::rng::RandomGenerator>(12345);
        test_die = std::make_unique<Die<int>>(6, rng);
    }
    
    std::unique_ptr<Die<int>> test_die;
};

// ==================== Basic Die Tests ====================

TEST_F(DiceTest, StandardDieCreation) {
    Die<int> d6(6);
    EXPECT_EQ(d6.sides(), 6);
    EXPECT_EQ(d6.min_value(), 1);
    EXPECT_EQ(d6.max_value(), 6);
    EXPECT_TRUE(d6.is_standard());
}

TEST_F(DiceTest, CustomDieCreation) {
    std::vector<int> faces = {2, 4, 6, 8, 10, 12};
    Die<int> custom_die(faces);
    
    EXPECT_EQ(custom_die.sides(), 6);
    EXPECT_EQ(custom_die.min_value(), 2);
    EXPECT_EQ(custom_die.max_value(), 12);
    EXPECT_FALSE(custom_die.is_standard());
}

TEST_F(DiceTest, InvalidDieCreation) {
    EXPECT_THROW(Die<int>(0), std::invalid_argument);
    EXPECT_THROW(Die<int>(-5), std::invalid_argument);
}

TEST_F(DiceTest, StaticFactoryMethods) {
    EXPECT_EQ(Die<int>::d4().sides(), 4);
    EXPECT_EQ(Die<int>::d6().sides(), 6);
    EXPECT_EQ(Die<int>::d8().sides(), 8);
    EXPECT_EQ(Die<int>::d10().sides(), 10);
    EXPECT_EQ(Die<int>::d12().sides(), 12);
    EXPECT_EQ(Die<int>::d20().sides(), 20);
    EXPECT_EQ(Die<int>::d100().sides(), 100);
    
    auto coin = Die<int>::coin();
    EXPECT_EQ(coin.min_value(), 0);
    EXPECT_EQ(coin.max_value(), 1);
    
    auto fudge = Die<int>::fudge();
    EXPECT_EQ(fudge.min_value(), -1);
    EXPECT_EQ(fudge.max_value(), 1);
}

// ==================== Basic Rolling Tests ====================

TEST_F(DiceTest, SingleRoll) {
    for (int i = 0; i < 100; ++i) {
        int result = test_die->roll();
        EXPECT_GE(result, 1);
        EXPECT_LE(result, 6);
    }
}

TEST_F(DiceTest, MultipleRolls) {
    auto results = test_die->roll(10);
    EXPECT_EQ(results.size(), 10);
    
    for (int result : results) {
        EXPECT_GE(result, 1);
        EXPECT_LE(result, 6);
    }
}

TEST_F(DiceTest, RollDistribution) {
    std::map<int, int> counts;
    const int trials = 6000;
    
    for (int i = 0; i < trials; ++i) {
        int result = test_die->roll();
        counts[result]++;
    }
    
    // Each face should appear roughly 1000 times (1/6 of trials)
    for (int face = 1; face <= 6; ++face) {
        EXPECT_GT(counts[face], 800) << "Face " << face << " appeared " << counts[face] << " times";
        EXPECT_LT(counts[face], 1200) << "Face " << face << " appeared " << counts[face] << " times";
    }
}

TEST_F(DiceTest, NegativeRollCount) {
    EXPECT_THROW(test_die->roll(-1), std::invalid_argument);
}

// ==================== Advantage/Disadvantage Tests ====================

TEST_F(DiceTest, Advantage) {
    // Test that advantage tends to give higher results
    int advantage_total = 0;
    int normal_total = 0;
    const int trials = 1000;
    
    auto rng1 = std::make_shared<wip::utils::rng::RandomGenerator>(123);
    auto rng2 = std::make_shared<wip::utils::rng::RandomGenerator>(123);
    
    Die<int> die1(20, rng1);
    Die<int> die2(20, rng2);
    
    for (int i = 0; i < trials; ++i) {
        advantage_total += die1.advantage();
        normal_total += die2.roll();
    }
    
    double advantage_avg = static_cast<double>(advantage_total) / trials;
    double normal_avg = static_cast<double>(normal_total) / trials;
    
    EXPECT_GT(advantage_avg, normal_avg);
    EXPECT_GT(advantage_avg, 12.0); // Should be well above 10.5 (normal average)
}

TEST_F(DiceTest, Disadvantage) {
    // Test that disadvantage tends to give lower results
    int disadvantage_total = 0;
    int normal_total = 0;
    const int trials = 1000;
    
    auto rng1 = std::make_shared<wip::utils::rng::RandomGenerator>(456);
    auto rng2 = std::make_shared<wip::utils::rng::RandomGenerator>(456);
    
    Die<int> die1(20, rng1);
    Die<int> die2(20, rng2);
    
    for (int i = 0; i < trials; ++i) {
        disadvantage_total += die1.disadvantage();
        normal_total += die2.roll();
    }
    
    double disadvantage_avg = static_cast<double>(disadvantage_total) / trials;
    double normal_avg = static_cast<double>(normal_total) / trials;
    
    EXPECT_LT(disadvantage_avg, normal_avg);
    EXPECT_LT(disadvantage_avg, 9.0); // Should be well below 10.5 (normal average)
}

TEST_F(DiceTest, RollType) {
    Die<int> d20 = Die<int>::d20();
    
    // Test that different roll types return valid values
    for (int i = 0; i < 10; ++i) {
        int normal = d20.roll(RollType::Normal);
        int adv = d20.roll(RollType::Advantage);
        int dis = d20.roll(RollType::Disadvantage);
        
        EXPECT_GE(normal, 1);
        EXPECT_LE(normal, 20);
        EXPECT_GE(adv, 1);
        EXPECT_LE(adv, 20);
        EXPECT_GE(dis, 1);
        EXPECT_LE(dis, 20);
    }
}

// ==================== Special Rolling Tests ====================

TEST_F(DiceTest, ExplodingDice) {
    Die<int> d6 = Die<int>::d6();
    
    // Test exploding on 6 (should often be > 6)
    int high_results = 0;
    const int trials = 1000;
    
    for (int i = 0; i < trials; ++i) {
        int result = d6.exploding(6);
        if (result > 6) {
            high_results++;
        }
        EXPECT_GE(result, 1); // Should never be less than 1
    }
    
    // Should get some high results from explosions
    EXPECT_GT(high_results, 50);
}

TEST_F(DiceTest, RerollOn) {
    Die<int> d6 = Die<int>::d6();
    
    // Reroll on 1 - should reduce frequency of 1s
    std::map<int, int> counts;
    const int trials = 3000;
    
    for (int i = 0; i < trials; ++i) {
        int result = d6.reroll_on(1, 5);
        counts[result]++;
        EXPECT_GE(result, 1);
        EXPECT_LE(result, 6);
    }
    
    // Should have fewer 1s than other numbers
    EXPECT_LT(counts[1], counts[2]);
    EXPECT_LT(counts[1], counts[3]);
}

// ==================== Modifier Tests ====================

TEST_F(DiceTest, BasicModifiers) {
    Die<int> d6 = Die<int>::d6();
    
    // Test add modifier
    d6.add_modifier(Modifier(ModifierType::Add, 5));
    auto result = d6.roll_detailed();
    EXPECT_EQ(result.total(), result.kept_values[0] + 5);
    
    // Clear and test subtract
    d6.clear_modifiers();
    d6.add_modifier(Modifier(ModifierType::Subtract, 2));
    result = d6.roll_detailed();
    EXPECT_EQ(result.total(), result.kept_values[0] - 2);
}

TEST_F(DiceTest, KeepHighestModifier) {
    Die<int> d6 = Die<int>::d6();
    d6.add_modifier(Modifier(ModifierType::KeepHighest, 2));
    
    auto result = d6.roll_detailed(4);
    EXPECT_EQ(result.kept_values.size(), 2);
    EXPECT_EQ(result.dropped_values.size(), 2);
    
    // Kept values should be higher than dropped values
    int min_kept = *std::min_element(result.kept_values.begin(), result.kept_values.end());
    int max_dropped = *std::max_element(result.dropped_values.begin(), result.dropped_values.end());
    EXPECT_GE(min_kept, max_dropped);
}

TEST_F(DiceTest, DropLowestModifier) {
    Die<int> d6 = Die<int>::d6();
    d6.add_modifier(Modifier(ModifierType::DropLowest, 1));
    
    auto result = d6.roll_detailed(4);
    EXPECT_EQ(result.kept_values.size(), 3);
    EXPECT_EQ(result.dropped_values.size(), 1);
}

// ==================== Detailed Roll Results ====================

TEST_F(DiceTest, DetailedRollResult) {
    auto result = test_die->roll_detailed(3);
    
    EXPECT_EQ(result.individual_rolls.size(), 3);
    EXPECT_EQ(result.kept_values.size(), 3);
    EXPECT_TRUE(result.dropped_values.empty());
    EXPECT_GT(result.total(), 2); // Minimum possible
    EXPECT_LT(result.total(), 19); // Maximum possible
    
    // Test string representation
    std::string result_str = result.to_string();
    EXPECT_FALSE(result_str.empty());
}

// ==================== DiceSet Tests ====================

TEST_F(DiceTest, DiceSetBasic) {
    DiceSet<int> set;
    EXPECT_TRUE(set.empty());
    EXPECT_EQ(set.size(), 0);
    
    set.add_die(Die<int>::d6());
    set.add_die(Die<int>::d8());
    EXPECT_EQ(set.size(), 2);
    EXPECT_FALSE(set.empty());
    
    auto results = set.roll();
    EXPECT_EQ(results.size(), 2);
    
    set.clear();
    EXPECT_TRUE(set.empty());
}

TEST_F(DiceTest, DiceSetMultiple) {
    DiceSet<int> set;
    set.add_dice(Die<int>::d6(), 3);
    EXPECT_EQ(set.size(), 3);
    
    int total = set.roll_total();
    EXPECT_GE(total, 3);
    EXPECT_LE(total, 18);
}

TEST_F(DiceTest, DiceSetDetailed) {
    DiceSet<int> set;
    set.add_dice(Die<int>::d6(), 2);
    
    auto result = set.roll_detailed();
    EXPECT_EQ(result.kept_values.size(), 2);
    EXPECT_GE(result.total(), 2);
    EXPECT_LE(result.total(), 12);
}

TEST_F(DiceTest, AbilityScore) {
    auto ability_set = DiceSet<int>::ability_score();
    EXPECT_EQ(ability_set.size(), 4);
    
    // Roll several ability scores and check they're in reasonable range
    for (int i = 0; i < 10; ++i) {
        auto result = ability_set.roll_detailed();
        EXPECT_EQ(result.kept_values.size(), 3); // Should keep 3 out of 4
        EXPECT_EQ(result.dropped_values.size(), 1); // Should drop 1
        EXPECT_GE(result.total(), 3); // Minimum possible
        EXPECT_LE(result.total(), 18); // Maximum possible
    }
}

// ==================== Expression Parser Tests ====================

TEST_F(DiceTest, DiceExpressionBasic) {
    EXPECT_TRUE(DiceExpression::is_valid("d20"));
    EXPECT_TRUE(DiceExpression::is_valid("3d6"));
    EXPECT_TRUE(DiceExpression::is_valid("2d8+3"));
    EXPECT_TRUE(DiceExpression::is_valid("4d6kh3"));
    
    EXPECT_FALSE(DiceExpression::is_valid("invalid"));
    EXPECT_FALSE(DiceExpression::is_valid("d"));
    EXPECT_FALSE(DiceExpression::is_valid("3x6"));
}

TEST_F(DiceTest, DiceExpressionEvaluation) {
    auto result = DiceExpression::evaluate("2d6+2");
    EXPECT_EQ(result.kept_values.size(), 2);
    EXPECT_GE(result.total(), 4); // 2 + 2 minimum
    EXPECT_LE(result.total(), 14); // 12 + 2 maximum
}

TEST_F(DiceTest, DiceExpressionHelp) {
    std::string help = DiceExpression::help_text();
    EXPECT_FALSE(help.empty());
    EXPECT_TRUE(help.find("d20") != std::string::npos);
    EXPECT_TRUE(help.find("kh") != std::string::npos);
}

// ==================== Gaming Utilities Tests ====================

TEST_F(DiceTest, DNDMechanics) {
    // Test ability score generation
    for (int i = 0; i < 10; ++i) {
        int ability = gaming::dnd::ability_score();
        EXPECT_GE(ability, 3);
        EXPECT_LE(ability, 18);
    }
    
    // Test advantage/disadvantage
    int adv = gaming::dnd::d20_advantage();
    int dis = gaming::dnd::d20_disadvantage();
    EXPECT_GE(adv, 1);
    EXPECT_LE(adv, 20);
    EXPECT_GE(dis, 1);
    EXPECT_LE(dis, 20);
    
    // Test attack roll
    int attack = gaming::dnd::attack_roll(5);
    EXPECT_GE(attack, 6); // 1 + 5 minimum
    EXPECT_LE(attack, 25); // 20 + 5 maximum
    
    // Test death save
    bool save = gaming::dnd::death_save();
    EXPECT_TRUE(save == true || save == false);
    
    // Test character generation
    auto stats = gaming::dnd::generate_character_stats();
    EXPECT_EQ(stats.size(), 6);
    EXPECT_TRUE(stats.find("Strength") != stats.end());
    EXPECT_TRUE(stats.find("Charisma") != stats.end());
}

TEST_F(DiceTest, GenericGamingUtils) {
    // Test percentile
    for (int i = 0; i < 10; ++i) {
        int percent = gaming::utils::percentile();
        EXPECT_GE(percent, 1);
        EXPECT_LE(percent, 100);
    }
    
    // Test dice pool
    int successes = gaming::utils::dice_pool_successes(5, 10, 7);
    EXPECT_GE(successes, 0);
    EXPECT_LE(successes, 5);
    
    // Test exploding pool
    int total = gaming::utils::exploding_dice_pool(3, 6, 6);
    EXPECT_GE(total, 3); // Minimum possible
    
    // Test random table
    std::map<int, std::string> table = {
        {30, "Common"},
        {70, "Uncommon"},
        {90, "Rare"},
        {100, "Legendary"}
    };
    
    std::set<std::string> results;
    for (int i = 0; i < 50; ++i) {
        results.insert(gaming::utils::random_table_entry(table));
    }
    
    // Should see all entries eventually
    EXPECT_GT(results.size(), 1);
}

// ==================== Global Functions Tests ====================

TEST_F(DiceTest, GlobalFunctions) {
    // Test individual die functions
    for (int i = 0; i < 10; ++i) {
        EXPECT_GE(d4(), 1);
        EXPECT_LE(d4(), 4);
        
        EXPECT_GE(d6(), 1);
        EXPECT_LE(d6(), 6);
        
        EXPECT_GE(d8(), 1);
        EXPECT_LE(d8(), 8);
        
        EXPECT_GE(d10(), 1);
        EXPECT_LE(d10(), 10);
        
        EXPECT_GE(d12(), 1);
        EXPECT_LE(d12(), 12);
        
        EXPECT_GE(d20(), 1);
        EXPECT_LE(d20(), 20);
        
        EXPECT_GE(d100(), 1);
        EXPECT_LE(d100(), 100);
    }
    
    // Test roll_dice function
    int total = roll_dice(3, 6, 2);
    EXPECT_GE(total, 5); // 3 + 2 minimum
    EXPECT_LE(total, 20); // 18 + 2 maximum
    
    // Test notation functions
    int notation_result = roll("2d6+1");
    EXPECT_GE(notation_result, 3);
    EXPECT_LE(notation_result, 13);
    
    auto detailed = roll_detailed("1d20");
    EXPECT_EQ(detailed.kept_values.size(), 1);
    EXPECT_GE(detailed.total(), 1);
    EXPECT_LE(detailed.total(), 20);
}

// ==================== String Die Tests ====================

TEST(StringDiceTest, CustomStringDie) {
    std::vector<std::string> faces = {"Red", "Blue", "Green", "Yellow"};
    Die<std::string> color_die(faces);
    
    EXPECT_EQ(color_die.sides(), 4);
    EXPECT_FALSE(color_die.is_standard());
    
    // Roll several times and ensure all results are valid
    std::set<std::string> seen_colors;
    for (int i = 0; i < 20; ++i) {
        std::string result = color_die.roll();
        seen_colors.insert(result);
        EXPECT_TRUE(std::find(faces.begin(), faces.end(), result) != faces.end());
    }
    
    // Should eventually see all colors
    EXPECT_EQ(seen_colors.size(), 4);
}

// ==================== Edge Cases and Error Handling ====================

TEST_F(DiceTest, EdgeCases) {
    // Test zero roll count
    auto results = test_die->roll(0);
    EXPECT_TRUE(results.empty());
    
    // Test single-sided die
    Die<int> d1(1);
    EXPECT_EQ(d1.roll(), 1);
    EXPECT_EQ(d1.advantage(), 1);
    EXPECT_EQ(d1.disadvantage(), 1);
    
    // Test large die
    Die<int> d1000(1000);
    int result = d1000.roll();
    EXPECT_GE(result, 1);
    EXPECT_LE(result, 1000);
}

TEST_F(DiceTest, ExpectedValue) {
    Die<int> d6 = Die<int>::d6();
    EXPECT_DOUBLE_EQ(d6.expected_value(), 3.5);
    
    Die<int> d20 = Die<int>::d20();
    EXPECT_DOUBLE_EQ(d20.expected_value(), 10.5);
    
    // Custom die
    std::vector<int> faces = {10, 20, 30};
    Die<int> custom(faces);
    EXPECT_DOUBLE_EQ(custom.expected_value(), 20.0);
}

TEST_F(DiceTest, StringRepresentation) {
    EXPECT_EQ(Die<int>::d6().to_string(), "d6");
    EXPECT_EQ(Die<int>::d20().to_string(), "d20");
    
    std::vector<int> custom_faces = {2, 4, 6, 8};
    Die<int> custom(custom_faces);
    std::string custom_str = custom.to_string();
    EXPECT_TRUE(custom_str.find("d{") == 0);
    EXPECT_TRUE(custom_str.find("2,4,6,8") != std::string::npos);
}

// ==================== Performance Test ====================

TEST(DicePerformanceTest, DISABLED_LargeNumberOfRolls) {
    // Disabled by default - enable for performance testing
    Die<int> d20 = Die<int>::d20();
    const int trials = 1000000;
    
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < trials; ++i) {
        d20.roll();
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Rolled " << trials << " dice in " << duration.count() << " microseconds" << std::endl;
    
    // Should be able to roll at least 10,000 dice per second
    EXPECT_LT(duration.count(), 100000); // 100ms max for 1M rolls
}