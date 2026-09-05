#include "Map.h"

#include <cassert>

int main()
{
    Map map{20, 15, 32, 80, 60};

    assert(map.GetWidth() == 20);
    assert(map.GetHeight() == 15);
    assert(map.GetTileSize() == 32);

    const Vector2 origin = map.GridToWorld(0, 0);
    assert(origin.x == 80.0F);
    assert(origin.y == 60.0F);

    const Vector2 tile = map.GridToWorld(19, 14);
    assert(tile.x == 688.0F);
    assert(tile.y == 508.0F);

    const GridPosition insideTile = map.WorldToGrid(111.0F, 91.0F);
    assert(insideTile.x == 0);
    assert(insideTile.y == 0);

    const GridPosition nextTile = map.WorldToGrid(112.0F, 92.0F);
    assert(nextTile.x == 1);
    assert(nextTile.y == 1);

    const GridPosition outsideMap = map.WorldToGrid(79.0F, 59.0F);
    assert(outsideMap.x == -1);
    assert(outsideMap.y == -1);

    assert(map.IsInside(0, 0));
    assert(map.IsInside(19, 14));
    assert(!map.IsInside(-1, 0));
    assert(!map.IsInside(0, -1));
    assert(!map.IsInside(20, 0));
    assert(!map.IsInside(0, 15));

    // 所有格子默认是可通行的空地。
    const Tile* emptyTile = map.GetTile(4, 7);
    assert(emptyTile != nullptr);
    assert(emptyTile->type == TileType::Empty);
    assert(map.IsWalkable(4, 7));

    assert(map.SetTile(4, 7, TileType::Wall));
    assert(map.GetTile(4, 7)->type == TileType::Wall);
    assert(!map.IsWalkable(4, 7));

    assert(map.ToggleTile(4, 7));
    assert(map.GetTile(4, 7)->type == TileType::Empty);
    assert(map.IsWalkable(4, 7));
    assert(map.ToggleTile(4, 7));
    assert(map.GetTile(4, 7)->type == TileType::Wall);

    // 所有公开数据接口都必须拒绝越界坐标，且不能改变合法格子。
    assert(map.GetTile(-1, 0) == nullptr);
    assert(map.GetTile(20, 0) == nullptr);
    assert(!map.SetTile(-1, 0, TileType::Wall));
    assert(!map.ToggleTile(20, 15));
    assert(!map.IsWalkable(-1, 5));
    assert(!map.IsWalkable(5, -1));
    assert(!map.IsWalkable(20, 5));
    assert(!map.IsWalkable(5, 15));
    assert(map.GetTile(0, 0)->type == TileType::Empty);

    return 0;
}
