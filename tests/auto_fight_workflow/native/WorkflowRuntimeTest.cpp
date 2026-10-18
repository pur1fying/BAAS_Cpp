#include "module/auto_fight/RuntimeState.h"

#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <vector>

#include "module/auto_fight/WorkflowRoute.h"
#include "module/auto_fight/conditions/condition_runtime_evaluator.h"

namespace {

void require(bool value)
{
    if (!value) {
        throw std::runtime_error("Runtime compatibility failure");
    }
}

class FixedCondition : public baas::BaseCondition {
public:
    explicit FixedCondition(std::optional<bool> value) : _value(value)
    {
    }
    std::optional<bool> try_match() override
    {
        return _value;
    }
private:
    std::optional<bool> _value;
};

} // namespace

int main()
{
    using namespace baas;
    StateConfig config;
    config.action = "act";
    config.transitions = {TransitionConfig{"c2", "end"}, TransitionConfig{"c1", "start"}};
    config.default_transition = "end";
    config.action_fail_transition = "end";
    const state_info state("start", config, {{"act", 0}}, {{"c1", 0}, {"c2", 1}},
            {{"end", 0}, {"start", 1}});
    require(state.act_id == 0 && state.default_trans == 0 && !state.act_fail_trans);
    require(state.transitions.size() == 2 && state.transitions[0].cond_idx == 1 &&
            state.transitions[0].next_sta_idx == 0 && state.transitions[1].next_sta_idx == 1);
    const auto route = select_baas_workflow_route(state.transitions,
            [](const trans_info& item) { return item.cond_idx; },
            [](const trans_info& item) { return item.next_sta_idx; },
            [](std::uint64_t) { return WorkflowConditionResult::SATISFIED; });
    require(route.condition_transition == 0);
    const auto pending = select_baas_workflow_route(state.transitions,
            [](const trans_info& item) { return item.cond_idx; },
            [](const trans_info& item) { return item.next_sta_idx; },
            [](std::uint64_t) { return WorkflowConditionResult::PENDING; });
    require(!pending.condition_transition && !pending.all_conditions_dissatisfied);

    std::vector<std::unique_ptr<BaseCondition>> conditions;
    conditions.push_back(std::make_unique<FixedCondition>(true));
    conditions.push_back(std::make_unique<FixedCondition>(false));
    conditions[0]->and_conditions = {1};
    std::vector<std::optional<bool>> matched(2), self_matched(2);
    std::vector<bool> checked(2);
    require(evaluate_baas_condition_recursive(conditions, 0, matched, self_matched, checked) == false);
    // Preserve the old evaluator's observed caching assignment, not a repaired policy.
    require(self_matched[1] == true);
    std::cout << "Production state binding, transition order and recursive evaluation passed.\n";
}
