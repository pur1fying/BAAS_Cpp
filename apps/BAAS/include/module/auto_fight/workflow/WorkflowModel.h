#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOW_WORKFLOWMODEL_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOW_WORKFLOWMODEL_H_

#include <array>
#include <cstddef>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "module/auto_fight/workflow/ActionConfig.h"
#include "module/auto_fight/workflow/ConditionConfig.h"
#include "module/auto_fight/workflow/StateConfig.h"
#include "core_defines.h"

BAAS_NAMESPACE_BEGIN

enum class DiagnosticSeverity { ERROR_DIAGNOSTIC, WARNING_DIAGNOSTIC };

struct WorkflowErrors {
    DiagnosticSeverity severity = DiagnosticSeverity::ERROR_DIAGNOSTIC;
    std::string code;
    std::string rule_id;
    std::string path; // RFC 6901 path within the input document.
    std::string message;
    nlohmann::json actual;
    nlohmann::json expected;
    std::vector<std::string> related_paths;
    std::string source_file;
    std::optional<std::size_t> byte_offset;
};

enum class NmsType { GROUP, WHOLE, NONE };
using OcrRegion = std::array<int, 4>;

struct FormationConfig {
    std::optional<std::vector<std::string>> all_appeared_skills;
    std::optional<std::int32_t> slot_count;
    std::optional<std::vector<std::string>> front;
};

struct YoloConfig {
    std::string model = "best.onnx";
    NmsType nms_type = NmsType::GROUP;
    std::int64_t update_interval = 100;
};

struct RecognitionConfig {
    OcrRegion current_region{};
    OcrRegion max_region{};
    OcrRegion region{};
    std::string model;
    std::uint8_t time_format = 0;
};

// An entry may exist without a parsed value. References remain source names.
struct WorkflowModel {
    std::optional<std::string> battle_type;
    FormationConfig formation;
    std::optional<YoloConfig> yolo;
    std::optional<RecognitionConfig> boss_health;
    std::optional<RecognitionConfig> battle_time;
    std::optional<std::string> start_state;
    std::map<std::string, std::optional<ActionConfig>> actions;
    std::map<std::string, std::optional<ConditionConfig>> conditions;
    std::map<std::string, std::optional<StateConfig>> states;
    std::vector<WorkflowErrors> Errors;

    bool has_errors() const;
};

BAAS_NAMESPACE_END

#endif // BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOW_WORKFLOWMODEL_H_
