#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOW_STATECONFIG_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOW_STATECONFIG_H_

#include <optional>
#include <string>
#include <vector>

#include "core_defines.h"

BAAS_NAMESPACE_BEGIN

struct TransitionConfig {
    std::string condition;
    std::string next_state;
};

struct StateConfig {
    std::optional<std::string> action;
    std::vector<std::optional<TransitionConfig>> transitions;
    std::optional<std::string> default_transition;
    std::optional<std::string> action_fail_transition;
    std::string desc;
};

BAAS_NAMESPACE_END

#endif // BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOW_STATECONFIG_H_
