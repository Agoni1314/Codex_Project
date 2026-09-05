#pragma once

#include "Food.h"

#include <cstddef>
#include <vector>

class Creature;
class CreatureManager;
class Map;

class FoodManager
{
public:
    int CreateFood(
        int gridX,
        int gridY,
        const Map& map,
        const CreatureManager& creatures);
    bool HasFoodAt(int gridX, int gridY) const;
    const Food* GetFoodAt(int gridX, int gridY) const;
    const Food* GetFoodById(int id) const;
    const Food* FindNearestFood(const Creature& creature) const;
    bool RemoveFood(int id);
    std::size_t GetCount() const;
    void Clear();
    void Draw(const Map& map) const;

private:
    std::vector<Food> foods_;
    int nextId_{1};
};
