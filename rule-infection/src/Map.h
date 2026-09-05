#pragma once

#include "Tile.h"

#include "raylib.h"

#include <cstddef>
#include <vector>

struct GridPosition
{
    int x;
    int y;
};

class Map
{
public:
    Map(int width, int height, int tileSize, int offsetX, int offsetY);

    void Draw(const GridPosition* highlightedTile = nullptr) const;

    // 游戏逻辑只传格子坐标，像素偏移统一由地图处理。
    Vector2 GridToWorld(int gridX, int gridY) const;
    GridPosition WorldToGrid(float worldX, float worldY) const;

    bool IsInside(int gridX, int gridY) const;
    const Tile* GetTile(int gridX, int gridY) const;
    bool SetTile(int gridX, int gridY, TileType type);
    bool ToggleTile(int gridX, int gridY);
    bool IsWalkable(int gridX, int gridY) const;

    int GetWidth() const;
    int GetHeight() const;
    int GetTileSize() const;

private:
    std::size_t GetIndex(int gridX, int gridY) const;

    int width_;
    int height_;
    int tileSize_;
    int offsetX_;
    int offsetY_;
    // Tile 数据属于地图；其他系统只能通过安全接口查询，不能自行计算下标。
    std::vector<Tile> tiles_;
};
