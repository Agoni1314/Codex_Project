#pragma once

#include "BehaviorType.h"
#include "Creature.h"
#include "Rule.h"

#include <cstddef>
#include <optional>
#include <random>
#include <vector>

class Map;
class RuleSystem;
class FoodManager;
class WorldStats;

class CreatureManager
{
public:
    // 成功时返回本局唯一 ID，失败时返回 -1。
    int CreateCreature(
        int gridX, int gridY, CreatureFaction faction, const Map& map);

    bool IsOccupied(int gridX, int gridY) const;
    bool HasCreatureAt(int gridX, int gridY) const;
    const Creature* GetCreatureAt(int gridX, int gridY) const;
    const Creature* GetCreatureById(int id) const;
    Creature* GetCreatureByIdMutable(int id);
    bool TryMoveCreature(int id, int targetGridX, int targetGridY, const Map& map);
    std::size_t GetCount() const;
    std::size_t GetCountByFaction(CreatureFaction faction) const;
    std::optional<float> GetAverageHPPercent(CreatureFaction faction) const;
    std::optional<float> GetAverageHunger(CreatureFaction faction) const;
    void Clear();

    static int ManhattanDistance(const Creature& first, const Creature& second);
    static bool AreEnemies(const Creature& first, const Creature& second);
    std::vector<const Creature*> GetCreaturesInRange(int creatureId, int range) const;
    const Creature* FindNearestEnemy(int creatureId) const;
    BehaviorType GetBehavior(
        const Creature& creature,
        const Map& map,
        const RuleSystem& rules,
        const FoodManager* foods = nullptr) const;

    bool AttackCreature(
        int attackerId, int targetId, WorldStats* stats = nullptr);

    // 执行指定 Creature 的一次确定性行为决策，便于测试和复用。
    bool UpdateCreatureForTick(
        int creatureId,
        const Map& map,
        const RuleSystem& rules,
        FoodManager* foods = nullptr,
        WorldStats* stats = nullptr);

    void Update(
        float deltaTime,
        const Map& map,
        const RuleSystem& rules,
        FoodManager* foods = nullptr,
        WorldStats* stats = nullptr);
    bool EatFood(int creatureId, int foodId, FoodManager& foods);
    void Draw(const Map& map) const;

private:
    Creature* FindCreatureByIdMutable(int id);
    bool CanMoveTo(
        const Creature& creature, int targetGridX, int targetGridY, const Map& map) const;
    bool ChooseChaseMove(
        const Creature& creature,
        const Creature& enemy,
        const Map& map,
        int& targetGridX,
        int& targetGridY) const;
    bool ChooseFleeMove(
        const Creature& creature,
        const Creature& enemy,
        const Map& map,
        int& targetGridX,
        int& targetGridY) const;
    bool ExecuteAction(
        int creatureId,
        RuleAction action,
        const Creature* enemy,
        const Map& map,
        FoodManager* foods,
        WorldStats* stats);
    bool ChooseMoveToward(
        const Creature& creature,
        int targetGridX,
        int targetGridY,
        const Map& map,
        int& resultGridX,
        int& resultGridY) const;
    void MoveCreaturesForTick(
        const Map& map,
        const RuleSystem& rules,
        FoodManager* foods,
        WorldStats* stats);
    void RemoveDeadCreatures();

    static constexpr float MoveInterval = 0.5F;

    std::vector<Creature> creatures_;
    int nextId_{1};
    float moveTimer_{0.0F};
    // 随机引擎只初始化一次，所有移动 tick 复用同一状态。
    std::mt19937 randomEngine_{std::random_device{}()};
};
