#include "Map.h"

#include <cmath>

Map::Map(int width, int height, int tileSize, int offsetX, int offsetY)
    : width_(width),
      height_(height),
      tileSize_(tileSize),
      offsetX_(offsetX),
      offsetY_(offsetY),
      tiles_(static_cast<std::size_t>(width * height), Tile{TileType::Empty})
{
}

void Map::Draw(const GridPosition* highlightedTile) const
{
    const int mapPixelWidth = width_ * tileSize_;
    const int mapPixelHeight = height_ * tileSize_;
    const Color emptyColor{38, 45, 56, 255};
    const Color wallColor{142, 91, 70, 255};
    const Color wallDetailColor{99, 62, 51, 255};
    const Color gridColor{115, 130, 145, 255};
    const Color highlightColor{244, 203, 92, 255};

    // 先绘制 Tile 内容，再统一绘制网格线，墙壁不会遮住原有网格。
    for (int y = 0; y < height_; ++y)
    {
        for (int x = 0; x < width_; ++x)
        {
            const Vector2 worldPosition = GridToWorld(x, y);
            const Tile& tile = tiles_[GetIndex(x, y)];
            DrawRectangle(
                static_cast<int>(worldPosition.x),
                static_cast<int>(worldPosition.y),
                tileSize_,
                tileSize_,
                tile.type == TileType::Wall ? wallColor : emptyColor);

            if (tile.type == TileType::Wall)
            {
                DrawRectangleLines(
                    static_cast<int>(worldPosition.x) + 4,
                    static_cast<int>(worldPosition.y) + 4,
                    tileSize_ - 8,
                    tileSize_ - 8,
                    wallDetailColor);
            }
        }
    }

    // 画 width + 1 和 height + 1 条线，确保最外侧边框也完整可见。
    for (int x = 0; x <= width_; ++x)
    {
        const int worldX = offsetX_ + x * tileSize_;
        DrawLine(worldX, offsetY_, worldX, offsetY_ + mapPixelHeight, gridColor);
    }

    for (int y = 0; y <= height_; ++y)
    {
        const int worldY = offsetY_ + y * tileSize_;
        DrawLine(offsetX_, worldY, offsetX_ + mapPixelWidth, worldY, gridColor);
    }

    if (highlightedTile != nullptr && IsInside(highlightedTile->x, highlightedTile->y))
    {
        const Vector2 worldPosition = GridToWorld(highlightedTile->x, highlightedTile->y);
        DrawRectangleLinesEx(
            Rectangle{worldPosition.x, worldPosition.y,
                      static_cast<float>(tileSize_), static_cast<float>(tileSize_)},
            3.0F,
            highlightColor);
    }
}

Vector2 Map::GridToWorld(int gridX, int gridY) const
{
    return Vector2{
        static_cast<float>(offsetX_ + gridX * tileSize_),
        static_cast<float>(offsetY_ + gridY * tileSize_)};
}

GridPosition Map::WorldToGrid(float worldX, float worldY) const
{
    // floor 让地图左侧或上方的像素得到负格子坐标，而不是被截断为 0。
    return GridPosition{
        static_cast<int>(std::floor((worldX - static_cast<float>(offsetX_)) / tileSize_)),
        static_cast<int>(std::floor((worldY - static_cast<float>(offsetY_)) / tileSize_))};
}

bool Map::IsInside(int gridX, int gridY) const
{
    // 集中检查边界，后续移动和编辑都不会产生越界数组访问。
    return gridX >= 0 && gridX < width_ && gridY >= 0 && gridY < height_;
}

const Tile* Map::GetTile(int gridX, int gridY) const
{
    if (!IsInside(gridX, gridY))
    {
        return nullptr;
    }

    return &tiles_[GetIndex(gridX, gridY)];
}

bool Map::SetTile(int gridX, int gridY, TileType type)
{
    if (!IsInside(gridX, gridY))
    {
        return false;
    }

    tiles_[GetIndex(gridX, gridY)].type = type;
    return true;
}

bool Map::ToggleTile(int gridX, int gridY)
{
    if (!IsInside(gridX, gridY))
    {
        return false;
    }

    Tile& tile = tiles_[GetIndex(gridX, gridY)];
    tile.type = tile.type == TileType::Empty ? TileType::Wall : TileType::Empty;
    return true;
}

bool Map::IsWalkable(int gridX, int gridY) const
{
    // 查询必须经过 GetTile：越界等同不可通行，调用方无需接触 vector。
    const Tile* tile = GetTile(gridX, gridY);
    return tile != nullptr && tile->type == TileType::Empty;
}

int Map::GetWidth() const
{
    return width_;
}

int Map::GetHeight() const
{
    return height_;
}

int Map::GetTileSize() const
{
    return tileSize_;
}

std::size_t Map::GetIndex(int gridX, int gridY) const
{
    return static_cast<std::size_t>(gridY * width_ + gridX);
}
