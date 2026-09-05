#include "Game.h"

#include "raylib.h"

void Game::Run()
{
    Init();

    while (!WindowShouldClose())
    {
        Update();
        Draw();
    }

    Shutdown();
}

void Game::Init()
{
    InitWindow(ScreenWidth, ScreenHeight, "Rule Infection");
    SetTargetFPS(TargetFps);
    RestartSimulation();
}

void Game::Update()
{
    const float deltaTime = GetFrameTime();

    if (IsKeyPressed(KEY_SPACE))
    {
        simulationPaused_ = !simulationPaused_;
    }

    ruleEconomy_.Update(deltaTime, simulationPaused_);

    const Vector2 mousePosition = GetMousePosition();
    const bool leftPressed = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    ruleEditorUI_.Update(
        mousePosition,
        leftPressed,
        deltaTime,
        ruleSystem_,
        ruleEconomy_);

    if (IsKeyPressed(KEY_F5)
        || worldStatsUI_.Update(mousePosition, leftPressed))
    {
        RestartSimulation();
        return;
    }

    CreatureManager& creatureManager = simulation_.GetCreatureManager();
    FoodManager& foodManager = simulation_.GetFoodManager();

    mouseGrid_ = map_.WorldToGrid(mousePosition.x, mousePosition.y);
    const bool mouseOverUi = ruleEditorUI_.IsMouseOver(mousePosition)
                             || worldStatsUI_.IsMouseOver(mousePosition);
    mouseInsideMap_ =
        !mouseOverUi && map_.IsInside(mouseGrid_.x, mouseGrid_.y);

    if (mouseInsideMap_ && IsKeyPressed(KEY_F))
    {
        foodManager.CreateFood(
            mouseGrid_.x, mouseGrid_.y, map_, creatureManager);
    }

    if (mouseInsideMap_ && leftPressed)
    {
        const Tile* tile = map_.GetTile(mouseGrid_.x, mouseGrid_.y);
        const bool removingWall =
            tile != nullptr && tile->type == TileType::Wall;

        // 已有 Creature 或 Food 的空地不能直接变墙，避免产生非法状态。
        if (removingWall
            || (!creatureManager.IsOccupied(mouseGrid_.x, mouseGrid_.y)
                && !foodManager.HasFoodAt(mouseGrid_.x, mouseGrid_.y)))
        {
            map_.ToggleTile(mouseGrid_.x, mouseGrid_.y);
        }
    }

    if (mouseInsideMap_ && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT))
    {
        creatureManager.CreateCreature(
            mouseGrid_.x, mouseGrid_.y, CreatureFaction::Red, map_);
    }

    if (mouseInsideMap_ && IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE))
    {
        creatureManager.CreateCreature(
            mouseGrid_.x, mouseGrid_.y, CreatureFaction::Blue, map_);
    }

    if (!simulationPaused_)
    {
        simulation_.Update(deltaTime, false, map_, ruleSystem_);
    }
}

void Game::Draw() const
{
    BeginDrawing();
    ClearBackground(Color{24, 28, 36, 255});

    map_.Draw(mouseInsideMap_ ? &mouseGrid_ : nullptr);
    const CreatureManager& creatureManager = simulation_.GetCreatureManager();
    const FoodManager& foodManager = simulation_.GetFoodManager();
    foodManager.Draw(map_);
    creatureManager.Draw(map_);
    DrawFPS(10, 10);
    DrawText(
        TextFormat(
            "Creatures: %d  Food: %d",
            static_cast<int>(creatureManager.GetCount()),
            static_cast<int>(foodManager.GetCount())),
        500,
        10,
        18,
        LIGHTGRAY);
    DrawText(simulationPaused_ ? "Simulation: Paused" : "Simulation: Running",
             270, 10, 20, simulationPaused_ ? GOLD : LIGHTGRAY);
    DrawText(
        "RMB: Red | MMB: Blue | F: Food at cursor",
        80,
        38,
        14,
        LIGHTGRAY);

    const Creature* hoveredCreature = mouseInsideMap_
                                          ? creatureManager.GetCreatureAt(
                                                mouseGrid_.x, mouseGrid_.y)
                                          : nullptr;

    if (hoveredCreature != nullptr)
    {
        DrawText(
            TextFormat("ID: %d  Faction: %s  Grid: (%d, %d)",
                       hoveredCreature->GetId(),
                       GetFactionName(hoveredCreature->GetFaction()),
                       hoveredCreature->GetGridX(),
                       hoveredCreature->GetGridY()),
            80, 542, 12, LIGHTGRAY);
        DrawText(
            TextFormat("HP: %d / %d  Damage: %d  Vision: %d",
                       hoveredCreature->GetHP(),
                       hoveredCreature->GetMaxHP(),
                       hoveredCreature->GetAttackDamage(),
                       hoveredCreature->GetVisionRange()),
            80, 558, 12, LIGHTGRAY);
        DrawText(
            TextFormat(
                "Hunger: %d / %d  Hungry: %s",
                hoveredCreature->GetHunger(),
                hoveredCreature->GetMaxHunger(),
                hoveredCreature->IsHungry() ? "Yes" : "No"),
            80,
            574,
            12,
            LIGHTGRAY);
        DrawText(
            TextFormat("Behavior: %s  Rule Action: %s  Attack CD: %.1f",
                       GetBehaviorName(
                           creatureManager.GetBehavior(
                               *hoveredCreature,
                               map_,
                               ruleSystem_,
                               &foodManager)),
                       GetRuleActionName(
                           ruleSystem_.Evaluate(
                               *hoveredCreature,
                               creatureManager,
                               &foodManager)),
                       hoveredCreature->GetAttackCooldownRemaining()),
            80, 590, 12, LIGHTGRAY);

        const Creature* nearestEnemy =
            creatureManager.FindNearestEnemy(hoveredCreature->GetId());
        if (nearestEnemy != nullptr)
        {
            DrawText(
                TextFormat("Nearest Enemy: %d  Distance: %d",
                           nearestEnemy->GetId(),
                           CreatureManager::ManhattanDistance(
                               *hoveredCreature, *nearestEnemy)),
                80, 606, 12, LIGHTGRAY);
        }
        else
        {
            DrawText("Nearest Enemy: None", 80, 606, 12, LIGHTGRAY);
        }
    }
    else if (mouseInsideMap_)
    {
        DrawText(TextFormat("Mouse Grid: (%d, %d)", mouseGrid_.x, mouseGrid_.y),
                 80, 574, 20, LIGHTGRAY);
    }
    else
    {
        DrawText("Mouse Grid: Outside", 80, 574, 20, LIGHTGRAY);
    }

    ruleEditorUI_.Draw(ruleSystem_, ruleEconomy_, simulationPaused_);
    worldStatsUI_.Draw(simulation_, ruleEconomy_);

    EndDrawing();
}

void Game::RestartSimulation()
{
    simulation_.Restart(map_, ruleEconomy_);
    simulationPaused_ = false;
}

void Game::Shutdown()
{
    CloseWindow();
}
