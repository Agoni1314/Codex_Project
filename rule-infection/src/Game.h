#pragma once

#include "Map.h"
#include "HealthReminderUI.h"
#include "RuleEditorUI.h"
#include "RuleEconomy.h"
#include "RuleSystem.h"
#include "WorldSimulation.h"
#include "WorldStatsUI.h"

class Game
{
public:
    void Run();

private:
    void Init();
    void Update();
    void Draw() const;
    void Shutdown();
    void RestartSimulation();

    static constexpr int ScreenWidth = 1280;
    static constexpr int ScreenHeight = 650;
    static constexpr int TargetFps = 60;

    Map map_{20, 15, 32, 80, 60};
    // 规则属于阵营而不是单个 Creature，因此整局只保存一份。
    RuleSystem ruleSystem_;
    RuleEconomy ruleEconomy_;
    RuleEditorUI ruleEditorUI_;
    WorldSimulation simulation_;
    WorldStatsUI worldStatsUI_;
    GridPosition mouseGrid_{-1, -1};
    bool mouseInsideMap_{false};
    bool simulationPaused_{false};
    HealthReminderUI healthReminderUI_;
    bool showingHealthReminder_{true};
};
