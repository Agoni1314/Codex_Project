#include "WorldStatsUI.h"

#include "RuleEconomy.h"
#include "WorldSimulation.h"

#include <optional>

namespace
{
void DrawAverage(const char* label, std::optional<float> value, int x, int y,
                 bool percent)
{
    if (value.has_value())
    {
        DrawText(TextFormat(percent ? "%s: %.0f%%" : "%s: %.0f",
                            label, *value), x, y, 16, LIGHTGRAY);
    }
    else
    {
        DrawText(TextFormat("%s: N/A", label), x, y, 16, LIGHTGRAY);
    }
}
}

bool WorldStatsUI::Update(Vector2 mousePosition, bool leftPressed) const
{
    return leftPressed && CheckCollisionPointRec(mousePosition, restartButton_);
}

void WorldStatsUI::Draw(
    const WorldSimulation& simulation,
    const RuleEconomy& economy) const
{
    DrawRectangleRec(panelBounds_, Color{28, 34, 43, 255});
    DrawRectangleLinesEx(panelBounds_, 2.0F, Color{92, 104, 124, 255});
    DrawText("WORLD STATS", 1050, 38, 23, RAYWHITE);

    const int totalSeconds = static_cast<int>(simulation.GetSimulationTime());
    DrawText(TextFormat("TIME  %02d:%02d", totalSeconds / 60, totalSeconds % 60),
             1050, 76, 20, GOLD);
    DrawText(TextFormat("FOOD  %d / %d",
                        static_cast<int>(simulation.GetFoodManager().GetCount()),
                        static_cast<int>(MAX_FOOD_COUNT)),
             1050, 106, 18, LIGHTGRAY);
    DrawText(TextFormat("RP    %d / %d", economy.GetPoints(), MAX_RULE_POINTS),
             1050, 132, 18, LIGHTGRAY);

    const CreatureManager& creatures = simulation.GetCreatureManager();
    const WorldStats& stats = simulation.GetStats();
    const FactionStats& red = stats.GetFactionStats(CreatureFaction::Red);
    DrawText("RED", 1050, 176, 21, Color{239, 112, 112, 255});
    DrawText(TextFormat("Alive: %d", static_cast<int>(
                            creatures.GetCountByFaction(CreatureFaction::Red))),
             1050, 208, 17, LIGHTGRAY);
    DrawText(TextFormat("Deaths: %d", red.deaths), 1050, 234, 17, LIGHTGRAY);
    DrawText(TextFormat("Kills: %d", red.kills), 1050, 260, 17, LIGHTGRAY);
    DrawAverage("Avg HP", creatures.GetAverageHPPercent(CreatureFaction::Red),
                1050, 286, true);
    DrawAverage("Avg Hunger", creatures.GetAverageHunger(CreatureFaction::Red),
                1050, 312, false);

    const FactionStats& blue = stats.GetFactionStats(CreatureFaction::Blue);
    DrawText("BLUE", 1050, 356, 21, Color{108, 160, 239, 255});
    DrawText(TextFormat("Alive: %d", static_cast<int>(
                            creatures.GetCountByFaction(CreatureFaction::Blue))),
             1050, 388, 17, LIGHTGRAY);
    DrawText(TextFormat("Deaths: %d", blue.deaths), 1050, 414, 17, LIGHTGRAY);
    DrawText(TextFormat("Kills: %d", blue.kills), 1050, 440, 17, LIGHTGRAY);
    DrawAverage("Avg HP", creatures.GetAverageHPPercent(CreatureFaction::Blue),
                1050, 466, true);
    DrawAverage("Avg Hunger", creatures.GetAverageHunger(CreatureFaction::Blue),
                1050, 492, false);

    const bool hovered = CheckCollisionPointRec(GetMousePosition(), restartButton_);
    DrawRectangleRec(restartButton_, hovered ? Color{139, 60, 62, 255}
                                             : Color{105, 48, 52, 255});
    DrawRectangleLinesEx(restartButton_, 2.0F, hovered ? RAYWHITE : GRAY);
    DrawText("RESTART", 1101, 560, 19, RAYWHITE);
    DrawText("F5: Restart (rules/walls kept)", 1050, 600, 12, LIGHTGRAY);
}

bool WorldStatsUI::IsMouseOver(Vector2 mousePosition) const
{
    return CheckCollisionPointRec(mousePosition, panelBounds_);
}
