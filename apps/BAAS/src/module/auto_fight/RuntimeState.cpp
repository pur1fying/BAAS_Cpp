#include "module/auto_fight/RuntimeState.h"

#include <string>

BAAS_NAMESPACE_BEGIN

state_info::state_info(
        const std::string& state_name,
        const StateConfig& config,
        const WorkflowNameIndex& actions,
        const WorkflowNameIndex& conditions,
        const WorkflowNameIndex& states
) : desc(config.desc), name(state_name)
{
    if (config.action) {
        act_id = actions.at(*config.action);
    }
    for (const auto& transition : config.transitions) {
        transitions.push_back({conditions.at(transition->condition), states.at(transition->next_state)});
    }
    if (config.default_transition) {
        default_trans = states.at(*config.default_transition);
    }
    // Preserve the old executable: failure edges are validated but not installed.
    act_fail_trans = std::nullopt;
}

BAAS_NAMESPACE_END
