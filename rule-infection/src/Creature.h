#pragma once

#include "CreatureFaction.h"
#include "Entity.h"

constexpr int MAX_HUNGER = 100;
constexpr int HUNGRY_THRESHOLD = 60;
constexpr int HUNGER_INCREASE_AMOUNT = 5;
constexpr float HUNGER_INCREASE_INTERVAL = 2.0F;
constexpr int FOOD_HUNGER_RESTORE = 40;

class Creature : public Entity
{
public:
    Creature(int id, int gridX, int gridY, CreatureFaction faction);

    // Creature 只更新坐标；目标是否合法由 CreatureManager 统一判断。
    void MoveTo(int gridX, int gridY);

    CreatureFaction GetFaction() const;
    int GetVisionRange() const;
    int GetHP() const;
    int GetMaxHP() const;
    int GetAttackDamage() const;
    float GetAttackCooldownRemaining() const;
    int GetHunger() const;
    int GetMaxHunger() const;

    void TakeDamage(int damage);
    bool IsDead() const;
    bool CanAttack() const;
    void ResetAttackCooldown();
    void UpdateAttackCooldown(float deltaTime);
    void UpdateHunger(float deltaTime, bool simulationPaused);
    void ReduceHunger(int amount);
    bool IsHungry() const;

private:
    static constexpr int VisionRange = 4;
    static constexpr int DefaultMaxHP = 100;
    static constexpr int DefaultAttackDamage = 25;
    static constexpr float AttackCooldown = 1.0F;

    // 阵营在出生时确定，当前不提供修改接口。
    CreatureFaction faction_;
    int maxHp_{DefaultMaxHP};
    int hp_{DefaultMaxHP};
    int attackDamage_{DefaultAttackDamage};
    float attackCooldownRemaining_{0.0F};
    int maxHunger_{MAX_HUNGER};
    int hunger_{0};
    float hungerTimer_{0.0F};
};
