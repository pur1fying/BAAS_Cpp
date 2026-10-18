#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOW_CONDITIONPARSER_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOW_CONDITIONPARSER_H_

#include <optional>
#include <string>

#include <nlohmann/json.hpp>

#include "module/auto_fight/workflow/WorkflowParser.h"

BAAS_NAMESPACE_BEGIN

std::optional<ConditionConfig> parse_condition(
        const nlohmann::json& input, const std::string& path,
        const FormationConfig& formation,
        WorkflowModel& result, WorkflowParseErrorMode mode
);

BAAS_NAMESPACE_END

#endif // BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOW_CONDITIONPARSER_H_
