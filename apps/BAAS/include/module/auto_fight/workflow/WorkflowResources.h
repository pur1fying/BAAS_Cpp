#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOW_WORKFLOWRESOURCES_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOW_WORKFLOWRESOURCES_H_

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "module/auto_fight/workflow/WorkflowModel.h"
#include "core_defines.h"

BAAS_NAMESPACE_BEGIN

struct SkillTemplates {
    std::vector<std::string> active;
    std::vector<std::string> inactive;
};

struct ModelResource {
    std::string precision;
    std::string yaml_path;
    std::vector<std::string> classes;
};

struct WorkflowResources {
    std::string identity;
    std::map<std::string, std::vector<std::string>> server_languages;
    std::map<std::string, std::map<std::string, SkillTemplates>> skills;
    std::map<std::string, ModelResource> models;
    std::vector<std::string> ocr_models;
    std::vector<OcrRegion> skill_cost_regions;
    std::string skill_cost_ocr_model;
    RecognitionConfig boss_health;
    std::vector<OcrRegion> time_regions;
    std::string time_ocr_model;
    std::uint8_t default_time_format = 0;
};

// Defined by the build-generated WorkflowResources.cpp.
const WorkflowResources& workflow_resources();

BAAS_NAMESPACE_END

#endif // BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOW_WORKFLOWRESOURCES_H_
