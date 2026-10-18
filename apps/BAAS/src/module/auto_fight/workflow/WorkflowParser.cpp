#include "module/auto_fight/workflow/WorkflowParser.h"

#include <cstddef>
#include <string>

#include <nlohmann/json.hpp>

#include "ActionParser.h"
#include "ConditionParser.h"
#include "ParseFields.h"
#include "WorkflowBasicInfoParser.h"
#include "StateParser.h"
#include "WorkflowGraph.h"

BAAS_NAMESPACE_BEGIN

WorkflowModel parse_workflow(
        const std::string& json,
        const WorkflowParseOptions& options
)
{
    WorkflowModel result;
    nlohmann::json input;
    try {
        // Default json preserves the executable's lexical named-object order.
        input = nlohmann::json::parse(json.begin(), json.end());
    } catch (const nlohmann::json::parse_error& exception) {
        result.Errors.push_back({.code = "JSON_PARSE_ERROR", .rule_id = "workflow.json",
                .message = exception.what(), .byte_offset = exception.byte - 1});
        return result;
    } catch (const nlohmann::json::exception& exception) {
        result.Errors.push_back({.code = "JSON_PARSE_ERROR", .rule_id = "workflow.json",
                .message = exception.what()});
        return result;
    }
    const auto mode = options.error_mode;
    if (!input.is_object()) {
        error(result, mode, "OBJECT_KIND", "", "Workflow must be an object.", input, "object");
        return result;
    }
    parse_workflow_basic_info(input, result, options);
    if (stop_parsing(result, mode)) {
        return result;
    }

    const auto* actions = find_json_object_field(
            input, "actions", true, "", result, mode);
    if (actions) {
        for (const auto& [name, value] : actions->items()) {
            result.actions.emplace(name, std::nullopt);
            if (!value.is_array()) {
                error(result, mode, "FIELD_KIND", child_path("/actions", name),
                        "Named action must be an array.", value, "array");
            }
        }
    }
    const auto* conditions = find_json_object_field(
            input, "conditions", true, "", result, mode);
    if (conditions) {
        for (const auto& [name, value] : conditions->items()) {
            result.conditions.emplace(name, std::nullopt);
            if (!value.is_object()) {
                error(result, mode, "FIELD_KIND", child_path("/conditions", name),
                        "Named condition must be an object.", value, "object");
            }
        }
    }
    const auto* states = find_json_object_field(
            input, "states", true, "", result, mode);
    if (states) {
        for (const auto& [name, value] : states->items()) {
            result.states.emplace(name, std::nullopt);
            if (!value.is_object()) {
                error(result, mode, "FIELD_KIND", child_path("/states", name),
                        "Named state must be an object.", value, "object");
            }
        }
    }
    if (stop_parsing(result, mode)) {
        return result;
    }
    if (conditions) {
        for (const auto& [name, value] : conditions->items()) {
            if (stop_parsing(result, mode)) {
                return result;
            }
            if (value.is_object()) {
                result.conditions[name] = parse_condition(value, child_path("/conditions", name),
                        result.formation, result, mode);
            }
        }
    }
    if (actions) {
        for (const auto& [name, value] : actions->items()) {
            if (stop_parsing(result, mode)) {
                return result;
            }
            if (!value.is_array()) {
                continue;
            }
            auto& action = result.actions[name].emplace();
            action.steps.resize(value.size());
            for (std::size_t i = 0; i < value.size() && !stop_parsing(result, mode); ++i) {
                action.steps[i] = parse_action_step(value[i],
                        child_path(child_path("/actions", name), std::to_string(i)),
                        result.formation, result, mode);
            }
        }
    }
    const auto* start = find_json_string_field(
            input, "start_state", true, "", result, mode);
    if (start) {
        result.start_state = start->get<std::string>();
        if (states && !result.states.contains(*result.start_state)) {
            error(result, mode, "REFERENCE_NOT_FOUND", "/start_state",
                    "Start state is not defined.", *start, nullptr, "workflow.reference");
        }
    }
    if (states) {
        for (const auto& [name, value] : states->items()) {
            if (stop_parsing(result, mode)) {
                return result;
            }
            if (value.is_object()) {
                result.states[name] = parse_state(value, child_path("/states", name), result, mode);
            }
        }
    }
    if (!stop_parsing(result, mode)) {
        validate_graph(result, mode);
    }
    return result;
}

BAAS_NAMESPACE_END
