#include "BehaviorType.h"
#include "CreatureManager.h"
#include "Map.h"
#include "RuleSystem.h"

#include <cassert>

int main()
{
    RuleSystem rules;
    {
        Map map{20, 15, 32, 80, 60};
        CreatureManager creatures;
        const int redId = creatures.CreateCreature(2, 2, CreatureFaction::Red, map);
        assert(creatures.GetBehavior(*creatures.GetCreatureById(redId), map, rules)
               == BehaviorType::Wander);

        CreatureManager blueCreatures;
        const int blueId =
            blueCreatures.CreateCreature(2, 2, CreatureFaction::Blue, map);
        assert(blueCreatures.GetBehavior(
                   *blueCreatures.GetCreatureById(blueId), map, rules)
               == BehaviorType::Wander);
    }

    {
        Map map{20, 15, 32, 80, 60};
        CreatureManager creatures;
        const int redId = creatures.CreateCreature(5, 5, CreatureFaction::Red, map);
        const int blueId = creatures.CreateCreature(8, 5, CreatureFaction::Blue, map);
        assert(creatures.GetBehavior(*creatures.GetCreatureById(redId), map, rules)
               == BehaviorType::Chase);
        assert(creatures.GetBehavior(*creatures.GetCreatureById(blueId), map, rules)
               == BehaviorType::Flee);

        const int oldDistance = CreatureManager::ManhattanDistance(
            *creatures.GetCreatureById(redId), *creatures.GetCreatureById(blueId));
        assert(creatures.UpdateCreatureForTick(redId, map, rules));
        const int newDistance = CreatureManager::ManhattanDistance(
            *creatures.GetCreatureById(redId), *creatures.GetCreatureById(blueId));
        assert(newDistance < oldDistance);
        assert(creatures.IsOccupied(6, 5));
    }

    {
        Map map{20, 15, 32, 80, 60};
        CreatureManager creatures;
        const int redId = creatures.CreateCreature(5, 5, CreatureFaction::Red, map);
        const int blueId = creatures.CreateCreature(8, 5, CreatureFaction::Blue, map);
        const int oldDistance = CreatureManager::ManhattanDistance(
            *creatures.GetCreatureById(redId), *creatures.GetCreatureById(blueId));
        assert(creatures.UpdateCreatureForTick(blueId, map, rules));
        const int newDistance = CreatureManager::ManhattanDistance(
            *creatures.GetCreatureById(redId), *creatures.GetCreatureById(blueId));
        assert(newDistance > oldDistance);
        // 上、右、下同为最远时，固定优先选择上。
        assert(creatures.IsOccupied(8, 4));
    }

    {
        Map map{20, 15, 32, 80, 60};
        CreatureManager creatures;
        const int redId = creatures.CreateCreature(5, 5, CreatureFaction::Red, map);
        assert(creatures.CreateCreature(8, 5, CreatureFaction::Blue, map) != -1);
        assert(map.SetTile(6, 5, TileType::Wall));
        assert(creatures.UpdateCreatureForTick(redId, map, rules));
        // 最佳的右侧被墙挡住后，按固定顺序选择其余最优格中的上方。
        assert(creatures.IsOccupied(5, 4));
        assert(!creatures.IsOccupied(6, 5));
    }

    {
        Map map{20, 15, 32, 80, 60};
        CreatureManager creatures;
        const int redId = creatures.CreateCreature(5, 5, CreatureFaction::Red, map);
        assert(creatures.CreateCreature(8, 5, CreatureFaction::Blue, map) != -1);
        assert(creatures.CreateCreature(6, 5, CreatureFaction::Red, map) != -1);
        assert(creatures.UpdateCreatureForTick(redId, map, rules));
        assert(creatures.IsOccupied(5, 4));
        assert(creatures.IsOccupied(6, 5));
    }

    {
        Map map{20, 15, 32, 80, 60};
        CreatureManager creatures;
        const int redId = creatures.CreateCreature(5, 5, CreatureFaction::Red, map);
        assert(creatures.CreateCreature(8, 5, CreatureFaction::Blue, map) != -1);
        assert(map.SetTile(5, 4, TileType::Wall));
        assert(map.SetTile(6, 5, TileType::Wall));
        assert(map.SetTile(5, 6, TileType::Wall));
        assert(map.SetTile(4, 5, TileType::Wall));
        assert(!creatures.UpdateCreatureForTick(redId, map, rules));
        assert(creatures.IsOccupied(5, 5));
    }

    {
        Map map{20, 15, 32, 80, 60};
        CreatureManager creatures;
        const int redId = creatures.CreateCreature(5, 5, CreatureFaction::Red, map);
        const int blueId = creatures.CreateCreature(9, 5, CreatureFaction::Blue, map);
        assert(creatures.GetBehavior(*creatures.GetCreatureById(redId), map, rules)
               == BehaviorType::Chase);
        assert(creatures.TryMoveCreature(blueId, 10, 5, map));
        assert(creatures.GetBehavior(*creatures.GetCreatureById(redId), map, rules)
               == BehaviorType::Wander);
    }

    {
        Map map{20, 15, 32, 80, 60};
        CreatureManager creatures;
        const int redId = creatures.CreateCreature(5, 5, CreatureFaction::Red, map);
        assert(creatures.CreateCreature(7, 7, CreatureFaction::Blue, map) != -1);
        assert(creatures.UpdateCreatureForTick(redId, map, rules));
        // 右和下同样接近时，固定优先选择右。
        assert(creatures.IsOccupied(6, 5));
    }

    {
        Map map{20, 15, 32, 80, 60};
        CreatureManager creatures;
        assert(creatures.CreateCreature(5, 5, CreatureFaction::Red, map) != -1);
        const int blueId = creatures.CreateCreature(8, 5, CreatureFaction::Blue, map);
        assert(map.SetTile(8, 4, TileType::Wall));
        assert(creatures.UpdateCreatureForTick(blueId, map, rules));
        // 逃跑首选上方被墙挡住，选择下一个同距离候选：右。
        assert(creatures.IsOccupied(9, 5));
        assert(!creatures.IsOccupied(8, 4));
    }

    return 0;
}
