#include "Creature.h"

#include <algorithm>

Creature::Creature(int id, int gridX, int gridY, CreatureFaction faction)
    : Entity(id, gridX, gridY), faction_(faction)
{
}

void Creature::MoveTo(int gridX, int gridY)
{
    gridX_ = gridX;
    gridY_ = gridY;
}

CreatureFaction Creature::GetFaction() const
{
    return faction_;
}

int Creature::GetVisionRange() const
{
    return VisionRange;
}

int Creature::GetHP() const
{
    return hp_;
}

int Creature::GetMaxHP() const
{
    return maxHp_;
}

int Creature::GetAttackDamage() const
{
    return attackDamage_;
}

float Creature::GetAttackCooldownRemaining() const
{
    return attackCooldownRemaining_;
}

int Creature::GetHunger() const
{
    return hunger_;
}

int Creature::GetMaxHunger() const
{
    return maxHunger_;
}

void Creature::TakeDamage(int damage)
{
    if (damage > 0)
    {
        hp_ = std::max(0, hp_ - damage);
    }
}

bool Creature::IsDead() const
{
    return hp_ <= 0;
}

bool Creature::CanAttack() const
{
    return !IsDead() && attackCooldownRemaining_ <= 0.0F;
}

void Creature::ResetAttackCooldown()
{
    attackCooldownRemaining_ = AttackCooldown;
}

void Creature::UpdateAttackCooldown(float deltaTime)
{
    if (deltaTime > 0.0F)
    {
        attackCooldownRemaining_ =
            std::max(0.0F, attackCooldownRemaining_ - deltaTime);
    }
}

void Creature::UpdateHunger(float deltaTime, bool simulationPaused)
{
    if (simulationPaused || deltaTime <= 0.0F)
    {
        return;
    }

    if (hunger_ >= maxHunger_)
    {
        hungerTimer_ = 0.0F;
        return;
    }

    hungerTimer_ += deltaTime;
    while (hungerTimer_ >= HUNGER_INCREASE_INTERVAL
           && hunger_ < maxHunger_)
    {
        hungerTimer_ -= HUNGER_INCREASE_INTERVAL;
        hunger_ = std::min(maxHunger_, hunger_ + HUNGER_INCREASE_AMOUNT);
    }

    if (hunger_ >= maxHunger_)
    {
        hungerTimer_ = 0.0F;
    }
}

void Creature::ReduceHunger(int amount)
{
    if (amount > 0)
    {
        hunger_ = std::max(0, hunger_ - amount);
    }
}

bool Creature::IsHungry() const
{
    return hunger_ >= HUNGRY_THRESHOLD;
}
