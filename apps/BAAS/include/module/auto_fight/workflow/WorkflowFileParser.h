#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOW_WORKFLOWFILEPARSER_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOW_WORKFLOWFILEPARSER_H_

#include <filesystem>

#include "module/auto_fight/workflow/WorkflowParser.h"
#include "core_defines.h"

BAAS_NAMESPACE_BEGIN

WorkflowModel parse_workflow_file(
        const std::filesystem::path& path,
        const WorkflowParseOptions& options
);

BAAS_NAMESPACE_END

#endif // BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOW_WORKFLOWFILEPARSER_H_
