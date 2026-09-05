#pragma once

#include "CreatureFaction.h"
#include "Rule.h"

#include <cstddef>

class RuleSystem;

constexpr int INITIAL_RULE_POINTS = 100;
constexpr int MAX_RULE_POINTS = 100;
constexpr int RULE_POINT_REGEN_AMOUNT = 5;
constexpr float RULE_POINT_REGEN_INTERVAL = 10.0F;
constexpr int CHANGE_CONDITION_TYPE_COST = 20;
constexpr int ADD_CONDITION_COST = 30;
constexpr int REMOVE_CONDITION_COST = 10;
constexpr int CHANGE_LOGIC_COST = 15;
constexpr int CHANGE_VALUE_COST = 5;
constexpr int ADD_RULE_COST = 40;
constexpr int DELETE_RULE_COST = 15;
constexpr int MOVE_RULE_COST = 10;

constexpr int GetActionCost(RuleAction action)
{
    switch (action)
    {
    case RuleAction::Wander:
        return 10;
    case RuleAction::Chase:
        return 25;
    case RuleAction::Flee:
        return 20;
    case RuleAction::Attack:
        return 35;
    case RuleAction::Stay:
        return 10;
    case RuleAction::SeekFood:
        return 20;
    }

    return 0;
}

class RuleEconomy
{
public:
    int GetPoints() const;
    bool CanAfford(int cost) const;
    bool Spend(int cost);
    void AddPoints(int amount);
    void Update(float deltaTime, bool simulationPaused);
    void Reset();

    RuleChangeResult TryChangeRule(
        RuleSystem& rules,
        CreatureFaction faction,
        std::size_t ruleIndex,
        RuleAction newAction);
    RuleChangeResult TryChangeConditionType(
        RuleSystem& rules,
        CreatureFaction faction,
        std::size_t ruleIndex,
        std::size_t conditionIndex,
        ConditionType newType);
    RuleChangeResult TryAddCondition(
        RuleSystem& rules,
        CreatureFaction faction,
        std::size_t ruleIndex,
        ConditionType type);
    RuleChangeResult TryRemoveCondition(
        RuleSystem& rules,
        CreatureFaction faction,
        std::size_t ruleIndex,
        std::size_t conditionIndex);
    RuleChangeResult TryChangeConditionLogic(
        RuleSystem& rules,
        CreatureFaction faction,
        std::size_t ruleIndex,
        ConditionLogic logic);
    RuleChangeResult TryChangeConditionValue(
        RuleSystem& rules,
        CreatureFaction faction,
        std::size_t ruleIndex,
        std::size_t conditionIndex,
        float value);
    RuleChangeResult TryAddRule(
        RuleSystem& rules,
        CreatureFaction faction,
        RuleId* addedRuleId = nullptr);
    RuleChangeResult TryDeleteRule(
        RuleSystem& rules, CreatureFaction faction, RuleId ruleId);
    RuleChangeResult TryMoveRuleUp(
        RuleSystem& rules, CreatureFaction faction, RuleId ruleId);
    RuleChangeResult TryMoveRuleDown(
        RuleSystem& rules, CreatureFaction faction, RuleId ruleId);

private:
    RuleChangeResult TryCommitRule(
        RuleSystem& rules,
        CreatureFaction faction,
        std::size_t ruleIndex,
        const Rule& candidate,
        int cost);

    int points_{INITIAL_RULE_POINTS};
    float regenTimer_{0.0F};
};
