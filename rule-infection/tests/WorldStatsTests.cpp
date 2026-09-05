#include "CreatureManager.h"
#include "Map.h"
#include "WorldStats.h"

#include <cassert>
#include <cmath>

int main()
{
    Map map{20, 15, 32, 80, 60};
    CreatureManager creatures;
    WorldStats stats;

    const int redOne = creatures.CreateCreature(
        2, 2, CreatureFaction::Red, map);
    const int redTwo = creatures.CreateCreature(
        3, 2, CreatureFaction::Red, map);
    assert(creatures.GetCountByFaction(CreatureFaction::Red) == 2);
    assert(creatures.GetCountByFaction(CreatureFaction::Blue) == 0);
    assert(!creatures.GetAverageHPPercent(CreatureFaction::Blue).has_value());
    assert(!creatures.GetAverageHunger(CreatureFaction::Blue).has_value());

    Creature* secondRed = creatures.GetCreatureByIdMutable(redTwo);
    secondRed->TakeDamage(50);
    secondRed->UpdateHunger(24.0F, false);
    assert(std::abs(*creatures.GetAverageHPPercent(CreatureFaction::Red) - 75.0F)
           < 0.01F);
    assert(std::abs(*creatures.GetAverageHunger(CreatureFaction::Red) - 30.0F)
           < 0.01F);
    assert(creatures.GetCreatureById(redOne) != nullptr);

    CreatureManager combatants;
    const int attackerId = combatants.CreateCreature(
        5, 5, CreatureFaction::Red, map);
    const int targetId = combatants.CreateCreature(
        6, 5, CreatureFaction::Blue, map);
    for (int hit = 0; hit < 4; ++hit)
    {
        assert(combatants.AttackCreature(attackerId, targetId, &stats));
        if (hit < 3)
        {
            combatants.GetCreatureByIdMutable(attackerId)
                ->UpdateAttackCooldown(1.0F);
        }
    }
    assert(stats.GetFactionStats(CreatureFaction::Blue).deaths == 1);
    assert(stats.GetFactionStats(CreatureFaction::Red).kills == 1);
    assert(combatants.GetCountByFaction(CreatureFaction::Blue) == 0);
    assert(!combatants.AttackCreature(attackerId, targetId, &stats));
    assert(stats.GetFactionStats(CreatureFaction::Blue).deaths == 1);
    assert(stats.GetFactionStats(CreatureFaction::Red).kills == 1);

    stats.RecordDeath(CreatureFaction::Red);
    assert(stats.GetFactionStats(CreatureFaction::Red).deaths == 1);
    assert(stats.GetFactionStats(CreatureFaction::Blue).kills == 0);

    stats.Reset();
    assert(stats.GetFactionStats(CreatureFaction::Red).deaths == 0);
    assert(stats.GetFactionStats(CreatureFaction::Red).kills == 0);
    assert(stats.GetFactionStats(CreatureFaction::Blue).deaths == 0);
    assert(stats.GetFactionStats(CreatureFaction::Blue).kills == 0);
    return 0;
}
