#pragma once

#include "CreatureFaction.h"
#include "RuleEconomy.h"

#include "raylib.h"

#include <cstddef>

class RuleSystem;

class RuleEditorUI
{
public:
    void Update(
        Vector2 mousePosition,
        bool leftPressed,
        float deltaTime,
        RuleSystem& rules,
        RuleEconomy& economy);
    void Draw(
        const RuleSystem& rules,
        const RuleEconomy& economy,
        bool simulationPaused) const;
    bool IsMouseOver(Vector2 mousePosition) const;
    RuleId GetSelectedRuleId() const;
    std::size_t GetSelectedConditionIndex() const;

private:
    void DrawButton(
        Rectangle bounds,
        const char* text,
        bool selected,
        Color selectedColor,
        bool affordable = true,
        int fontSize = 17) const;

    Rectangle panelBounds_{740.0F, 20.0F, 280.0F, 610.0F};
    CreatureFaction selectedFaction_{CreatureFaction::Red};
    RuleId selectedRuleId_{INVALID_RULE_ID};
    std::size_t selectedConditionIndex_{0};
    bool conditionMenuOpen_{false};
    RuleChangeResult feedbackResult_{RuleChangeResult::AlreadyActive};
    int feedbackCost_{0};
    float feedbackTimer_{0.0F};
};
