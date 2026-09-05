#include "BehaviorType.h"
#include "CreatureManager.h"
#include "Map.h"
#include "RuleSystem.h"

#include <cassert>

int main()
{
    RuleSystem rules;
    {
        Creature creature{1, 2, 3, CreatureFaction::Red};
        assert(creature.GetMaxHP() == 100);
        assert(creature.GetHP() == 100);
        assert(creature.GetAttackDamage() == 25);
        assert(!creature.IsDead());

        creature.TakeDamage(25);
        assert(creature.GetHP() == 75);
        creature.TakeDamage(500);
        assert(creature.GetHP() == 0);
        assert(creature.IsDead());
    }

    {
        Map map{20, 15, 32, 80, 60};
        CreatureManager creatures;
        const int redId = creatures.CreateCreature(5, 5, CreatureFaction::Red, map);
        const int blueId = creatures.CreateCreature(6, 5, CreatureFaction::Blue, map);
        const int allyId = creatures.CreateCreature(5, 6, CreatureFaction::Red, map);
        const int farBlueId = creatures.CreateCreature(8, 5, CreatureFaction::Blue, map);

        assert(creatures.GetBehavior(*creatures.GetCreatureById(redId), map, rules)
               == BehaviorType::Attack);
        assert(!creatures.AttackCreature(redId, redId));
        assert(!creatures.AttackCreature(redId, allyId));
        assert(!creatures.AttackCreature(redId, farBlueId));
        assert(creatures.AttackCreature(redId, blueId));
        assert(creatures.GetCreatureById(blueId)->GetHP() == 75);
        assert(!creatures.AttackCreature(redId, blueId));
        assert(creatures.GetCreatureById(blueId)->GetHP() == 75);
    }

    {
        Map map{20, 15, 32, 80, 60};
        CreatureManager creatures;
        const int redId = creatures.CreateCreature(2, 2, CreatureFaction::Red, map);
        const int diagonalBlueId =
            creatures.CreateCreature(3, 3, CreatureFaction::Blue, map);
        assert(!creatures.AttackCreature(redId, diagonalBlueId));
        assert(creatures.GetCreatureById(diagonalBlueId)->GetHP() == 100);
    }

    {
        Map map{20, 15, 32, 80, 60};
        CreatureManager creatures;
        const int redId = creatures.CreateCreature(5, 5, CreatureFaction::Red, map);
        const int blueId = creatures.CreateCreature(6, 5, CreatureFaction::Blue, map);

        // Blue 的上、右、下都封住，左侧又被 Red 占用，因此只能反击。
        assert(map.SetTile(6, 4, TileType::Wall));
        assert(map.SetTile(7, 5, TileType::Wall));
        assert(map.SetTile(6, 6, TileType::Wall));
        assert(creatures.GetBehavior(*creatures.GetCreatureById(blueId), map, rules)
               == BehaviorType::Attack);

        assert(creatures.AttackCreature(redId, blueId));
        assert(creatures.GetCreatureById(blueId)->GetHP() == 75);
        assert(creatures.GetCreatureById(redId)->GetAttackCooldownRemaining() == 1.0F);

        // 半秒后的 tick 中 Red 仍在冷却，不能再次造成伤害。
        creatures.Update(0.5F, map, rules);
        assert(creatures.GetCreatureById(blueId)->GetHP() == 75);

        // 再过半秒冷却结束，Red 可以再次攻击。
        creatures.Update(0.5F, map, rules);
        assert(creatures.GetCreatureById(blueId)->GetHP() == 50);

        // 继续运行到 Blue 死亡；统一清理后格子立即释放。
        creatures.Update(0.5F, map, rules);
        creatures.Update(0.5F, map, rules);
        creatures.Update(0.5F, map, rules);
        creatures.Update(0.5F, map, rules);
        assert(creatures.GetCreatureById(blueId) == nullptr);
        assert(!creatures.IsOccupied(6, 5));
        assert(creatures.GetCount() == 1);

        // Blue 在致死 tick 已经不会继续行动，否则 Red 也会同时死亡。
        assert(creatures.GetCreatureById(redId) != nullptr);
        assert(creatures.GetCreatureById(redId)->GetHP() == 25);
    }

    return 0;
}
