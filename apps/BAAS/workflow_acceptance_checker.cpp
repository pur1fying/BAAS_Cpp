//
// Created by Administrator on 2025/5/29.
//

#include "fstream"

#include "BAASLogger.h"
#include "utils.h"
#include "module/auto_fight/conditions/BaseCondition.h"
#include "simdutf.h"

using namespace baas;
using namespace std;

/*
 * 1. path / file type
 * 2. json parse
 * 2. action / state / condition
 * 3. yolo setting
 */

// error info

std::string error_type;

std::string error_key;

std::string global_error_message;

// end error info

nlohmann::json wf_j;
nlohmann::json out_j;

std::vector<std::string> all_action;
std::vector<std::string> all_condition;
std::vector<std::string> all_state;

void _init();

void _path_check(const std::string& path);

void check_workflow(const std::string& path);

void _parse_json(std::filesystem::path& path);

void _basic_config_check();

void _formation_check();

void _yolo_setting_check();

void _act_cond_sta_pre_check();

void _act_pre_check();

void _cond_pre_check();

void _sta_pre_check();

void _actions_check();

void _conditions_check();

void _states_check();

int main(int argc, char **argv) {
    system("chcp 65001");
    _init();
    if (argc < 2) {
        BAASGlobalLogger->BAASInfo("Usage : ");
        BAASGlobalLogger->BAASInfo("1. Enter workflow path to check if the workflow is valid.");
        return 0;
    }
    std::string _p = argv[1];

    try {
        check_workflow(_p);
        BAASGlobalLogger->BAASInfo("Accept!");
    }
    catch(std::exception &e) {
        BAASGlobalLogger->BAASError("WorkFlow invalid, reason : ");
        BAASGlobalLogger->BAASError("Error T : " + error_type);
        BAASGlobalLogger->BAASError("Error M : " + global_error_message);
        BAASGlobalLogger->BAASError(e.what());
    }

    return 0;
}

void check_workflow(const std::string& path) {

    _path_check(path);

    _act_cond_sta_pre_check();

}

void _init() {
    init_path();
    BAASGlobalLogger = GlobalLogger::getGlobalLogger();
    log_git_info();
}

void _path_check(const std::string& path) {

#ifdef WIN32
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wpath = converter.from_bytes(path);
    std::filesystem::path _p(wpath);
#elif UNIX_LIKE_PLATFORM
    std::filesystem::path _p(path);
#endif // WIN32

    if (!std::filesystem::exists(_p)) {
        error_type = "filepath";
        global_error_message = "Workflow path does not exist : " + path;
        throw PathError("Workflow File Path Not Exists");
    }

    BAASGlobalLogger->BAASInfo(_p.string());
    _parse_json(_p);
}

void _parse_json(std::filesystem::path& path)  {
    std::ifstream file(path);
    try {
        wf_j = nlohmann::json::parse(file);
    } catch (nlohmann::json::parse_error &e) {
        error_type = "json_prase";
        global_error_message = "Workflow json parse error : " + std::string(e.what());
        throw std::runtime_error("Workflow JSON Parse Error");
    }
}

void _act_cond_sta_pre_check() {
    _act_pre_check();
    _cond_pre_check();
    _sta_pre_check();
}

void _act_pre_check() {

    if (!wf_j.contains("actions")) {
        error_type = "whole_actions";
        global_error_message = "Workflow json must contain [ actions ].";
        throw std::runtime_error("Workflow Actions Not Found");
    }

    if (!wf_j["actions"].is_object()) {
        error_type = "whole_actions";
        global_error_message = "Workflow [ actions ] config must be a object.";
        throw std::runtime_error("Invalid [ actions ] Config Type");
    }

    all_action.clear();

    for (const auto& [key, value] : wf_j["actions"].items()) {
        if (!value.is_array()) {
            error_type = "action_type";
            global_error_message = "Workflow [ single action ] config must be an array. \nError action key : [ " + key + " ]";
            error_key = key;
            throw std::runtime_error("Invalid [ single action ] Config Type.");
        }
        all_action.push_back(key);
    }
}

void _cond_pre_check() {

    if (!wf_j.contains("conditions")) {
        error_type = "whole_conditions";
        global_error_message = "Workflow json must contain [ conditions ].";
        throw std::runtime_error("Workflow Conditions Not Found");
    }

    if (!wf_j["conditions"].is_object()) {
        error_type = "whole_conditions";
        global_error_message = "Workflow [ conditions ] config must be a object.";
        throw std::runtime_error("Invalid [ conditions ] Config Type");
    }

    all_condition.clear();

    for (const auto& [key, value] : wf_j["conditions"].items()) {
        if (!value.is_object()) {
            error_type = "condition_type";
            global_error_message = "Workflow [ single condition ] config must be an object. \nError condition key : [ " + key + " ]";
            error_key = key;
            throw std::runtime_error("Invalid [ single condition ] Config Type.");
        }
        all_condition.push_back(key);
    }
}

void _sta_pre_check() {

    if (!wf_j.contains("states")) {
        error_type = "whole_states";
        global_error_message = "Workflow json must contain [ states ].";
        throw std::runtime_error("Workflow States Not Found");
    }

    if (!wf_j["states"].is_object()) {
        error_type = "whole_states";
        global_error_message = "Workflow [ states ] config must be a object.";
        throw std::runtime_error("Invalid [ states ] Config Type");
    }

    all_state.clear();

    for (const auto& [key, value] : wf_j["states"].items()) {
        if (!value.is_object()) {
            error_type = "state_type";
            global_error_message = "Workflow [ single state ] config must be an object. \nError state key : [ " + key + " ]";
            error_key = key;
            throw std::runtime_error("Invalid [ single state ] Config Type.");
        }
        all_state.push_back(key);
    }
}