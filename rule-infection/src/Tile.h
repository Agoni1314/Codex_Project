#pragma once

enum class TileType
{
    Empty,
    Wall
};

struct Tile
{
    TileType type{TileType::Empty};
};
