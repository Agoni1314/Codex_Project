#include "Creature.h"

#include <cassert>

int main()
{
    Creature creature{1, 2, 3, CreatureFaction::Red};
    assert(creature.GetHunger() == 0);
    assert(creature.GetMaxHunger() == 100);
    assert(!creature.IsHungry());

    creature.UpdateHunger(1.9F, false);
    assert(creature.GetHunger() == 0);
    creature.UpdateHunger(0.1F, false);
    assert(creature.GetHunger() == 5);

    creature.UpdateHunger(22.0F, false);
    assert(creature.GetHunger() == 60);
    assert(creature.IsHungry());

    creature.UpdateHunger(100.0F, false);
    assert(creature.GetHunger() == 100);

    creature.UpdateHunger(20.0F, true);
    assert(creature.GetHunger() == 100);

    creature.ReduceHunger(40);
    assert(creature.GetHunger() == 60);
    creature.ReduceHunger(500);
    assert(creature.GetHunger() == 0);

    return 0;
}
