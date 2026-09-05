#include "CreatureManager.h"
#include "FoodManager.h"
#include "Map.h"
#include "RuleSystem.h"

#include <cassert>
#include <vector>

int main()
{
    Map map{20, 15, 32, 80, 60};
    CreatureManager creatures;
    FoodManager foods;
    RuleSystem rules;
    const int redId =
        creatures.CreateCreature(5, 5, CreatureFaction::Red, map);
    const int blueId =
        creatures.CreateCreature(8, 5, CreatureFaction::Blue, map);
    Creature* red = creatures.GetCreatureByIdMutable(redId);
    assert(red != nullptr);

    const Rule singleAll{
        {{ConditionType::EnemyVisible}}, ConditionLogic::All, RuleAction::Chase};
    assert(rules.EvaluateRule(singleAll, *red, creatures, &foods));

    const Rule hungryAndFood{
        {{ConditionType::Hungry}, {ConditionType::FoodVisible}},
        ConditionLogic::All,
        RuleAction::SeekFood};
    assert(foods.CreateFood(7, 5, map, creatures) != -1);
    assert(!rules.EvaluateRule(hungryAndFood, *red, creatures, &foods));
    red->UpdateHunger(24.0F, false);
    assert(rules.EvaluateRule(hungryAndFood, *red, creatures, &foods));

    FoodManager noFoods;
    assert(!rules.EvaluateRule(hungryAndFood, *red, creatures, &noFoods));

    const Rule anyOneTrue{
        {{ConditionType::EnemyAdjacent}, {ConditionType::Hungry}},
        ConditionLogic::Any,
        RuleAction::Flee};
    assert(rules.EvaluateRule(anyOneTrue, *red, creatures, &foods));

    CreatureManager isolatedCreatures;
    const int isolatedId = isolatedCreatures.CreateCreature(
        1, 1, CreatureFaction::Red, map);
    const Creature* isolated = isolatedCreatures.GetCreatureById(isolatedId);
    const Rule anyAllFalse{
        {{ConditionType::EnemyVisible}, {ConditionType::Hungry}},
        ConditionLogic::Any,
        RuleAction::Attack};
    assert(!rules.EvaluateRule(
        anyAllFalse, *isolated, isolatedCreatures, &noFoods));

    const Rule emptyRule{{}, ConditionLogic::All, RuleAction::Attack};
    assert(!rules.EvaluateRule(emptyRule, *red, creatures, &foods));

    assert(rules.CheckCondition(
        {ConditionType::Hungry}, *red, creatures, &foods));
    assert(rules.CheckCondition(
        {ConditionType::FoodVisible}, *red, creatures, &foods));
    assert(rules.CheckCondition(
        {ConditionType::EnemyVisible}, *red, creatures, &foods));
    assert(!rules.CheckCondition(
        {ConditionType::EnemyAdjacent}, *red, creatures, &foods));
    assert(rules.CheckCondition(
        {ConditionType::NoEnemyVisible},
        *isolated,
        isolatedCreatures,
        &noFoods));

    assert(creatures.TryMoveCreature(blueId, 7, 5, map));
    assert(creatures.TryMoveCreature(blueId, 6, 5, map));
    assert(rules.CheckCondition(
        {ConditionType::EnemyAdjacent}, *red, creatures, &foods));

    CreatureManager hp29Creatures;
    const int hp29Id =
        hp29Creatures.CreateCreature(2, 2, CreatureFaction::Red, map);
    Creature* hp29 = hp29Creatures.GetCreatureByIdMutable(hp29Id);
    hp29->TakeDamage(71);
    assert(rules.CheckCondition(
        {ConditionType::HpBelowPercent, 30.0F},
        *hp29,
        hp29Creatures,
        nullptr));
    // 非法百分比会被限制到 0%~100%，避免产生不可预测的比较结果。
    assert(rules.CheckCondition(
        {ConditionType::HpBelowPercent, 150.0F},
        *hp29,
        hp29Creatures,
        nullptr));
    assert(!rules.CheckCondition(
        {ConditionType::HpBelowPercent, -20.0F},
        *hp29,
        hp29Creatures,
        nullptr));
    // Below 使用严格小于，因此恰好 30% 不匹配。
    CreatureManager hp30Creatures;
    const int hp30Id =
        hp30Creatures.CreateCreature(3, 3, CreatureFaction::Red, map);
    Creature* hp30 = hp30Creatures.GetCreatureByIdMutable(hp30Id);
    hp30->TakeDamage(70);
    assert(!rules.CheckCondition(
        {ConditionType::HpBelowPercent, 30.0F},
        *hp30,
        hp30Creatures,
        nullptr));
    CreatureManager hp50Creatures;
    const int hp50Id =
        hp50Creatures.CreateCreature(4, 4, CreatureFaction::Red, map);
    Creature* hp50 = hp50Creatures.GetCreatureByIdMutable(hp50Id);
    hp50->TakeDamage(50);
    assert(!rules.CheckCondition(
        {ConditionType::HpBelowPercent, 30.0F},
        *hp50,
        hp50Creatures,
        nullptr));

    const Rule lowHpAndEnemy{
        {{ConditionType::HpBelowPercent, 30.0F},
         {ConditionType::EnemyVisible}},
        ConditionLogic::All,
        RuleAction::Flee};
    assert(rules.EvaluateRule(
        lowHpAndEnemy, *hp29, hp29Creatures, nullptr)
           == false);

    CreatureManager lowHpWithEnemyCreatures;
    const int lowHpRedId = lowHpWithEnemyCreatures.CreateCreature(
        5, 5, CreatureFaction::Red, map);
    assert(lowHpWithEnemyCreatures.CreateCreature(
               8, 5, CreatureFaction::Blue, map)
           != -1);
    Creature* lowHpRed =
        lowHpWithEnemyCreatures.GetCreatureByIdMutable(lowHpRedId);
    lowHpRed->TakeDamage(71);
    assert(rules.EvaluateRule(
        lowHpAndEnemy, *lowHpRed, lowHpWithEnemyCreatures, nullptr));

    RuleSystem orderedRules{
        std::vector<Rule>{
            {{{ConditionType::EnemyVisible}},
             ConditionLogic::All,
             RuleAction::Stay},
            {{{ConditionType::EnemyVisible}},
             ConditionLogic::All,
             RuleAction::Chase}},
        std::vector<Rule>{}};
    assert(orderedRules.Evaluate(*lowHpRed, lowHpWithEnemyCreatures)
           == RuleAction::Stay);

    RuleSystem unmatchedRules{
        std::vector<Rule>{
            {{{ConditionType::FoodVisible}},
             ConditionLogic::All,
             RuleAction::SeekFood}},
        std::vector<Rule>{}};
    assert(unmatchedRules.Evaluate(
               *isolated, isolatedCreatures, &noFoods)
           == RuleAction::Stay);

    return 0;
}
