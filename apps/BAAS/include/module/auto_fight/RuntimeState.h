#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_RUNTIMESTATE_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_RUNTIMESTATE_H_

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "module/auto_fight/workflow/StateConfig.h"

BAAS_NAMESPACE_BEGIN

using WorkflowNameIndex = std::map<std::string, std::uint64_t>;

struct trans_info {
    std::uint64_t cond_idx;
    std::uint64_t next_sta_idx;
};

struct state_info {
    std::optional<std::uint64_t> act_id;
    std::vector<trans_info> transitions;
    std::optional<std::uint64_t> default_trans;
    std::optional<std::uint64_t> act_fail_trans;
    std::string desc;
    std::string name;

    state_info() = default;
    state_info(
            const std::string& state_name,
            const StateConfig& config,
            const WorkflowNameIndex& actions,
            const WorkflowNameIndex& conditions,
            const WorkflowNameIndex& states
    );
};

BAAS_NAMESPACE_END

#endif // BAAS_CPP_MODULE_AUTO_FIGHT_RUNTIMESTATE_H_
