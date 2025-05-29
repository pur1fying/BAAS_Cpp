//
// Created by pc on 2025/4/23.
//

#include "module/auto_fight/auto_fight.h"

#include <format>

#include <BAASImageResource.h>

#include "utils.h"
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

void AutoFight::init_workflow(const std::filesystem::path &name)
{
    // load workflow json
    _init_d_fight(name);

    // appeared skills
    _init_skills();

    // conditions
    _init_all_cond();

    // updaters
    _init_data_updaters();

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
    // active templates
    std::string j_ptr = template_j_ptr_prefix + "/" + skill_name + "/active";

    if (static_config->contains(j_ptr)) temp = static_config->get<std::vector<std::string>>(j_ptr);
    else temp = default_active_skill_template;

    template_info _t_info;
    std::string res_ptr;
    BAASImage _img;
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

    logger->BAASInfo("Skill Name: [ " + skill_name + " ]");
    logger->BAASInfo("Index     : [ " + std::to_string(d_auto_f.all_possible_skills.size()) + " ]");
    logger->BAASInfo("Active    : [ " + std::to_string(_template.skill_active_templates.size()) + " ]");
    logger->BAASInfo("Inactive  : [ " + std::to_string(_template.skill_inactive_templates.size()) + " ]");

    d_auto_f.all_possible_skills.push_back(_template);
}

void AutoFight::_init_cond_and_or_idx()
{
    logger->sub_title("Init Condition And / Or Index.");
    // convert st --> idx
    std::vector<uint64_t> _t_idx;
    std::vector<std::string> _t_st;
    for (int i = 0; i < all_cond.size(); ++i) {
        // and cond index
        _t_idx.clear();
        _t_st = all_cond[i]->get_and_cond_st();
        for (const auto &st : _t_st) {
            if (cond_name_idx_map.find(st) == cond_name_idx_map.end()) {
                logger->BAASError("In Condition [ " + std::to_string(i) +" ] And Cond [ " + st + " ] not found.");
                throw ValueError("Undefined condition found in [ and ] condition");
            }
            _t_idx.push_back(cond_name_idx_map[st]);
        }
        all_cond[i]->set_and_cond(_t_idx);

        // or cond index
        _t_idx.clear();
        _t_st = all_cond[i]->get_or_cond_st();
        for (const auto &st : _t_st) {
            if (cond_name_idx_map.find(st) == cond_name_idx_map.end()) {
                logger->BAASError("In Condition [ " + std::to_string(i) +" ] Or Cond [ " + st + " ] not found.");
                throw ValueError("Undefined condition found in [ or ] condition");
            }
            _t_idx.push_back(cond_name_idx_map[st]);
        }
        all_cond[i]->set_or_cond(_t_idx);
    }
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

void AutoFight::_init_all_cond()
{
    // Condition
    logger->hr("Init Conditions");

    _init_self_cond();

    _cond_checked.resize(all_cond.size(), false);
    _cond_self_matched.resize(all_cond.size(), false);
    _cond_is_matched_recorder.resize(all_cond.size(), std::nullopt);
    _init_cond_and_or_idx();
}

void AutoFight::_init_self_cond()
{
    // condition in this json file
    if (!d_auto_f.d_fight.contains("/conditions")) {
        logger->BAASWarn("No condition in workflow file.");
        return;
    }

    BAASConfig all_conditions;
    d_auto_f.d_fight.getBAASConfig("/conditions", all_conditions, logger);
    if(!all_conditions.get_config().is_object()) {
        logger->BAASError("Elements of [ conditions ] must be object.");
        throw TypeError("In Workflow file [ conditions ] must be object.");
    }
    int suc_cnt = 0;
    bool ret;
    for (auto &cond : all_conditions.get_config().items()) {
        ret = _init_single_cond(BAASConfig(cond.value(), logger));
        if (ret) {
            ++suc_cnt;
            cond_name_idx_map[cond.key()] = all_cond.size() - 1;
        }
    }

    logger->BAASInfo("[ SELF ] Successfully Load [ " + std::to_string(suc_cnt) + " ] Conditions");
}

bool AutoFight::_init_single_cond(const BAASConfig &d_cond)
{
    if(!d_cond.contains("type")) {
        logger->BAASInfo("Condition must contains [ type ].");
        return false;
    }
    if(d_cond.value_type("type") != nlohmann::detail::value_t::string) {
        logger->BAASInfo("[ type ] must be string.");
        return false;
    }

    _cond_type = d_cond.getString("type");
    if(!BaseCondition::is_condition_valid(_cond_type)) {
        logger->BAASInfo("Invalid condition type [ " + _cond_type + " ]");
        return false;
    }

    BaseCondition::ConditionType tp = BaseCondition::type_st_to_idx(_cond_type);
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
    return true;
}

void AutoFight::display_all_cond_info() const noexcept
{
    logger->hr("Display All Conditions.");
    display_cond_idx_name_map();
    for (const auto &cond : cond_name_idx_map) {
        logger->sub_title(cond.first);
        all_cond[cond.second]->display();
    }
}

void AutoFight::display_cond_idx_name_map() const noexcept
{
    logger->sub_title("Condition Index Name Mapping");
    for (const auto &cond : cond_name_idx_map)
        logger->BAASInfo(std::to_string(cond.second) + " : [ " + cond.first + " ] " );
}

void AutoFight::_init_all_state()
{
    logger->hr("Init States");

    _init_self_state();

    _init_start_state();

    _conv_tans_state_name_st_to_idx();
}

void AutoFight::_init_self_state()
{
    // condition in this json file
    if (!d_auto_f.d_fight.contains("/states")) {
        logger->BAASWarn("No state in workflow file.");
        return;
    }

    BAASConfig _t_all_state;
    d_auto_f.d_fight.getBAASConfig("/states", _t_all_state, logger);
    if(!_t_all_state.get_config().is_object()) {
        logger->BAASError("Elements of [ states ] must be object.");
        throw TypeError("In Workflow file [ states ] must be object.");
    }
    int suc_cnt = 0;
    bool ret;
    for (auto &stat : _t_all_state.get_config().items()) {
        ret = _init_single_state(BAASConfig(stat.value(), logger), stat.key());
        if (ret) {
            ++suc_cnt;
            state_name_idx_map[stat.key()] = all_states.size() - 1;
        }
    }

    logger->BAASInfo("[ SELF ] Successfully Load [ " + std::to_string(suc_cnt) + " ] States");
}

bool AutoFight::_init_single_state(const BAASConfig &d_state, const std::string& name)
{
    state_info _state;
    // action
    std::string act = d_state.getString("action");
    if (act.empty()) _state.act_id = std::nullopt;
    else {
        if (!_actions->act_exist(act)) {;
            logger->BAASError("Action [ " + act + " ] not found.");
            throw ValueError("Action not found.");
        }
        _state.act_id = _actions->get_act_id(act);
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

            if (cond_name_idx_map.find(_t_st) == cond_name_idx_map.end()) {
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

            _state.transitions.push_back({cond_name_idx_map[_t_st], 0});
            _trans_next_state_name_recoder.push_back(*it);
        }

    }

    // "next" , "default_transition" , "action_fail_transition" int64 value should not be init here
    if(d_state.contains("default_transition"))  {
        if(d_state.value_type("default_transition") != nlohmann::detail::value_t::string) {
            logger->BAASError("In state [ " + name + " ] , value of [ default_transition ] must be string.");
            throw TypeError("Invalid [ default_transition ] type in state.");
        }
        _state_default_trans_name_recorder.emplace_back(d_state.getString("default_transition"));
    }
    else _state_default_trans_name_recorder.emplace_back(std::nullopt);

    if(d_state.contains("action_fail_transition")) {
        if(d_state.value_type("action_fail_transition") != nlohmann::detail::value_t::string) {
            logger->BAASError("In state [ " + name + " ] , value of [ action_fail_transition ] must be string.");
            throw TypeError("Invalid [ action_fail_transition ] type in state.");
        }
        _state_act_fail_trans_name_recorder.emplace_back(d_state.getString("action_fail_transition"));
    }
    else _state_act_fail_trans_name_recorder.emplace_back(std::nullopt);

    _state_trans_name_recorder.push_back(_trans_next_state_name_recoder);

    // desc
    _state.desc = d_state.getString("desc");

    // name
    _state.name = name;

    all_states.push_back(_state);
    return true;
}

// start state
void AutoFight::_init_start_state()
{
    if (!d_auto_f.d_fight.contains("start_state")) {
        logger->BAASError("Didn't find [ start_state ] in auto fight workflow.");
        throw ValueError("[ start_state ] not found.");
    }
    start_state_name = d_auto_f.d_fight.getString("start_state");
    auto it = state_name_idx_map.find(start_state_name);
    if (it == state_name_idx_map.end()) {
        logger->BAASError("Undefined [ start state ] --> [ " + start_state_name + " ] .");
        throw ValueError("Start State Invalid.");
    }

    _curr_state_idx = it->second;
    logger->sub_title("Start State");
    logger->BAASInfo("Name  : [ " + start_state_name + " ]");
    logger->BAASInfo("Index : [ " + std::to_string(_curr_state_idx) + " ]");

}

void AutoFight::_conv_tans_state_name_st_to_idx()
{

    for (int i = 0; i < all_states.size(); ++i) {
        // convert transitions
        for(int j = 0; j < all_states[i].transitions.size(); ++j) {
            auto _trans_next_state_name_it = state_name_idx_map.find(_state_trans_name_recorder[i][j]);
            if (_trans_next_state_name_it == state_name_idx_map.end()) {
                logger->BAASError("Undefined state [ " + _state_trans_name_recorder[i][j] + " ] found in transitions.");
                throw ValueError("Invalid State Name.");
            }
            all_states[i].transitions[j].next_sta_idx = _trans_next_state_name_it->second;
        }

        // convert default transitions
        if (_state_default_trans_name_recorder[i].has_value()) {
            auto _default_trans_state_name_it = state_name_idx_map.find(_state_default_trans_name_recorder[i].value());
            if(_default_trans_state_name_it == state_name_idx_map.end()) {
                logger->BAASError("Undefined state [ " + _state_default_trans_name_recorder[i].value() + " ] "
                                  "found in [ Default Transition ].");
                throw ValueError("Invalid State Name.");
            }
            all_states[i].default_trans = _default_trans_state_name_it->second;
        }

        // convert action fail transitions
        if (_state_act_fail_trans_name_recorder[i].has_value()) {
            auto _act_fail_trans_state_name_it = state_name_idx_map.find(_state_act_fail_trans_name_recorder[i].value());
            if (_act_fail_trans_state_name_it == state_name_idx_map.end()) {
                logger->BAASError("Undefined state [ " + _state_act_fail_trans_name_recorder[i].value() + " ] "
                                  "found in [ Action Fail Transition ].");
                throw ValueError("Invalid State Name.");
            }
        }
    }

    _state_trans_name_recorder.clear();
    _state_default_trans_name_recorder.clear();
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
    for (const auto &state : state_name_idx_map)
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
    _actions = std::make_unique<auto_fight_act>(baas, &d_auto_f);
}

BAAS_NAMESPACE_END
