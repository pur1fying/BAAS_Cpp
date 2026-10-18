#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOW_STATEPARSER_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOW_STATEPARSER_H_

#include <optional>
#include <string>

#include <nlohmann/json.hpp>

#include "module/auto_fight/workflow/WorkflowParser.h"

BAAS_NAMESPACE_BEGIN

std::optional<StateConfig> parse_state(
        const nlohmann::json& input, const std::string& path,
        WorkflowModel& result, WorkflowParseErrorMode mode
);

BAAS_NAMESPACE_END

#endif // BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOW_STATEPARSER_H_
