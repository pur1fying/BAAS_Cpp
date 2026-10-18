#include "StateParser.h"

#include <cstddef>
#include <optional>
#include <string>
#include <utility>

#include "ParseFields.h"

BAAS_NAMESPACE_BEGIN
namespace {

template<class Map>
void reference(
        const Map& names, const std::string& group, const std::string& name, const std::string& path,
        WorkflowModel& result, WorkflowParseErrorMode mode
)
{
    for (const auto& diagnostic : result.Errors) {
        if (diagnostic.path == group && diagnostic.severity == DiagnosticSeverity::ERROR_DIAGNOSTIC) {
            return;
        }
    }
    if (!names.contains(name)) {
        error(result, mode, "REFERENCE_NOT_FOUND", path, "Referenced name is not defined.",
                name, nullptr, "workflow.reference");
    }
}

} // namespace

std::optional<StateConfig> parse_state(
        const nlohmann::json& input, const std::string& path,
        WorkflowModel& result, WorkflowParseErrorMode mode
)
{
    if (!input.is_object()) {
        error(result, mode, "FIELD_KIND", path, "State must be an object.", input, "object");
        return std::nullopt;
    }
    StateConfig state;
    const auto* action = find_json_string_field(input, "action", false, path, result, mode);
    if (action) {
        state.action = action->get<std::string>();
        reference(result.actions, "/actions", *state.action, child_path(path, "action"), result, mode);
    }
    const auto* transitions = find_json_array_field(input, "transitions", false, path, result, mode);
    if (transitions) {
        state.transitions.resize(transitions->size());
        for (std::size_t i = 0; i < transitions->size() && !stop_parsing(result, mode); ++i) {
            const auto transition_path = child_path(child_path(path, "transitions"), std::to_string(i));
            const auto& item = (*transitions)[i];
            if (!item.is_object()) {
                error(result, mode, "FIELD_KIND", transition_path, "Transition must be an object.", item, "object");
                continue;
            }
            TransitionConfig transition;
            const auto* condition = find_json_string_field(
                    item, "condition", true, transition_path, result, mode);
            if (condition) {
                transition.condition = condition->get<std::string>();
                reference(result.conditions, "/conditions", transition.condition,
                        child_path(transition_path, "condition"), result, mode);
            }
            const auto* next = find_json_string_field(
                    item, "next", true, transition_path, result, mode);
            if (next) {
                transition.next_state = next->get<std::string>();
                reference(result.states, "/states", transition.next_state,
                        child_path(transition_path, "next"), result, mode);
            }
            // Keep locally parsed names even if their references failed.
            if (condition && next) {
                state.transitions[i] = std::move(transition);
            }
        }
    }
    const auto* fallback = find_json_string_field(
            input, "default_transition", false, path, result, mode);
    if (fallback) {
        state.default_transition = fallback->get<std::string>();
        reference(result.states, "/states", *state.default_transition,
                child_path(path, "default_transition"), result, mode);
    }
    const auto* failure = find_json_string_field(
            input, "action_fail_transition", false, path, result, mode);
    if (failure) {
        state.action_fail_transition = failure->get<std::string>();
        reference(result.states, "/states", *state.action_fail_transition,
                child_path(path, "action_fail_transition"), result, mode);
    }
    state.desc = json_string_or_default(input, "desc");
    return state;
}

BAAS_NAMESPACE_END
