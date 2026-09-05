#include "CreatureManager.h"
#include "Map.h"

#include <cassert>

int main()
{
    Map map{20, 15, 32, 80, 60};
    CreatureManager creatures;

    const int firstId = creatures.CreateCreature(2, 2, CreatureFaction::Red, map);
    assert(firstId != -1);
    assert(creatures.IsOccupied(2, 2));
    assert(!creatures.IsOccupied(3, 2));

    // 合法移动后，新格被占用且旧格立即释放。
    assert(creatures.TryMoveCreature(firstId, 3, 2, map));
    assert(!creatures.IsOccupied(2, 2));
    assert(creatures.IsOccupied(3, 2));

    // 墙壁会阻挡，失败后位置不变。
    assert(map.SetTile(4, 2, TileType::Wall));
    assert(!creatures.TryMoveCreature(firstId, 4, 2, map));
    assert(creatures.IsOccupied(3, 2));

    // 已占用格会阻挡后续 Creature。
    const int secondId = creatures.CreateCreature(3, 3, CreatureFaction::Blue, map);
    assert(secondId != -1);
    assert(!creatures.TryMoveCreature(firstId, 3, 3, map));
    assert(creatures.IsOccupied(3, 2));
    assert(creatures.IsOccupied(3, 3));

    // 接口只允许移动到相邻的上下左右格。
    assert(!creatures.TryMoveCreature(firstId, 4, 3, map));
    assert(!creatures.TryMoveCreature(firstId, 5, 2, map));
    assert(creatures.IsOccupied(3, 2));

    // 位于边缘的 Creature 不能移动到地图外。
    const int edgeId = creatures.CreateCreature(0, 0, CreatureFaction::Red, map);
    assert(edgeId != -1);
    assert(!creatures.TryMoveCreature(edgeId, -1, 0, map));
    assert(!creatures.TryMoveCreature(edgeId, 0, -1, map));
    assert(creatures.IsOccupied(0, 0));

    // 不存在的 ID 不能移动，也不能改变已有占位。
    assert(!creatures.TryMoveCreature(9999, 1, 0, map));
    assert(creatures.IsOccupied(0, 0));

    return 0;
}
