#include "Entity.h"

Entity::Entity(int id, int gridX, int gridY)
    : id_(id), gridX_(gridX), gridY_(gridY)
{
}

int Entity::GetId() const
{
    return id_;
}

int Entity::GetGridX() const
{
    return gridX_;
}

int Entity::GetGridY() const
{
    return gridY_;
}
