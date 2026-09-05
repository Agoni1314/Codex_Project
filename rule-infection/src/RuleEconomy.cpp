#include "RuleEconomy.h"

#include "RuleSystem.h"

#include <algorithm>

int RuleEconomy::GetPoints() const
{
    return points_;
}

bool RuleEconomy::CanAfford(int cost) const
{
    return cost >= 0 && points_ >= cost;
}

bool RuleEconomy::Spend(int cost)
{
    if (!CanAfford(cost))
    {
        return false;
    }

    points_ -= cost;
    return true;
}

void RuleEconomy::AddPoints(int amount)
{
    if (amount <= 0)
    {
        return;
    }

    points_ = std::min(MAX_RULE_POINTS, points_ + amount);
}

void RuleEconomy::Update(float deltaTime, bool simulationPaused)
{
    if (simulationPaused || deltaTime <= 0.0F)
    {
        return;
    }

    // 满 RP 时不积攒隐藏时间，消费后需要完整等待下一次恢复周期。
    if (points_ >= MAX_RULE_POINTS)
    {
        regenTimer_ = 0.0F;
        return;
    }

    regenTimer_ += deltaTime;
    while (regenTimer_ >= RULE_POINT_REGEN_INTERVAL
           && points_ < MAX_RULE_POINTS)
    {
        regenTimer_ -= RULE_POINT_REGEN_INTERVAL;
        AddPoints(RULE_POINT_REGEN_AMOUNT);
    }

    if (points_ >= MAX_RULE_POINTS)
    {
        regenTimer_ = 0.0F;
    }
}

void RuleEconomy::Reset()
{
    points_ = INITIAL_RULE_POINTS;
    regenTimer_ = 0.0F;
}

RuleChangeResult RuleEconomy::TryChangeRule(
    RuleSystem& rules,
    CreatureFaction faction,
    std::size_t ruleIndex,
    RuleAction newAction)
{
    const std::vector<Rule>& factionRules = rules.GetRules(faction);
    if (ruleIndex >= factionRules.size())
    {
        return RuleChangeResult::InvalidRule;
    }

    if (factionRules[ruleIndex].action == newAction)
    {
        return RuleChangeResult::AlreadyActive;
    }

    Rule candidate = factionRules[ruleIndex];
    candidate.action = newAction;
    return TryCommitRule(
        rules, faction, ruleIndex, candidate, GetActionCost(newAction));
}

RuleChangeResult RuleEconomy::TryChangeConditionType(
    RuleSystem& rules,
    CreatureFaction faction,
    std::size_t ruleIndex,
    std::size_t conditionIndex,
    ConditionType newType)
{
    const std::vector<Rule>& factionRules = rules.GetRules(faction);
    if (ruleIndex >= factionRules.size())
    {
        return RuleChangeResult::InvalidRule;
    }
    const Rule& current = factionRules[ruleIndex];
    if (conditionIndex >= current.conditions.size())
    {
        return RuleChangeResult::InvalidCondition;
    }
    if (current.conditions[conditionIndex].type == newType)
    {
        return RuleChangeResult::AlreadyActive;
    }

    Rule candidate = current;
    candidate.conditions[conditionIndex] = {
        newType,
        newType == ConditionType::HpBelowPercent
            ? DEFAULT_HP_BELOW_PERCENT
            : 0.0F};
    return TryCommitRule(
        rules, faction, ruleIndex, candidate, CHANGE_CONDITION_TYPE_COST);
}

RuleChangeResult RuleEconomy::TryAddCondition(
    RuleSystem& rules,
    CreatureFaction faction,
    std::size_t ruleIndex,
    ConditionType type)
{
    const std::vector<Rule>& factionRules = rules.GetRules(faction);
    if (ruleIndex >= factionRules.size())
    {
        return RuleChangeResult::InvalidRule;
    }
    if (factionRules[ruleIndex].conditions.size()
        >= MAX_CONDITIONS_PER_RULE)
    {
        return RuleChangeResult::MaxConditionsReached;
    }

    Rule candidate = factionRules[ruleIndex];
    candidate.conditions.push_back({
        type,
        type == ConditionType::HpBelowPercent
            ? DEFAULT_HP_BELOW_PERCENT
            : 0.0F});
    return TryCommitRule(
        rules, faction, ruleIndex, candidate, ADD_CONDITION_COST);
}

RuleChangeResult RuleEconomy::TryRemoveCondition(
    RuleSystem& rules,
    CreatureFaction faction,
    std::size_t ruleIndex,
    std::size_t conditionIndex)
{
    const std::vector<Rule>& factionRules = rules.GetRules(faction);
    if (ruleIndex >= factionRules.size())
    {
        return RuleChangeResult::InvalidRule;
    }
    const Rule& current = factionRules[ruleIndex];
    if (conditionIndex >= current.conditions.size())
    {
        return RuleChangeResult::InvalidCondition;
    }
    if (current.conditions.size() <= 1)
    {
        return RuleChangeResult::MinConditionsReached;
    }

    Rule candidate = current;
    candidate.conditions.erase(
        candidate.conditions.begin()
        + static_cast<std::ptrdiff_t>(conditionIndex));
    return TryCommitRule(
        rules, faction, ruleIndex, candidate, REMOVE_CONDITION_COST);
}

RuleChangeResult RuleEconomy::TryChangeConditionLogic(
    RuleSystem& rules,
    CreatureFaction faction,
    std::size_t ruleIndex,
    ConditionLogic logic)
{
    const std::vector<Rule>& factionRules = rules.GetRules(faction);
    if (ruleIndex >= factionRules.size())
    {
        return RuleChangeResult::InvalidRule;
    }
    if (factionRules[ruleIndex].logic == logic)
    {
        return RuleChangeResult::AlreadyActive;
    }

    Rule candidate = factionRules[ruleIndex];
    candidate.logic = logic;
    return TryCommitRule(
        rules, faction, ruleIndex, candidate, CHANGE_LOGIC_COST);
}

RuleChangeResult RuleEconomy::TryChangeConditionValue(
    RuleSystem& rules,
    CreatureFaction faction,
    std::size_t ruleIndex,
    std::size_t conditionIndex,
    float value)
{
    const std::vector<Rule>& factionRules = rules.GetRules(faction);
    if (ruleIndex >= factionRules.size())
    {
        return RuleChangeResult::InvalidRule;
    }
    const Rule& current = factionRules[ruleIndex];
    if (conditionIndex >= current.conditions.size()
        || current.conditions[conditionIndex].type
               != ConditionType::HpBelowPercent)
    {
        return RuleChangeResult::InvalidCondition;
    }
    if (value < MIN_UI_HP_PERCENT || value > MAX_UI_HP_PERCENT)
    {
        return RuleChangeResult::InvalidValue;
    }
    if (current.conditions[conditionIndex].value == value)
    {
        return RuleChangeResult::AlreadyActive;
    }

    Rule candidate = current;
    candidate.conditions[conditionIndex].value = value;
    return TryCommitRule(
        rules, faction, ruleIndex, candidate, CHANGE_VALUE_COST);
}

RuleChangeResult RuleEconomy::TryAddRule(
    RuleSystem& rules,
    CreatureFaction faction,
    RuleId* addedRuleId)
{
    if (rules.GetRules(faction).size() >= MAX_RULES_PER_FACTION)
    {
        return RuleChangeResult::MaxRulesReached;
    }
    if (!CanAfford(ADD_RULE_COST))
    {
        return RuleChangeResult::NotEnoughPoints;
    }

    Spend(ADD_RULE_COST);
    const RuleChangeResult result = rules.AddDefaultRule(faction, addedRuleId);
    if (result != RuleChangeResult::Success)
    {
        AddPoints(ADD_RULE_COST);
    }
    return result;
}

RuleChangeResult RuleEconomy::TryDeleteRule(
    RuleSystem& rules, CreatureFaction faction, RuleId ruleId)
{
    if (!rules.FindRuleIndex(faction, ruleId).has_value())
    {
        return RuleChangeResult::InvalidRule;
    }
    if (rules.GetRules(faction).size() <= MIN_RULES_PER_FACTION)
    {
        return RuleChangeResult::MinRulesReached;
    }
    if (!CanAfford(DELETE_RULE_COST))
    {
        return RuleChangeResult::NotEnoughPoints;
    }

    Spend(DELETE_RULE_COST);
    const RuleChangeResult result = rules.DeleteRule(faction, ruleId);
    if (result != RuleChangeResult::Success)
    {
        AddPoints(DELETE_RULE_COST);
    }
    return result;
}

RuleChangeResult RuleEconomy::TryMoveRuleUp(
    RuleSystem& rules, CreatureFaction faction, RuleId ruleId)
{
    const std::optional<std::size_t> index = rules.FindRuleIndex(faction, ruleId);
    if (!index.has_value())
    {
        return RuleChangeResult::InvalidRule;
    }
    if (*index == 0)
    {
        return RuleChangeResult::AlreadyHighestPriority;
    }
    if (!CanAfford(MOVE_RULE_COST))
    {
        return RuleChangeResult::NotEnoughPoints;
    }

    Spend(MOVE_RULE_COST);
    const RuleChangeResult result = rules.MoveRuleUp(faction, ruleId);
    if (result != RuleChangeResult::Success)
    {
        AddPoints(MOVE_RULE_COST);
    }
    return result;
}

RuleChangeResult RuleEconomy::TryMoveRuleDown(
    RuleSystem& rules, CreatureFaction faction, RuleId ruleId)
{
    const std::optional<std::size_t> index = rules.FindRuleIndex(faction, ruleId);
    if (!index.has_value())
    {
        return RuleChangeResult::InvalidRule;
    }
    if (*index + 1 >= rules.GetRules(faction).size())
    {
        return RuleChangeResult::AlreadyLowestPriority;
    }
    if (!CanAfford(MOVE_RULE_COST))
    {
        return RuleChangeResult::NotEnoughPoints;
    }

    Spend(MOVE_RULE_COST);
    const RuleChangeResult result = rules.MoveRuleDown(faction, ruleId);
    if (result != RuleChangeResult::Success)
    {
        AddPoints(MOVE_RULE_COST);
    }
    return result;
}

RuleChangeResult RuleEconomy::TryCommitRule(
    RuleSystem& rules,
    CreatureFaction faction,
    std::size_t ruleIndex,
    const Rule& candidate,
    int cost)
{
    // 先完整验证候选规则，再扣费，最后提交；失败路径不会改变规则或 RP。
    const RuleChangeResult validation = rules.ValidateRule(candidate);
    if (validation != RuleChangeResult::Success)
    {
        return validation;
    }
    if (!CanAfford(cost))
    {
        return RuleChangeResult::NotEnoughPoints;
    }
    if (!Spend(cost))
    {
        return RuleChangeResult::NotEnoughPoints;
    }

    const RuleChangeResult result =
        rules.ReplaceRule(faction, ruleIndex, candidate);
    if (result != RuleChangeResult::Success)
    {
        AddPoints(cost);
    }
    return result;
}
