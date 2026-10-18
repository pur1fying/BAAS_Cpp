#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOW_WORKFLOWBASICINFOPARSER_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOW_WORKFLOWBASICINFOPARSER_H_

#include <nlohmann/json.hpp>

#include "module/auto_fight/workflow/WorkflowParser.h"

BAAS_NAMESPACE_BEGIN

void parse_workflow_basic_info(
        const nlohmann::json& input, WorkflowModel& result,
        const WorkflowParseOptions& options
);

BAAS_NAMESPACE_END

#endif // BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOW_WORKFLOWBASICINFOPARSER_H_
