#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

constexpr std::size_t MAX_CONDITIONS_PER_RULE = 3;
constexpr std::size_t MIN_RULES_PER_FACTION = 1;
constexpr std::size_t MAX_RULES_PER_FACTION = 8;
constexpr float DEFAULT_HP_BELOW_PERCENT = 30.0F;
constexpr float MIN_UI_HP_PERCENT = 5.0F;
constexpr float MAX_UI_HP_PERCENT = 95.0F;

enum class ConditionType
{
    EnemyVisible,
    EnemyAdjacent,
    FoodVisible,
    Hungry,
    HpBelowPercent,
    NoEnemyVisible
};

struct Condition
{
    ConditionType type;
    float value{0.0F};
};

enum class ConditionLogic
{
    All,
    Any
};

enum class RuleAction
{
    Wander,
    Chase,
    Flee,
    Attack,
    Stay,
    SeekFood
};

using RuleId = std::uint32_t;
constexpr RuleId INVALID_RULE_ID = 0;

struct Rule
{
    std::vector<Condition> conditions;
    ConditionLogic logic{ConditionLogic::All};
    RuleAction action{RuleAction::Stay};
    // ID 只表示稳定身份；规则在 vector 中的位置才是优先级。
    RuleId id{INVALID_RULE_ID};
};

enum class RuleChangeResult
{
    Success,
    AlreadyActive,
    NotEnoughPoints,
    InvalidRule,
    InvalidCondition,
    MaxConditionsReached,
    MinConditionsReached,
    DuplicateCondition,
    InvalidValue,
    MaxRulesReached,
    MinRulesReached,
    AlreadyHighestPriority,
    AlreadyLowestPriority
};

constexpr const char* GetConditionTypeName(ConditionType type)
{
    switch (type)
    {
    case ConditionType::EnemyVisible:
        return "EnemyVisible";
    case ConditionType::EnemyAdjacent:
        return "EnemyAdjacent";
    case ConditionType::FoodVisible:
        return "FoodVisible";
    case ConditionType::Hungry:
        return "Hungry";
    case ConditionType::HpBelowPercent:
        return "HP Below";
    default:
        return "NoEnemyVisible";
    }
}

constexpr const char* GetConditionLogicName(ConditionLogic logic)
{
    return logic == ConditionLogic::All ? "AND" : "OR";
}

constexpr const char* GetRuleActionName(RuleAction action)
{
    switch (action)
    {
    case RuleAction::Wander:
        return "Wander";
    case RuleAction::Chase:
        return "Chase";
    case RuleAction::Flee:
        return "Flee";
    case RuleAction::Attack:
        return "Attack";
    case RuleAction::Stay:
        return "Stay";
    default:
        return "SeekFood";
    }
}
