//
// Created by Administrator on 2025/5/29.
//
//
//
#include <fstream>

#include <simdutf.h>
#include <BAASGlobals.h>
#include <config/BAASConfig.h>
#include <module/auto_fight/conditions/BaseCondition.h>

#include <string>
#include <nlohmann/json.hpp>
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif // __EMSCRIPTEN__

using namespace baas;
using namespace std;


extern const char* embedded_skill_name_json_str;
extern const char* embedded_data_yaml_str;

enum WORKFLOW_CHECK_ERROR_TYPE {
    WORKFLOW_PATH_NOT_EXIST,
    JSON_PARSE,
    SKILL_NAME_DUPLICATE,
    SKILL_TEMPLATE_NOT_FOUND,
    YOLO_OBJECT_POSITION_DUPLICATE,
    YOLO_OBJECT_POSITION_NOT_FOUND,
    START_STATE,
    ACTIONS,
    ACTION,
    STATES,
    STATE,
    CONDITIONS,
    CONDITION,

};

/*
 * error type
 * 1. actions / action
 * 2. conditions / condition
 * 3. states / state
 */
WORKFLOW_CHECK_ERROR_TYPE error_type;

// key in actions / conditions / states
string error_key;

// whole error log
vector<string> global_error_message;

// workflow json
nlohmann::json wf_j;
BAASConfig wf_config;

// output json
nlohmann::json out_j;

// names in data.yaml
vector<string> all_yolo_obj_names;

// names in /formation/front
vector<string> valid_yolo_obj_names;

// names in skill_active.json
vector<string> all_skill_names;

// names in /formation/all_appeared_skills
vector<string> valid_skill_names;

vector<string> all_action;
vector<string> all_condition;
vector<string> all_state;

BAAS_NAMESPACE_BEGIN

void _log_valid_op_string(
        const std::string& name,
        const std::vector<std::string>& op_st_list
) noexcept
{
    global_error_message.emplace_back("Valid  <<< " + name + " >>> are listed below :");
    int cnt = 0;
    for (const auto& op: op_st_list)
        global_error_message.emplace_back(to_string(++cnt) + " : \"" + op + "\"");
}

void _init();

enum CheckReturn {
    ACCEPT,
    KEY_NOT_FOUND,
    INVALID_VALUE_TYPE,
    UNDEFINED
};

CheckReturn _check_state_defined(const string& key, const nlohmann::json& cfg) {
    assert(cfg.is_object());

    auto _it = cfg.find(key);

    if (_it == cfg.end()) {
        return KEY_NOT_FOUND;
    }
    if (!_it->is_string()) {
        return INVALID_VALUE_TYPE;
    }
    string state_name = *_it;
    if (find(all_state.begin(), all_state.end(), state_name) == all_state.end()) {
        return UNDEFINED;
    }

    return ACCEPT;
}

CheckReturn _check_action_defined(const string& key, const nlohmann::json& cfg) {
    assert(cfg.is_object());

    auto _it = cfg.find(key);

    if (_it == cfg.end()) {
        return KEY_NOT_FOUND;
    }
    if (!_it->is_string()) {
        return INVALID_VALUE_TYPE;
    }
    string action_name = *_it;
    if (find(all_action.begin(), all_action.end(), action_name) == all_action.end()) {
        return UNDEFINED;
    }
    return ACCEPT;
}

CheckReturn _check_condition_defined(const string& key, const nlohmann::json& cfg) {
    assert(cfg.is_object());

    auto _it = cfg.find(key);

    if (_it == cfg.end()) {
        return KEY_NOT_FOUND;
    }
    if (!_it->is_string()) {
        return INVALID_VALUE_TYPE;
    }
    string condition_name = *_it;
    if (find(all_condition.begin(), all_condition.end(), condition_name) == all_condition.end()) {
        return UNDEFINED;
    }
    return ACCEPT;
}

void _start_state_check() {
    CheckReturn ret = _check_state_defined("start_state", wf_j);
    if (ret == ACCEPT) return;
    error_type = START_STATE;

    switch (ret) {
        case KEY_NOT_FOUND:
            global_error_message.emplace_back("Workflow json must contain [ start_state ].");
            throw runtime_error("Workflow Start State Not Found");
        case INVALID_VALUE_TYPE:
            global_error_message.emplace_back("Workflow [ start_state ] config must be a string.");
            throw runtime_error("Invalid [ start_state ] Config Type");
        case UNDEFINED:
            global_error_message.emplace_back("Undefined state found in [ start_state ].");
            throw runtime_error("Start State Undefined");
    }
}

void _path_check(const string& path);

void check_workflow(const string& path);

void _parse_json(filesystem::path& path);

void _basic_config_check();

void _yolo_setting_check();

void _act_cond_sta_pre_check();

void _act_pre_check();

void _cond_pre_check();

void _sta_pre_check();

void _actions_check();

void _single_action_check(const nlohmann::json& action);

void _conditions_check();

void _single_condition_check(const nlohmann::json& condition);

void _states_check();

void _single_state_check(const nlohmann::json& state);

void _init_all_yolo_obj_names();

void _init_formation_front();

void _init_formation_all_appeared_skills();

void _formation_check();

void _init_all_skill_names();

BAAS_NAMESPACE_END

int main(int argc, char **argv) {
    system("chcp 65001");
    _init("");

    if (argc < 2) {
        BAASGlobalLogger->BAASInfo("Usage : ");
        BAASGlobalLogger->BAASInfo("1. Enter workflow path to check if the workflow is valid.");
        return 0;
    }
    string _p = argv[1];

    try {
        check_workflow(_p);
        BAASGlobalLogger->BAASInfo("Accept!");
    }
    catch(exception &e) {
        BAASGlobalLogger->BAASError("WorkFlow invalid, reason : ");
        BAASGlobalLogger->BAASError("Error T : " + to_string(error_type));
        if (!error_key.empty())
            BAASGlobalLogger->BAASError("Error K : " + error_key);
        if (strlen(e.what()) > 0) {

        }
        if (!global_error_message.empty()) {
            BAASGlobalLogger->BAASError("Error M : " + global_error_message[0]);
            for (size_t i = 1; i < global_error_message.size(); ++i)
                BAASGlobalLogger->BAASError(global_error_message[i]);
        }
    }
    return 0;
}

BAAS_NAMESPACE_BEGIN

void check_workflow(const string& path) {

    _path_check(path);

    _formation_check();

    _act_cond_sta_pre_check();

    _conditions_check();

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
        catch (const exception &e) {
            error_type = STATE;
            error_key = key;
            global_error_message.emplace_back("Error state name : " + key);
            if (strlen(e.what()) > 0) global_error_message.emplace_back(e.what());
            throw;
        }
    }
}

void _single_state_check(const nlohmann::json& state, const string& name) {
    auto it = state.find("action");
    if (it != state.end()) {
        if (!it->is_string()) {
            error_type = STATE;
            error_key = name;
            global_error_message.emplace_back("[ single state ] [ action ] must be string.");
            global_error_message.emplace_back("Error state name : [ " + name + " ]");
            throw runtime_error("Invalid State Action Type");
        }
        string act = *it;
        if (find(all_action.begin(), all_action.end(), act) == all_action.end()) {
            error_type = STATE;
            error_key = name;
            global_error_message.emplace_back("[ single state ] [ action ] not found in [ actions ].");
            global_error_message.emplace_back("Error state name : [ " + name + " ]");
            throw runtime_error("State Action Not Found");
        }
    }

}

void _init() {
    init_path();
    BAASGlobalLogger = GlobalLogger::getGlobalLogger();
#ifdef __EMSCRIPTEN__
    BAASGlobalLogger->set_enable(0b01); // disable file logger
#endif //__EMSCRIPTEN__
    log_git_info();
    _init_all_yolo_obj_names();
    BAASGlobalLogger->BAASInfo("YOLO Object Cnt: " + to_string(all_yolo_obj_names.size()));
    _init_all_skill_names();
    BAASGlobalLogger->BAASInfo("Skill Name Cnt : " + to_string(all_skill_names.size()));
}

void _path_check(const string& path) {

#ifdef WIN32
    wstring wpath;
    BAASStringUtil::str2wstr(path, wpath);
    filesystem::path _p(wpath);
#elif defined(__EMSCRIPTEN__) || defined(UNIX_LIKE_PLATFORM)
    filesystem::path _p(path);
#endif // WIN32

    if (!filesystem::exists(_p)) {
        error_type = WORKFLOW_PATH_NOT_EXIST;
        global_error_message.emplace_back("Workflow path does not exist : " + path);
        throw PathError("Workflow File Path Not Exists");
    }

    BAASGlobalLogger->BAASInfo(_p.string());
    _parse_json(_p);
}

void _parse_json(filesystem::path& path)  {
    ifstream file(path);
    try {
        wf_j = nlohmann::json::parse(file);
        wf_config = BAASConfig(wf_j, (BAASLogger*) BAASGlobalLogger);
    } catch (nlohmann::json::parse_error &e) {
        error_type = JSON_PARSE;
        global_error_message.emplace_back("Workflow json parse error : " + string(e.what()));
        throw runtime_error("Workflow JSON Parse Error");
    }
}

void _act_cond_sta_pre_check() {
    _act_pre_check();
    _cond_pre_check();
    _sta_pre_check();
}

void _act_pre_check() {

    if (!wf_j.contains("actions")) {
        error_type = ACTIONS;
        global_error_message.emplace_back("Workflow json must contain [ actions ].");
        throw runtime_error("Workflow Actions Not Found");
    }

    if (!wf_j["actions"].is_object()) {
        error_type = ACTIONS;
        global_error_message.emplace_back("Workflow [ actions ] config must be a object.");
        throw runtime_error("Invalid [ actions ] Config Type");
    }

    all_action.clear();

    for (const auto& [key, value] : wf_j["actions"].items()) {
        if (!value.is_array()) {
            error_type = ACTION;
            global_error_message.emplace_back("Workflow [ single action ] config must be an array.");
            global_error_message.emplace_back("Error action key : [ " + key + " ]");
            error_key = key;
            throw runtime_error("Invalid [ single action ] Config Type.");
        }
        all_action.push_back(key);
    }
}

void _cond_pre_check() {

    if (!wf_j.contains("conditions")) {
        error_type = CONDITIONS;
        global_error_message.emplace_back("Workflow json must contain [ conditions ].");
        throw runtime_error("Workflow Conditions Not Found");
    }

    if (!wf_j["conditions"].is_object()) {
        error_type = CONDITION;
        global_error_message.emplace_back("Workflow [ conditions ] config must be a object.");
        throw runtime_error("Invalid [ conditions ] Config Type");
    }

    all_condition.clear();

    for (const auto& [key, value] : wf_j["conditions"].items()) {
        if (!value.is_object()) {
            error_type = CONDITION;
            global_error_message.emplace_back("Workflow [ single condition ] config must be an object.");
            global_error_message.emplace_back("Error condition key : [ " + key + " ]");
            error_key = key;
            throw runtime_error("Invalid [ single condition ] Config Type.");
        }
        all_condition.push_back(key);
    }
}

void _sta_pre_check() {

    if (!wf_j.contains("states")) {
        error_type = STATES;
        global_error_message.emplace_back("Workflow json must contain [ states ].");
        throw runtime_error("Workflow States Not Found");
    }

    if (!wf_j["states"].is_object()) {
        error_type = STATES;
        global_error_message.emplace_back("Workflow [ states ] config must be a object.");
        throw runtime_error("Invalid [ states ] Config Type");
    }

    all_state.clear();

    for (const auto& [key, value] : wf_j["states"].items()) {
        if (!value.is_object()) {
            error_type = STATE;
            global_error_message.emplace_back("Workflow [ single state ] config must be an object.");
            global_error_message.emplace_back("Error state key : [ " + key + " ]");
            error_key = key;
            throw runtime_error("Invalid [ single state ] Config Type.");
        }
        all_state.push_back(key);
    }
}


void _init_all_skill_names()
{
    assert(filesystem::exists("skill_active.json"));
    ifstream file("skill_active.json");
    nlohmann::json skill_json = nlohmann::json::parse(file);
    for (const auto& [key, _] : skill_json["image"].items())
        all_skill_names.push_back(key);
}

void _init_all_yolo_obj_names()
{
    assert(filesystem::exists("data.yaml"));
    ifstream file("data.yaml");

    // Read the file line by line
    string line;
    vector<string> lines;
    while (getline(file, line)) lines.push_back(line);

    // Find the start and end of the names section
    size_t start = 0;
    size_t end = 0;
    for (size_t i = 0; i < lines.size(); i++) {
        if (lines[i].find("names:") != string::npos) start = i + 1;
        else if ((start > 0 && lines[i].find(':') == string::npos)) {
            end = i;
            break;
        }
        if (i == lines.size() - 1) end = lines.size();
    }

    // Extract the names
    for (size_t i = start; i < end; i++) {
        stringstream ss(lines[i]);
        string name;
        getline(ss, name, ':'); // Extract the number before the delimiter
        getline(ss, name); // Extract the string after the delimiter
        all_yolo_obj_names.push_back(name.substr(1)); // remove space after ':'
    }
}

void _single_state_check(const nlohmann::json& state)
{

}

void _init_formation_all_appeared_skills()
{
    auto skill_names = wf_config.get<vector<string>>("/formation/all_appeared_skills");
    for (const auto& skill_name : skill_names) {
        if (find(valid_skill_names.begin(), valid_skill_names.end(), skill_name) != valid_skill_names.end()) {
            error_type = SKILL_NAME_DUPLICATE;
            global_error_message.emplace_back("Duplicate skill [ " + skill_name + " ] found in [ /formation/all_appeared_skills ].");
            throw runtime_error("Duplicate Skill Name.");
        }
        if (find(all_skill_names.begin(), all_skill_names.end(), skill_name) == all_skill_names.end()) {
            error_type = SKILL_TEMPLATE_NOT_FOUND;
            global_error_message.emplace_back("In [ /formation/all_appeared_skills ], skill [ " + skill_name + " ] template not exist.");
            throw runtime_error("Skill Template Not Exist");
        }
        valid_skill_names.push_back(skill_name);
    }
    BAASGlobalLogger->BAASInfo("Valid skills   count : " + to_string(valid_skill_names.size()));
}

void _formation_check()
{
    _init_formation_front();
    _init_formation_all_appeared_skills();
}

void _init_formation_front()
{
    auto front_names = wf_config.get<vector<string>>("/formation/front");
    for (const auto& name : front_names) {
        if (find(valid_yolo_obj_names.begin(), valid_yolo_obj_names.end(), name) != valid_yolo_obj_names.end()) {
            error_type = YOLO_OBJECT_POSITION_DUPLICATE;
            global_error_message.emplace_back("Duplicate yolo object name [ " + name + " ] found in [ /formation/front ].");
            throw runtime_error("Duplicate Yolo Object Name.");
        }
        if (find(all_yolo_obj_names.begin(), all_yolo_obj_names.end(), name) == all_yolo_obj_names.end()) {
            error_type = YOLO_OBJECT_POSITION_NOT_FOUND;
            global_error_message.emplace_back("In [ /formation/front ], yolo object name [ " + name + " ] not exist.");
            throw runtime_error("Yolo Object Not Exist");
        }
        valid_yolo_obj_names.push_back(name);
    }
    BAASGlobalLogger->BAASInfo("Valid yolo obj count : " + to_string(valid_yolo_obj_names.size()));
}

void _actions_check()
{
    for (const auto& [key, value] : wf_j["actions"].items()) {
        try {
            _single_action_check(value);
        }
        catch (const exception &e) {
            error_type = ACTION;
            error_key = key;
            global_error_message.emplace_back("Error action name : " + key);
            if (strlen(e.what()) > 0) global_error_message.emplace_back(e.what());
            throw;
        }
    }
}

void _single_action_check(const nlohmann::json& action) {

}

void _conditions_check()
{
    for (const auto& [key, value] : wf_j["conditions"].items()) {
        try {
            _single_condition_check(value);
        }
        catch (const exception &e) {
            error_type = CONDITION;
            error_key = key;
            global_error_message.emplace_back("Error condition name : " + key);
            if (strlen(e.what()) > 0) global_error_message.emplace_back(e.what());
            throw;
        }
    }
}

void _single_condition_check(const nlohmann::json& d_cond) {
    auto _it = d_cond.find("type");
    if (_it == d_cond.end()) {
        global_error_message.push_back("[ single condition ] must contain key [ type ].");
        _log_valid_op_string("[ single condition ] [ type ]", BaseCondition::cond_type_st_list);
        throw ValueError("[ single condition ] [ type ] not found.");
    }
}


extern "C" {
    EMSCRIPTEN_KEEPALIVE const char* is_valid_workflow(const char* str) {
        cout << "is_valid_workflow called with input: " << str << endl;
        try {
            wf_j = nlohmann::json::parse(str);
            wf_config = BAASConfig(wf_j, (BAASLogger*) BAASGlobalLogger);
        } catch (const nlohmann::json::parse_error&) {
            error_type = JSON_PARSE;
            return "Workflow json parse error";
        }
    _init("");

        return "Workflow json parsed successfully.";
    }
}

BAAS_NAMESPACE_END