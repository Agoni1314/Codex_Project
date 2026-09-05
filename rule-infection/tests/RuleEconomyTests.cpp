#include "CreatureManager.h"
#include "Map.h"
#include "RuleEconomy.h"
#include "RuleSystem.h"

#include <cassert>

int main()
{
    {
        RuleEconomy economy;
        assert(economy.GetPoints() == 100);
        assert(economy.CanAfford(100));
        assert(economy.Spend(25));
        assert(economy.GetPoints() == 75);
        assert(!economy.Spend(80));
        assert(economy.GetPoints() == 75);
        assert(economy.Spend(75));
        assert(economy.GetPoints() == 0);
        assert(!economy.Spend(1));
        assert(economy.GetPoints() == 0);

        economy.AddPoints(40);
        assert(economy.GetPoints() == 40);
        economy.AddPoints(500);
        assert(economy.GetPoints() == 100);
    }

    assert(GetActionCost(RuleAction::Wander) == 10);
    assert(GetActionCost(RuleAction::Chase) == 25);
    assert(GetActionCost(RuleAction::Flee) == 20);
    assert(GetActionCost(RuleAction::Attack) == 35);
    assert(GetActionCost(RuleAction::Stay) == 10);
    assert(GetActionCost(RuleAction::SeekFood) == 20);

    {
        RuleEconomy economy;
        RuleSystem rules;

        assert(economy.TryChangeRule(
                   rules,
                   CreatureFaction::Blue,
                   2,
                   RuleAction::Chase)
               == RuleChangeResult::Success);
        assert(economy.GetPoints() == 75);
        assert(rules.GetRules(CreatureFaction::Blue)[2].action
               == RuleAction::Chase);
        assert(rules.GetRules(CreatureFaction::Red)[2].action
               == RuleAction::Chase);

        // 重复选择当前 Action 不收费。
        assert(economy.TryChangeRule(
                   rules,
                   CreatureFaction::Blue,
                   2,
                   RuleAction::Chase)
               == RuleChangeResult::AlreadyActive);
        assert(economy.GetPoints() == 75);

        // 消耗到不足以购买 Attack，失败时规则和 RP 都保持原样。
        assert(economy.Spend(65));
        assert(economy.GetPoints() == 10);
        assert(economy.TryChangeRule(
                   rules,
                   CreatureFaction::Blue,
                   2,
                   RuleAction::Attack)
               == RuleChangeResult::NotEnoughPoints);
        assert(economy.GetPoints() == 10);
        assert(rules.GetRules(CreatureFaction::Blue)[2].action
               == RuleAction::Chase);

        assert(economy.TryChangeRule(
                   rules,
                   CreatureFaction::Red,
                   99,
                   RuleAction::Stay)
               == RuleChangeResult::InvalidRule);
        assert(economy.GetPoints() == 10);
    }

    {
        Map map{20, 15, 32, 80, 60};
        CreatureManager creatures;
        RuleSystem rules;
        RuleEconomy economy;
        const int redId =
            creatures.CreateCreature(5, 5, CreatureFaction::Red, map);
        const int blueId =
            creatures.CreateCreature(8, 5, CreatureFaction::Blue, map);
        assert(rules.Evaluate(
                   *creatures.GetCreatureById(blueId), creatures)
               == RuleAction::Flee);
        assert(economy.TryChangeRule(
                   rules,
                   CreatureFaction::Blue,
                   2,
                   RuleAction::Chase)
               == RuleChangeResult::Success);
        assert(rules.Evaluate(
                   *creatures.GetCreatureById(blueId), creatures)
               == RuleAction::Chase);
        assert(creatures.GetCreatureById(redId) != nullptr);
    }

    {
        RuleEconomy economy;
        assert(economy.Spend(50));
        economy.Update(9.9F, false);
        assert(economy.GetPoints() == 50);
        economy.Update(0.1F, false);
        assert(economy.GetPoints() == 55);
        economy.Update(20.0F, false);
        assert(economy.GetPoints() == 65);

        // 暂停的 10 秒不进入恢复计时。
        economy.Update(10.0F, true);
        assert(economy.GetPoints() == 65);
        economy.Update(9.9F, false);
        assert(economy.GetPoints() == 65);
        economy.Update(0.1F, false);
        assert(economy.GetPoints() == 70);

        economy.AddPoints(100);
        economy.Update(100.0F, false);
        assert(economy.GetPoints() == 100);
    }

    return 0;
}
