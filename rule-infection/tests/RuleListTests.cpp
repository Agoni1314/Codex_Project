#include "CreatureManager.h"
#include "Map.h"
#include "RuleEconomy.h"
#include "RuleSystem.h"

#include <cassert>
#include <vector>

int main()
{
    {
        RuleSystem rules;
        const auto& red = rules.GetRules(CreatureFaction::Red);
        const auto& blue = rules.GetRules(CreatureFaction::Blue);
        for (const Rule& first : red)
        {
            assert(first.id != 0);
            for (const Rule& second : red)
            {
                assert(&first == &second || first.id != second.id);
            }
            for (const Rule& second : blue)
            {
                assert(first.id != second.id);
            }
        }

        const RuleId movingId = red[2].id;
        const RuleId stableId = red[0].id;
        assert(rules.MoveRuleUp(CreatureFaction::Red, movingId)
               == RuleChangeResult::Success);
        assert(rules.FindRuleIndex(CreatureFaction::Red, movingId).value() == 1);
        assert(rules.GetRuleById(CreatureFaction::Red, movingId)->id == movingId);
        assert(rules.MoveRuleDown(CreatureFaction::Red, movingId)
               == RuleChangeResult::Success);
        assert(rules.FindRuleIndex(CreatureFaction::Red, movingId).value() == 2);
        assert(rules.DeleteRule(CreatureFaction::Red, red[1].id)
               == RuleChangeResult::Success);
        assert(rules.GetRuleById(CreatureFaction::Red, stableId)->id == stableId);
    }

    {
        RuleSystem rules;
        const auto& red = rules.GetRules(CreatureFaction::Red);
        assert(rules.MoveRuleUp(CreatureFaction::Red, red.front().id)
               == RuleChangeResult::AlreadyHighestPriority);
        assert(rules.MoveRuleDown(CreatureFaction::Red, red.back().id)
               == RuleChangeResult::AlreadyLowestPriority);
    }

    {
        RuleSystem rules{
            std::vector<Rule>{
                {{{ConditionType::EnemyVisible}},
                 ConditionLogic::All,
                 RuleAction::Chase},
                {{{ConditionType::HpBelowPercent, 30.0F},
                  {ConditionType::EnemyVisible}},
                 ConditionLogic::All,
                 RuleAction::Flee}},
            std::vector<Rule>{
                {{{ConditionType::NoEnemyVisible}},
                 ConditionLogic::All,
                 RuleAction::Wander}}};
        Map map{20, 15, 32, 80, 60};
        CreatureManager creatures;
        const int redId = creatures.CreateCreature(
            5, 5, CreatureFaction::Red, map);
        assert(creatures.CreateCreature(8, 5, CreatureFaction::Blue, map) != -1);
        Creature* red = creatures.GetCreatureByIdMutable(redId);
        red->TakeDamage(81);
        assert(rules.Evaluate(*red, creatures) == RuleAction::Chase);
        const RuleId fleeRuleId = rules.GetRules(CreatureFaction::Red)[1].id;
        assert(rules.MoveRuleUp(CreatureFaction::Red, fleeRuleId)
               == RuleChangeResult::Success);
        assert(rules.Evaluate(*red, creatures) == RuleAction::Flee);
    }

    {
        RuleSystem rules;
        RuleId addedId = 0;
        assert(rules.AddDefaultRule(CreatureFaction::Red, &addedId)
               == RuleChangeResult::Success);
        const auto& red = rules.GetRules(CreatureFaction::Red);
        assert(red.size() == 5);
        assert(red.back().id == addedId);
        assert(red.back().conditions.size() == 1);
        assert(red.back().conditions[0].type == ConditionType::EnemyVisible);
        assert(red.back().logic == ConditionLogic::All);
        assert(red.back().action == RuleAction::Stay);
        while (rules.GetRules(CreatureFaction::Red).size()
               < MAX_RULES_PER_FACTION)
        {
            assert(rules.AddDefaultRule(CreatureFaction::Red)
                   == RuleChangeResult::Success);
        }
        assert(rules.AddDefaultRule(CreatureFaction::Red)
               == RuleChangeResult::MaxRulesReached);
        assert(rules.GetRules(CreatureFaction::Red).size()
               == MAX_RULES_PER_FACTION);
        RuleEconomy economy;
        assert(economy.TryAddRule(rules, CreatureFaction::Red)
               == RuleChangeResult::MaxRulesReached);
        assert(economy.GetPoints() == INITIAL_RULE_POINTS);
    }

    {
        RuleSystem rules{
            std::vector<Rule>{
                {{{ConditionType::EnemyVisible}},
                 ConditionLogic::All,
                 RuleAction::Stay}},
            std::vector<Rule>{
                {{{ConditionType::EnemyVisible}},
                 ConditionLogic::All,
                 RuleAction::Stay}}};
        const RuleId onlyRedId = rules.GetRules(CreatureFaction::Red)[0].id;
        assert(rules.DeleteRule(CreatureFaction::Red, onlyRedId)
               == RuleChangeResult::MinRulesReached);
        assert(rules.GetRules(CreatureFaction::Red).size() == 1);
        RuleEconomy economy;
        assert(economy.TryDeleteRule(
                   rules, CreatureFaction::Red, onlyRedId)
               == RuleChangeResult::MinRulesReached);
        assert(economy.GetPoints() == INITIAL_RULE_POINTS);
    }

    {
        RuleSystem rules;
        RuleEconomy economy;
        RuleId newId = 0;
        assert(economy.TryAddRule(rules, CreatureFaction::Blue, &newId)
               == RuleChangeResult::Success);
        assert(economy.GetPoints() == 60);
        assert(rules.GetRules(CreatureFaction::Blue).back().id == newId);
        assert(economy.TryMoveRuleUp(rules, CreatureFaction::Blue, newId)
               == RuleChangeResult::Success);
        assert(economy.GetPoints() == 50);
        assert(economy.TryMoveRuleDown(rules, CreatureFaction::Blue, newId)
               == RuleChangeResult::Success);
        assert(economy.GetPoints() == 40);
        assert(economy.TryDeleteRule(rules, CreatureFaction::Blue, newId)
               == RuleChangeResult::Success);
        assert(economy.GetPoints() == 25);
    }

    {
        RuleSystem rules;
        RuleEconomy economy;
        assert(economy.Spend(95));
        const std::vector<Rule> before = rules.GetRules(CreatureFaction::Red);
        assert(economy.TryAddRule(rules, CreatureFaction::Red)
               == RuleChangeResult::NotEnoughPoints);
        assert(economy.GetPoints() == 5);
        assert(rules.GetRules(CreatureFaction::Red).size() == before.size());
        assert(economy.TryMoveRuleDown(
                   rules, CreatureFaction::Red, before.front().id)
               == RuleChangeResult::NotEnoughPoints);
        assert(economy.GetPoints() == 5);
        assert(rules.GetRules(CreatureFaction::Red).front().id
               == before.front().id);
    }

    return 0;
}
