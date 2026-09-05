#include "Map.h"
#include "RuleEconomy.h"
#include "RuleSystem.h"
#include "WorldSimulation.h"

#include <cassert>
#include <cmath>

int main()
{
    Map map{20, 15, 32, 80, 60};
    RuleSystem rules;

    {
        WorldSimulation simulation{12345};
        simulation.Update(1.0F, false, map, rules);
        assert(std::abs(simulation.GetSimulationTime() - 1.0F) < 0.01F);
        assert(simulation.GetFoodManager().GetCount() == 0);
        simulation.Update(5.0F, true, map, rules);
        assert(std::abs(simulation.GetSimulationTime() - 1.0F) < 0.01F);
        assert(simulation.GetFoodManager().GetCount() == 0);
        simulation.Update(2.0F, false, map, rules);
        assert(std::abs(simulation.GetSimulationTime() - 3.0F) < 0.01F);
        assert(simulation.GetFoodManager().GetCount() == 1);

        std::size_t locatedFood = 0;
        for (int y = 0; y < map.GetHeight(); ++y)
        {
            for (int x = 0; x < map.GetWidth(); ++x)
            {
                if (simulation.GetFoodManager().HasFoodAt(x, y))
                {
                    ++locatedFood;
                    assert(map.IsWalkable(x, y));
                    assert(!simulation.GetCreatureManager().IsOccupied(x, y));
                }
            }
        }
        assert(locatedFood == simulation.GetFoodManager().GetCount());
    }

    {
        WorldSimulation simulation{7};
        FoodManager& foods = simulation.GetFoodManager();
        CreatureManager& creatures = simulation.GetCreatureManager();
        for (int y = 0; y < map.GetHeight()
                        && foods.GetCount() < MAX_FOOD_COUNT;
             ++y)
        {
            for (int x = 0; x < map.GetWidth()
                            && foods.GetCount() < MAX_FOOD_COUNT;
                 ++x)
            {
                assert(foods.CreateFood(x, y, map, creatures) != -1);
            }
        }
        assert(foods.GetCount() == MAX_FOOD_COUNT);
        simulation.Update(FOOD_SPAWN_INTERVAL, false, map, rules);
        assert(foods.GetCount() == MAX_FOOD_COUNT);
    }

    {
        WorldSimulation simulation{2026};
        RuleEconomy economy;
        assert(map.SetTile(0, 0, TileType::Wall));
        const RuleId retainedRuleId =
            rules.GetRules(CreatureFaction::Red)[2].id;
        assert(rules.SetRuleActionByIndex(
            CreatureFaction::Red, 2, RuleAction::Stay));
        assert(economy.Spend(65));
        simulation.Update(2.9F, false, map, rules);
        simulation.GetStats().RecordDeath(
            CreatureFaction::Blue, CreatureFaction::Red);

        simulation.Restart(map, economy);
        assert(simulation.GetSimulationTime() == 0.0F);
        assert(simulation.GetFoodSpawnTimer() == 0.0F);
        assert(simulation.GetCreatureManager().GetCountByFaction(
                   CreatureFaction::Red)
               == INITIAL_RED_COUNT);
        assert(simulation.GetCreatureManager().GetCountByFaction(
                   CreatureFaction::Blue)
               == INITIAL_BLUE_COUNT);
        assert(simulation.GetFoodManager().GetCount() == INITIAL_FOOD_COUNT);
        assert(simulation.GetStats()
                   .GetFactionStats(CreatureFaction::Blue)
                   .deaths
               == 0);
        assert(simulation.GetStats()
                   .GetFactionStats(CreatureFaction::Red)
                   .kills
               == 0);
        assert(economy.GetPoints() == INITIAL_RULE_POINTS);
        assert(map.GetTile(0, 0)->type == TileType::Wall);
        assert(rules.GetRuleById(CreatureFaction::Red, retainedRuleId)->action
               == RuleAction::Stay);

        // Restart 后计时器从零开始，0.2 秒不会立刻补充 Food。
        simulation.Update(0.2F, false, map, rules);
        assert(simulation.GetFoodManager().GetCount() == INITIAL_FOOD_COUNT);

        for (int y = 0; y < map.GetHeight(); ++y)
        {
            for (int x = 0; x < map.GetWidth(); ++x)
            {
                if (simulation.GetFoodManager().HasFoodAt(x, y))
                {
                    assert(map.IsWalkable(x, y));
                    assert(!simulation.GetCreatureManager().IsOccupied(x, y));
                }
            }
        }
    }

    return 0;
}
