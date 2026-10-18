#include "WorkflowBasicInfoParser.h"

#include <cstddef>
#include <cstdint>
#include <set>
#include <string>
#include <utility>

#include "ParseFields.h"
#include "module/auto_fight/workflow/WorkflowResources.h"

BAAS_NAMESPACE_BEGIN
namespace {

void parse_ocr_region(
        const nlohmann::json& object,
        const std::string& field_name,
        OcrRegion& output,
        const std::string& path,
        WorkflowModel& result,
        WorkflowParseErrorMode mode
)
{
    const auto* value = find_json_array_field(object, field_name, false, path, result, mode);
    if (!value) {
        return;
    }
    const auto location = child_path(path, field_name);
    if (value->size() != 4) {
        error(result, mode, "FIELD_RANGE", location, "OCR region requires four coordinates.", *value, 4);
        return;
    }
    for (std::size_t i = 0; i < 4; ++i) {
        const auto& coordinate = (*value)[i];
        // Original BAASRectangle uses json.get<int>(), which also accepts bool.
        if (coordinate.is_boolean()) {
            output[i] = coordinate.get<bool>() ? 1 : 0;
        } else if (coordinate.is_number()) {
            output[i] = json_number_to_int32_or_default(coordinate);
        } else {
            error(result, mode, "FIELD_KIND", child_path(location, std::to_string(i)),
                    "OCR coordinate must convert to an integer.", coordinate, "number");
        }
    }
}

void parse_ocr_config(
        const nlohmann::json& input,
        const std::string& section_name,
        RecognitionConfig& config,
        WorkflowModel& result,
        WorkflowParseErrorMode mode
)
{
    const auto object = input.find(section_name);
    if (object == input.end() || !object->is_object()) {
        // Legacy JSON-pointer lookup falls back when an optional section has no fields.
        return;
    }
    const auto path = child_path("", section_name);
    if (section_name == "BossHealth") {
        parse_ocr_region(*object, "current_ocr_region", config.current_region, path, result, mode);
        parse_ocr_region(*object, "max_ocr_region", config.max_region, path, result, mode);
    }
    parse_ocr_region(*object, "ocr_region", config.region, path, result, mode);
    const auto* model = find_json_string_field(*object, "ocr_model_name", false, path, result, mode);
    if (model) {
        config.model = model->get<std::string>();
    }
    if (!contains(workflow_resources().ocr_models, config.model)) {
        error(result, mode, "RESOURCE_NOT_FOUND", child_path(path, "ocr_model_name"),
                "OCR model is not in the embedded resources.", config.model);
    }
}

void parse_formation(
        const nlohmann::json& input, WorkflowModel& result,
        const WorkflowParseOptions& options
)
{
    const auto mode = options.error_mode;
    auto& formation = result.formation;
    const auto* object = find_json_object_field(input, "formation", true, "", result, mode);
    if (!object) {
        return;
    }
    const auto* names = find_json_array_field(
            *object, "all_appeared_skills", true, "/formation", result, mode);
    if (names) {
        formation.all_appeared_skills = strings(*names, "/formation/all_appeared_skills", result, mode);
    }
    if (formation.all_appeared_skills) {
        std::set<std::string> unique;
        const auto& resources = workflow_resources();
        const auto profile = resources.skills.find(options.EmulatorServer + "." + options.EmulatorLanguage);
        for (std::size_t i = 0; i < formation.all_appeared_skills->size(); ++i) {
            const auto& name = (*formation.all_appeared_skills)[i];
            const auto path = child_path("/formation/all_appeared_skills", std::to_string(i));
            if (!unique.insert(name).second) {
                error(result, mode, "DUPLICATE_SKILL", path, "Duplicate skill name.", name);
            } else if (profile != resources.skills.end() && !profile->second.contains(name)) {
                error(result, mode, "RESOURCE_NOT_FOUND", path,
                        "No skill template for this server and language.", name,
                        nullptr, "workflow.resource");
            }
        }
    }
    const auto slot = object->find("slot_count");
    formation.slot_count = slot == object->end() ? 3 : json_number_to_int32_or_default(*slot, 3);
    const auto cost_regions = workflow_resources().skill_cost_regions.size();
    if (*formation.slot_count < 0 || static_cast<std::size_t>(*formation.slot_count) != cost_regions) {
        error(result, mode, "FIELD_VALUE", "/formation/slot_count",
                "Slot count must match the packaged skill-cost OCR regions.",
                *formation.slot_count, cost_regions, "workflow.resource");
    }
    const auto* front = find_json_array_field(
            *object, "front", true, "/formation", result, mode);
    if (front) {
        formation.front = strings(*front, "/formation/front", result, mode);
    }
}

} // namespace

void parse_workflow_basic_info(
        const nlohmann::json& input, WorkflowModel& result,
        const WorkflowParseOptions& options
)
{
    const auto mode = options.error_mode;
    const auto& resources = workflow_resources();
    const auto server = resources.server_languages.find(options.EmulatorServer);
    if (server == resources.server_languages.end()) {
        error(result, mode, "SERVER_NOT_SUPPORTED", "", "Unknown EmulatorServer.",
                options.EmulatorServer, nullptr, "workflow.options");
    } else if (!contains(server->second, options.EmulatorLanguage)) {
        error(result, mode, "LANGUAGE_NOT_SUPPORTED", "", "Unknown EmulatorLanguage for server.",
                options.EmulatorLanguage, server->second, "workflow.options");
    }
    if (stop_parsing(result, mode)) {
        return;
    }
    result.battle_type = json_string_or_default(input, "battle_type", "normal");
    if (*result.battle_type != "normal" && *result.battle_type != "infinite_assault") {
        error(result, mode, "FIELD_ENUM", "/battle_type", "Unknown battle type.",
                *result.battle_type, {"normal", "infinite_assault"});
    }
    parse_formation(input, result, options);
    if (stop_parsing(result, mode)) {
        return;
    }

    result.boss_health = resources.boss_health;
    parse_ocr_config(input, "BossHealth", *result.boss_health, result, mode);

    YoloConfig yolo;
    auto setting = input.find("yolo_setting");
    if (setting != input.end() && setting->is_object()) {
        yolo.model = json_string_or_default(*setting, "model", "best.onnx");
        const auto nms = json_string_or_default(*setting, "nms_type", "group");
        if (nms == "group") {
            yolo.nms_type = NmsType::GROUP;
        } else if (nms == "whole") {
            yolo.nms_type = NmsType::WHOLE;
        } else if (nms == "no") {
            yolo.nms_type = NmsType::NONE;
        } else {
            error(result, mode, "FIELD_ENUM", "/yolo_setting/nms_type", "Unknown NMS type.",
                    nms, {"group", "whole", "no"});
        }
        auto interval = setting->find("update_interval");
        if (interval != setting->end()) {
            yolo.update_interval = json_number_to_int64_or_default(*interval, 100);
        }
    }
    result.yolo = yolo;
    const auto selected = resources.models.find(yolo.model);
    if (selected == resources.models.end()) {
        error(result, mode, "RESOURCE_NOT_FOUND", "/yolo_setting/model",
                "YOLO model is not in embedded resources.", yolo.model);
    } else if (result.formation.front) {
        for (std::size_t i = 0; i < result.formation.front->size(); ++i) {
            const auto& name = (*result.formation.front)[i];
            if (!contains(selected->second.classes, name)) {
                error(result, mode, "RESOURCE_NOT_FOUND", child_path("/formation/front", std::to_string(i)),
                        "Object is not a class in the selected YOLO result.", name, selected->second.classes);
            }
        }
    }

    RecognitionConfig time;
    time.model = resources.time_ocr_model;
    time.time_format = resources.default_time_format;
    // Historical defect: presence is checked under BattleTime, but the value
    // is read from BossHealth/current_ocr_region. Fix separately from this refactor.
    const auto section = input.find("BattleTime");
    if (section != input.end() && section->is_object() && section->contains("time_format")) {
        time.time_format = 0;
        const auto health = input.find("BossHealth");
        if (health != input.end() && health->is_object()) {
            const auto value = health->find("current_ocr_region");
            if (value != health->end()) {
                if (value->is_boolean()) {
                    time.time_format = value->get<bool>() ? 1 : 0;
                } else {
                    time.time_format = static_cast<std::uint8_t>(json_number_to_int32_or_default(*value, 0));
                }
            }
        }
    }
    if (time.time_format >= resources.time_regions.size()) {
        error(result, mode, "FIELD_RANGE", "/BattleTime/time_format", "Unsupported time format.",
                time.time_format, {0, 1});
    } else {
        time.region = resources.time_regions[time.time_format];
    }
    parse_ocr_config(input, "BattleTime", time, result, mode);
    result.battle_time = std::move(time);
}

BAAS_NAMESPACE_END
