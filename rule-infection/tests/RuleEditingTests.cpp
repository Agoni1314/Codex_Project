#include "CreatureManager.h"
#include "FoodManager.h"
#include "Map.h"
#include "RuleEconomy.h"
#include "RuleSystem.h"

#include <cassert>

int main()
{
    Map map{20, 15, 32, 80, 60};

    {
        RuleSystem rules;
        RuleEconomy economy;
        assert(economy.TryChangeConditionType(
                   rules, CreatureFaction::Blue, 2, 0, ConditionType::Hungry)
               == RuleChangeResult::Success);
        assert(economy.GetPoints() == 80);
        assert(rules.GetRules(CreatureFaction::Blue)[2].conditions[0].type
               == ConditionType::Hungry);

        CreatureManager creatures;
        const int redId = creatures.CreateCreature(
            5, 5, CreatureFaction::Red, map);
        const int blueId = creatures.CreateCreature(
            8, 5, CreatureFaction::Blue, map);
        assert(redId != -1 && blueId != -1);
        // 已有 Blue 不饥饿，因此修改后的规则立即不再因看见敌人而 Flee。
        assert(rules.Evaluate(
                   *creatures.GetCreatureById(blueId), creatures)
               == RuleAction::Stay);

        assert(economy.Spend(80));
        assert(economy.TryChangeConditionType(
                   rules,
                   CreatureFaction::Blue,
                   2,
                   0,
                   ConditionType::EnemyVisible)
               == RuleChangeResult::NotEnoughPoints);
        assert(economy.GetPoints() == 0);
        assert(rules.GetRules(CreatureFaction::Blue)[2].conditions[0].type
               == ConditionType::Hungry);
    }

    {
        RuleSystem rules;
        RuleEconomy economy;
        assert(economy.TryAddCondition(
                   rules,
                   CreatureFaction::Red,
                   0,
                   ConditionType::EnemyVisible)
               == RuleChangeResult::Success);
        assert(economy.GetPoints() == 70);
        assert(rules.GetRules(CreatureFaction::Red)[0].conditions.size() == 2);
        assert(economy.TryAddCondition(
                   rules,
                   CreatureFaction::Red,
                   0,
                   ConditionType::Hungry)
               == RuleChangeResult::Success);
        assert(rules.GetRules(CreatureFaction::Red)[0].conditions.size() == 3);
        const int pointsAtMaximum = economy.GetPoints();
        assert(economy.TryAddCondition(
                   rules,
                   CreatureFaction::Red,
                   0,
                   ConditionType::FoodVisible)
               == RuleChangeResult::MaxConditionsReached);
        assert(economy.GetPoints() == pointsAtMaximum);

        assert(economy.TryRemoveCondition(
                   rules, CreatureFaction::Red, 0, 2)
               == RuleChangeResult::Success);
        assert(rules.GetRules(CreatureFaction::Red)[0].conditions.size() == 2);
        assert(economy.TryRemoveCondition(
                   rules, CreatureFaction::Red, 0, 1)
               == RuleChangeResult::Success);
        assert(rules.GetRules(CreatureFaction::Red)[0].conditions.size() == 1);
        const int pointsAtMinimum = economy.GetPoints();
        assert(economy.TryRemoveCondition(
                   rules, CreatureFaction::Red, 0, 0)
               == RuleChangeResult::MinConditionsReached);
        assert(economy.GetPoints() == pointsAtMinimum);
    }

    {
        RuleSystem rules;
        RuleEconomy economy;
        const int initialPoints = economy.GetPoints();
        assert(economy.TryChangeConditionType(
                   rules,
                   CreatureFaction::Red,
                   1,
                   0,
                   ConditionType::FoodVisible)
               == RuleChangeResult::DuplicateCondition);
        assert(economy.GetPoints() == initialPoints);
        assert(economy.TryAddCondition(
                   rules,
                   CreatureFaction::Red,
                   1,
                   ConditionType::Hungry)
               == RuleChangeResult::DuplicateCondition);
        assert(economy.GetPoints() == initialPoints);
    }

    {
        RuleSystem rules;
        RuleEconomy economy;
        FoodManager foods;
        CreatureManager creatures;
        const int redId = creatures.CreateCreature(
            2, 2, CreatureFaction::Red, map);
        assert(foods.CreateFood(3, 2, map, creatures) != -1);
        const Creature* red = creatures.GetCreatureById(redId);
        // Hungry AND FoodVisible：初始不饥饿，所以不匹配。
        assert(!rules.EvaluateRule(
            rules.GetRules(CreatureFaction::Red)[1],
            *red,
            creatures,
            &foods));
        assert(economy.TryChangeConditionLogic(
                   rules, CreatureFaction::Red, 1, ConditionLogic::Any)
               == RuleChangeResult::Success);
        assert(economy.GetPoints() == 85);
        assert(rules.EvaluateRule(
            rules.GetRules(CreatureFaction::Red)[1],
            *red,
            creatures,
            &foods));
        assert(economy.TryChangeConditionLogic(
                   rules, CreatureFaction::Red, 1, ConditionLogic::All)
               == RuleChangeResult::Success);
        assert(!rules.EvaluateRule(
            rules.GetRules(CreatureFaction::Red)[1],
            *red,
            creatures,
            &foods));
    }

    {
        RuleSystem rules;
        RuleEconomy economy;
        assert(economy.TryChangeConditionType(
                   rules,
                   CreatureFaction::Red,
                   2,
                   0,
                   ConditionType::HpBelowPercent)
               == RuleChangeResult::Success);
        assert(rules.GetRules(CreatureFaction::Red)[2].conditions[0].value
               == 30.0F);
        assert(economy.TryChangeConditionValue(
                   rules, CreatureFaction::Red, 2, 0, 35.0F)
               == RuleChangeResult::Success);
        assert(economy.GetPoints() == 75);
        assert(economy.TryChangeConditionValue(
                   rules, CreatureFaction::Red, 2, 0, 30.0F)
               == RuleChangeResult::Success);
        const int pointsBeforeInvalidValue = economy.GetPoints();
        assert(economy.TryChangeConditionValue(
                   rules, CreatureFaction::Red, 2, 0, 0.0F)
               == RuleChangeResult::InvalidValue);
        assert(economy.TryChangeConditionValue(
                   rules, CreatureFaction::Red, 2, 0, 100.0F)
               == RuleChangeResult::InvalidValue);
        assert(economy.GetPoints() == pointsBeforeInvalidValue);
        assert(rules.GetRules(CreatureFaction::Red)[2].conditions[0].value
               == 30.0F);
    }

    {
        RuleSystem rules;
        RuleEconomy economy;
        CreatureManager creatures;
        const int redId = creatures.CreateCreature(
            5, 5, CreatureFaction::Red, map);
        const int blueId = creatures.CreateCreature(
            8, 5, CreatureFaction::Blue, map);
        Creature* blue = creatures.GetCreatureByIdMutable(blueId);
        assert(redId != -1 && blue != nullptr);

        // 把 Blue 的食物规则改成 HP < 30 AND EnemyVisible -> Flee。
        assert(economy.TryChangeConditionType(
                   rules,
                   CreatureFaction::Blue,
                   1,
                   0,
                   ConditionType::HpBelowPercent)
               == RuleChangeResult::Success);
        assert(economy.TryChangeConditionType(
                   rules,
                   CreatureFaction::Blue,
                   1,
                   1,
                   ConditionType::EnemyVisible)
               == RuleChangeResult::Success);
        assert(economy.TryChangeRule(
                   rules, CreatureFaction::Blue, 1, RuleAction::Flee)
               == RuleChangeResult::Success);
        const Rule& edited = rules.GetRules(CreatureFaction::Blue)[1];
        assert(!rules.EvaluateRule(edited, *blue, creatures));
        blue->TakeDamage(71);
        assert(rules.EvaluateRule(edited, *blue, creatures));
    }

    {
        RuleSystem rules;
        RuleEconomy economy;
        assert(economy.TryChangeRule(
                   rules, CreatureFaction::Red, 2, RuleAction::Stay)
               == RuleChangeResult::Success);
        assert(rules.GetRules(CreatureFaction::Red)[2].action
               == RuleAction::Stay);
    }

    return 0;
}
