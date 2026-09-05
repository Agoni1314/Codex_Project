#include "FoodManager.h"

#include "Creature.h"
#include "CreatureManager.h"
#include "Map.h"

#include "raylib.h"

#include <algorithm>
#include <cstdlib>

int FoodManager::CreateFood(
    int gridX,
    int gridY,
    const Map& map,
    const CreatureManager& creatures)
{
    if (!map.IsWalkable(gridX, gridY)
        || creatures.IsOccupied(gridX, gridY)
        || HasFoodAt(gridX, gridY))
    {
        return -1;
    }

    const int id = nextId_++;
    foods_.emplace_back(id, gridX, gridY);
    return id;
}

bool FoodManager::HasFoodAt(int gridX, int gridY) const
{
    return GetFoodAt(gridX, gridY) != nullptr;
}

const Food* FoodManager::GetFoodAt(int gridX, int gridY) const
{
    for (const Food& food : foods_)
    {
        if (food.GetGridX() == gridX && food.GetGridY() == gridY)
        {
            return &food;
        }
    }

    return nullptr;
}

const Food* FoodManager::GetFoodById(int id) const
{
    for (const Food& food : foods_)
    {
        if (food.GetId() == id)
        {
            return &food;
        }
    }

    return nullptr;
}

const Food* FoodManager::FindNearestFood(const Creature& creature) const
{
    const Food* nearest = nullptr;
    int nearestDistance = creature.GetVisionRange() + 1;

    for (const Food& food : foods_)
    {
        const int distance =
            std::abs(creature.GetGridX() - food.GetGridX())
            + std::abs(creature.GetGridY() - food.GetGridY());
        if (distance > creature.GetVisionRange())
        {
            continue;
        }

        if (distance < nearestDistance
            || (distance == nearestDistance
                && (nearest == nullptr || food.GetId() < nearest->GetId())))
        {
            nearest = &food;
            nearestDistance = distance;
        }
    }

    return nearest;
}

bool FoodManager::RemoveFood(int id)
{
    const auto newEnd = std::remove_if(
        foods_.begin(),
        foods_.end(),
        [id](const Food& food) { return food.GetId() == id; });
    if (newEnd == foods_.end())
    {
        return false;
    }

    foods_.erase(newEnd, foods_.end());
    return true;
}

std::size_t FoodManager::GetCount() const
{
    return foods_.size();
}

void FoodManager::Clear()
{
    foods_.clear();
    nextId_ = 1;
}

void FoodManager::Draw(const Map& map) const
{
    const float centerOffset = static_cast<float>(map.GetTileSize()) * 0.5F;
    const float radius = static_cast<float>(map.GetTileSize()) * 0.18F;

    for (const Food& food : foods_)
    {
        const Vector2 tilePosition =
            map.GridToWorld(food.GetGridX(), food.GetGridY());
        const Vector2 center{
            tilePosition.x + centerOffset,
            tilePosition.y + centerOffset};
        DrawCircleV(center, radius, Color{105, 204, 105, 255});
        DrawCircleLinesV(center, radius, Color{34, 105, 51, 255});
    }
}
