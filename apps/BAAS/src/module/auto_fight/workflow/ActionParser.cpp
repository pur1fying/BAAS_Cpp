#include "ActionParser.h"

#include <cstddef>
#include <optional>
#include <string>
#include <utility>

#include "ParseFields.h"
#include "module/auto_fight/workflow/WorkflowNames.h"

BAAS_NAMESPACE_BEGIN
namespace {

void parse_acc(
        const nlohmann::json& input, ActionStepConfig& config, const std::string& path,
        WorkflowModel& result, WorkflowParseErrorMode mode
)
{
    const auto* op = find_json_string_field(input, "op", true, path, result, mode);
    if (!op) {
        return;
    }
    const auto parsed = operation<AccOp>(*op, ACC_OP_NAMES, child_path(path, "op"), result, mode);
    if (parsed) {
        config.acc_op = *parsed;
    }
}

void parse_auto(
        const nlohmann::json& input, ActionStepConfig& config, const std::string& path,
        WorkflowModel& result, WorkflowParseErrorMode mode
)
{
    const auto* op = find_json_string_field(input, "op", true, path, result, mode);
    if (!op) {
        return;
    }
    const auto parsed = operation<AutoOp>(*op, AUTO_OP_NAMES, child_path(path, "op"), result, mode);
    if (parsed) {
        config.auto_op = *parsed;
    }
}

void parse_skill_target(
        const nlohmann::json& input, ActionStepConfig& config, const std::string& path,
        const FormationConfig& formation, WorkflowModel& result,
        WorkflowParseErrorMode mode
)
{
    const auto* target = find_json_object_field(input, "target", true, path, result, mode);
    if (!target) {
        return;
    }
    const auto target_path = child_path(path, "target");
    const auto* op = find_json_string_field(*target, "op", true, target_path, result, mode);
    if (!op) {
        return;
    }
    const auto parsed = operation<SkillTargetOp>(*op, TARGET_OP_NAMES,
            child_path(target_path, "op"), result, mode);
    if (!parsed) {
        return;
    }
    config.target_op = *parsed;
    if (config.target_op == SkillTargetOp::FIXED) {
        const auto* x = find_json_number_field(*target, "x", true, target_path, result, mode);
        const auto* y = find_json_number_field(*target, "y", true, target_path, result, mode);
        if (x) {
            config.fixed_x = json_number_to_int32_or_default(*x);
        }
        if (y) {
            config.fixed_y = json_number_to_int32_or_default(*y);
        }
        return;
    }
    const auto* objects = find_json_array_field(*target, "obj", true, target_path, result, mode);
    if (!objects) {
        return;
    }
    const auto objects_path = child_path(target_path, "obj");
    if (objects->empty()) {
        error(result, mode, "FIELD_RANGE", objects_path, "Target list cannot be empty.");
        return;
    }
    auto names = strings(*objects, objects_path, result, mode);
    if (!names) {
        return;
    }
    config.target_objects = std::move(*names);
    if (formation.front) {
        for (std::size_t i = 0; i < config.target_objects.size(); ++i) {
            const auto& name = config.target_objects[i];
            if (!contains(*formation.front, name)) {
                error(result, mode, "RESOURCE_NOT_FOUND", child_path(objects_path, std::to_string(i)),
                        "Target is not in formation.front.", name, *formation.front, "action.skill.resource");
            }
        }
    }
}

void parse_skill_check(
        const nlohmann::json& input, ActionStepConfig& config, const std::string& path,
        WorkflowModel& result, WorkflowParseErrorMode mode
)
{
    const auto* check = find_json_object_field(input, "check", false, path, result, mode);
    if (!check) {
        return;
    }
    const auto check_path = child_path(path, "check");
    const auto* op = find_json_string_field(*check, "op", true, check_path, result, mode);
    if (!op) {
        return;
    }
    if (*op == "none") {
        config.check_op = SkillCheckOp::NONE;
    } else if (*op == "C_decrease") {
        config.check_op = SkillCheckOp::COST_DECREASE;
        const auto* value = find_json_number_field(*check, "value", true, check_path, result, mode);
        if (value) {
            config.check_value = value->get<double>();
        }
        auto timeout = check->find("timeout");
        if (timeout != check->end()) {
            config.check_timeout = json_number_to_int64_or_default(*timeout, 5000);
        }
    } else {
        error(result, mode, "FIELD_ENUM", child_path(check_path, "op"),
                "Unknown skill check operation.", *op, {"none", "C_decrease"});
    }
}

void parse_skill(
        const nlohmann::json& input, ActionStepConfig& config, const std::string& path,
        const FormationConfig& formation, WorkflowModel& result,
        WorkflowParseErrorMode mode
)
{
    const auto* op = find_json_string_field(input, "op", true, path, result, mode);
    if (!op) {
        return;
    }
    if (*op == "auto") {
        config.release_op = SkillReleaseOp::AUTO;
    } else if (*op == "name") {
        config.release_op = SkillReleaseOp::NAME;
        const auto* name = find_json_string_field(input, "skill_n", true, path, result, mode);
        if (name) {
            config.skill_id = name->get<std::string>();
            if (formation.all_appeared_skills && !contains(*formation.all_appeared_skills, config.skill_id)) {
                error(result, mode, "RESOURCE_NOT_FOUND", child_path(path, "skill_n"),
                        "Skill is not in formation.all_appeared_skills.", *name,
                        *formation.all_appeared_skills, "action.skill.resource");
            }
        }
    } else if (*op == "l_rel_p") {
        config.release_op = SkillReleaseOp::LAST_RELEASE_POSITION;
        const auto* index = find_json_unsigned_integer_field(
                input, "l_rel_idx", true, path, result, mode);
        if (index) {
            config.last_release_index = json_number_to_int32_or_default(*index);
            if (config.last_release_index < 0 || config.last_release_index > 2) {
                error(result, mode, "FIELD_RANGE", child_path(path, "l_rel_idx"),
                        "Release index must narrow to 0, 1 or 2.", *index, {0, 1, 2});
            }
        }
    } else {
        error(result, mode, "FIELD_ENUM", child_path(path, "op"),
                "Unknown skill release operation.", *op, {"auto", "name", "l_rel_p"});
        return;
    }
    if (config.release_op != SkillReleaseOp::AUTO) {
        parse_skill_target(input, config, path, formation, result, mode);
    }
    parse_skill_check(input, config, path, result, mode);
}

} // namespace

std::optional<ActionStepConfig> parse_action_step(
        const nlohmann::json& input, const std::string& path,
        const FormationConfig& formation,
        WorkflowModel& result, WorkflowParseErrorMode mode
)
{
    if (stop_parsing(result, mode)) {
        return std::nullopt;
    }
    const auto begin = result.Errors.size();
    if (!input.is_object()) {
        error(result, mode, "FIELD_KIND", path, "Action step must be an object.", input, "object");
        return std::nullopt;
    }
    const auto* type = find_json_string_field(input, "t", true, path, result, mode);
    if (!type) {
        return std::nullopt;
    }
    ActionStepConfig config;
    config.desc = json_string_or_default(input, "desc");
    if (*type == "acc") {
        config.type = ActionType::ACC;
        parse_acc(input, config, path, result, mode);
    } else if (*type == "auto") {
        config.type = ActionType::AUTO;
        parse_auto(input, config, path, result, mode);
    } else if (*type == "restart") {
        config.type = ActionType::RESTART;
    } else if (*type == "skip_animation") {
        config.type = ActionType::SKIP_ANIMATION;
        config.skip_animation_timeout = json_number_or_default(input, "timeout", 3.0);
    } else if (*type == "skill") {
        config.type = ActionType::SKILL;
        parse_skill(input, config, path, formation, result, mode);
    } else {
        error(result, mode, "TYPE_NOT_DEFINED", child_path(path, "t"),
                "Unknown action type.", *type, nullptr, "action.type");
    }
    return errors_since(result, begin) ? std::nullopt :
            std::optional<ActionStepConfig>(std::move(config));
}

BAAS_NAMESPACE_END
