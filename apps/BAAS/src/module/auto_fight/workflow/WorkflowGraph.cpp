#include "WorkflowGraph.h"

#include <cstddef>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "ParseFields.h"

BAAS_NAMESPACE_BEGIN

void validate_graph(WorkflowModel& result, WorkflowParseErrorMode mode)
{
    std::map<std::string, std::vector<std::string>> edges;
    std::set<std::string> used_conditions;
    for (const auto& [name, condition] : result.conditions) {
        if (!condition || stop_parsing(result, mode)) {
            continue;
        }
        auto& dependencies = edges[name];
        for (const auto& [key, references] : {
                std::pair{std::string{"and"}, &condition->and_conditions},
                std::pair{std::string{"or"}, &condition->or_conditions}}) {
            for (std::size_t i = 0; i < references->size(); ++i) {
                const auto& reference = (*references)[i];
                const auto path = child_path(child_path(child_path("/conditions", name), key),
                        std::to_string(i));
                const auto found = result.conditions.find(reference);
                if (found == result.conditions.end()) {
                    error(result, mode, "REFERENCE_NOT_FOUND", path,
                            "Referenced condition is not defined.", reference, nullptr, "workflow.reference");
                } else if (found->second) {
                    dependencies.push_back(reference);
                    used_conditions.insert(reference);
                }
            }
        }
    }

    // Iterative DFS: cycles in condition dependencies are invalid; state cycles are allowed.
    std::map<std::string, int> color;
    for (const auto& [name, ignored] : edges) {
        if (color[name] != 0 || stop_parsing(result, mode)) {
            continue;
        }
        std::vector<std::pair<std::string, std::size_t>> pending{{name, 0}};
        color[name] = 1;
        while (!pending.empty() && !stop_parsing(result, mode)) {
            auto& [node, index] = pending.back();
            const auto& children = edges.at(node);
            if (index == children.size()) {
                color[node] = 2;
                pending.pop_back();
                continue;
            }
            const std::string next = children[index++];
            if (color[next] == 1) {
                error(result, mode, "CONDITION_DEPENDENCY_CYCLE", child_path("/conditions", node),
                        "Condition dependency cycle.", next, nullptr, "condition.dependency");
                result.Errors.back().related_paths.push_back(child_path("/conditions", next));
            } else if (color[next] == 0) {
                color[next] = 1;
                pending.emplace_back(next, 0);
            }
        }
    }
    // Reachability warnings require a complete valid graph, not guessed missing units.
    if (result.has_errors() || !result.start_state) {
        return;
    }
    std::set<std::string> reached;
    std::set<std::string> used_actions;
    std::vector<std::string> pending{*result.start_state};
    while (!pending.empty()) {
        const auto name = std::move(pending.back());
        pending.pop_back();
        if (!reached.insert(name).second) {
            continue;
        }
        const auto& state = *result.states.at(name);
        for (const auto& transition : state.transitions) {
            pending.push_back(transition->next_state);
        }
        if (state.default_transition) {
            pending.push_back(*state.default_transition);
        }
        if (state.action_fail_transition) {
            pending.push_back(*state.action_fail_transition);
        }
    }
    for (const auto& [name, state] : result.states) {
        if (state->action) {
            used_actions.insert(*state->action);
        }
        for (const auto& transition : state->transitions) {
            used_conditions.insert(transition->condition);
        }
        if (!reached.contains(name)) {
            warning(result, "STATE_UNREACHABLE", child_path("/states", name),
                    "State cannot be reached from start_state.");
        }
    }
    for (const auto& [name, ignored] : result.actions) {
        if (!used_actions.contains(name)) {
            warning(result, "ACTION_UNREFERENCED", child_path("/actions", name), "Action is not referenced.");
        }
    }
    for (const auto& [name, ignored] : result.conditions) {
        if (!used_conditions.contains(name)) {
            warning(result, "CONDITION_UNREFERENCED", child_path("/conditions", name), "Condition is not referenced.");
        }
    }
}

BAAS_NAMESPACE_END
