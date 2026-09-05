#include "CreatureManager.h"
#include "Map.h"

#include <cassert>

int main()
{
    Map map{20, 15, 32, 80, 60};
    CreatureManager creatures;

    assert(creatures.GetCount() == 0);

    // 空地可以创建，实体保存的必须是逻辑格子坐标。
    const int firstId = creatures.CreateCreature(2, 3, CreatureFaction::Red, map);
    assert(firstId != -1);
    const Creature* firstCreature = creatures.GetCreatureAt(2, 3);
    assert(firstCreature != nullptr);
    assert(firstCreature->GetGridX() == 2);
    assert(firstCreature->GetGridY() == 3);
    assert(firstCreature->GetId() == firstId);
    assert(creatures.GetCount() == 1);
    assert(creatures.HasCreatureAt(2, 3));

    // 同一格不能重叠，失败时数量不变。
    assert(creatures.CreateCreature(2, 3, CreatureFaction::Blue, map) == -1);
    assert(creatures.GetCount() == 1);

    // 墙壁和地图四侧之外都不能创建。
    assert(map.SetTile(4, 5, TileType::Wall));
    assert(creatures.CreateCreature(4, 5, CreatureFaction::Red, map) == -1);
    assert(creatures.CreateCreature(-1, 0, CreatureFaction::Red, map) == -1);
    assert(creatures.CreateCreature(0, -1, CreatureFaction::Red, map) == -1);
    assert(creatures.CreateCreature(20, 0, CreatureFaction::Blue, map) == -1);
    assert(creatures.CreateCreature(0, 15, CreatureFaction::Blue, map) == -1);
    assert(creatures.GetCount() == 1);

    // 不同空地允许多个生物，并且同一局内 ID 不重复。
    const int secondId = creatures.CreateCreature(8, 9, CreatureFaction::Blue, map);
    assert(secondId != -1);
    const Creature* secondCreature = creatures.GetCreatureAt(8, 9);
    assert(secondCreature != nullptr);
    assert(secondCreature->GetId() == secondId);
    assert(secondId != firstId);
    assert(creatures.GetCount() == 2);
    assert(creatures.GetCreatureAt(8, 9) != nullptr);
    assert(creatures.GetCreatureAt(7, 9) == nullptr);

    return 0;
}
