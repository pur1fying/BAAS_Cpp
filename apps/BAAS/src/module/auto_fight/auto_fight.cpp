//
// Created by pc on 2025/4/23.
//

#include "module/auto_fight/auto_fight.h"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <format>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>

#include "BAASImageResource.h"
#include "module/auto_fight/workflow/ConditionConfig.h"
#include "module/auto_fight/workflow/WorkflowFileParser.h"
#include "module/auto_fight/workflow/WorkflowResources.h"
#include "config/BAASStaticConfig.h"
#include "module/auto_fight/WorkflowRoute.h"
#include "module/auto_fight/conditions/AndCombinedCondition.h"
#include "module/auto_fight/conditions/BattleTimeCondition.h"
#include "module/auto_fight/conditions/BossHealthCondition.h"
#include "module/auto_fight/conditions/ConditionFactory.h"
#include "module/auto_fight/conditions/condition_runtime_evaluator.h"
#include "module/auto_fight/conditions/CostCondition.h"
#include "module/auto_fight/conditions/OrCombinedCondition.h"
#include "module/auto_fight/conditions/SkillNameCondition.h"
#include "module/auto_fight/screenshot_data/AccelerationPhaseUpdater.h"
#include "module/auto_fight/screenshot_data/AutoStateUpdater.h"
#include "module/auto_fight/screenshot_data/BattleTimeUpdater.h"
#include "module/auto_fight/screenshot_data/BossHealthUpdater.h"
#include "module/auto_fight/screenshot_data/CostUpdater.h"
#include "module/auto_fight/screenshot_data/ObjectPositionUpdater.h"
#include "module/auto_fight/screenshot_data/SkillCostUpdater.h"
#include "module/auto_fight/screenshot_data/SkillNameUpdater.h"
#include "utils.h"
#include "utils/BAASChronoUtil.h"

BAAS_NAMESPACE_BEGIN

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
void AutoFight::_init_data_updaters(const WorkflowModel& workflow)
{
    d_auto_f.d_updaters.clear();
    d_updater_map.clear();
    d_auto_f.all_possible_obj_names = *workflow.formation.front;

    // BIT [ 0 ]
    // cost
    d_auto_f.d_updaters.push_back(std::make_unique<CostUpdater>(baas, &d_auto_f));
    d_updater_map["cost"] = 1LL << 0;

    // BIT [ 1 ]
    // boss health
    d_auto_f.d_updaters.push_back(std::make_unique<BossHealthUpdater>(baas, &d_auto_f, *workflow.boss_health));
    d_updater_map["boss_health"] = 1LL << 1;

    // BIT [ 2 ]
    // skill name
    d_auto_f.d_updaters.push_back(std::make_unique<SkillNameUpdater>(baas, &d_auto_f));
    d_updater_map["skill_name"] = 1LL << 2;

    // BIT [ 3 ]
    // skill cost
    d_auto_f.d_updaters.push_back(std::make_unique<SkillCostUpdater>(baas, &d_auto_f,
            workflow_resources().skill_cost_regions,
            workflow_resources().skill_cost_ocr_model));
    d_updater_map["skill_cost"] = 1LL << 3;

    // BIT [ 4 ]
    // acc phase
    d_auto_f.d_updaters.push_back(std::make_unique<AccelerationPhaseUpdater>(baas, &d_auto_f));
    d_updater_map["acc_phase"] = 1LL << 4;

    // BIT [ 5 ]
    // auto state
    d_auto_f.d_updaters.push_back(std::make_unique<AutoStateUpdater>(baas, &d_auto_f));
    d_updater_map["auto_state"] = 1LL << 5;

    // BIT [ 6 ]
    // object position
    d_auto_f.d_updaters.push_back(std::make_unique<ObjectPositionUpdater>(baas, &d_auto_f, *workflow.yolo));
    d_updater_map["object_position"] = 1LL << 6;

    // BIT [ 7 ]
    // battle left time
    d_auto_f.d_updaters.push_back(std::make_unique<BattleTimeUpdater>(baas, &d_auto_f, *workflow.battle_time));
    d_updater_map["battle_time"] = 1LL << 7;

    d_auto_f.default_d_updater_mask = 0b01000000;
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
            d_auto_f.d_updaters[d_auto_f.last_updated_data_idx]->write_result_into_data();
            // condition judgement
        }
        ++d_updater_running_thread_count;
        unsigned char idx = d_updater_queue.front();
        d_updater_queue.pop();
        d_update_thread_pool->submit([this, idx]() {
            submit_data_updater_task(this, idx, d_auto_f.round_cnt);
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
        d_auto_f.d_updaters[d_auto_f.last_updated_data_idx]->write_result_into_data();
        // condition judgement
    }

    assert(d_updater_running_thread_count == 0);
}

void AutoFight::submit_data_updater_task(AutoFight* self, unsigned char idx, uint64_t round)
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

    self->notify_d_update_thread_end(idx, round);
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
    _resolve_workflow_path(name);
    const auto* connection = baas->get_connection();
    const WorkflowParseOptions options{
        .EmulatorServer = connection->get_server(),
        .EmulatorLanguage = connection->get_language(),
    };
    const auto workflow = parse_workflow_file(workflow_path, options);
    for (const auto& diagnostic : workflow.Errors) {
        const auto message = diagnostic.code + " [ " + diagnostic.path + " ] " + diagnostic.message;
        if (diagnostic.severity == DiagnosticSeverity::ERROR_DIAGNOSTIC) {
            logger->BAASError(message);
        } else {
            logger->BAASWarn(message);
        }
    }
    if (workflow.has_errors()) {
        throw ValueError("Invalid Workflow");
    }
    d_auto_f.battle_type = *workflow.battle_type;
    _init_skills(workflow.formation);
    _init_data_updaters(workflow);
    _cond_name_idx_map.clear();
    _state_name_idx_map.clear();
    for (const auto& [condition_name, config] : workflow.conditions) {
        _cond_name_idx_map.emplace(condition_name, _cond_name_idx_map.size());
    }
    for (const auto& [state_name, config] : workflow.states) {
        _state_name_idx_map.emplace(state_name, _state_name_idx_map.size());
    }
    _init_all_cond(workflow.conditions);
    _init_actions(workflow.actions);
    _init_all_state(workflow);
}

void AutoFight::_init_single_skill_template(const std::string& skill_name)
{
    skill_template _template;
    _template.name = skill_name;

    const auto* connection = baas->get_connection();
    const auto profile = connection->get_server() + "." + connection->get_language();
    const auto& templates = workflow_resources().skills.at(profile).at(skill_name);
    template_info _t_info;
    std::string res_ptr;
    BAASImage _img;

    for (const auto& group : templates.active) {
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

    for (const auto& group : templates.inactive) {
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

void AutoFight::_resolve_workflow_path(const std::filesystem::path& name)
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
    logger->hr("Load WorkFlow");
    logger->BAASInfo("Name : " + workflow_name);
    logger->BAASInfo("Path : " + workflow_path.string());

}

void AutoFight::_init_skills(const FormationConfig& formation)
{
    logger->hr("Init Skills");
    d_auto_f.skill_name_to_index_map.clear();
    d_auto_f.all_possible_skills.clear();
    const auto& names = *formation.all_appeared_skills;
    for (std::size_t i = 0; i < names.size(); ++i) {
        d_auto_f.skill_name_to_index_map.emplace(names[i], i);
        _init_single_skill_template(names[i]);
    }
    d_auto_f.slot_count = *formation.slot_count;
    d_auto_f.skills.resize(d_auto_f.slot_count);
    d_auto_f.skill_last_detect.resize(d_auto_f.slot_count);
    d_auto_f.each_slot_possible_templates.resize(d_auto_f.slot_count);
}

void AutoFight::_init_all_cond(
        const std::map<std::string, std::optional<ConditionConfig>>& conditions
)
{
    all_cond.clear();
    all_cond.reserve(conditions.size());
    for (const auto& [name, config] : conditions) {
        all_cond.push_back(create_baas_condition(baas, &d_auto_f, *config, _cond_name_idx_map));
    }
    _cond_checked.assign(all_cond.size(), false);
    _cond_self_matched.assign(all_cond.size(), std::nullopt);
    _cond_is_matched_recorder.assign(all_cond.size(), std::nullopt);
}


void AutoFight::display_all_cond_info() const noexcept
{
    logger->hr("Display All Conditions");
    display_cond_idx_name_map();
    logger->sub_title("Cond_Cnt : " + std::to_string(_cond_name_idx_map.size()));
    logger->BAASLine();
    for (const auto &cond : _cond_name_idx_map) {
        logger->BAASInfo("Name    : " + cond.first);
        all_cond[cond.second]->display();
        logger->BAASLine();
    }
}

void AutoFight::display_cond_idx_name_map() const noexcept
{
    logger->sub_title("Condition Index Name Mapping");
    for (const auto &cond : _cond_name_idx_map)
        logger->BAASInfo(std::to_string(cond.second) + " : [ " + cond.first + " ] " );
}

void AutoFight::_init_all_state(const WorkflowModel& workflow)
{
    start_state_name = *workflow.start_state;
    _curr_state_idx = _state_name_idx_map.at(start_state_name);
    all_states.clear();
    all_states.reserve(workflow.states.size());
    for (const auto& [name, config] : workflow.states) {
        all_states.emplace_back(name, *config, _actions->name_indices(),
                _cond_name_idx_map, _state_name_idx_map);
    }
}

void AutoFight::display_all_state() const noexcept
{
    logger->hr("Display All States.");
    display_state_idx_name_map();
        logger->sub_title("State_Cnt : " + std::to_string(all_states.size()) + " ");
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
            logger->BAASInfo(std::format("{:>7}  {:>7}", tran.cond_idx, tran.next_sta_idx));
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
    logger->BAASInfo("Auto Fight Start");
    enter_fight();
    // get max cost since max cost may not be 10.0
    auto tmp = (CostUpdater* )(d_auto_f.d_updaters[0].get());
    if (tmp->get_detect_max_cost_when() == "enter_fight") tmp->detect_max_cost();
    _start_state_transition_loop();
}

void AutoFight::ensure_fighting_page()
{

}

void AutoFight::enter_fight()
{
    logger->BAASInfo("Enter Fight");
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
    _state_cond_j_start_t = BAASChronoUtil::getCurrentTimeMS();
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
        // conditions recursively set d_updater_flag to decide which data to be updated in this cycle
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

        // at this line, running thread count may not be 0
        // submit updaters into thread pool
        for (int i = 0; i < d_wait_to_update_idx.size(); ++i) {
            if (d_updater_running_thread_count >= d_update_max_thread) {
                std::unique_lock<std::mutex> d_update_thread_lock(d_update_thread_mutex);
                d_update_thread_finish_notifier.wait(d_update_thread_lock, [&]() {
                    return d_updater_running_thread_count < d_update_max_thread;
                });

                // last round data, no need to judge
                if(d_auto_f.last_updated_data_round != d_auto_f.round_cnt) continue;
                // write data && condition judgement
                d_auto_f.d_updaters[d_auto_f.last_updated_data_idx]->write_result_into_data();
                _state_cond_j();

                if(_state_trans_next_state_idx.has_value()) return true;
                if(_state_flg_all_trans_cond_dissatisfied)  return _try_default_trans();
            }
            ++d_updater_running_thread_count;
            unsigned char idx = d_updater_queue.front();
            d_updater_queue.pop();
            d_update_thread_pool->submit([this, idx]() {
                submit_data_updater_task(this, idx, d_auto_f.round_cnt);
            });
        }

        // all the updaters submitted, whenever a this round updater finished, do condition judgement
        int start_running_t, initial_t_count = d_updater_running_thread_count;
        for (int i = 1; i <= initial_t_count; ++i) {
            std::unique_lock<std::mutex> d_update_thread_lock(d_update_thread_mutex);
            if (d_updater_running_thread_count == 0) break;
            start_running_t = d_updater_running_thread_count;
            d_update_thread_finish_notifier.wait(d_update_thread_lock, [&]() {
                return d_updater_running_thread_count < start_running_t;
            });

            // last round data, no need to judge
            if(d_auto_f.last_updated_data_round != d_auto_f.round_cnt) continue;
            // write data && condition judgement
            d_auto_f.d_updaters[d_auto_f.last_updated_data_idx]->write_result_into_data();
            _state_cond_j();

            if(_state_trans_next_state_idx.has_value())   return true;
            if(_state_flg_all_trans_cond_dissatisfied)    return _try_default_trans();
        }

        display_screenshot_extracted_data();
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
    const auto route = select_baas_workflow_route(
        all_states[_curr_state_idx].transitions,
        [](const auto& transition) { return transition.cond_idx; },
        [](const auto& transition) { return transition.next_sta_idx; },
        [this](std::uint64_t condition_index) {
            if (!_cond_is_pending(condition_index))
                return _cond_is_satisfied(condition_index)
                    ? WorkflowConditionResult::SATISFIED
                    : WorkflowConditionResult::DISSATISFIED;
            if (_cond_checked[condition_index]) return WorkflowConditionResult::DISSATISFIED;
            const auto recursive = _recursive_cond_j(condition_index);
            if (!recursive) return WorkflowConditionResult::PENDING;
            return *recursive ? WorkflowConditionResult::SATISFIED : WorkflowConditionResult::DISSATISFIED;
        });
    _state_trans_next_state_idx = route.condition_transition;
    _state_flg_all_trans_cond_dissatisfied = route.all_conditions_dissatisfied;
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
    return evaluate_baas_condition_recursive(
        all_cond, cond_idx, _cond_is_matched_recorder, _cond_self_matched, _cond_checked);
}

void AutoFight::_state_set_d_update_flags()
{
    std::fill(_cond_checked.begin(), _cond_checked.end(), false);
    d_auto_f.d_updater_mask = d_auto_f.default_d_updater_mask;
    d_auto_f.stop_when_skill_is_activate.clear();
    d_auto_f.boss_health_update_flag = 0b000;
    d_auto_f.skill_cost_update_flag = 0b000;
    d_auto_f.round_cnt ++;

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
    _state_cond_j_loop_start_t = BAASChronoUtil::getCurrentTimeMS();
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

void AutoFight::_init_actions(
        const std::map<std::string, std::optional<ActionConfig>>& actions)
{
    _actions = std::make_unique<auto_fight_act>(baas, &d_auto_f);
    _actions->_init_all_act(actions);
}

void AutoFight::update_screenshot()
{
    auto t_start = BAASChronoUtil::getCurrentTimeMS();
    baas->i_update_screenshot_array();
    auto t_end = BAASChronoUtil::getCurrentTimeMS();
    logger->BAASInfo("[ Screenshot ] update | Time: " + std::to_string(t_end - t_start) + "ms");
}

BAAS_NAMESPACE_END
