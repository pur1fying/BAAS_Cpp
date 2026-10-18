#include "ConditionParser.h"

#include <bit>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "ParseFields.h"
#include "module/auto_fight/workflow/WorkflowNames.h"

BAAS_NAMESPACE_BEGIN
namespace {

std::vector<std::string> read_condition_dependencies_or_empty(const nlohmann::json& object, const std::string& key)
{
    const auto it = object.find(key);
    if (it == object.end() || !it->is_array()) {
        return {};
    }
    std::vector<std::string> output;
    for (const auto& item : *it) {
        if (!item.is_string()) {
            return {};
        }
        output.push_back(item.get<std::string>());
    }
    return output;
}

void parse_numeric(
        const nlohmann::json& input, const std::string& path, ConditionConfig& config,
        WorkflowModel& result, WorkflowParseErrorMode mode
)
{
        const auto* op = find_json_string_field(input, "op", true, path, result, mode);
    if (!op) {
        return;
    }
    bool range = false;
    if (config.type == ConditionType::BOSS_HEALTH) {
        const auto parsed = operation<BossHealthConditionOp>(*op, HEALTH_OP_NAMES,
                child_path(path, "op"), result, mode, "FIELD_VALUE");
        if (!parsed) {
            return;
        }
        config.health_op = *parsed;
        range = config.health_op == BossHealthConditionOp::CURRENT_IN_RANGE;
    } else {
        const auto parsed = operation<NumericConditionOp>(*op, NUMERIC_OP_NAMES,
                child_path(path, "op"), result, mode, "FIELD_VALUE");
        if (!parsed) {
            return;
        }
        config.numeric_op = *parsed;
        range = config.numeric_op == NumericConditionOp::IN_RANGE;
    }
    if (!range) {
        const auto* value = find_json_number_field(input, "value", true, path, result, mode);
        if (value) {
            if (config.type == ConditionType::BATTLE_TIME) {
                config.time_value = json_number_to_int32_or_default(*value);
            } else {
                config.value = value->get<double>();
            }
        }
        return;
    }
        const auto* values = find_json_array_field(input, "range", true, path, result, mode);
    if (!values) {
        return;
    }
    if (values->size() != 2) {
        error(result, mode, "FIELD_VALUE", child_path(path, "range"),
                "Range must contain exactly two numbers.", *values, 2);
        return;
    }
    for (std::size_t i = 0; i < 2; ++i) {
        const auto& value = (*values)[i];
        if (!value.is_number()) {
            error(result, mode, "FIELD_KIND", child_path(child_path(path, "range"), std::to_string(i)),
                    "Range element must be numeric.", value, "number");
        } else if (config.type == ConditionType::BATTLE_TIME) {
            (i == 0 ? config.time_range_min : config.time_range_max) = json_number_to_int32_or_default(value);
        } else {
            (i == 0 ? config.range_min : config.range_max) = value.get<double>();
        }
    }
}

void parse_skill_name(
        const nlohmann::json& input, const std::string& path, ConditionConfig& config,
        const FormationConfig& formation, WorkflowModel& result,
        WorkflowParseErrorMode mode
)
{
    const auto* op = find_json_string_field(input, "op", true, path, result, mode);
    if (!op) {
        return;
    }
    const auto parsed = operation<SkillNameConditionOp>(*op, SKILL_NAME_OP_NAMES,
            child_path(path, "op"), result, mode, "FIELD_VALUE");
    if (!parsed) {
        return;
    }
    config.skill_op = *parsed;
    const bool at = config.skill_op == SkillNameConditionOp::AT ||
                    config.skill_op == SkillNameConditionOp::AT_READY;
    const nlohmann::json* position = nullptr;
    if (at) {
        position = find_json_unsigned_integer_field(input, "p", true, path, result, mode);
        if (position) {
            config.position = position->get<std::uint64_t>();
        }
    }
    const auto* name = find_json_string_field(input, "name", true, path, result, mode);
    if (name) {
        config.skill_name = name->get<std::string>();
    }
    if (position && formation.slot_count) {
        const auto last = std::bit_cast<std::int32_t>(
                static_cast<std::uint32_t>(*formation.slot_count) - std::uint32_t{1});
        const auto bound = static_cast<std::uint64_t>(static_cast<std::int64_t>(last));
        if (config.position > bound) {
            error(result, mode, "FIELD_VALUE", child_path(path, "p"),
                    "Position exceeds the configured skill slots.", *position, bound);
        }
    }
    if (name && formation.all_appeared_skills &&
        !contains(*formation.all_appeared_skills, config.skill_name))
        {
        error(result, mode, "RESOURCE_NOT_FOUND", child_path(path, "name"),
                "Skill is not in formation.all_appeared_skills.", *name,
                *formation.all_appeared_skills, "condition.skill.resource");
    }
}

} // namespace

std::optional<ConditionConfig> parse_condition(
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
        error(result, mode, "FIELD_KIND", path, "Condition must be an object.", input, "object");
        return std::nullopt;
    }
    const auto* type = find_json_string_field(input, "type", true, path, result, mode);
    if (!type) {
        return std::nullopt;
    }
    const bool unsupported = *type == "skill_cost" || *type == "acc_phase" || *type == "auto_state";
    const auto parsed = operation<ConditionType>(*type, CONDITION_NAMES,
            child_path(path, "type"), result, mode,
            unsupported ? "TYPE_NOT_EXECUTABLE" : "TYPE_NOT_DEFINED");
    if (!parsed) {
        return std::nullopt;
    }
    ConditionConfig config;
    config.type = *parsed;
    config.desc = json_string_or_default(input, "desc");
    config.and_conditions = read_condition_dependencies_or_empty(input, "and");
    config.or_conditions = read_condition_dependencies_or_empty(input, "or");
    const auto timeout = input.find("timeout");
    if (timeout != input.end()) {
        config.timeout = json_number_to_int64_or_default(*timeout, 5000);
    }
    switch (config.type) {
        case ConditionType::COST:
        case ConditionType::BOSS_HEALTH:
        case ConditionType::BATTLE_TIME:
            parse_numeric(input, path, config, result, mode);
            break;
        case ConditionType::SKILL_NAME:
            parse_skill_name(input, path, config, formation, result, mode);
            break;
        case ConditionType::AND_COMBINED:
        case ConditionType::OR_COMBINED:
            break;
    }
    return errors_since(result, begin) ? std::nullopt :
            std::optional<ConditionConfig>(std::move(config));
}

BAAS_NAMESPACE_END
