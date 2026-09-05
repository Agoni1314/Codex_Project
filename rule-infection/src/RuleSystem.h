#pragma once

#include "CreatureFaction.h"
#include "Rule.h"

#include <cstddef>
#include <optional>
#include <vector>

class Creature;
class CreatureManager;
class FoodManager;

class RuleSystem
{
public:
    RuleSystem();
    RuleSystem(std::vector<Rule> redRules, std::vector<Rule> blueRules);

    RuleAction Evaluate(
        const Creature& creature,
        const CreatureManager& creatures,
        const FoodManager* foods = nullptr) const;
    bool EvaluateRule(
        const Rule& rule,
        const Creature& creature,
        const CreatureManager& creatures,
        const FoodManager* foods = nullptr) const;
    bool CheckCondition(
        const Condition& condition,
        const Creature& creature,
        const CreatureManager& creatures,
        const FoodManager* foods = nullptr) const;

    bool SetRuleAction(
        CreatureFaction faction, ConditionType condition, RuleAction action);
    bool SetRuleActionByIndex(
        CreatureFaction faction, std::size_t ruleIndex, RuleAction action);
    RuleChangeResult ReplaceRule(
        CreatureFaction faction, std::size_t ruleIndex, const Rule& rule);
    RuleChangeResult ValidateRule(const Rule& rule) const;
    RuleChangeResult AddDefaultRule(
        CreatureFaction faction, RuleId* addedRuleId = nullptr);
    RuleChangeResult DeleteRule(CreatureFaction faction, RuleId ruleId);
    RuleChangeResult MoveRuleUp(CreatureFaction faction, RuleId ruleId);
    RuleChangeResult MoveRuleDown(CreatureFaction faction, RuleId ruleId);
    std::optional<std::size_t> FindRuleIndex(
        CreatureFaction faction, RuleId ruleId) const;
    const Rule* GetRuleById(CreatureFaction faction, RuleId ruleId) const;
    RuleAction GetRuleAction(
        CreatureFaction faction, ConditionType condition) const;
    const std::vector<Rule>& GetRules(CreatureFaction faction) const;

private:
    std::vector<Rule>& GetRulesMutable(CreatureFaction faction);
    RuleId AllocateRuleId();
    void AssignInitialRuleIds();

    std::vector<Rule> redRules_;
    std::vector<Rule> blueRules_;
    RuleId nextRuleId_{1};
};
