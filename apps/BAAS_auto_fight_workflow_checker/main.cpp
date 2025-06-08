//
// Created by Administrator on 2025/5/29.
//

#include <fstream>

#include <utils.h>
#include <simdutf.h>
#include <BAASLogger.h>

using namespace baas;
using namespace std;

/*
 * error type
 * 1. actions / action
 * 2. conditions / condition
 * 3. states / state
 */
std::string error_type;

// key in actions / conditions / states
std::string error_key;

// whole error log
std::vector<std::string> global_error_message;

// workflow json
nlohmann::json wf_j;
//BAASConfig wf_config;

// output json
nlohmann::json out_j;

// names in data.yaml
std::vector<std::string> all_yolo_obj_names;

// names in /formation/front
std::vector<std::string> valid_yolo_obj_names;

// names in skill_active.json
std::vector<std::string> all_skill_names;

// names in /formation/all_appeared_skills
std::vector<std::string> valid_skill_names;

std::vector<std::string> all_action;
std::vector<std::string> all_condition;
std::vector<std::string> all_state;

BAAS_NAMESPACE_BEGIN

void _init();

enum CheckReturn {
    ACCEPT,
    KEY_NOT_FOUND,
    INVALID_VALUE_TYPE,
    UNDEFINED
};

CheckReturn _check_state_defined(const std::string& key, const nlohmann::json& cfg) {
    assert(cfg.is_object());

    auto _it = cfg.find(key);

    if (_it == cfg.end()) {
        return KEY_NOT_FOUND;
    }
    if (!_it->is_string()) {
        return INVALID_VALUE_TYPE;
    }
    std::string state_name = *_it;
    if (std::find(all_state.begin(), all_state.end(), state_name) == all_state.end()) {
        return UNDEFINED;
    }

    return ACCEPT;
}

CheckReturn _check_action_defined(const std::string& key, const nlohmann::json& cfg) {
    assert(cfg.is_object());

    auto _it = cfg.find(key);

    if (_it == cfg.end()) {
        return KEY_NOT_FOUND;
    }
    if (!_it->is_string()) {
        return INVALID_VALUE_TYPE;
    }
    std::string action_name = *_it;
    if (std::find(all_action.begin(), all_action.end(), action_name) == all_action.end()) {
        return UNDEFINED;
    }
    return ACCEPT;
}

CheckReturn _check_condition_defined(const std::string& key, const nlohmann::json& cfg) {
    assert(cfg.is_object());

    auto _it = cfg.find(key);

    if (_it == cfg.end()) {
        return KEY_NOT_FOUND;
    }
    if (!_it->is_string()) {
        return INVALID_VALUE_TYPE;
    }
    std::string condition_name = *_it;
    if (std::find(all_condition.begin(), all_condition.end(), condition_name) == all_condition.end()) {
        return UNDEFINED;
    }
    return ACCEPT;
}

void _start_state_check() {
    CheckReturn ret = _check_state_defined("start_state", wf_j);
    if (ret == ACCEPT) return;
    error_type = "start_state";

    switch (ret) {
        case KEY_NOT_FOUND:
            global_error_message.emplace_back("Workflow json must contain [ start_state ].");
            throw std::runtime_error("Workflow Start State Not Found");
        case INVALID_VALUE_TYPE:
            global_error_message.emplace_back("Workflow [ start_state ] config must be a string.");
            throw std::runtime_error("Invalid [ start_state ] Config Type");
        case UNDEFINED:
            global_error_message.emplace_back("Undefined state found in [ start_state ].");
            throw std::runtime_error("Start State Undefined");
    }
}

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

void _actions_check() {

}

void _single_action_check(const nlohmann::json& action);

void _conditions_check();

void _single_condition_check(const nlohmann::json& condition);

void _states_check();

void _single_state_check(const nlohmann::json& state);

void _init_all_yolo_obj_names();

void _init_all_skill_names();

BAAS_NAMESPACE_END

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
        if (!error_key.empty())
            BAASGlobalLogger->BAASError("Error K : " + error_key);
        if (!global_error_message.empty()) {
            BAASGlobalLogger->BAASError("Error M : " + global_error_message[0]);
            for (size_t i = 1; i < global_error_message.size(); ++i)
                BAASGlobalLogger->BAASError(global_error_message[i]);
        }
        BAASGlobalLogger->BAASError(e.what());
    }
    return 0;
}

BAAS_NAMESPACE_BEGIN

void check_workflow(const std::string& path) {

    _path_check(path);

    _act_cond_sta_pre_check();

    _states_check();

    _actions_check();

    _states_check();

}

void _states_check()
{
    _start_state_check();
    for (const auto& [key, value]: wf_j["states"].items()) {
        try {
            _single_state_check(value);
        }
        catch (const std::exception &e) {
            error_type = "state";
            error_key = key;
            global_error_message.emplace_back("Error state name : " + key);
            global_error_message.emplace_back(e.what());
            throw;
        }
    }
}

void _single_state_check(const nlohmann::json& state, const std::string& name) {
    auto it = state.find("action");
    if (it != state.end()) {
        if (!it->is_string()) {
            error_type = "state";
            error_key = name;
            global_error_message.emplace_back("[ single state ] [ action ] must be string.");
            global_error_message.emplace_back("Error state name : [ " + name + " ]");
            throw std::runtime_error("Invalid State Action Type");
        }
        string act = *it;
        if (find(all_action.begin(), all_action.end(), act) == all_action.end()) {
            error_type = "state";
            error_key = name;
            global_error_message.emplace_back("[ single state ] [ action ] not found in [ actions ].");
            global_error_message.emplace_back("Error state name : [ " + name + " ]");
            throw std::runtime_error("State Action Not Found");
        }
    }

}

void _init() {
    init_path();
    BAASGlobalLogger = GlobalLogger::getGlobalLogger();
    log_git_info();
    _init_all_yolo_obj_names();
    BAASGlobalLogger->BAASInfo("YOLO Object Cnt: " + std::to_string(all_yolo_obj_names.size()));
    _init_all_skill_names();
    BAASGlobalLogger->BAASInfo("Skill Name Cnt : " + std::to_string(all_skill_names.size()));


}

void _path_check(const std::string& path) {

#ifdef WIN32
    std::wstring wpath;
    BAASStringUtil::str2wstr(path, wpath);
    std::filesystem::path _p(wpath);
#elif UNIX_LIKE_PLATFORM
    std::filesystem::path _p(path);
#endif // WIN32

    if (!std::filesystem::exists(_p)) {
        error_type = "filepath";
        global_error_message.emplace_back("Workflow path does not exist : " + path);
        throw PathError("Workflow File Path Not Exists");
    }

    BAASGlobalLogger->BAASInfo(_p.string());
    _parse_json(_p);
}

void _parse_json(std::filesystem::path& path)  {
    std::ifstream file(path);
    try {
        wf_j = nlohmann::json::parse(file);
//        wf_config = BAASConfig(wf_j, (BAASLogger*) BAASGlobalLogger);
    } catch (nlohmann::json::parse_error &e) {
        error_type = "json_prase";
        global_error_message.emplace_back("Workflow json parse error : " + std::string(e.what()));
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
        error_type = "actions";
        global_error_message.emplace_back("Workflow json must contain [ actions ].");
        throw std::runtime_error("Workflow Actions Not Found");
    }

    if (!wf_j["actions"].is_object()) {
        error_type = "actions";
        global_error_message.emplace_back("Workflow [ actions ] config must be a object.");
        throw std::runtime_error("Invalid [ actions ] Config Type");
    }

    all_action.clear();

    for (const auto& [key, value] : wf_j["actions"].items()) {
        if (!value.is_array()) {
            error_type = "action";
            global_error_message.emplace_back("Workflow [ single action ] config must be an array.");
            global_error_message.emplace_back("Error action key : [ " + key + " ]");
            error_key = key;
            throw std::runtime_error("Invalid [ single action ] Config Type.");
        }
        all_action.push_back(key);
    }
}

void _cond_pre_check() {

    if (!wf_j.contains("conditions")) {
        error_type = "conditions";
        global_error_message.emplace_back("Workflow json must contain [ conditions ].");
        throw std::runtime_error("Workflow Conditions Not Found");
    }

    if (!wf_j["conditions"].is_object()) {
        error_type = "conditions";
        global_error_message.emplace_back("Workflow [ conditions ] config must be a object.");
        throw std::runtime_error("Invalid [ conditions ] Config Type");
    }

    all_condition.clear();

    for (const auto& [key, value] : wf_j["conditions"].items()) {
        if (!value.is_object()) {
            error_type = "condition";
            global_error_message.emplace_back("Workflow [ single condition ] config must be an object.");
            global_error_message.emplace_back("Error condition key : [ " + key + " ]");
            error_key = key;
            throw std::runtime_error("Invalid [ single condition ] Config Type.");
        }
        all_condition.push_back(key);
    }
}

void _sta_pre_check() {

    if (!wf_j.contains("states")) {
        error_type = "states";
        global_error_message.emplace_back("Workflow json must contain [ states ].");
        throw std::runtime_error("Workflow States Not Found");
    }

    if (!wf_j["states"].is_object()) {
        error_type = "states";
        global_error_message.emplace_back("Workflow [ states ] config must be a object.");
        throw std::runtime_error("Invalid [ states ] Config Type");
    }

    all_state.clear();

    for (const auto& [key, value] : wf_j["states"].items()) {
        if (!value.is_object()) {
            error_type = "state";
            global_error_message.emplace_back("Workflow [ single state ] config must be an object.");
            global_error_message.emplace_back("Error state key : [ " + key + " ]");
            error_key = key;
            throw std::runtime_error("Invalid [ single state ] Config Type.");
        }
        all_state.push_back(key);
    }
}


void _init_all_skill_names()
{
    assert(filesystem::exists("skill_active.json"));
    std::ifstream file("skill_active.json");
    nlohmann::json skill_json = nlohmann::json::parse(file);
    for (const auto& [key, _] : skill_json["image"].items())
        all_skill_names.push_back(key);
}

void _init_all_yolo_obj_names()
{
    assert(filesystem::exists("data.yaml"));
    std::ifstream file("data.yaml");

    // Read the file line by line
    std::string line;
    std::vector<std::string> lines;
    while (std::getline(file, line)) lines.push_back(line);

    // Find the start and end of the names section
    std::size_t start = 0;
    std::size_t end = 0;
    for (std::size_t i = 0; i < lines.size(); i++) {
        if (lines[i].find("names:") != std::string::npos) start = i + 1;
        else if ((start > 0 && lines[i].find(':') == std::string::npos)) {
            end = i;
            break;
        }
        if (i == lines.size() - 1) end = lines.size();
    }

    // Extract the names
    for (std::size_t i = start; i < end; i++) {
        std::stringstream ss(lines[i]);
        std::string name;
        std::getline(ss, name, ':'); // Extract the number before the delimiter
        std::getline(ss, name); // Extract the string after the delimiter
        all_yolo_obj_names.push_back(name.substr(1)); // remove space after ':'
    }
}

void baas::_single_state_check(const nlohmann::json& state)
{
    
}


BAAS_NAMESPACE_END