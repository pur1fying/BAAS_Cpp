#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOWROUTE_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOWROUTE_H_

#include <cstdint>
#include <optional>

#include "core_defines.h"

BAAS_NAMESPACE_BEGIN

enum class WorkflowConditionResult { PENDING, SATISFIED, DISSATISFIED };

struct WorkflowRouteResult {
    std::optional<std::uint64_t> condition_transition;
    bool all_conditions_dissatisfied = true;
};

template<class Range, class ConditionIndex, class StateIndex, class Evaluate>
WorkflowRouteResult select_baas_workflow_route(
        const Range& transitions,
        ConditionIndex condition_index,
        StateIndex state_index,
        Evaluate evaluate
)
{
    WorkflowRouteResult result;
    for (const auto& transition : transitions) {
        switch (evaluate(condition_index(transition))) {
            case WorkflowConditionResult::SATISFIED:
                result.condition_transition = state_index(transition);
                return result;
            case WorkflowConditionResult::PENDING:
                result.all_conditions_dissatisfied = false;
                break;
            case WorkflowConditionResult::DISSATISFIED:
                break;
        }
    }
    return result;
}

BAAS_NAMESPACE_END

#endif // BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOWROUTE_H_
