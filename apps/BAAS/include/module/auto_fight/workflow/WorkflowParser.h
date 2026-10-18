#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOW_WORKFLOWPARSER_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOW_WORKFLOWPARSER_H_

#include <string>

#include "module/auto_fight/workflow/WorkflowModel.h"
#include "core_defines.h"

BAAS_NAMESPACE_BEGIN

enum class WorkflowParseErrorMode { FIRST, ALL };

struct WorkflowParseOptions {
    std::string EmulatorServer;
    std::string EmulatorLanguage;
    WorkflowParseErrorMode error_mode = WorkflowParseErrorMode::FIRST;
};

WorkflowModel parse_workflow(
        const std::string& json,
        const WorkflowParseOptions& options
);

BAAS_NAMESPACE_END

#endif // BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOW_WORKFLOWPARSER_H_
