#include "CreatureManager.h"
#include "Map.h"

#include <algorithm>
#include <cassert>
#include <vector>

namespace
{
bool ContainsId(const std::vector<const Creature*>& creatures, int id)
{
    return std::any_of(creatures.begin(), creatures.end(), [id](const Creature* creature) {
        return creature->GetId() == id;
    });
}
}

int main()
{
    Map map{20, 15, 32, 80, 60};
    CreatureManager creatures;

    const int redId = creatures.CreateCreature(5, 5, CreatureFaction::Red, map);
    const int blueLowId = creatures.CreateCreature(7, 5, CreatureFaction::Blue, map);
    const int blueHighId = creatures.CreateCreature(5, 7, CreatureFaction::Blue, map);
    const int allyId = creatures.CreateCreature(6, 5, CreatureFaction::Red, map);
    const int farBlueId = creatures.CreateCreature(10, 5, CreatureFaction::Blue, map);

    assert(redId != -1 && blueLowId != -1 && blueHighId != -1);
    assert(allyId != -1 && farBlueId != -1);

    const Creature* red = creatures.GetCreatureById(redId);
    const Creature* blueLow = creatures.GetCreatureById(blueLowId);
    const Creature* ally = creatures.GetCreatureById(allyId);
    assert(red != nullptr && blueLow != nullptr && ally != nullptr);
    assert(red->GetFaction() == CreatureFaction::Red);
    assert(blueLow->GetFaction() == CreatureFaction::Blue);
    assert(red->GetVisionRange() == 4);

    assert(!CreatureManager::AreEnemies(*red, *ally));
    assert(CreatureManager::AreEnemies(*red, *blueLow));
    assert(CreatureManager::AreEnemies(*blueLow, *red));

    assert(CreatureManager::ManhattanDistance(*red, *blueLow) == 2);
    const Creature coordinateExample{999, 7, 6, CreatureFaction::Blue};
    assert(CreatureManager::ManhattanDistance(*red, coordinateExample) == 3);

    const std::vector<const Creature*> nearby =
        creatures.GetCreaturesInRange(redId, red->GetVisionRange());
    assert(ContainsId(nearby, blueLowId));
    assert(ContainsId(nearby, blueHighId));
    assert(ContainsId(nearby, allyId));
    assert(!ContainsId(nearby, redId));
    assert(!ContainsId(nearby, farBlueId));

    // 两个敌人距离同为 2，选择 ID 更小的一个。
    const Creature* nearestEnemy = creatures.FindNearestEnemy(redId);
    assert(nearestEnemy != nullptr);
    assert(nearestEnemy->GetId() == blueLowId);

    CreatureManager alliesOnly;
    const int sourceId = alliesOnly.CreateCreature(1, 1, CreatureFaction::Red, map);
    assert(alliesOnly.CreateCreature(2, 1, CreatureFaction::Red, map) != -1);
    assert(alliesOnly.FindNearestEnemy(sourceId) == nullptr);
    assert(alliesOnly.FindNearestEnemy(9999) == nullptr);

    return 0;
}
