#pragma once

#include "raylib.h"

class RuleEconomy;
class WorldSimulation;

class WorldStatsUI
{
public:
    bool Update(Vector2 mousePosition, bool leftPressed) const;
    void Draw(
        const WorldSimulation& simulation,
        const RuleEconomy& economy) const;
    bool IsMouseOver(Vector2 mousePosition) const;

private:
    Rectangle panelBounds_{1030.0F, 20.0F, 230.0F, 610.0F};
    Rectangle restartButton_{1050.0F, 550.0F, 190.0F, 38.0F};
};
