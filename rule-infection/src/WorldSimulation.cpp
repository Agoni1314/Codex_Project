#include "WorldSimulation.h"

#include "Map.h"
#include "RuleEconomy.h"
#include "RuleSystem.h"

#include <random>

namespace
{
constexpr int MaxCreatureSpawnAttempts = 300;
}

WorldSimulation::WorldSimulation()
    : randomEngine_(std::random_device{}())
{
}

WorldSimulation::WorldSimulation(std::uint32_t randomSeed)
    : randomEngine_(randomSeed)
{
}

void WorldSimulation::Update(
    float deltaTime,
    bool simulationPaused,
    const Map& map,
    const RuleSystem& rules)
{
    if (simulationPaused || deltaTime <= 0.0F)
    {
        return;
    }

    simulationTime_ += deltaTime;
    creatures_.Update(deltaTime, map, rules, &foods_, &stats_);

    foodSpawnTimer_ += deltaTime;
    while (foodSpawnTimer_ >= FOOD_SPAWN_INTERVAL)
    {
        foodSpawnTimer_ -= FOOD_SPAWN_INTERVAL;
        if (foods_.GetCount() < MAX_FOOD_COUNT)
        {
            TrySpawnFood(map);
        }
    }
}

void WorldSimulation::Restart(const Map& map, RuleEconomy& economy)
{
    creatures_.Clear();
    foods_.Clear();
    stats_.Reset();
    economy.Reset();
    simulationTime_ = 0.0F;
    foodSpawnTimer_ = 0.0F;

    for (int count = 0; count < INITIAL_RED_COUNT; ++count)
    {
        TrySpawnCreature(map, CreatureFaction::Red);
    }
    for (int count = 0; count < INITIAL_BLUE_COUNT; ++count)
    {
        TrySpawnCreature(map, CreatureFaction::Blue);
    }
    for (int count = 0; count < INITIAL_FOOD_COUNT; ++count)
    {
        TrySpawnFood(map);
    }
}

bool WorldSimulation::TrySpawnFood(const Map& map)
{
    if (foods_.GetCount() >= MAX_FOOD_COUNT)
    {
        return false;
    }

    std::uniform_int_distribution<int> xDistribution(0, map.GetWidth() - 1);
    std::uniform_int_distribution<int> yDistribution(0, map.GetHeight() - 1);
    for (int attempt = 0; attempt < MAX_FOOD_SPAWN_ATTEMPTS; ++attempt)
    {
        const int gridX = xDistribution(randomEngine_);
        const int gridY = yDistribution(randomEngine_);
        if (foods_.CreateFood(gridX, gridY, map, creatures_) != -1)
        {
            return true;
        }
    }
    return false;
}

float WorldSimulation::GetSimulationTime() const
{
    return simulationTime_;
}

float WorldSimulation::GetFoodSpawnTimer() const
{
    return foodSpawnTimer_;
}

CreatureManager& WorldSimulation::GetCreatureManager()
{
    return creatures_;
}

const CreatureManager& WorldSimulation::GetCreatureManager() const
{
    return creatures_;
}

FoodManager& WorldSimulation::GetFoodManager()
{
    return foods_;
}

const FoodManager& WorldSimulation::GetFoodManager() const
{
    return foods_;
}

WorldStats& WorldSimulation::GetStats()
{
    return stats_;
}

const WorldStats& WorldSimulation::GetStats() const
{
    return stats_;
}

bool WorldSimulation::TrySpawnCreature(
    const Map& map, CreatureFaction faction)
{
    std::uniform_int_distribution<int> xDistribution(0, map.GetWidth() - 1);
    std::uniform_int_distribution<int> yDistribution(0, map.GetHeight() - 1);
    for (int attempt = 0; attempt < MaxCreatureSpawnAttempts; ++attempt)
    {
        const int gridX = xDistribution(randomEngine_);
        const int gridY = yDistribution(randomEngine_);
        if (creatures_.CreateCreature(gridX, gridY, faction, map) != -1)
        {
            return true;
        }
    }
    return false;
}
