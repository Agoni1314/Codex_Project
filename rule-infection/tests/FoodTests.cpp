#include "CreatureManager.h"
#include "FoodManager.h"
#include "Map.h"
#include "RuleSystem.h"

#include <cassert>

int main()
{
    {
        Map map{20, 15, 32, 80, 60};
        CreatureManager creatures;
        FoodManager foods;

        const int firstFoodId = foods.CreateFood(7, 5, map, creatures);
        assert(firstFoodId != -1);
        assert(foods.GetCount() == 1);
        assert(foods.HasFoodAt(7, 5));
        assert(foods.CreateFood(7, 5, map, creatures) == -1);

        assert(map.SetTile(2, 2, TileType::Wall));
        assert(foods.CreateFood(2, 2, map, creatures) == -1);
        assert(foods.CreateFood(-1, 0, map, creatures) == -1);
        assert(foods.CreateFood(20, 0, map, creatures) == -1);

        assert(creatures.CreateCreature(
                   4, 4, CreatureFaction::Red, map)
               != -1);
        assert(foods.CreateFood(4, 4, map, creatures) == -1);
    }

    {
        Map map{20, 15, 32, 80, 60};
        CreatureManager creatures;
        FoodManager foods;
        const int creatureId =
            creatures.CreateCreature(5, 5, CreatureFaction::Red, map);
        const int lowId = foods.CreateFood(7, 5, map, creatures);
        const int highId = foods.CreateFood(5, 7, map, creatures);
        assert(lowId != -1 && highId != -1);
        assert(foods.CreateFood(10, 5, map, creatures) != -1);

        const Creature* creature = creatures.GetCreatureById(creatureId);
        const Food* nearest = foods.FindNearestFood(*creature);
        assert(nearest != nullptr);
        assert(nearest->GetId() == lowId);

        FoodManager farFoods;
        assert(farFoods.CreateFood(10, 5, map, creatures) != -1);
        assert(farFoods.FindNearestFood(*creature) == nullptr);
    }

    {
        Map map{20, 15, 32, 80, 60};
        CreatureManager creatures;
        FoodManager foods;
        RuleSystem rules;
        const int creatureId =
            creatures.CreateCreature(5, 5, CreatureFaction::Red, map);
        Creature* creature = creatures.GetCreatureByIdMutable(creatureId);
        assert(creature != nullptr);
        const Rule hungryAndFoodRule{
            {{ConditionType::Hungry}, {ConditionType::FoodVisible}},
            ConditionLogic::All,
            RuleAction::SeekFood};

        assert(foods.CreateFood(8, 5, map, creatures) != -1);
        assert(!rules.EvaluateRule(
            hungryAndFoodRule,
            *creature,
            creatures,
            &foods));

        creature->UpdateHunger(24.0F, false);
        assert(creature->GetHunger() == 60);
        assert(rules.EvaluateRule(
            hungryAndFoodRule,
            *creature,
            creatures,
            &foods));
        assert(rules.Evaluate(*creature, creatures, &foods)
               == RuleAction::SeekFood);

        assert(creatures.UpdateCreatureForTick(
            creatureId, map, rules, &foods));
        assert(creatures.IsOccupied(6, 5));
        assert(foods.GetCount() == 1);
        assert(creatures.UpdateCreatureForTick(
            creatureId, map, rules, &foods));
        assert(creatures.IsOccupied(7, 5));
        assert(creatures.UpdateCreatureForTick(
            creatureId, map, rules, &foods));
        assert(creatures.IsOccupied(8, 5));
        assert(foods.GetCount() == 0);
        assert(creature->GetHunger() == 20);
        assert(!foods.HasFoodAt(8, 5));

        // Food 不进入 Creature 占位，因此 Creature 可以走入并吃掉它。
        assert(!foods.FindNearestFood(*creature));
    }

    {
        Map map{20, 15, 32, 80, 60};
        CreatureManager creatures;
        FoodManager foods;
        RuleSystem rules;
        const int creatureId =
            creatures.CreateCreature(1, 1, CreatureFaction::Blue, map);
        Creature* creature = creatures.GetCreatureByIdMutable(creatureId);
        creature->UpdateHunger(24.0F, false);
        const Rule hungryAndFoodRule{
            {{ConditionType::Hungry}, {ConditionType::FoodVisible}},
            ConditionLogic::All,
            RuleAction::SeekFood};
        assert(!rules.EvaluateRule(
            hungryAndFoodRule,
            *creature,
            creatures,
            &foods));
    }

    return 0;
}
