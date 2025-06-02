//
// Created by pc on 2025/4/23.
//

#include "module/auto_fight/auto_fight.h"

#include <format>

#include <BAASImageResource.h>

#include "utils.h"
#include "module/auto_fight/constants.h"

#include "module/auto_fight/screenshot_data/CostUpdater.h"
#include "module/auto_fight/screenshot_data/SkillCostUpdater.h"
#include "module/auto_fight/screenshot_data/SkillNameUpdater.h"
#include "module/auto_fight/screenshot_data/BossHealthUpdater.h"
#include "module/auto_fight/screenshot_data/AccelerationPhaseUpdater.h"
#include "module/auto_fight/screenshot_data/AutoStateUpdater.h"
#include "module/auto_fight/screenshot_data/ObjectPositionUpdater.h"

#include "module/auto_fight/conditions/CostCondition.h"
#include "module/auto_fight/conditions/BossHealthCondition.h"
#include "module/auto_fight/conditions/SkillNameCondition.h"
#include "module/auto_fight/conditions/OrCombinedCondition.h"
#include "module/auto_fight/conditions/AndCombinedCondition.h"

BAAS_NAMESPACE_BEGIN

// Default skill recognition template
const std::vector<std::string> AutoFight::default_active_skill_template = {
        "skill_active"
};
const std::vector<std::string> AutoFight::default_inactive_skill_template = {
        "skill_l_inactive", "skill_r_inactive",
};

const std::string AutoFight::template_j_ptr_prefix = "/BAAS/auto_fight/Skill/templates";

AutoFight::AutoFight(BAAS *baas)
{
    this->baas = baas;
    this->config = baas->get_config();
    logger = baas->get_logger();
    d_update_max_thread = config->getInt("/auto_fight/d_update_max_thread");
    d_update_thread_pool = std::make_unique<ThreadPool>(d_update_max_thread);
    d_update_thread_pool->init();
}

// Add data updaters here
void AutoFight::_init_data_updaters()
{
    // BIT [ 1 ]
    // cost
    d_auto_f.d_updaters.push_back(std::make_unique<CostUpdater>(baas, &d_auto_f));
    d_updater_map["cost"] = 1LL << 0;

    // BIT [ 2 ]
    // boss health
    d_auto_f.d_updaters.push_back(std::make_unique<BossHealthUpdater>(baas, &d_auto_f));
    d_updater_map["boss_health"] = 1LL << 1;

    // BIT [ 3 ]
    // skill name
    d_auto_f.d_updaters.push_back(std::make_unique<SkillNameUpdater>(baas, &d_auto_f));
    d_updater_map["skill_name"] = 1LL << 2;

    // BIT [ 4 ]
    // skill cost
    d_auto_f.d_updaters.push_back(std::make_unique<SkillCostUpdater>(baas, &d_auto_f));
    d_updater_map["skill_cost"] = 1LL << 3;

    // BIT [ 5 ]
    // acc phase
    d_auto_f.d_updaters.push_back(std::make_unique<AccelerationPhaseUpdater>(baas, &d_auto_f));
    d_updater_map["acc_phase"] = 1LL << 4;

    // BIT [ 6 ]
    // auto state
    d_auto_f.d_updaters.push_back(std::make_unique<AutoStateUpdater>(baas, &d_auto_f));
    d_updater_map["auto_state"] = 1LL << 5;

    // BIT [ 7 ]
    // object position
    d_auto_f.d_updaters.push_back(std::make_unique<ObjectPositionUpdater>(baas, &d_auto_f));
    d_updater_map["object_position"] = 1LL << 6;

}

void AutoFight::update_data()
{
    d_wait_to_update_idx.clear();

    for (int i = 0; i < d_auto_f.d_updaters.size(); ++i) {
        if((1LL << i) & d_auto_f.d_updater_mask) {
            d_wait_to_update_idx.push_back(i);
        }
    }

    if (d_wait_to_update_idx.empty()) {
        return;
    }

    // sort by cost
    std::sort(d_wait_to_update_idx.begin(),d_wait_to_update_idx.end(), [this](auto a, auto b) {
        return d_auto_f.d_updaters[a]->estimated_time_cost() < d_auto_f.d_updaters[b]->estimated_time_cost();
    });


    for (auto idx : d_wait_to_update_idx) {
        d_updater_queue.push(idx);
    }

    // put updater in queue into thread pool
    for (int i = 0; i < d_wait_to_update_idx.size(); ++i) {
        if (d_updater_running_thread_count >= d_update_max_thread){
            std::unique_lock<std::mutex> d_update_thread_lock(d_update_thread_mutex);
            d_update_thread_finish_notifier.wait(d_update_thread_lock, [&]() {
                return d_updater_running_thread_count < d_update_max_thread;
            });
            // condition judgement
        }
        ++d_updater_running_thread_count;
        unsigned char idx = d_updater_queue.front();
        d_updater_queue.pop();
        d_update_thread_pool->submit([this, idx]() {
            submit_data_updater_task(this, idx);
        });
    }

    // all updater submitted
    int start_running_t, initial_t_count = d_updater_running_thread_count;
    for (int i = 1; i <= initial_t_count; ++i) {
        std::unique_lock<std::mutex> d_update_thread_lock(d_update_thread_mutex);
        if (d_updater_running_thread_count == 0) break;
        start_running_t = d_updater_running_thread_count;
        d_update_thread_finish_notifier.wait(d_update_thread_lock, [&]() {
            return d_updater_running_thread_count < start_running_t;
        });
        // condition judgement
    }

    assert(d_updater_running_thread_count == 0);
}

void AutoFight::submit_data_updater_task(AutoFight* self, unsigned char idx)
{
//    auto start_t = std::chrono::high_resolution_clock::now();
    try{
        self->d_auto_f.d_updaters[idx]->update();
    }
    catch (const std::exception &e) {
        self->logger->BAASError("In [ " + self->d_auto_f.d_updaters[idx]->data_name() + " ] update | Error: " + e.what());
    }
    auto end_t = std::chrono::high_resolution_clock::now();
    // us
//    self->logger->BAASInfo("In [ " + self->d_auto_f.d_updaters[idx]->data_name() + " ] update | Time: " + std::to_string(
//            std::chrono::duration_cast<std::chrono::microseconds>(end_t - start_t).count()) + "us");

    self->notify_d_update_thread_end();
}

void AutoFight::display_screenshot_extracted_data()
{
    for (int i = 0; i <= d_auto_f.d_updaters.size(); ++i)
        if ((1LL << i) & d_auto_f.d_updater_mask)
            d_auto_f.d_updaters[i]->display_data();
}


AutoFight::~AutoFight()
{
    logger->BAASInfo("AutoFight Exited.");
    for (auto &updater : d_auto_f.d_updaters) updater.reset();

    d_auto_f.d_updaters.clear();
    _wait_d_update_running_thread_end();
    d_update_thread_pool->shutdown();
}

void AutoFight::init_workflow(const std::filesystem::path& name)
{
    // load workflow json
    _init_d_fight(name);

    // appeared skills
    _init_skills();

    // updaters
    _init_data_updaters();

    // check (record all condition / action / state)
    _action_pre_check();
    _condition_pre_check();
    _state_pre_check();

    // show state cond act count
    logger->sub_title("State Condition Action Count");
    logger->BAASInfo(std::format("|{:>10}|{:>10}|{:>10}|", "state", "action", "condition"));
    logger->BAASInfo(std::format("|{:>10}|{:>10}|{:>10}|",
                                  std::to_string(_state_name_idx_map.size()),
                                  std::to_string(_actions->size()),
                                  std::to_string(_cond_name_idx_map.size())));

    // conditions
    _init_all_cond();

    // actions
    _init_actions();

    // states
    _init_all_state();

}

void AutoFight::_init_single_skill_template(std::string &skill_name)
{
    skill_template _template;
    _template.name = skill_name;

    std::vector<std::string> temp;
    template_info _t_info;
    std::string res_ptr;
    BAASImage _img;

    // active templates
    std::string j_ptr = template_j_ptr_prefix + "/" + skill_name + "/active";
    if (static_config->contains(j_ptr)) temp = static_config->get<std::vector<std::string>>(j_ptr);
    else temp = default_active_skill_template;

    for (const auto &group : temp) {
        res_ptr = BAASImageResource::resource_pointer(baas->get_image_resource_prefix(), group, skill_name);
        if(resource->is_loaded(res_ptr)) resource->get(res_ptr, _img);
        else {
            logger->BAASWarn("Image Resource [ " + res_ptr + " ] not find.");
            continue;
        }
        _t_info.template_image = _img.image;
        _t_info.mean_rgb = _img.mean_rgb;
        _template.skill_active_templates.push_back(_t_info);
    }

    // inactive templates
    j_ptr = template_j_ptr_prefix + "/" + skill_name + "/inactive";
    if (static_config->contains(j_ptr)) temp = static_config->get<std::vector<std::string>>(j_ptr);
    else temp = default_inactive_skill_template;

    for (const auto &group : temp) {
        res_ptr = BAASImageResource::resource_pointer(baas->get_image_resource_prefix(), group, skill_name);
        if(resource->is_loaded(res_ptr)) resource->get(res_ptr, _img);
        else {
            logger->BAASWarn("Image Resource [ " + res_ptr + " ] not find.");
            continue;
        }
        _t_info.template_image = _img.image;
        _t_info.mean_rgb = _img.mean_rgb;
        _template.skill_inactive_templates.push_back(_t_info);
    }

    if (_template.skill_active_templates.empty() && _template.skill_inactive_templates.empty()) {
        logger->BAASError("Didn't find any template for skill [ " + skill_name + " ]. Possible reason : ");
        logger->BAASError("1. Image resource of this skill is not implemented.");
        logger->BAASError("2. You set the wrong skill name in workflow.");
        throw ValueError("Skill Template Not Found");
    }

    logger->BAASInfo("Skill Name: [ " + skill_name + " ]");
    logger->BAASInfo("Index     : [ " + std::to_string(d_auto_f.all_possible_skills.size()) + " ]");
    logger->BAASInfo("Active_T  : [ " + std::to_string(_template.skill_active_templates.size()) + " ]");
    logger->BAASInfo("Inactive_T: [ " + std::to_string(_template.skill_inactive_templates.size()) + " ]");

    d_auto_f.all_possible_skills.push_back(_template);
}

void AutoFight::_init_d_fight(const std::filesystem::path &name)
{
    // consider chinese name
    if(name.empty())  {
        workflow_name = config->getString("/auto_fight/workflow_name");
        workflow_path = config->getPath("/auto_fight/workflow_name").string() + ".json";
    }

    else {
        const std::filesystem::path& temp = name;
        workflow_name = name.string();
        workflow_path = temp.string() + ".json";
    }

    workflow_path = BAAS_AUTO_FIGHT_WORKFLOW_DIR / workflow_path;
    if(!std::filesystem::exists(workflow_path)) {
        logger->BAASCritical("Workflow [ " + workflow_name + " ] do not exist. Please check your config.");
        throw PathError("WorkFlow do not exist.");
    }

    logger->hr("Load WorkFlow");
    logger->BAASInfo("Name : " + workflow_name);
    logger->BAASInfo("Path : " + workflow_path.string());
    d_auto_f.d_fight = BAASConfig(workflow_path, logger);

}

void AutoFight::_init_skills()
{
    logger->hr("Init Skills");
    auto skill_names = d_auto_f.d_fight.get<std::vector<std::string>>("/formation/all_appeared_skills");
    for (int i = 0; i < skill_names.size(); ++i) {
        d_auto_f.skill_name_to_index_map[skill_names[i]] = i;
        _init_single_skill_template(skill_names[i]);
    }
    d_auto_f.slot_count = d_auto_f.d_fight.getInt("/formation/slot_count", 3);
    logger->BAASInfo("Slot Count : [ " + std::to_string(d_auto_f.slot_count) + " ]");

    d_auto_f.skills.resize(d_auto_f.slot_count);
    d_auto_f.skill_last_detect.resize(d_auto_f.slot_count);
    d_auto_f.each_slot_possible_templates.resize(d_auto_f.slot_count);
}

void AutoFight::_action_pre_check()
{
    _actions = std::make_unique<auto_fight_act>(baas, &d_auto_f);
    _actions->_action_pre_check();
}

void AutoFight::_condition_pre_check()
{
    auto _it = d_auto_f.d_fight.find("conditions");
    if (_it == d_auto_f.d_fight.end()) {
        logger->BAASError("Workflow json must contain [ conditions ].");
        throw ValueError("Workflow Conditions Not Found");
    }
    if (!_it->is_object()) {
        logger->BAASError("Workflow [ conditions ] config must be a object.");
        throw TypeError("Invalid [ conditions ] Config Type");
    }

    uint64_t condition_cnt = 0;
    for (const auto& [key, value] : _it->items()) {
        if (!value.is_object()) {
            logger->BAASError("Workflow [ single condition ] config must be an object.");
            logger->BAASError("Error condition key : [ " + key + " ]");
            throw TypeError("Invalid [ single condition ] Config Type");
        }
        _cond_name_idx_map[key] = condition_cnt;
        ++condition_cnt;
    }

}

void AutoFight::_init_all_cond()
{
    // Condition
    logger->hr("Init Conditions");

    _init_self_cond();

    _cond_checked.resize(all_cond.size(), false);
    _cond_self_matched.resize(all_cond.size(), false);
    _cond_is_matched_recorder.resize(all_cond.size(), std::nullopt);
}

void AutoFight::_init_self_cond()
{
    BAASConfig all_conditions;
    d_auto_f.d_fight.getBAASConfig("/conditions", all_conditions, logger);

    for (auto &cond : all_conditions.get_config().items()) {
        try {
            _init_single_cond(BAASConfig(cond.value(), logger));
        }
        catch (const std::exception &e) {
            logger->BAASError("Error condition name : [ " + cond.key() + " ]");
            throw e;
        }
    }
}

void AutoFight::_init_single_cond(const BAASConfig &d_cond)
{
    auto _it = d_cond.find("type");
    if(_it == d_cond.end()) {
        logger->BAASError("[ single condition ] must contain key [ type ].");
        _log_valid_op("[ single condition ] [ type ]", logger, BaseCondition::cond_type_st_list);
        throw ValueError("[ single condition ] [ type ] not found.");
    }

    if(!_it->is_string()) {
        logger->BAASError("[ single condition ] [ type ] must be a string.");
        _log_valid_op("[ single condition ] [ type ]", logger, BaseCondition::cond_type_st_list);
        throw TypeError("Invalid [ single condition ] [ type ] Config Type.");
    }

    std::string _cond_type = *_it;
    auto it = BaseCondition::cond_type_map.find(_cond_type);
    if(it == BaseCondition::cond_type_map.end()) {
        logger->BAASError("Invalid [ single condition ] [ type ] : " + _cond_type);
        _log_valid_op("[ single condition ] [ type ]", logger, BaseCondition::cond_type_st_list);
        throw ValueError("Invalid [ single condition ] [ type ].");
    }

    BaseCondition::ConditionType tp = it->second;
    switch (tp) {
        case BaseCondition::ConditionType::COST:
            all_cond.push_back(std::make_unique<CostCondition>(baas, &d_auto_f, d_cond));
            break;
        case BaseCondition::ConditionType::SKILL_COST:
            break;
        case BaseCondition::ConditionType::BOSS_HEALTH:
            all_cond.push_back(std::make_unique<BossHealthCondition>(baas, &d_auto_f, d_cond));
            break;
        case BaseCondition::ConditionType::ACC_PHASE:
            break;
        case BaseCondition::ConditionType::AUTO_STATE:
            break;
        case BaseCondition::O_COMBINED:
            all_cond.push_back(std::make_unique<OrCombinedCondition>(baas, &d_auto_f, d_cond));
            break;
        case BaseCondition::A_COMBINED:
            all_cond.push_back(std::make_unique<AndCombinedCondition>(baas, &d_auto_f, d_cond));
            break;
        case BaseCondition::SKILL_NAME:
            all_cond.push_back(std::make_unique<SkillNameCondition>(baas, &d_auto_f, d_cond));
            break;
    }

    // and / or cond

    std::vector<uint64_t> _t_idx;
    std::vector<std::string> _t_st;
    // and cond index
    _t_idx.clear();
    _t_st = all_cond.back()->get_and_cond_st();
    for (const auto &st : _t_st) {
        auto _and_it = _cond_name_idx_map.find(st);
        if (_and_it == _cond_name_idx_map.end()) {
            logger->BAASError("Undefined condition found in [ single condition ] [ and ]");
            logger->BAASError("Undefined condition name : [ " + st + " ]");
            throw ValueError("Undefined condition found.");
        }
        _t_idx.push_back(_and_it->second);
    }
    all_cond.back()->set_and_cond(_t_idx);

    // or cond index
    _t_idx.clear();
    _t_st = all_cond.back()->get_or_cond_st();
    for (const auto &st : _t_st) {
        auto _or_it = _cond_name_idx_map.find(st);
        if (_or_it == _cond_name_idx_map.end()) {
            logger->BAASError("Undefined condition found in [ single condition ] [ or ]");
            logger->BAASError("Undefined condition name : [ " + st + " ]");
            throw ValueError("Undefined condition found.");
        }
        _t_idx.push_back(_or_it->second);
    }
    all_cond.back()->set_or_cond(_t_idx);
}

void AutoFight::display_all_cond_info() const noexcept
{
    logger->hr("Display All Conditions");
    display_cond_idx_name_map();
    for (const auto &cond : _cond_name_idx_map) {
        logger->sub_title(cond.first);
        all_cond[cond.second]->display();
    }
}

void AutoFight::display_cond_idx_name_map() const noexcept
{
    logger->sub_title("Condition Index Name Mapping");
    for (const auto &cond : _cond_name_idx_map)
        logger->BAASInfo(std::to_string(cond.second) + " : [ " + cond.first + " ] " );
}

void AutoFight::_init_all_state()
{
    logger->hr("Init States");
    _init_start_state();

    _init_self_state();
}

void AutoFight::_init_self_state()
{
    BAASConfig _t_all_state;
    d_auto_f.d_fight.getBAASConfig("/states", _t_all_state, logger);
    for (auto &stat : _t_all_state.get_config().items())
        _init_single_state(BAASConfig(stat.value(), logger), stat.key());
}

void AutoFight::_init_single_state(const BAASConfig &d_state, const std::string& name)
{
    state_info _state;
    // action
    std::string act = d_state.getString("action");
    if (act.empty()) _state.act_id = std::nullopt;
    else {
        auto it = _actions->act_find(act);
        if (it == _actions->act_end()) {;
            logger->BAASError("Undefined [ action ] found in [ single state ].");
            logger->BAASError("Error state  name : [ " + name + " ]");
            logger->BAASError("Error action name : [ " + act + " ]");
            throw ValueError("State Action Not Found");
        }
        _state.act_id = it->second;
    }

    // transitions
    std::vector<std::string> _trans_next_state_name_recoder;
    if(d_state.contains("transitions")) {
        BAASConfig _t_transitions;
        d_state.getBAASConfig("transitions", _t_transitions, logger);
        if(!_t_transitions.get_config().is_array())  {
            logger->BAASError("In state [ " + name + " ], [ transitions ] must be an array.");
            throw TypeError("Invalid [ transitions ] type.");
        }
        for (auto &trans : _t_transitions.get_config()) {
            auto it = trans.find("condition");
            if(it == trans.end()) {
                logger->BAASError("In state [ " + name + " ],  single transition must contain key [ condition ].");
                throw TypeError("Didn't find [ condition ] in transition.");
            }

            if(!it->is_string()) {
                logger->BAASError("In state [ " + name + " ], value of [ condition ] must be string.");
                throw TypeError("Invalid [ condition ] type in transition.");
            }

            std::string _t_st = *it;

            if (_cond_name_idx_map.find(_t_st) == _cond_name_idx_map.end()) {
                logger->BAASError(fmt::format("Undefined Condition [ {} ] in state transition [ {} ]", _t_st, name));
                throw ValueError("Undefined condition found in [ transition ] ");
            }

            it = trans.find("next");

            if(it == trans.end()) {
                logger->BAASError("In state [ " + name + " ], single transition must contain key [ next ].");
                throw TypeError("Didn't find [ next ] in transition.");
            }

            if(!it->is_string()) {
                logger->BAASError("In state [ " + name + " ], value of [ next ] must be string.");
                throw TypeError("Invalid [ next ] type in transition.");
            }

            _state.transitions.push_back({_cond_name_idx_map[_t_st], 0});
            _trans_next_state_name_recoder.push_back(*it);
        }

    }

    // default transition
    auto _it = d_state.find("default_transition");
    if(_it != d_state.end())  {

        if(!_it->is_string()) {
            logger->BAASError("In [ single state ] [ default_transition ] must be string.");
            logger->BAASError("Error state name : [ " + name + " ]");
            throw TypeError("Invalid [ single state ] [ default_transition ] type");
        }

        std::string _t_st = *_it;
        auto it = _state_name_idx_map.find(_t_st);
        if(it == _state_name_idx_map.end()) {
            logger->BAASError("Undefined [ single state ] [ default_transition ] state name found");
            logger->BAASError("Error state name : [ " + name + " ]");
            logger->BAASError("Undefined state name : [ " + _t_st + " ]");
            throw ValueError("Undefined state found in [ single state ] [ default_transition ]");
        }
        _state.default_trans = it->second;

    }
    else _state.default_trans = std::nullopt;

    // act fail transition
    _it = d_state.find("action_fail_transition");
    if(_it != d_state.end()) {

        if(!_it->is_string()) {
            logger->BAASError("In [ single state ] [ action_fail_transition ] must be string.");
            logger->BAASError("Error state name : [ " + name + " ]");
            throw TypeError("Invalid [ single state ] [ action_fail_transition ] type");
        }

        std::string _t_st = *_it;
        auto it = _state_name_idx_map.find(_t_st);
        if(it == _state_name_idx_map.end()) {
            logger->BAASError("Undefined [ single state ] [ action_fail_transition ] state name found");
            logger->BAASError("Error state name : [ " + name + " ]");
            logger->BAASError("Undefined state name : [ " + _t_st + " ]");
            throw ValueError("Undefined state found in [ single state ] [ action_fail_transition ]");
        }

    }
    else _state.act_fail_trans = std::nullopt;

    // desc
    _state.desc = d_state.getString("desc");

    // name
    _state.name = name;

    all_states.push_back(_state);
}

// start state
void AutoFight::_init_start_state()
{
    auto _it = d_auto_f.d_fight.find("start_state");
    if (_it == d_auto_f.d_fight.end()) {
        logger->BAASError("Workflow json must contain [ start_state ].");
        throw ValueError("Workflow Start State Not Found");
    }

    if (!_it->is_string()) {
        logger->BAASError("Workflow [ start_state ] config must be a string.");
        throw TypeError("Invalid [ start_state ] Config Type");
    }

    start_state_name = *_it;
    auto it = _state_name_idx_map.find(start_state_name);
    if (it == _state_name_idx_map.end()) {
        logger->BAASError("Undefined [ start_state ] --> [ " + start_state_name + " ] .");
        throw ValueError("Undefined State Found.");
    }

    _curr_state_idx = it->second;
    logger->sub_title("Start State");
    logger->BAASInfo("Name  : [ " + start_state_name + " ]");
    logger->BAASInfo("Index : [ " + std::to_string(_curr_state_idx) + " ]");

}

void AutoFight::display_all_state() const noexcept
{
    logger->hr("Display All States.");
    display_state_idx_name_map();
        logger->BAASInfo("State_Cnt : [ " + std::to_string(all_states.size()) + " ]");
        logger->BAASLine();
    for (int i = 0; i < all_states.size(); ++i) {
        logger->BAASInfo("Name      : " + all_states[i].name);
        logger->BAASInfo("Index     : " + std::to_string(i));
        if(all_states[i].act_id.has_value())
        logger->BAASInfo("Act_Idx   : " + std::to_string(all_states[i].act_id.value()));

        logger->BAASInfo("T_Cnt     : " + std::to_string(all_states[i].transitions.size()));

        if(all_states[i].default_trans.has_value())
        logger->BAASInfo("D_T_Idx   : " + std::to_string(all_states[i].default_trans.value()));

        if(all_states[i].act_fail_trans.has_value())
        logger->BAASInfo("A_F_T_Idx : " + std::to_string(all_states[i].act_fail_trans.value()));

        if(!all_states[i].desc.empty())
        logger->BAASInfo("Desc      : " + all_states[i].desc);

        if(!all_states[i].transitions.empty()) {
        logger->BAASInfo("CondIdx  NextIdx");
        for (const auto& tran : all_states[i].transitions)
            logger->BAASInfo(fmt::format("{:>7}  {:>7}", tran.cond_idx, tran.next_sta_idx));
        }

        logger->BAASLine();
    }
}

void AutoFight::display_state_idx_name_map() const noexcept
{
    logger->sub_title("State Index Name Mapping");
    for (const auto &state : _state_name_idx_map)
        logger->BAASInfo(std::to_string(state.second) + " : [ " + state.first + " ] " );
}

void AutoFight::start_state_transition()
{
    enter_fight();
    _start_state_transition_loop();
}

void AutoFight::ensure_fighting_page()
{

}

void AutoFight::enter_fight()
{
    baas->solve_procedure("UI-FROM_PAGE_formation_TO_PAGE_fighting", true);
}

void AutoFight::restart_fight(bool update_room_left_time)
{
    baas->solve_procedure("UI-GO-TO_PAGE_fight-pause", true);
    if(update_room_left_time) {
        // TODO:: update room left time
    }
    baas->solve_procedure("fight_confirm_restart", true);
}

void AutoFight::_state_pre_check()
{
    auto _it = d_auto_f.d_fight.find("states");
    if (_it == d_auto_f.d_fight.end()) {
        logger->BAASError("Workflow json must contain [ states ].");
        throw std::runtime_error("Workflow States Not Found");
    }

    if (!_it->is_object()) {
        logger->BAASError("Workflow [ states ] config must be a object.");
        throw std::runtime_error("Invalid [ states ] Config Type");
    }

    uint64_t state_cnt = 0;

    for (const auto& [key, value] : _it->items()) {
        if (!value.is_object()) {
            logger->BAASError("Workflow [ single state ] config must be an object.");
            logger->BAASError("Error state key : [ " + key + " ]");
            throw std::runtime_error("Invalid [ single state ] Config Type.");
        }
        _state_name_idx_map[key] = state_cnt;
        ++state_cnt;
    }

}

/*
 * Returns:
 *      true  --> need to transit to next state
 *      false --> stop state transition
 */

bool AutoFight::_state_start_trans_cond_j_loop()
{
    _state_reset_all_cond();
    _state_cond_j_start_t = BAASUtil::getCurrentTimeMS();
    _state_cond_j_loop_running_flg = true;

    while(_state_cond_j_loop_running_flg) {
        // time elapsed update
        _state_update_cond_j_loop_start_t();

        // timeout check
        if (!_state_cond_timeout_update()) {
            _state_flg_all_trans_cond_dissatisfied = true;
            return _try_default_trans();
        }

        // screenshot update
        baas->i_update_screenshot_array();
        baas->reset_all_feature();

        // check at fighting page
        if (!d_auto_f.d_updaters[0]->at_fight_page()) continue;

        // data update with condition judgement
        _state_set_d_update_flags();

        d_wait_to_update_idx.clear();

        for (int i = 0; i < d_auto_f.d_updaters.size(); ++i)
            if((1LL << i) & d_auto_f.d_updater_mask)
                d_wait_to_update_idx.push_back(i);

        // sort by cost
        std::sort(d_wait_to_update_idx.begin(),d_wait_to_update_idx.end(), [this](auto a, auto b) {
            return d_auto_f.d_updaters[a]->estimated_time_cost() < d_auto_f.d_updaters[b]->estimated_time_cost();
        });


        for (auto idx : d_wait_to_update_idx) {
            d_updater_queue.push(idx);
        }

        // wait d_updater_thread_count = 0
        if(d_updater_running_thread_count > 0) _wait_d_update_running_thread_end();

        assert(d_updater_running_thread_count == 0);

        // submit updaters into thread pool
        for (int i = 0; i < d_wait_to_update_idx.size(); ++i) {
            if (d_updater_running_thread_count >= d_update_max_thread){
                std::unique_lock<std::mutex> d_update_thread_lock(d_update_thread_mutex);
                d_update_thread_finish_notifier.wait(d_update_thread_lock, [&]() {
                    return d_updater_running_thread_count < d_update_max_thread;
                });

                // condition judgement
                _state_cond_j();
                if(_state_trans_next_state_idx.has_value()) return true;
                if(_state_flg_all_trans_cond_dissatisfied)  return _try_default_trans();
            }
            ++d_updater_running_thread_count;
            unsigned char idx = d_updater_queue.front();
            d_updater_queue.pop();
            d_update_thread_pool->submit([this, idx]() {
                submit_data_updater_task(this, idx);
            });
        }

        // when ever a updater finished, do condition judgement
        int start_running_t, initial_t_count = d_updater_running_thread_count;
        for (int i = 1; i <= initial_t_count; ++i) {
            std::unique_lock<std::mutex> d_update_thread_lock(d_update_thread_mutex);
            if (d_updater_running_thread_count == 0) break;
            start_running_t = d_updater_running_thread_count;
            d_update_thread_finish_notifier.wait(d_update_thread_lock, [&]() {
                return d_updater_running_thread_count < start_running_t;
            });

            // condition judgement
            _state_cond_j();
            if(_state_trans_next_state_idx.has_value())   return true;
            if(_state_flg_all_trans_cond_dissatisfied)    return _try_default_trans();
        }
    }
    return false;
}

// reset all cond
void AutoFight::_state_reset_all_cond()
{
    _state_flg_all_trans_cond_dissatisfied = false;
    _state_trans_next_state_idx.reset();
    std::fill(_cond_self_matched.begin(), _cond_self_matched.end(), std::nullopt);
    std::fill(_cond_is_matched_recorder.begin(), _cond_is_matched_recorder.end(), std::nullopt);

    for(const auto& cond : all_cond) cond->reset_state();

}

/*
 * Returns:
 *          true  --> there is a pending condition
 *          false --> all conditions dissatisfied
 */

bool AutoFight::_state_cond_timeout_update()
{
    std::fill(_cond_checked.begin(), _cond_checked.end(), false);

    bool f_has_pending_cond = false;
    for(const auto& trans : all_states[_curr_state_idx].transitions) {
        // has_value  /  checked
        if(!_cond_is_pending(trans.cond_idx) || _cond_checked[trans.cond_idx]) continue;
        if (_recursive_check_cond_timeout(trans.cond_idx)) f_has_pending_cond = true;
    }
    return f_has_pending_cond;
}

void AutoFight::_start_state_transition_loop()
{

    while (1) {
        if(all_states[_curr_state_idx].act_id.has_value()) {
            bool act_ret = _actions->_execute(all_states[_curr_state_idx].act_id.value());
            if (!act_ret)
                if(all_states[_curr_state_idx].act_fail_trans.has_value()) {
                    logger->BAASInfo("Execute State [ Action Fail Transition ]");
                    _state_trans_next_state_idx = all_states[_curr_state_idx].act_fail_trans.value();

                }

        }


        if(!_state_start_trans_cond_j_loop()) break;

        assert(_state_trans_next_state_idx.has_value());

        _execute_state_transition();
    }
}

// cond j should tell if there is a transition condition satisfied or no condition satisfied
void AutoFight::_state_cond_j()
{
    std::fill(_cond_checked.begin(), _cond_checked.end(), false);
    _state_flg_all_trans_cond_dissatisfied = true;
    std::optional<bool> ret;
    // top conditions
    for (int i = 0; i < all_states[_curr_state_idx].transitions.size(); ++i) {
        auto tran = all_states[_curr_state_idx].transitions[i];

        // cond has value
        if (!_cond_is_pending(tran.cond_idx))
            if(_cond_is_satisfied(tran.cond_idx)) {
                _state_trans_next_state_idx = tran.next_sta_idx;
                return;
            }

        if(_cond_checked[tran.cond_idx]) continue;

        ret = _recursive_cond_j(tran.cond_idx);
        if(ret.has_value()) {
            if(ret.value()) {
                _state_trans_next_state_idx = tran.next_sta_idx;
                return;
            }
        }
        else _state_flg_all_trans_cond_dissatisfied = false;
    }
}

/*
 * Returns:
 *          std::nullopt --> pending
 *          true         --> satisfied
 *          false        --> dissatisfied
 *
 *  self pending      --> check and
 *
 *                        and dissatisfied        --> return false
 *                        and satisfied / pending --> check or
 *
 *                                                    or satisfied              --> return true
 *                                                    or dissatisfied / pending --> return nullopt
 *
 *  self dissatisfied --> check or
 *
 *                        or satisfied    --> return true
 *                        or pending      --> return nullopt
 *                        or dissatisfied --> return false
 *
 *  self satisfied    --> check and
 *
 *                        and satisfied    --> return true
 *                        and pending      --> return nullopt
 *                        and dissatisfied --> return false
 */

std::optional<bool> AutoFight::_recursive_cond_j(uint64_t cond_idx)
{
    std::optional<bool> final_ret = std::nullopt;
    _cond_checked[cond_idx] = true;

    // self match state
    std::optional<bool> ret;
    if (!_cond_self_matched[cond_idx].has_value())
        ret = all_cond[cond_idx]->try_match();
    else
        ret = _cond_self_matched[cond_idx];

    // self pending
    if(!ret.has_value()) {
         // check and
         for(const auto& _and : all_cond[cond_idx]->get_and_cond())  {
            if(!_cond_is_pending(_and)) {
                if(_cond_is_dissatisfied(_and)) {
                    final_ret = false;
                    break;
                }
                continue;
            }

            if(_cond_checked[_and]) continue;
            ret = _recursive_cond_j(_and);

            if(ret.has_value() && !ret.value()) {
                final_ret = false;
                break;
            }
         }

         if(final_ret.has_value()) {
             _cond_is_matched_recorder[cond_idx] = final_ret;
             return final_ret;
         }

         // check or
         for(const auto& _or : all_cond[cond_idx]->get_or_cond()) {
             if(!_cond_is_pending(_or)) {
                 if(_cond_is_satisfied(_or)) {
                     final_ret = true;
                     break;
                 }
                 continue;
             }


             if(_cond_checked[_or]) continue;
             ret = _recursive_cond_j(_or);

             if(ret.has_value() && ret.value()) {
                 final_ret = true;
                 break;
             }
         }
    }
    else {
        _cond_self_matched[cond_idx] = true;

        // self satisfied
        if(ret.value()) {
            // check and
            bool has_pending = false;
            if(all_cond[cond_idx]->has_and_cond())
                for (const auto &and_cond : all_cond[cond_idx]->get_and_cond()) {
                    if (!_cond_is_pending(and_cond) || _cond_checked[and_cond]) continue;
                    ret = _recursive_cond_j(and_cond);
                    if (!ret.has_value()) {
                        has_pending = true;
                        break;
                    }
                    if (!ret.value()) {
                        final_ret = false;
                        break;
                    }
                }
            // all and cond satisfied
            if (final_ret != false && !has_pending) final_ret = true;
        }

        // self dissatisfied
        else {
            // check or
            bool has_pending = false;
            if(all_cond[cond_idx]->has_or_cond())
                for (const auto &or_cond : all_cond[cond_idx]->get_or_cond()) {
                    if (_cond_is_pending(or_cond) || _cond_checked[or_cond]) continue;
                    ret = _recursive_cond_j(or_cond);
                    if (!ret.has_value()) {
                        has_pending = true;
                        break;
                    }
                    if (ret.value()) {
                        final_ret = true;
                        break;
                    }
                }
            // all or cond dissatisfied
            if (final_ret != true && !has_pending) final_ret = false;
        }
    }

    _cond_is_matched_recorder[cond_idx] = final_ret;
    return final_ret;
}

void AutoFight::_state_set_d_update_flags()
{
    std::fill(_cond_checked.begin(), _cond_checked.end(), false);
    d_auto_f.d_updater_mask = 0b1000000;
    d_auto_f.boss_health_update_flag = 0b000;
    d_auto_f.skill_cost_update_flag = 0b000;

    for (auto& s : d_auto_f.each_slot_possible_templates) s.clear();
    for (auto& s : d_auto_f.skills) s.reset();

    for (const auto tran : all_states[_curr_state_idx].transitions) {
        if (!_cond_is_pending(tran.cond_idx) || _cond_checked[tran.cond_idx]) continue;
        _recursive_set_d_update_flags(tran.cond_idx);
    }
}

bool AutoFight::_recursive_check_cond_timeout(uint64_t cond_idx)
{
    _cond_checked[cond_idx] = true;
    BaseCondition* cond = all_cond[cond_idx].get();

    // check self timeout
    if(!_cond_self_matched[cond_idx].has_value())
        if(_state_cond_j_elapsed_t >= cond->get_timeout()) {
            _cond_is_matched_recorder[cond_idx] = false;
            return false;
        }

    // check and cond timeout
    if(cond->has_and_cond()) {
        for (const auto &and_cond : cond->get_and_cond()) {
            // has_value  /  checked
            if(!_cond_is_pending(and_cond) || _cond_checked[and_cond]) continue;
            // and cond timeout
            if (!_recursive_check_cond_timeout(and_cond)) {
                _cond_is_matched_recorder[cond_idx] = false;
                return false;
            }
        }
    }

    // check or timeout
    if(cond->has_or_cond()) {
        for (const auto &or_cond : cond->get_or_cond()) {
            // has_value  /  checked
            if(!_cond_is_pending(or_cond) || _cond_checked[or_cond]) continue;
            _recursive_check_cond_timeout(or_cond);
        }
    }

    // self / and cond not timeout
    return true;
}

void AutoFight::_state_update_cond_j_loop_start_t()
{
    _state_cond_j_loop_start_t = BAASUtil::getCurrentTimeMS();
    _state_cond_j_elapsed_t = _state_cond_j_loop_start_t - _state_cond_j_start_t;
}

void AutoFight::_recursive_set_d_update_flags(uint64_t cond_idx)
{
    _cond_checked[cond_idx] = true;
    // self still pending
    if (!_cond_self_matched[cond_idx].has_value())
        all_cond[cond_idx]->set_d_update_flag();

    if(all_cond[cond_idx]->has_and_cond()) {
        for (const auto &and_cond : all_cond[cond_idx]->get_and_cond()) {
            if(!_cond_is_pending(and_cond) || _cond_checked[and_cond]) continue;
            _recursive_set_d_update_flags(and_cond);
        }
    }

    if(all_cond[cond_idx]->has_or_cond()) {
        for (const auto &or_cond : all_cond[cond_idx]->get_or_cond()) {
            if(_cond_is_pending(or_cond) || _cond_checked[or_cond]) continue;
            _recursive_set_d_update_flags(or_cond);
        }
    }
}

void AutoFight::_wait_d_update_running_thread_end()
{
    int start_running_t, initial_t_count = d_updater_running_thread_count;
    for (int i = 1; i <= initial_t_count; ++i) {
        std::unique_lock<std::mutex> d_update_thread_lock(d_update_thread_mutex);
        if (d_updater_running_thread_count == 0) return;
        start_running_t = d_updater_running_thread_count;
        d_update_thread_finish_notifier.wait(d_update_thread_lock, [&]() {
            return d_updater_running_thread_count < start_running_t;
        });
    }
}

bool AutoFight::_try_default_trans()
{
    logger->sub_title("All trans cond dissatisfied, try default trans");
    if(all_states[_curr_state_idx].default_trans.has_value()) {
        logger->BAASInfo("[ Default Trans ]");
        _state_trans_next_state_idx = all_states[_curr_state_idx].default_trans.value();
        return true;
    }
    return false;
}

void AutoFight::_init_actions()
{
    _actions->_init_all_act();
}

BAAS_NAMESPACE_END
