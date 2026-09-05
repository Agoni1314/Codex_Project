#include "CreatureManager.h"

#include "Map.h"
#include "FoodManager.h"
#include "RuleSystem.h"
#include "WorldStats.h"

#include "raylib.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <limits>

namespace
{
// 平局时始终保留最先遇到的方向：上、右、下、左。
constexpr std::array<GridPosition, 4> MovementDirections{
    GridPosition{0, -1},
    GridPosition{1, 0},
    GridPosition{0, 1},
    GridPosition{-1, 0}};
}

int CreatureManager::CreateCreature(
    int gridX, int gridY, CreatureFaction faction, const Map& map)
{
    // Map 负责地形合法性，管理器负责生物占位；Creature 本身无需依赖地图。
    if (!map.IsWalkable(gridX, gridY) || IsOccupied(gridX, gridY))
    {
        return -1;
    }

    const int id = nextId_++;
    creatures_.emplace_back(id, gridX, gridY, faction);
    return id;
}

bool CreatureManager::IsOccupied(int gridX, int gridY) const
{
    return GetCreatureAt(gridX, gridY) != nullptr;
}

bool CreatureManager::HasCreatureAt(int gridX, int gridY) const
{
    return IsOccupied(gridX, gridY);
}

const Creature* CreatureManager::GetCreatureAt(int gridX, int gridY) const
{
    for (const Creature& creature : creatures_)
    {
        if (!creature.IsDead()
            && creature.GetGridX() == gridX && creature.GetGridY() == gridY)
        {
            return &creature;
        }
    }

    return nullptr;
}

const Creature* CreatureManager::GetCreatureById(int id) const
{
    for (const Creature& creature : creatures_)
    {
        if (!creature.IsDead() && creature.GetId() == id)
        {
            return &creature;
        }
    }

    return nullptr;
}

Creature* CreatureManager::GetCreatureByIdMutable(int id)
{
    return FindCreatureByIdMutable(id);
}

bool CreatureManager::TryMoveCreature(
    int id, int targetGridX, int targetGridY, const Map& map)
{
    Creature* creature = FindCreatureByIdMutable(id);
    if (creature == nullptr)
    {
        return false;
    }

    if (!CanMoveTo(*creature, targetGridX, targetGridY, map))
    {
        return false;
    }

    creature->MoveTo(targetGridX, targetGridY);
    return true;
}

std::size_t CreatureManager::GetCount() const
{
    return creatures_.size();
}

std::size_t CreatureManager::GetCountByFaction(CreatureFaction faction) const
{
    return static_cast<std::size_t>(std::count_if(
        creatures_.begin(), creatures_.end(),
        [faction](const Creature& creature) {
            return !creature.IsDead() && creature.GetFaction() == faction;
        }));
}

std::optional<float> CreatureManager::GetAverageHPPercent(
    CreatureFaction faction) const
{
    float total = 0.0F;
    std::size_t count = 0;
    for (const Creature& creature : creatures_)
    {
        if (!creature.IsDead() && creature.GetFaction() == faction)
        {
            total += 100.0F * static_cast<float>(creature.GetHP())
                     / static_cast<float>(creature.GetMaxHP());
            ++count;
        }
    }
    return count == 0
               ? std::nullopt
               : std::optional<float>{total / static_cast<float>(count)};
}

std::optional<float> CreatureManager::GetAverageHunger(
    CreatureFaction faction) const
{
    float total = 0.0F;
    std::size_t count = 0;
    for (const Creature& creature : creatures_)
    {
        if (!creature.IsDead() && creature.GetFaction() == faction)
        {
            total += static_cast<float>(creature.GetHunger());
            ++count;
        }
    }
    return count == 0
               ? std::nullopt
               : std::optional<float>{total / static_cast<float>(count)};
}

void CreatureManager::Clear()
{
    creatures_.clear();
    nextId_ = 1;
    moveTimer_ = 0.0F;
}

int CreatureManager::ManhattanDistance(const Creature& first, const Creature& second)
{
    return std::abs(first.GetGridX() - second.GetGridX())
           + std::abs(first.GetGridY() - second.GetGridY());
}

bool CreatureManager::AreEnemies(const Creature& first, const Creature& second)
{
    return first.GetFaction() != second.GetFaction();
}

std::vector<const Creature*> CreatureManager::GetCreaturesInRange(
    int creatureId, int range) const
{
    std::vector<const Creature*> result;
    const Creature* source = GetCreatureById(creatureId);
    if (source == nullptr)
    {
        return result;
    }

    for (const Creature& candidate : creatures_)
    {
        if (candidate.GetId() != creatureId
            && ManhattanDistance(*source, candidate) <= range)
        {
            result.push_back(&candidate);
        }
    }

    return result;
}

const Creature* CreatureManager::FindNearestEnemy(int creatureId) const
{
    const Creature* source = GetCreatureById(creatureId);
    if (source == nullptr)
    {
        return nullptr;
    }

    const Creature* nearestEnemy = nullptr;
    int nearestDistance = source->GetVisionRange() + 1;

    for (const Creature* candidate :
         GetCreaturesInRange(creatureId, source->GetVisionRange()))
    {
        if (!AreEnemies(*source, *candidate))
        {
            continue;
        }

        const int distance = ManhattanDistance(*source, *candidate);
        if (distance < nearestDistance
            || (distance == nearestDistance
                && (nearestEnemy == nullptr || candidate->GetId() < nearestEnemy->GetId())))
        {
            nearestEnemy = candidate;
            nearestDistance = distance;
        }
    }

    return nearestEnemy;
}

BehaviorType CreatureManager::GetBehavior(
    const Creature& creature,
    const Map& map,
    const RuleSystem& rules,
    const FoodManager* foods) const
{
    const RuleAction action = rules.Evaluate(creature, *this, foods);
    const Creature* enemy = FindNearestEnemy(creature.GetId());
    if (action == RuleAction::Flee && enemy != nullptr)
    {
        int targetGridX = creature.GetGridX();
        int targetGridY = creature.GetGridY();
        if (!ChooseFleeMove(creature, *enemy, map, targetGridX, targetGridY)
            && ManhattanDistance(creature, *enemy) == 1)
        {
            return BehaviorType::Attack;
        }
    }

    switch (action)
    {
    case RuleAction::Wander:
        return BehaviorType::Wander;
    case RuleAction::Chase:
        return BehaviorType::Chase;
    case RuleAction::Flee:
        return BehaviorType::Flee;
    case RuleAction::Attack:
        return BehaviorType::Attack;
    case RuleAction::SeekFood:
        return BehaviorType::SeekFood;
    default:
        return BehaviorType::Stay;
    }
}

bool CreatureManager::AttackCreature(
    int attackerId, int targetId, WorldStats* stats)
{
    Creature* attacker = FindCreatureByIdMutable(attackerId);
    Creature* target = FindCreatureByIdMutable(targetId);

    if (attacker == nullptr || target == nullptr || attackerId == targetId
        || !AreEnemies(*attacker, *target)
        || ManhattanDistance(*attacker, *target) != 1
        || !attacker->CanAttack() || target->IsDead())
    {
        return false;
    }

    target->TakeDamage(attacker->GetAttackDamage());
    attacker->ResetAttackCooldown();
    if (target->IsDead() && stats != nullptr)
    {
        // 本次攻击造成 Alive -> Dead 时记录一次；死者随后不会再被查到。
        stats->RecordDeath(target->GetFaction(), attacker->GetFaction());
    }
    return true;
}

bool CreatureManager::UpdateCreatureForTick(
    int creatureId,
    const Map& map,
    const RuleSystem& rules,
    FoodManager* foods,
    WorldStats* stats)
{
    Creature* creature = FindCreatureByIdMutable(creatureId);
    if (creature == nullptr)
    {
        return false;
    }

    const Creature* enemy = FindNearestEnemy(creatureId);
    const RuleAction action = rules.Evaluate(*creature, *this, foods);
    if (ExecuteAction(creatureId, action, enemy, map, foods, stats))
    {
        return true;
    }

    // 保留第 7 批语义：Flee 没有任何增距格且敌人相邻时才反击。
    if (action == RuleAction::Flee && enemy != nullptr
        && ManhattanDistance(*creature, *enemy) == 1)
    {
        return AttackCreature(creatureId, enemy->GetId(), stats);
    }

    return false;
}

bool CreatureManager::ExecuteAction(
    int creatureId,
    RuleAction action,
    const Creature* enemy,
    const Map& map,
    FoodManager* foods,
    WorldStats* stats)
{
    Creature* creature = FindCreatureByIdMutable(creatureId);
    if (creature == nullptr)
    {
        return false;
    }

    if (action == RuleAction::Wander)
    {
        std::uniform_int_distribution<int> directionDistribution(
            0, static_cast<int>(MovementDirections.size()) - 1);
        const GridPosition direction =
            MovementDirections[directionDistribution(randomEngine_)];
        return TryMoveCreature(
            creatureId,
            creature->GetGridX() + direction.x,
            creature->GetGridY() + direction.y,
            map);
    }

    if (action == RuleAction::SeekFood)
    {
        if (foods == nullptr)
        {
            return false;
        }

        const Food* food = foods->FindNearestFood(*creature);
        if (food == nullptr)
        {
            return false;
        }

        const int foodId = food->GetId();
        const int foodGridX = food->GetGridX();
        const int foodGridY = food->GetGridY();
        if (creature->GetGridX() == foodGridX
            && creature->GetGridY() == foodGridY)
        {
            return EatFood(creatureId, foodId, *foods);
        }

        int targetGridX = creature->GetGridX();
        int targetGridY = creature->GetGridY();
        if (!ChooseMoveToward(
                *creature,
                foodGridX,
                foodGridY,
                map,
                targetGridX,
                targetGridY)
            || !TryMoveCreature(creatureId, targetGridX, targetGridY, map))
        {
            return false;
        }

        creature = FindCreatureByIdMutable(creatureId);
        if (creature != nullptr
            && creature->GetGridX() == foodGridX
            && creature->GetGridY() == foodGridY)
        {
            EatFood(creatureId, foodId, *foods);
        }
        return true;
    }

    if (enemy == nullptr)
    {
        return false;
    }

    int targetGridX = creature->GetGridX();
    int targetGridY = creature->GetGridY();
    switch (action)
    {
    case RuleAction::Chase:
        return ChooseChaseMove(
                   *creature, *enemy, map, targetGridX, targetGridY)
               && TryMoveCreature(creatureId, targetGridX, targetGridY, map);
    case RuleAction::Flee:
        return ChooseFleeMove(
                   *creature, *enemy, map, targetGridX, targetGridY)
               && TryMoveCreature(creatureId, targetGridX, targetGridY, map);
    case RuleAction::Attack:
        return AttackCreature(creatureId, enemy->GetId(), stats);
    case RuleAction::Wander:
    case RuleAction::Stay:
    case RuleAction::SeekFood:
        return false;
    }

    return false;
}

void CreatureManager::Update(
    float deltaTime,
    const Map& map,
    const RuleSystem& rules,
    FoodManager* foods,
    WorldStats* stats)
{
    for (Creature& creature : creatures_)
    {
        if (!creature.IsDead())
        {
            creature.UpdateAttackCooldown(deltaTime);
            creature.UpdateHunger(deltaTime, false);
        }
    }

    moveTimer_ += deltaTime;
    if (moveTimer_ >= MoveInterval)
    {
        moveTimer_ -= MoveInterval;
        MoveCreaturesForTick(map, rules, foods, stats);
    }

    // 删除集中在行为遍历结束之后，避免 vector 迭代器和指针失效。
    RemoveDeadCreatures();
}

bool CreatureManager::EatFood(
    int creatureId, int foodId, FoodManager& foods)
{
    Creature* creature = FindCreatureByIdMutable(creatureId);
    const Food* food = foods.GetFoodById(foodId);
    if (creature == nullptr || food == nullptr
        || creature->GetGridX() != food->GetGridX()
        || creature->GetGridY() != food->GetGridY())
    {
        return false;
    }

    creature->ReduceHunger(FOOD_HUNGER_RESTORE);
    return foods.RemoveFood(foodId);
}

void CreatureManager::Draw(const Map& map) const
{
    const float radius = static_cast<float>(map.GetTileSize()) * 0.34F;
    const float centerOffset = static_cast<float>(map.GetTileSize()) * 0.5F;
    for (const Creature& creature : creatures_)
    {
        if (creature.IsDead())
        {
            continue;
        }

        const bool isRed = creature.GetFaction() == CreatureFaction::Red;
        const Color creatureColor = isRed
                                        ? Color{224, 78, 78, 255}
                                        : Color{73, 132, 224, 255};
        const Color outlineColor = isRed
                                       ? Color{112, 31, 38, 255}
                                       : Color{29, 57, 122, 255};
        const Vector2 tilePosition =
            map.GridToWorld(creature.GetGridX(), creature.GetGridY());
        const Vector2 center{
            tilePosition.x + centerOffset,
            tilePosition.y + centerOffset};

        DrawCircleV(center, radius, creatureColor);
        DrawCircleLinesV(center, radius, outlineColor);
    }
}

Creature* CreatureManager::FindCreatureByIdMutable(int id)
{
    for (Creature& creature : creatures_)
    {
        if (!creature.IsDead() && creature.GetId() == id)
        {
            return &creature;
        }
    }

    return nullptr;
}

bool CreatureManager::CanMoveTo(
    const Creature& creature,
    int targetGridX,
    int targetGridY,
    const Map& map) const
{
    const int distance = std::abs(targetGridX - creature.GetGridX())
                         + std::abs(targetGridY - creature.GetGridY());
    return distance == 1 && map.IsWalkable(targetGridX, targetGridY)
           && !IsOccupied(targetGridX, targetGridY);
}

bool CreatureManager::ChooseChaseMove(
    const Creature& creature,
    const Creature& enemy,
    const Map& map,
    int& targetGridX,
    int& targetGridY) const
{
    return ChooseMoveToward(
        creature,
        enemy.GetGridX(),
        enemy.GetGridY(),
        map,
        targetGridX,
        targetGridY);
}

bool CreatureManager::ChooseMoveToward(
    const Creature& creature,
    int targetGridX,
    int targetGridY,
    const Map& map,
    int& resultGridX,
    int& resultGridY) const
{
    bool foundMove = false;
    int bestDistance = std::numeric_limits<int>::max();

    for (const GridPosition direction : MovementDirections)
    {
        const int candidateX = creature.GetGridX() + direction.x;
        const int candidateY = creature.GetGridY() + direction.y;
        if (!CanMoveTo(creature, candidateX, candidateY, map))
        {
            continue;
        }

        const int distance =
            std::abs(candidateX - targetGridX)
            + std::abs(candidateY - targetGridY);
        if (distance < bestDistance)
        {
            foundMove = true;
            bestDistance = distance;
            resultGridX = candidateX;
            resultGridY = candidateY;
        }
    }

    return foundMove;
}

bool CreatureManager::ChooseFleeMove(
    const Creature& creature,
    const Creature& enemy,
    const Map& map,
    int& targetGridX,
    int& targetGridY) const
{
    bool foundMove = false;
    int bestDistance = std::numeric_limits<int>::min();

    for (const GridPosition direction : MovementDirections)
    {
        const int candidateX = creature.GetGridX() + direction.x;
        const int candidateY = creature.GetGridY() + direction.y;
        if (!CanMoveTo(creature, candidateX, candidateY, map))
        {
            continue;
        }

        const int distance =
            std::abs(candidateX - enemy.GetGridX())
            + std::abs(candidateY - enemy.GetGridY());
        if (distance > bestDistance)
        {
            foundMove = true;
            bestDistance = distance;
            targetGridX = candidateX;
            targetGridY = candidateY;
        }
    }

    const int currentDistance = ManhattanDistance(creature, enemy);
    return foundMove && bestDistance > currentDistance;
}

void CreatureManager::MoveCreaturesForTick(
    const Map& map,
    const RuleSystem& rules,
    FoodManager* foods,
    WorldStats* stats)
{

    // 按 vector 顺序移动；前面的新位置会立即阻挡后面的 Creature。
    for (Creature& creature : creatures_)
    {
        if (creature.IsDead())
        {
            continue;
        }

        const int creatureId = creature.GetId();
        UpdateCreatureForTick(creatureId, map, rules, foods, stats);
    }
}

void CreatureManager::RemoveDeadCreatures()
{
    creatures_.erase(
        std::remove_if(
            creatures_.begin(),
            creatures_.end(),
            [](const Creature& creature) { return creature.IsDead(); }),
        creatures_.end());
}
