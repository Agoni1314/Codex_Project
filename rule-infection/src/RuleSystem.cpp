#include "RuleSystem.h"

#include "Creature.h"
#include "CreatureManager.h"
#include "FoodManager.h"

#include <algorithm>
#include <utility>

RuleSystem::RuleSystem()
    : redRules_{
          {{{ConditionType::EnemyAdjacent}},
           ConditionLogic::All,
           RuleAction::Attack},
          {{{ConditionType::Hungry}, {ConditionType::FoodVisible}},
           ConditionLogic::All,
           RuleAction::SeekFood},
          {{{ConditionType::EnemyVisible}},
           ConditionLogic::All,
           RuleAction::Chase},
          {{{ConditionType::NoEnemyVisible}},
           ConditionLogic::All,
           RuleAction::Wander}},
      blueRules_{
          // Blue 先尝试逃跑；逃跑无效且相邻时由执行层回退到 Attack。
          {{{ConditionType::EnemyAdjacent}},
           ConditionLogic::All,
           RuleAction::Flee},
          {{{ConditionType::Hungry}, {ConditionType::FoodVisible}},
           ConditionLogic::All,
           RuleAction::SeekFood},
          {{{ConditionType::EnemyVisible}},
           ConditionLogic::All,
           RuleAction::Flee},
          {{{ConditionType::NoEnemyVisible}},
           ConditionLogic::All,
           RuleAction::Wander}}
{
    AssignInitialRuleIds();
}

RuleSystem::RuleSystem(std::vector<Rule> redRules, std::vector<Rule> blueRules)
    : redRules_(std::move(redRules)), blueRules_(std::move(blueRules))
{
    AssignInitialRuleIds();
}

RuleAction RuleSystem::Evaluate(
    const Creature& creature,
    const CreatureManager& creatures,
    const FoodManager* foods) const
{
    // 列表顺序就是优先级；匹配首条后立即返回，结果因此稳定可测试。
    for (const Rule& rule : GetRules(creature.GetFaction()))
    {
        if (EvaluateRule(rule, creature, creatures, foods))
        {
            return rule.action;
        }
    }

    return RuleAction::Stay;
}

bool RuleSystem::EvaluateRule(
    const Rule& rule,
    const Creature& creature,
    const CreatureManager& creatures,
    const FoodManager* foods) const
{
    // 空 All 不能借由数学上的“真空真”变成无条件规则。
    if (rule.conditions.empty())
    {
        return false;
    }

    const auto matches = [&](const Condition& condition) {
        return CheckCondition(condition, creature, creatures, foods);
    };

    return rule.logic == ConditionLogic::All
               ? std::all_of(
                     rule.conditions.begin(), rule.conditions.end(), matches)
               : std::any_of(
                     rule.conditions.begin(), rule.conditions.end(), matches);
}

bool RuleSystem::CheckCondition(
    const Condition& condition,
    const Creature& creature,
    const CreatureManager& creatures,
    const FoodManager* foods) const
{
    // 感知统一复用 CreatureManager，规则系统不维护另一份空间查询逻辑。
    const Creature* enemy = creatures.FindNearestEnemy(creature.GetId());
    const bool enemyVisible = enemy != nullptr;

    switch (condition.type)
    {
    case ConditionType::EnemyVisible:
        return enemyVisible;
    case ConditionType::EnemyAdjacent:
        return enemyVisible
               && CreatureManager::ManhattanDistance(creature, *enemy) == 1;
    case ConditionType::FoodVisible:
        return foods != nullptr && foods->FindNearestFood(creature) != nullptr;
    case ConditionType::Hungry:
        return creature.IsHungry();
    case ConditionType::HpBelowPercent:
    {
        if (creature.GetMaxHP() <= 0)
        {
            return false;
        }
        const float threshold = std::clamp(condition.value, 0.0F, 100.0F);
        const float hpPercent =
            100.0F * static_cast<float>(creature.GetHP())
            / static_cast<float>(creature.GetMaxHP());
        return hpPercent < threshold;
    }
    case ConditionType::NoEnemyVisible:
        return !enemyVisible;
    }

    return false;
}

bool RuleSystem::SetRuleAction(
    CreatureFaction faction, ConditionType condition, RuleAction action)
{
    const std::vector<Rule>& rules = GetRules(faction);
    for (std::size_t index = 0; index < rules.size(); ++index)
    {
        if (rules[index].conditions.size() == 1
            && rules[index].conditions.front().type == condition)
        {
            return SetRuleActionByIndex(faction, index, action);
        }
    }

    return false;
}

bool RuleSystem::SetRuleActionByIndex(
    CreatureFaction faction, std::size_t ruleIndex, RuleAction action)
{
    const std::vector<Rule>& rules = GetRules(faction);
    if (ruleIndex >= rules.size())
    {
        return false;
    }

    Rule candidate = rules[ruleIndex];
    candidate.action = action;
    return ReplaceRule(faction, ruleIndex, candidate)
           == RuleChangeResult::Success;
}

RuleChangeResult RuleSystem::ReplaceRule(
    CreatureFaction faction, std::size_t ruleIndex, const Rule& rule)
{
    std::vector<Rule>& rules = GetRulesMutable(faction);
    if (ruleIndex >= rules.size())
    {
        return RuleChangeResult::InvalidRule;
    }

    const RuleChangeResult validation = ValidateRule(rule);
    if (validation != RuleChangeResult::Success)
    {
        return validation;
    }

    if (rule.id != rules[ruleIndex].id)
    {
        return RuleChangeResult::InvalidRule;
    }

    rules[ruleIndex] = rule;
    return RuleChangeResult::Success;
}

RuleChangeResult RuleSystem::ValidateRule(const Rule& rule) const
{
    if (rule.conditions.empty())
    {
        return RuleChangeResult::MinConditionsReached;
    }
    if (rule.conditions.size() > MAX_CONDITIONS_PER_RULE)
    {
        return RuleChangeResult::MaxConditionsReached;
    }

    switch (rule.logic)
    {
    case ConditionLogic::All:
    case ConditionLogic::Any:
        break;
    default:
        return RuleChangeResult::InvalidRule;
    }

    switch (rule.action)
    {
    case RuleAction::Wander:
    case RuleAction::Chase:
    case RuleAction::Flee:
    case RuleAction::Attack:
    case RuleAction::Stay:
    case RuleAction::SeekFood:
        break;
    default:
        return RuleChangeResult::InvalidRule;
    }

    for (std::size_t index = 0; index < rule.conditions.size(); ++index)
    {
        const Condition& condition = rule.conditions[index];
        switch (condition.type)
        {
        case ConditionType::EnemyVisible:
        case ConditionType::EnemyAdjacent:
        case ConditionType::FoodVisible:
        case ConditionType::Hungry:
        case ConditionType::NoEnemyVisible:
            break;
        case ConditionType::HpBelowPercent:
            if (condition.value < MIN_UI_HP_PERCENT
                || condition.value > MAX_UI_HP_PERCENT)
            {
                return RuleChangeResult::InvalidValue;
            }
            break;
        default:
            return RuleChangeResult::InvalidCondition;
        }

        for (std::size_t other = index + 1;
             other < rule.conditions.size();
             ++other)
        {
            if (condition.type == rule.conditions[other].type)
            {
                return RuleChangeResult::DuplicateCondition;
            }
        }
    }

    return RuleChangeResult::Success;
}

RuleChangeResult RuleSystem::AddDefaultRule(
    CreatureFaction faction, RuleId* addedRuleId)
{
    std::vector<Rule>& rules = GetRulesMutable(faction);
    if (rules.size() >= MAX_RULES_PER_FACTION)
    {
        return RuleChangeResult::MaxRulesReached;
    }

    Rule rule{
        {{ConditionType::EnemyVisible}},
        ConditionLogic::All,
        RuleAction::Stay,
        AllocateRuleId()};
    rules.push_back(rule);
    if (addedRuleId != nullptr)
    {
        *addedRuleId = rule.id;
    }
    return RuleChangeResult::Success;
}

RuleChangeResult RuleSystem::DeleteRule(
    CreatureFaction faction, RuleId ruleId)
{
    std::vector<Rule>& rules = GetRulesMutable(faction);
    if (rules.size() <= MIN_RULES_PER_FACTION)
    {
        return RuleChangeResult::MinRulesReached;
    }
    const std::optional<std::size_t> index = FindRuleIndex(faction, ruleId);
    if (!index.has_value())
    {
        return RuleChangeResult::InvalidRule;
    }

    rules.erase(rules.begin() + static_cast<std::ptrdiff_t>(*index));
    return RuleChangeResult::Success;
}

RuleChangeResult RuleSystem::MoveRuleUp(
    CreatureFaction faction, RuleId ruleId)
{
    const std::optional<std::size_t> index = FindRuleIndex(faction, ruleId);
    if (!index.has_value())
    {
        return RuleChangeResult::InvalidRule;
    }
    if (*index == 0)
    {
        return RuleChangeResult::AlreadyHighestPriority;
    }

    std::vector<Rule>& rules = GetRulesMutable(faction);
    std::swap(rules[*index], rules[*index - 1]);
    return RuleChangeResult::Success;
}

RuleChangeResult RuleSystem::MoveRuleDown(
    CreatureFaction faction, RuleId ruleId)
{
    const std::optional<std::size_t> index = FindRuleIndex(faction, ruleId);
    if (!index.has_value())
    {
        return RuleChangeResult::InvalidRule;
    }
    std::vector<Rule>& rules = GetRulesMutable(faction);
    if (*index + 1 >= rules.size())
    {
        return RuleChangeResult::AlreadyLowestPriority;
    }

    std::swap(rules[*index], rules[*index + 1]);
    return RuleChangeResult::Success;
}

std::optional<std::size_t> RuleSystem::FindRuleIndex(
    CreatureFaction faction, RuleId ruleId) const
{
    const std::vector<Rule>& rules = GetRules(faction);
    for (std::size_t index = 0; index < rules.size(); ++index)
    {
        if (rules[index].id == ruleId)
        {
            return index;
        }
    }
    return std::nullopt;
}

const Rule* RuleSystem::GetRuleById(
    CreatureFaction faction, RuleId ruleId) const
{
    const std::optional<std::size_t> index = FindRuleIndex(faction, ruleId);
    return index.has_value() ? &GetRules(faction)[*index] : nullptr;
}

RuleAction RuleSystem::GetRuleAction(
    CreatureFaction faction, ConditionType condition) const
{
    for (const Rule& rule : GetRules(faction))
    {
        if (rule.conditions.size() == 1
            && rule.conditions.front().type == condition)
        {
            return rule.action;
        }
    }

    return RuleAction::Stay;
}

const std::vector<Rule>& RuleSystem::GetRules(CreatureFaction faction) const
{
    return faction == CreatureFaction::Red ? redRules_ : blueRules_;
}

std::vector<Rule>& RuleSystem::GetRulesMutable(CreatureFaction faction)
{
    return faction == CreatureFaction::Red ? redRules_ : blueRules_;
}

RuleId RuleSystem::AllocateRuleId()
{
    if (nextRuleId_ == INVALID_RULE_ID)
    {
        ++nextRuleId_;
    }
    return nextRuleId_++;
}

void RuleSystem::AssignInitialRuleIds()
{
    // 构造时统一重新编号，保证两个阵营之间也不会出现重复 ID。
    for (Rule& rule : redRules_)
    {
        rule.id = AllocateRuleId();
    }
    for (Rule& rule : blueRules_)
    {
        rule.id = AllocateRuleId();
    }
}
