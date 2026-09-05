#include "RuleEditorUI.h"
#include "RuleSystem.h"

#include <cassert>

int main()
{
    {
        RuleEditorUI ui;
        RuleSystem rules;
        RuleEconomy economy;

        // 第一条规则从一个条件增加到两个；新增项会被自动选中。
        ui.Update({810.0F, 429.0F}, true, 0.0F, rules, economy);
        assert(rules.GetRules(CreatureFaction::Red)[0].conditions.size() == 2);
        assert(ui.GetSelectedConditionIndex() == 1);

        // 删除新增条件后，选中索引应回到最后一个合法条件。
        ui.Update({965.0F, 368.0F}, true, 0.0F, rules, economy);
        assert(rules.GetRules(CreatureFaction::Red)[0].conditions.size() == 1);
        assert(ui.GetSelectedConditionIndex() == 0);
    }

    {
        RuleEditorUI ui;
        RuleSystem rules;
        RuleEconomy economy;
        ui.Update({}, false, 0.0F, rules, economy);

        // 点击 P3 的 Up 后，UI 仍通过 ID 选中原来的规则。
        const RuleId movingId = rules.GetRules(CreatureFaction::Red)[2].id;
        ui.Update({950.0F, 170.0F}, true, 0.0F, rules, economy);
        assert(ui.GetSelectedRuleId() == movingId);
        assert(rules.FindRuleIndex(CreatureFaction::Red, movingId).value() == 1);

        // 新规则自动选中；删除最后一条后选中状态仍指向合法规则。
        ui.Update({810.0F, 590.0F}, true, 0.0F, rules, economy);
        const RuleId addedId = ui.GetSelectedRuleId();
        assert(rules.GetRules(CreatureFaction::Red).back().id == addedId);
        ui.Update({940.0F, 590.0F}, true, 0.0F, rules, economy);
        assert(rules.GetRuleById(
                   CreatureFaction::Red, ui.GetSelectedRuleId())
               != nullptr);
        assert(ui.GetSelectedConditionIndex() == 0);
    }

    return 0;
}
