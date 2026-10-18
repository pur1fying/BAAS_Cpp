#include "module/auto_fight/workflow/WorkflowFileParser.h"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "ActionParser.h"
#include "ConditionParser.h"
#include "WorkflowGraph.h"
#include "ParseFields.h"
#include "module/auto_fight/workflow/WorkflowNames.h"
#include "module/auto_fight/workflow/WorkflowResources.h"

using namespace baas;
using Json = nlohmann::json;

namespace {

void require(bool value, const std::string& message)
{
    if (!value) {
        throw std::runtime_error(message);
    }
}

WorkflowParseOptions options(WorkflowParseErrorMode mode = WorkflowParseErrorMode::FIRST)
{
    return {.EmulatorServer = "CN", .EmulatorLanguage = "zh-cn", .error_mode = mode};
}

Json read_json(const std::filesystem::path& path)
{
    std::ifstream file(path, std::ios::binary);
    return Json::parse(file);
}

Json minimal()
{
    return {
        {"formation", {{"all_appeared_skills", Json::array()}, {"front", Json::array()}}},
        {"actions", Json::object()}, {"conditions", Json::object()},
        {"start_state", "start"}, {"states", {{"start", Json::object()}}},
    };
}

void expect_acceptance(const WorkflowModel& parsed, const Json& item)
{
    const auto id = item.value("id", item.value("path", std::string("case")));
    if (parsed.has_errors() == item.at("accepted").get<bool>()) {
        std::string detail;
        for (const auto& diagnostic : parsed.Errors) {
            detail += diagnostic.code + " " + diagnostic.path + "; ";
        }
        throw std::runtime_error(id + ": acceptance mismatch: " + detail);
    }
    if (!item.at("accepted").get<bool>()) {
        require(!parsed.Errors.empty(), id + ": missing diagnostic");
        require(parsed.Errors.front().code == item.at("first_failure").at("code").get<std::string>(),
                id + ": first error code: " + parsed.Errors.front().code);
    }
}

void action_oracle(const std::filesystem::path& source)
{
    const auto oracle = read_json(source / "tests/auto_fight_workflow/base_oracle/base-386f4379-action-oracle.json");
    for (const auto& item : oracle.at("numeric_conversions")) {
        require(json_number_to_int32_or_default(item.at("wire")) == item.at("int32"), item.at("id"));
        require(json_number_to_int64_or_default(item.at("wire")) == item.at("int64"), item.at("id"));
    }
    for (const auto& item : oracle.at("behavior_cases")) {
        WorkflowModel result;
        FormationConfig formation;
        formation.slot_count = 3;
        if (item.contains("catalog")) {
            formation.all_appeared_skills = item["catalog"]["skills"].get<std::vector<std::string>>();
            formation.front = item["catalog"]["objects"].get<std::vector<std::string>>();
        }
        const auto config = parse_action_step(item["input"], "/actions/case/0",
                formation, result, WorkflowParseErrorMode::FIRST);
        expect_acceptance(result, item);
        if (!config) {
            continue;
        }
        const auto& expected = item["handler"];
        require(config->desc == expected.at("desc").get<std::string>(), item.at("id"));
        if (expected.contains("op")) {
            const auto actual = config->type == ActionType::ACC ?
                    ACC_OP_NAMES.at(static_cast<std::size_t>(config->acc_op)) :
                    AUTO_OP_NAMES.at(static_cast<std::size_t>(config->auto_op));
            require(actual == expected["op"].get<std::string>(), item.at("id"));
        }
        if (expected.contains("timeout")) {
            require(config->skip_animation_timeout == expected["timeout"], item.at("id"));
        }
        if (expected.contains("release_op")) {
            require(RELEASE_OP_NAMES.at(static_cast<std::size_t>(config->release_op)) ==
                    expected["release_op"].get<std::string>(), item.at("id"));
            require(CHECK_OP_NAMES.at(static_cast<std::size_t>(config->check_op)) ==
                    expected["check_op"].get<std::string>(), item.at("id"));
        }
        if (expected.contains("fixed_x")) {
            require(config->fixed_x == expected["fixed_x"] && config->fixed_y == expected["fixed_y"], item.at("id"));
        }
        if (expected.contains("last_release_index")) {
            require(config->last_release_index == expected["last_release_index"], item.at("id"));
        }
        if (expected.contains("check_value")) {
            require(config->check_value == expected["check_value"] &&
                    config->check_timeout == expected["check_timeout"], item.at("id"));
        }
        if (expected.contains("object_indices")) {
            Json indices = Json::array();
            for (const auto& name : config->target_objects) {
                indices.push_back(item["resource_indices"]["objects"].at(name));
            }
            require(indices == expected["object_indices"], item.at("id"));
        }
    }
    for (const auto& item : oracle.at("sequence_cases")) {
        auto input = minimal();
        input["actions"] = item["input"];
        const auto result = parse_workflow(input.dump(), options());
        expect_acceptance(result, item);
        if (item.at("accepted").get<bool>()) {
            Json names = Json::array(), steps = Json::array();
            for (const auto& [name, action] : result.actions) {
                names.push_back(name);
                Json types = Json::array();
                for (const auto& step : action->steps) {
                    types.push_back(ACTION_NAMES.at(static_cast<std::size_t>(step->type)));
                }
                steps.push_back(types);
            }
            require(names == item["named_order"] && steps == item["step_types"], item.at("id"));
        }
    }
}

void condition_oracle(const std::filesystem::path& source)
{
    const auto oracle = read_json(source / "tests/auto_fight_workflow/condition_base_oracle/base-167c4e5-condition-oracle.json");
    for (const auto& item : oracle.at("parse_cases")) {
        // The frozen condition oracle supplies already-prepared runtime resources.
        // Exercise its element parser directly, independently of updater slot counts.
        WorkflowModel result;
        auto& model = result;
        model.formation.all_appeared_skills = item.value("skills", Json::array()).get<std::vector<std::string>>();
        model.formation.slot_count = item.value("slot_count", 3);
        const auto siblings = item.value("siblings", Json::object());
        for (const auto& [name, value] : siblings.items()) {
            model.conditions[name] = parse_condition(value, "/conditions/" + name,
                    model.formation, result, WorkflowParseErrorMode::FIRST);
        }
        model.conditions["case"] = parse_condition(item["input"], "/conditions/case",
                model.formation, result, WorkflowParseErrorMode::FIRST);
        if (!result.has_errors()) {
            validate_graph(result, WorkflowParseErrorMode::FIRST);
        }
        expect_acceptance(result, item);
        if (result.has_errors()) {
            continue;
        }
        const auto& config = *result.conditions.at("case");
        const auto& expected = item["snapshot"];
        require(config.timeout == expected["timeout"] && config.desc == expected["desc"].get<std::string>(), item.at("id"));
        if (expected.contains("op")) {
            const auto op = config.type == ConditionType::BOSS_HEALTH ? wire_name(config.health_op) :
                    config.type == ConditionType::SKILL_NAME ? wire_name(config.skill_op) :
                    wire_name(config.numeric_op);
            require(op == expected["op"].get<std::string>(), item.at("id"));
        }
        if (expected.contains("value")) {
            const Json actual = config.type == ConditionType::BATTLE_TIME ? Json(config.time_value) : Json(config.value);
            require(actual == expected["value"], item.at("id"));
        }
        if (expected.contains("range")) {
            const Json actual = config.type == ConditionType::BATTLE_TIME ?
                    Json::array({config.time_range_min, config.time_range_max}) :
                    Json::array({config.range_min, config.range_max});
            require(actual == expected["range"], item.at("id"));
        }
        if (expected.contains("name")) {
            require(config.skill_name == expected["name"].get<std::string>(), item.at("id"));
        }
        if (expected.contains("p")) {
            require(config.position == expected["p"], item.at("id"));
        }
    }
}

void state_oracle(const std::filesystem::path& source)
{
    const auto oracle = read_json(source / "tests/auto_fight_workflow/workflow_base_oracle/base-14070d5-workflow-oracle.json");
    for (const auto& item : oracle.at("state_cases")) {
        auto input = item["input"];
        // This oracle captured state initialization after resource preparation.
        // Supply the newly extracted updater prerequisite, leaving state inputs unchanged.
        input["formation"]["front"] = Json::array();
        input["formation"]["slot_count"] = workflow_resources().skill_cost_regions.size();
        const auto first = parse_workflow(input.dump(), options());
        const auto all = parse_workflow(input.dump(), options(WorkflowParseErrorMode::ALL));
        expect_acceptance(first, item);
        expect_acceptance(all, item);
        if (item.at("accepted").get<bool>()) {
            Json names = Json::array();
            std::size_t index = 0;
            for (const auto& [name, state] : first.states) {
                names.push_back(Json::array({name, index++}));
            }
            require(names == item["state_order"], item.at("id"));
        }
    }
    for (const auto& item : oracle["axis_cases"]) {
        const auto result = parse_workflow_file(source / item["path"].get<std::string>(), options());
        expect_acceptance(result, item);
    }
}

void new_contract()
{
    auto input = minimal();
    input["actions"]["mixed"] = Json::array({{{"t", "restart"}}, {{"t", "acc"}, {"op", "bad"}}, {{"t", "auto"}, {"op", "off"}}});
    auto first = parse_workflow(input.dump(), options());
    auto all = parse_workflow(input.dump(), options(WorkflowParseErrorMode::ALL));
    require(first.has_errors() && all.has_errors(), "error modes reject");
    require(first.Errors.front().code == all.Errors.front().code &&
            first.Errors.front().path == all.Errors.front().path, "same first error");
    const auto& steps = all.actions.at("mixed")->steps;
    require(steps.size() == 3 && steps[0] && !steps[1] && steps[2], "partial step positions");
    require(first.actions.at("mixed")->steps[0].has_value(), "FIRST retains completed units");

    input["conditions"]["bad"] = {{"type", "cost"}, {"op", "over"}, {"value", "wrong"}};
    input["conditions"]["good"] = {{"type", "and_combined"}};
    input["states"]["start"]["transitions"] = Json::array({
            {{"condition", "good"}, {"next", "start"}}, {{"condition", 4}}});
    all = parse_workflow(input.dump(), options(WorkflowParseErrorMode::ALL));
    require(all.Errors.size() >= 3 && all.conditions.at("good") &&
            !all.conditions.at("bad"), "independent units");
    require(all.states.at("start")->transitions[0].has_value() &&
            !all.states.at("start")->transitions[1], "partial transitions");

    input = minimal();
    input["conditions"] = {{"a", {{"type", "and_combined"}, {"and", {"b"}}}},
                           {"b", {{"type", "or_combined"}, {"or", {"a"}}}}};
    auto cycle = parse_workflow(input.dump(), options());
    require(cycle.has_errors() && cycle.Errors.front().code == "CONDITION_DEPENDENCY_CYCLE", "condition cycle");
    input["conditions"] = Json::object();
    input["states"]["start"]["default_transition"] = "start";
    require(!parse_workflow(input.dump(), options()).has_errors(), "state cycles allowed");

    input = minimal();
    input["BattleTime"]["time_format"] = 1;
    input["yolo_setting"] = {{"nms_type", "no"}, {"update_interval", -10}};
    input["BossHealth"]["ocr_region"] = {-10, true, 4.5, -6};
    auto root = parse_workflow(input.dump(), options());
    require(!root.has_errors() && root.battle_time->time_format == 0, "legacy time-format defect retained");
    require(root.boss_health->region == OcrRegion{-10, 1, 4, -6}, "legacy rectangle conversion");
    require(root.yolo->update_interval == -10 && root.yolo->nms_type == NmsType::NONE,
            "no invented updater range");
    auto wrong_slots = minimal();
    wrong_slots["formation"]["slot_count"] = workflow_resources().skill_cost_regions.size() + 1;
    const auto slot_result = parse_workflow(wrong_slots.dump(), options());
    require(slot_result.has_errors() && slot_result.Errors[0].path == "/formation/slot_count",
            "existing runtime slot/region check is now in Core");
    input["formation"]["front"] = {"not-a-model-class"};
    require(parse_workflow(input.dump(), options()).has_errors(), "model class membership");

    input = minimal();
    for (int i = 0; i < 1100; ++i) {
        input["actions"]["many"].push_back({{"t", "missing"}});
    }
    all = parse_workflow(input.dump(), options(WorkflowParseErrorMode::ALL));
    require(all.Errors.size() == 1100, "no diagnostic budget");
    auto invalid = parse_workflow("{", options());
    require(invalid.has_errors() && invalid.Errors[0].byte_offset &&
            invalid.Errors[0].code == "JSON_PARSE_ERROR", "syntax error in model");
    require(!invalid.battle_type && !invalid.start_state && invalid.actions.empty() &&
            invalid.conditions.empty() && invalid.states.empty(), "syntax failure has empty configuration");
    const auto wrong_root = parse_workflow("[]", options());
    require(wrong_root.has_errors() && wrong_root.Errors[0].code == "OBJECT_KIND" &&
            !wrong_root.battle_type && wrong_root.states.empty(), "root failure has errors in empty model");

    auto warning_input = minimal();
    warning_input["states"]["unused"] = Json::object();
    const auto warnings = parse_workflow(warning_input.dump(), options());
    require(!warnings.has_errors() && warnings.Errors.size() == 1 &&
            warnings.Errors[0].severity == DiagnosticSeverity::WARNING_DIAGNOSTIC &&
            warnings.Errors[0].path == "/states/unused", "warnings in Errors do not block execution");

    auto reference_input = minimal();
    reference_input["states"]["start"]["action"] = "missing";
    const auto reference_error = parse_workflow(reference_input.dump(), options(WorkflowParseErrorMode::ALL));
    require(reference_error.has_errors() && reference_error.Errors[0].code == "REFERENCE_NOT_FOUND" &&
            reference_error.states.at("start")->action == "missing", "reference error retains local state configuration");
    auto bad_options = options();
    bad_options.EmulatorServer = "absent";
    require(parse_workflow(minimal().dump(), bad_options).has_errors(), "server selection");

    const auto path = std::filesystem::temp_directory_path() / std::filesystem::path(u8"BAAS-轴校验-测试.json");
    const auto text = minimal().dump();
    {
        std::ofstream output(path, std::ios::binary);
        output << text;
    }
    const auto from_file = parse_workflow_file(path, options());
    require(!from_file.has_errors() && from_file.start_state == "start", "native UTF-8 file");
    require(read_json(path) == minimal(), "input file unchanged");
    std::filesystem::remove(path);
    const auto missing = parse_workflow_file(path, options());
    require(missing.has_errors() && missing.Errors.front().code == "FILE_READ_ERROR" &&
            !missing.Errors.front().source_file.empty(), "file read failure");
    require(!missing.battle_type && !missing.start_state && missing.actions.empty() &&
            missing.conditions.empty() && missing.states.empty(), "file failure has empty configuration");
}

} // namespace

int main()
{
    try {
        const std::filesystem::path source = BAAS_WORKFLOW_SOURCE_ROOT;
        action_oracle(source);
        condition_oracle(source);
        state_oracle(source);
        new_contract();
        std::cout << "Frozen action/condition/state oracles and native parser contracts passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
