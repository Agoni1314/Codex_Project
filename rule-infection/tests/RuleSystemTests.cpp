#include "CreatureManager.h"
#include "Map.h"
#include "RuleSystem.h"

#include <array>
#include <cassert>
#include <vector>

int main()
{
    Map map{20, 15, 32, 80, 60};
    CreatureManager creatures;
    RuleSystem rules;

    const int redId = creatures.CreateCreature(5, 5, CreatureFaction::Red, map);
    const int blueId = creatures.CreateCreature(8, 5, CreatureFaction::Blue, map);
    const Creature* red = creatures.GetCreatureById(redId);
    const Creature* blue = creatures.GetCreatureById(blueId);
    assert(red != nullptr && blue != nullptr);

    assert(rules.CheckCondition({ConditionType::EnemyVisible}, *red, creatures));
    assert(!rules.CheckCondition(
        {ConditionType::EnemyAdjacent}, *red, creatures));
    assert(!rules.CheckCondition(
        {ConditionType::NoEnemyVisible}, *red, creatures));
    assert(rules.Evaluate(*red, creatures) == RuleAction::Chase);
    assert(rules.Evaluate(*blue, creatures) == RuleAction::Flee);
    assert(rules.GetRules(CreatureFaction::Red).size() == 4);
    assert(rules.GetRules(CreatureFaction::Blue).size() == 4);

    assert(creatures.TryMoveCreature(blueId, 7, 5, map));
    assert(creatures.TryMoveCreature(blueId, 6, 5, map));
    red = creatures.GetCreatureById(redId);
    assert(rules.CheckCondition(
        {ConditionType::EnemyAdjacent}, *red, creatures));
    // Red 的 EnemyAdjacent 规则排在 EnemyVisible 前面，因此优先攻击。
    assert(rules.Evaluate(*red, creatures) == RuleAction::Attack);

    CreatureManager isolated;
    const int isolatedRedId =
        isolated.CreateCreature(1, 1, CreatureFaction::Red, map);
    const Creature* isolatedRed = isolated.GetCreatureById(isolatedRedId);
    assert(isolatedRed != nullptr);
    assert(!rules.CheckCondition(
        {ConditionType::EnemyVisible}, *isolatedRed, isolated));
    assert(rules.CheckCondition(
        {ConditionType::NoEnemyVisible}, *isolatedRed, isolated));
    assert(rules.Evaluate(*isolatedRed, isolated) == RuleAction::Wander);

    // 同阵营和视野外的敌人都不能触发 EnemyVisible。
    assert(isolated.CreateCreature(2, 1, CreatureFaction::Red, map) != -1);
    assert(isolated.CreateCreature(10, 1, CreatureFaction::Blue, map) != -1);
    assert(!rules.CheckCondition(
        {ConditionType::EnemyVisible}, *isolatedRed, isolated));

    CreatureManager isolatedBlueCreatures;
    const int isolatedBlueId = isolatedBlueCreatures.CreateCreature(
        3, 3, CreatureFaction::Blue, map);
    assert(rules.Evaluate(
               *isolatedBlueCreatures.GetCreatureById(isolatedBlueId),
               isolatedBlueCreatures)
           == RuleAction::Wander);

    // 修改的是阵营共享规则；无需重建已存在 Creature 就会得到新结果。
    assert(creatures.TryMoveCreature(blueId, 7, 5, map));
    assert(creatures.TryMoveCreature(blueId, 8, 5, map));
    assert(rules.SetRuleActionByIndex(
        CreatureFaction::Blue,
        2,
        RuleAction::Chase));
    assert(rules.GetRules(CreatureFaction::Blue)[2].action == RuleAction::Chase);
    assert(rules.GetRules(CreatureFaction::Red)[2].action == RuleAction::Chase);
    assert(!rules.SetRuleActionByIndex(
        CreatureFaction::Blue, 99, RuleAction::Attack));
    blue = creatures.GetCreatureById(blueId);
    assert(rules.Evaluate(*blue, creatures) == RuleAction::Chase);
    const int distanceBeforeChangedRule =
        CreatureManager::ManhattanDistance(*red, *blue);
    assert(creatures.UpdateCreatureForTick(blueId, map, rules));
    blue = creatures.GetCreatureById(blueId);
    assert(CreatureManager::ManhattanDistance(*red, *blue)
           < distanceBeforeChangedRule);
    assert(rules.SetRuleAction(
        CreatureFaction::Red,
        ConditionType::EnemyVisible,
        RuleAction::Wander));
    red = creatures.GetCreatureById(redId);
    assert(rules.Evaluate(*red, creatures) == RuleAction::Wander);

    RuleSystem unmatchedRules{
        std::vector<Rule>{
            {{{ConditionType::EnemyAdjacent}},
             ConditionLogic::All,
             RuleAction::Attack}},
        std::vector<Rule>{}};
    assert(unmatchedRules.Evaluate(*isolatedRed, isolated) == RuleAction::Stay);
    assert(!unmatchedRules.SetRuleAction(
        CreatureFaction::Red,
        ConditionType::NoEnemyVisible,
        RuleAction::Wander));

    // Red 的修改不能改变 Blue；GetRules 必须直接反映最新真正规则。
    assert(rules.GetRules(CreatureFaction::Blue)[2].action == RuleAction::Chase);
    assert(rules.GetRules(CreatureFaction::Red)[2].action == RuleAction::Wander);

    RuleSystem actionSelectionRules;
    constexpr std::array<RuleAction, 6> actions{
        RuleAction::Wander,
        RuleAction::Chase,
        RuleAction::Flee,
        RuleAction::Attack,
        RuleAction::Stay,
        RuleAction::SeekFood};
    for (const RuleAction action : actions)
    {
        assert(actionSelectionRules.SetRuleActionByIndex(
            CreatureFaction::Blue, 1, action));
        assert(actionSelectionRules.GetRules(CreatureFaction::Blue)[1].action
               == action);
    }

    return 0;
}
