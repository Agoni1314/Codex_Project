#pragma once

#include "CreatureManager.h"
#include "FoodManager.h"
#include "WorldStats.h"

#include <cstdint>
#include <random>

class Map;
class RuleEconomy;
class RuleSystem;

constexpr float FOOD_SPAWN_INTERVAL = 3.0F;
constexpr std::size_t MAX_FOOD_COUNT = 20;
constexpr int MAX_FOOD_SPAWN_ATTEMPTS = 20;
constexpr int INITIAL_RED_COUNT = 5;
constexpr int INITIAL_BLUE_COUNT = 5;
constexpr int INITIAL_FOOD_COUNT = 10;

class WorldSimulation
{
public:
    WorldSimulation();
    explicit WorldSimulation(std::uint32_t randomSeed);

    void Update(
        float deltaTime,
        bool simulationPaused,
        const Map& map,
        const RuleSystem& rules);
    void Restart(const Map& map, RuleEconomy& economy);
    bool TrySpawnFood(const Map& map);

    float GetSimulationTime() const;
    float GetFoodSpawnTimer() const;
    CreatureManager& GetCreatureManager();
    const CreatureManager& GetCreatureManager() const;
    FoodManager& GetFoodManager();
    const FoodManager& GetFoodManager() const;
    WorldStats& GetStats();
    const WorldStats& GetStats() const;

private:
    bool TrySpawnCreature(const Map& map, CreatureFaction faction);

    CreatureManager creatures_;
    FoodManager foods_;
    WorldStats stats_;
    float simulationTime_{0.0F};
    float foodSpawnTimer_{0.0F};
    std::mt19937 randomEngine_;
};
