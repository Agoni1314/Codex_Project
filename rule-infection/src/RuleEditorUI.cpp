#include "RuleEditorUI.h"

#include "Rule.h"
#include "RuleSystem.h"

#include <algorithm>
#include <array>
#include <optional>
#include <string>
#include <vector>

namespace
{
constexpr Rectangle RedButton{760.0F, 78.0F, 110.0F, 28.0F};
constexpr Rectangle BlueButton{890.0F, 78.0F, 110.0F, 28.0F};
constexpr float RuleRowY = 124.0F;
constexpr float RuleRowHeight = 20.0F;
constexpr float RuleRowGap = 2.0F;
constexpr float ConditionRowY = 326.0F;
constexpr float ConditionRowGap = 30.0F;
constexpr Rectangle AddConditionButton{760.0F, 416.0F, 112.0F, 26.0F};
constexpr Rectangle LogicButton{884.0F, 416.0F, 116.0F, 26.0F};
constexpr Rectangle HpMinusButton{760.0F, 446.0F, 42.0F, 24.0F};
constexpr Rectangle HpPlusButton{958.0F, 446.0F, 42.0F, 24.0F};
constexpr Rectangle AddRuleButton{760.0F, 578.0F, 112.0F, 24.0F};
constexpr Rectangle DeleteRuleButton{884.0F, 578.0F, 116.0F, 24.0F};

constexpr std::array<ConditionType, 6> AvailableConditionTypes{
    ConditionType::EnemyVisible,
    ConditionType::EnemyAdjacent,
    ConditionType::FoodVisible,
    ConditionType::Hungry,
    ConditionType::HpBelowPercent,
    ConditionType::NoEnemyVisible};

constexpr std::array<RuleAction, 6> AvailableActions{
    RuleAction::Wander, RuleAction::Chase, RuleAction::Flee,
    RuleAction::Attack, RuleAction::Stay, RuleAction::SeekFood};

constexpr std::array<Rectangle, 6> ActionButtons{
    Rectangle{760.0F, 490.0F, 110.0F, 24.0F},
    Rectangle{890.0F, 490.0F, 110.0F, 24.0F},
    Rectangle{760.0F, 518.0F, 110.0F, 24.0F},
    Rectangle{890.0F, 518.0F, 110.0F, 24.0F},
    Rectangle{760.0F, 546.0F, 110.0F, 24.0F},
    Rectangle{890.0F, 546.0F, 110.0F, 24.0F}};

Rectangle GetRuleRow(std::size_t index)
{
    return {760.0F,
            RuleRowY + static_cast<float>(index) * (RuleRowHeight + RuleRowGap),
            240.0F,
            RuleRowHeight};
}

Rectangle GetRuleUpButton(std::size_t index)
{
    const Rectangle row = GetRuleRow(index);
    return {936.0F, row.y, 30.0F, row.height};
}

Rectangle GetRuleDownButton(std::size_t index)
{
    const Rectangle row = GetRuleRow(index);
    return {970.0F, row.y, 30.0F, row.height};
}

Rectangle GetConditionRow(std::size_t index)
{
    return {760.0F,
            ConditionRowY + static_cast<float>(index) * ConditionRowGap,
            240.0F,
            24.0F};
}

Rectangle GetConditionTypeButton(std::size_t index)
{
    return {764.0F, 318.0F + static_cast<float>(index) * 34.0F,
            232.0F, 28.0F};
}

std::string FormatCondition(const Condition& condition)
{
    if (condition.type == ConditionType::HpBelowPercent)
    {
        return "HP < " + std::to_string(static_cast<int>(condition.value)) + "%";
    }
    return GetConditionTypeName(condition.type);
}

std::string FormatConditions(const Rule& rule)
{
    std::string text;
    const char* separator = rule.logic == ConditionLogic::All ? " AND " : " OR ";
    for (std::size_t index = 0; index < rule.conditions.size(); ++index)
    {
        if (index > 0)
        {
            text += separator;
        }
        text += FormatCondition(rule.conditions[index]);
    }
    return text.empty() ? "No Conditions" : text;
}

std::string FormatRuleSummary(const Rule& rule)
{
    std::string text = FormatConditions(rule) + " -> " + GetRuleActionName(rule.action);
    if (text.size() > 24)
    {
        text.resize(21);
        text += "...";
    }
    return text;
}

ConditionType FindAvailableConditionType(const Rule& rule)
{
    for (ConditionType type : AvailableConditionTypes)
    {
        const bool exists = std::any_of(
            rule.conditions.begin(), rule.conditions.end(),
            [type](const Condition& condition) { return condition.type == type; });
        if (!exists)
        {
            return type;
        }
    }
    return ConditionType::EnemyVisible;
}
}

void RuleEditorUI::Update(
    Vector2 mousePosition,
    bool leftPressed,
    float deltaTime,
    RuleSystem& rules,
    RuleEconomy& economy)
{
    if (feedbackTimer_ > 0.0F)
    {
        feedbackTimer_ -= deltaTime;
    }

    const std::vector<Rule>& currentFactionRules = rules.GetRules(selectedFaction_);
    std::optional<std::size_t> selectedIndex =
        rules.FindRuleIndex(selectedFaction_, selectedRuleId_);
    if (!selectedIndex.has_value() && !currentFactionRules.empty())
    {
        selectedRuleId_ = currentFactionRules.front().id;
        selectedIndex = 0;
        selectedConditionIndex_ = 0;
        conditionMenuOpen_ = false;
    }

    if (!leftPressed || !IsMouseOver(mousePosition))
    {
        return;
    }

    if (conditionMenuOpen_ && selectedIndex.has_value())
    {
        for (std::size_t index = 0; index < AvailableConditionTypes.size(); ++index)
        {
            if (!CheckCollisionPointRec(mousePosition, GetConditionTypeButton(index)))
            {
                continue;
            }
            feedbackCost_ = CHANGE_CONDITION_TYPE_COST;
            feedbackResult_ = economy.TryChangeConditionType(
                rules, selectedFaction_, *selectedIndex, selectedConditionIndex_,
                AvailableConditionTypes[index]);
            feedbackTimer_ = 1.5F;
            if (feedbackResult_ == RuleChangeResult::Success
                || feedbackResult_ == RuleChangeResult::AlreadyActive)
            {
                conditionMenuOpen_ = false;
            }
            return;
        }
        conditionMenuOpen_ = false;
        return;
    }

    if (CheckCollisionPointRec(mousePosition, RedButton)
        || CheckCollisionPointRec(mousePosition, BlueButton))
    {
        selectedFaction_ = CheckCollisionPointRec(mousePosition, RedButton)
                               ? CreatureFaction::Red
                               : CreatureFaction::Blue;
        const std::vector<Rule>& factionRules = rules.GetRules(selectedFaction_);
        selectedRuleId_ = factionRules.front().id;
        selectedConditionIndex_ = 0;
        conditionMenuOpen_ = false;
        return;
    }

    const std::vector<Rule>& factionRules = rules.GetRules(selectedFaction_);
    for (std::size_t index = 0; index < factionRules.size(); ++index)
    {
        const RuleId clickedRuleId = factionRules[index].id;
        if (CheckCollisionPointRec(mousePosition, GetRuleUpButton(index)))
        {
            selectedRuleId_ = clickedRuleId;
            feedbackCost_ = MOVE_RULE_COST;
            feedbackResult_ = economy.TryMoveRuleUp(
                rules, selectedFaction_, clickedRuleId);
            feedbackTimer_ = 1.5F;
            return;
        }
        if (CheckCollisionPointRec(mousePosition, GetRuleDownButton(index)))
        {
            selectedRuleId_ = clickedRuleId;
            feedbackCost_ = MOVE_RULE_COST;
            feedbackResult_ = economy.TryMoveRuleDown(
                rules, selectedFaction_, clickedRuleId);
            feedbackTimer_ = 1.5F;
            return;
        }
        if (CheckCollisionPointRec(mousePosition, GetRuleRow(index)))
        {
            selectedRuleId_ = clickedRuleId;
            selectedConditionIndex_ = 0;
            conditionMenuOpen_ = false;
            return;
        }
    }

    selectedIndex = rules.FindRuleIndex(selectedFaction_, selectedRuleId_);
    if (!selectedIndex.has_value())
    {
        return;
    }
    const Rule& selectedRule = rules.GetRules(selectedFaction_)[*selectedIndex];
    selectedConditionIndex_ = std::min(
        selectedConditionIndex_, selectedRule.conditions.size() - 1);

    for (std::size_t index = 0; index < selectedRule.conditions.size(); ++index)
    {
        const Rectangle row = GetConditionRow(index);
        const Rectangle deleteButton{920.0F, row.y, 80.0F, row.height};
        if (CheckCollisionPointRec(mousePosition, deleteButton))
        {
            feedbackCost_ = REMOVE_CONDITION_COST;
            feedbackResult_ = economy.TryRemoveCondition(
                rules, selectedFaction_, *selectedIndex, index);
            feedbackTimer_ = 1.5F;
            if (feedbackResult_ == RuleChangeResult::Success)
            {
                const Rule* updated = rules.GetRuleById(selectedFaction_, selectedRuleId_);
                selectedConditionIndex_ =
                    std::min(index, updated->conditions.size() - 1);
            }
            return;
        }
        if (CheckCollisionPointRec(
                mousePosition, Rectangle{row.x, row.y, 154.0F, row.height}))
        {
            selectedConditionIndex_ = index;
            conditionMenuOpen_ = true;
            return;
        }
    }

    if (CheckCollisionPointRec(mousePosition, AddConditionButton))
    {
        feedbackCost_ = ADD_CONDITION_COST;
        feedbackResult_ = economy.TryAddCondition(
            rules, selectedFaction_, *selectedIndex,
            FindAvailableConditionType(selectedRule));
        feedbackTimer_ = 1.5F;
        if (feedbackResult_ == RuleChangeResult::Success)
        {
            selectedConditionIndex_ =
                rules.GetRuleById(selectedFaction_, selectedRuleId_)->conditions.size() - 1;
        }
        return;
    }

    if (CheckCollisionPointRec(mousePosition, LogicButton))
    {
        feedbackCost_ = CHANGE_LOGIC_COST;
        feedbackResult_ = economy.TryChangeConditionLogic(
            rules, selectedFaction_, *selectedIndex,
            selectedRule.logic == ConditionLogic::All
                ? ConditionLogic::Any
                : ConditionLogic::All);
        feedbackTimer_ = 1.5F;
        return;
    }

    if (selectedRule.conditions[selectedConditionIndex_].type
        == ConditionType::HpBelowPercent)
    {
        const float value = selectedRule.conditions[selectedConditionIndex_].value;
        if (CheckCollisionPointRec(mousePosition, HpMinusButton)
            || CheckCollisionPointRec(mousePosition, HpPlusButton))
        {
            const float delta = CheckCollisionPointRec(mousePosition, HpMinusButton)
                                    ? -5.0F
                                    : 5.0F;
            feedbackCost_ = CHANGE_VALUE_COST;
            feedbackResult_ = economy.TryChangeConditionValue(
                rules, selectedFaction_, *selectedIndex, selectedConditionIndex_,
                value + delta);
            feedbackTimer_ = 1.5F;
            return;
        }
    }

    for (std::size_t index = 0; index < AvailableActions.size(); ++index)
    {
        if (CheckCollisionPointRec(mousePosition, ActionButtons[index]))
        {
            feedbackCost_ = GetActionCost(AvailableActions[index]);
            feedbackResult_ = economy.TryChangeRule(
                rules, selectedFaction_, *selectedIndex, AvailableActions[index]);
            feedbackTimer_ = 1.5F;
            return;
        }
    }

    if (CheckCollisionPointRec(mousePosition, AddRuleButton))
    {
        RuleId addedId = INVALID_RULE_ID;
        feedbackCost_ = ADD_RULE_COST;
        feedbackResult_ = economy.TryAddRule(rules, selectedFaction_, &addedId);
        feedbackTimer_ = 1.5F;
        if (feedbackResult_ == RuleChangeResult::Success)
        {
            selectedRuleId_ = addedId;
            selectedConditionIndex_ = 0;
            conditionMenuOpen_ = false;
        }
        return;
    }

    if (CheckCollisionPointRec(mousePosition, DeleteRuleButton))
    {
        const std::size_t deletedIndex = *selectedIndex;
        feedbackCost_ = DELETE_RULE_COST;
        feedbackResult_ = economy.TryDeleteRule(
            rules, selectedFaction_, selectedRuleId_);
        feedbackTimer_ = 1.5F;
        if (feedbackResult_ == RuleChangeResult::Success)
        {
            const std::vector<Rule>& remaining = rules.GetRules(selectedFaction_);
            const std::size_t nearbyIndex =
                std::min(deletedIndex, remaining.size() - 1);
            selectedRuleId_ = remaining[nearbyIndex].id;
            selectedConditionIndex_ = 0;
            conditionMenuOpen_ = false;
        }
    }
}

void RuleEditorUI::Draw(
    const RuleSystem& rules,
    const RuleEconomy& economy,
    bool simulationPaused) const
{
    DrawRectangleRec(panelBounds_, Color{31, 36, 47, 255});
    DrawRectangleLinesEx(panelBounds_, 2.0F, Color{92, 104, 124, 255});
    DrawText("RULE EDITOR", 760, 32, 21, RAYWHITE);
    DrawText(TextFormat("RP: %d", economy.GetPoints()), 930, 36, 16, GOLD);
    DrawText(simulationPaused ? "PAUSED - EDITOR ACTIVE" : "SIMULATION RUNNING",
             760, 58, 12,
             simulationPaused ? GOLD : Color{119, 210, 134, 255});
    DrawButton(RedButton, "RED", selectedFaction_ == CreatureFaction::Red,
               Color{184, 62, 68, 255}, true, 14);
    DrawButton(BlueButton, "BLUE", selectedFaction_ == CreatureFaction::Blue,
               Color{54, 104, 190, 255}, true, 14);
    DrawText(selectedFaction_ == CreatureFaction::Red ? "RED RULES" : "BLUE RULES",
             760, 108, 14,
             selectedFaction_ == CreatureFaction::Red
                 ? Color{239, 112, 112, 255}
                 : Color{108, 160, 239, 255});

    const std::vector<Rule>& factionRules = rules.GetRules(selectedFaction_);
    const std::optional<std::size_t> selectedIndex =
        rules.FindRuleIndex(selectedFaction_, selectedRuleId_);
    for (std::size_t index = 0; index < factionRules.size(); ++index)
    {
        const Rectangle row = GetRuleRow(index);
        const bool selected = factionRules[index].id == selectedRuleId_;
        DrawRectangleRec(row, selected ? Color{70, 78, 96, 255}
                                       : Color{42, 48, 60, 255});
        DrawRectangleLinesEx(row, selected ? 2.0F : 1.0F,
                             selected ? GOLD : Color{82, 92, 110, 255});
        DrawText(TextFormat("P%d", static_cast<int>(index + 1)), 764,
                 static_cast<int>(row.y + 4.0F), 11, GOLD);
        const std::string summary = FormatRuleSummary(factionRules[index]);
        DrawText(summary.c_str(), 790, static_cast<int>(row.y + 4.0F), 10,
                 RAYWHITE);
        DrawButton(GetRuleUpButton(index), "Up", false, DARKBLUE,
                   index > 0 && economy.CanAfford(MOVE_RULE_COST), 9);
        DrawButton(GetRuleDownButton(index), "Dn", false, DARKBLUE,
                   index + 1 < factionRules.size()
                       && economy.CanAfford(MOVE_RULE_COST), 9);
    }

    DrawText("Selected Rule", 760, 292, 16, LIGHTGRAY);
    if (selectedIndex.has_value())
    {
        const Rule& selectedRule = factionRules[*selectedIndex];
        DrawText(TextFormat("P%d  ID:%u  WHEN",
                            static_cast<int>(*selectedIndex + 1),
                            static_cast<unsigned int>(selectedRule.id)),
                 760, 310, 12, GOLD);
        for (std::size_t index = 0; index < selectedRule.conditions.size(); ++index)
        {
            const Rectangle row = GetConditionRow(index);
            const std::string prefix = index == 0
                                           ? ""
                                           : std::string(GetConditionLogicName(
                                                 selectedRule.logic)) + " ";
            const std::string label = prefix + FormatCondition(selectedRule.conditions[index]);
            DrawButton({row.x, row.y, 154.0F, row.height}, label.c_str(),
                       index == selectedConditionIndex_,
                       Color{73, 91, 122, 255}, true, 11);
            DrawButton({920.0F, row.y, 80.0F, row.height}, "Del 10", false,
                       MAROON,
                       selectedRule.conditions.size() > 1
                           && economy.CanAfford(REMOVE_CONDITION_COST), 10);
        }

        DrawButton(AddConditionButton,
                   selectedRule.conditions.size() >= MAX_CONDITIONS_PER_RULE
                       ? "Max: 3" : "+ Cond 30",
                   false, DARKBLUE,
                   selectedRule.conditions.size() < MAX_CONDITIONS_PER_RULE
                       && economy.CanAfford(ADD_CONDITION_COST), 11);
        DrawButton(LogicButton,
                   TextFormat("Logic:%s 15", GetConditionLogicName(selectedRule.logic)),
                   true, Color{96, 75, 130, 255},
                   economy.CanAfford(CHANGE_LOGIC_COST), 10);

        if (selectedConditionIndex_ < selectedRule.conditions.size()
            && selectedRule.conditions[selectedConditionIndex_].type
                   == ConditionType::HpBelowPercent)
        {
            const int value = static_cast<int>(
                selectedRule.conditions[selectedConditionIndex_].value);
            DrawButton(HpMinusButton, "-5", false, DARKGRAY,
                       value > static_cast<int>(MIN_UI_HP_PERCENT)
                           && economy.CanAfford(CHANGE_VALUE_COST), 10);
            DrawText(TextFormat("HP Below: %d%% (5 RP)", value), 808, 452, 11,
                     RAYWHITE);
            DrawButton(HpPlusButton, "+5", false, DARKGRAY,
                       value < static_cast<int>(MAX_UI_HP_PERCENT)
                           && economy.CanAfford(CHANGE_VALUE_COST), 10);
        }
        else
        {
            DrawText("Click condition: change type (20 RP)", 760, 452, 10,
                     LIGHTGRAY);
        }

        DrawText("THEN / Action", 760, 474, 13, GOLD);
        for (std::size_t index = 0; index < AvailableActions.size(); ++index)
        {
            const int cost = GetActionCost(AvailableActions[index]);
            const bool current = selectedRule.action == AvailableActions[index];
            DrawButton(ActionButtons[index],
                       TextFormat("%s %d", GetRuleActionName(AvailableActions[index]), cost),
                       current, Color{104, 125, 73, 255},
                       current || economy.CanAfford(cost), 10);
        }
    }

    DrawButton(AddRuleButton, "+ New Rule 40", false, DARKBLUE,
               factionRules.size() < MAX_RULES_PER_FACTION
                   && economy.CanAfford(ADD_RULE_COST), 10);
    DrawButton(DeleteRuleButton, "Delete Rule 15", false, MAROON,
               factionRules.size() > MIN_RULES_PER_FACTION
                   && economy.CanAfford(DELETE_RULE_COST), 10);

    if (feedbackTimer_ > 0.0F)
    {
        const char* message = "";
        Color color = Color{239, 112, 112, 255};
        switch (feedbackResult_)
        {
        case RuleChangeResult::Success:
            message = TextFormat("Success  -%d RP", feedbackCost_);
            color = Color{119, 210, 134, 255};
            break;
        case RuleChangeResult::AlreadyActive:
            message = "Already Active";
            color = GOLD;
            break;
        case RuleChangeResult::NotEnoughPoints:
            message = "Not Enough RP";
            break;
        case RuleChangeResult::DuplicateCondition:
            message = "Duplicate Condition";
            break;
        case RuleChangeResult::MaxConditionsReached:
            message = "Max Conditions Reached";
            break;
        case RuleChangeResult::MinConditionsReached:
            message = "Rule Must Have One Condition";
            break;
        case RuleChangeResult::MaxRulesReached:
            message = "Max Rules Reached";
            break;
        case RuleChangeResult::MinRulesReached:
            message = "At Least One Rule Required";
            break;
        case RuleChangeResult::AlreadyHighestPriority:
            message = "Already Highest Priority";
            break;
        case RuleChangeResult::AlreadyLowestPriority:
            message = "Already Lowest Priority";
            break;
        case RuleChangeResult::InvalidCondition:
            message = "Invalid Condition";
            break;
        case RuleChangeResult::InvalidValue:
            message = "HP Range: 5% - 95%";
            break;
        default:
            message = "Invalid Rule";
            break;
        }
        DrawText(message, 760, 608, 12, color);
    }

    if (conditionMenuOpen_ && selectedIndex.has_value())
    {
        DrawRectangle(750, 296, 260, 236, Color{24, 28, 36, 250});
        DrawRectangleLines(750, 296, 260, 236, GOLD);
        DrawText("Choose Condition Type - 20 RP", 764, 302, 13, GOLD);
        const Rule& rule = factionRules[*selectedIndex];
        for (std::size_t index = 0; index < AvailableConditionTypes.size(); ++index)
        {
            const bool current = selectedConditionIndex_ < rule.conditions.size()
                                 && rule.conditions[selectedConditionIndex_].type
                                        == AvailableConditionTypes[index];
            DrawButton(GetConditionTypeButton(index),
                       GetConditionTypeName(AvailableConditionTypes[index]),
                       current, Color{73, 91, 122, 255},
                       current || economy.CanAfford(CHANGE_CONDITION_TYPE_COST), 12);
        }
    }
}

bool RuleEditorUI::IsMouseOver(Vector2 mousePosition) const
{
    return CheckCollisionPointRec(mousePosition, panelBounds_);
}

RuleId RuleEditorUI::GetSelectedRuleId() const
{
    return selectedRuleId_;
}

std::size_t RuleEditorUI::GetSelectedConditionIndex() const
{
    return selectedConditionIndex_;
}

void RuleEditorUI::DrawButton(
    Rectangle bounds,
    const char* text,
    bool selected,
    Color selectedColor,
    bool affordable,
    int fontSize) const
{
    const bool hovered = CheckCollisionPointRec(GetMousePosition(), bounds);
    const Color background =
        selected ? selectedColor
                 : (!affordable ? Color{40, 43, 50, 255}
                                : (hovered ? Color{70, 78, 94, 255}
                                           : Color{48, 55, 69, 255}));
    DrawRectangleRec(bounds, background);
    DrawRectangleLinesEx(bounds, selected ? 2.0F : 1.0F,
                         selected ? RAYWHITE
                                  : (affordable ? GRAY : DARKGRAY));
    const int textWidth = MeasureText(text, fontSize);
    DrawText(text,
             static_cast<int>(bounds.x + (bounds.width - textWidth) * 0.5F),
             static_cast<int>(bounds.y + (bounds.height - fontSize) * 0.5F),
             fontSize,
             affordable || selected ? RAYWHITE : GRAY);
}
