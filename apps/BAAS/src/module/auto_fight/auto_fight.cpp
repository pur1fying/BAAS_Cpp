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
    d_updaters.push_back(std::make_unique<CostUpdater>(baas, &d_auto_f));
    d_updater_map["cost"] = 1LL << 0;

    // BIT [ 2 ]
    // boss health
    d_updaters.push_back(std::make_unique<BossHealthUpdater>(baas, &d_auto_f));
    d_updater_map["boss_health"] = 1LL << 1;

    // BIT [ 3 ]
    // skill name
    d_updaters.push_back(std::make_unique<SkillNameUpdater>(baas, &d_auto_f));
    d_updater_map["skill_name"] = 1LL << 2;

    // BIT [ 4 ]
    // skill cost
    d_updaters.push_back(std::make_unique<SkillCostUpdater>(baas, &d_auto_f));
    d_updater_map["skill_cost"] = 1LL << 3;

    // BIT [ 5 ]
    // acc phase
    d_updaters.push_back(std::make_unique<AccelerationPhaseUpdater>(baas, &d_auto_f));
    d_updater_map["acc_phase"] = 1LL << 4;

    // BIT [ 6 ]
    // auto state
    d_updaters.push_back(std::make_unique<AutoStateUpdater>(baas, &d_auto_f));
    d_updater_map["auto_state"] = 1LL << 5;

    // BIT [ 7 ]
    // object position
    d_updaters.push_back(std::make_unique<ObjectPositionUpdater>(baas, &d_auto_f));
    d_updater_map["object_position"] = 1LL << 6;

}

void AutoFight::update_data()
{
    d_wait_to_update_idx.clear();

    for (int i = 0; i < d_updaters.size(); ++i) {
        if((1LL << i) & d_updater_mask) {
            d_wait_to_update_idx.push_back(i);
        }
    }

    if (d_wait_to_update_idx.empty()) {
        return;
    }

    // sort by cost
    std::sort(d_wait_to_update_idx.begin(),d_wait_to_update_idx.end(), [this](auto a, auto b) {
        return d_updaters[a]->estimated_time_cost() < d_updaters[b]->estimated_time_cost();
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
//        logger->BAASInfo("Start Running Threads : " + std::to_string(start_running_t));
        d_update_thread_finish_notifier.wait(d_update_thread_lock, [&]() {
            return d_updater_running_thread_count < start_running_t;
        });
        // condition judgement
//        logger->BAASInfo("Running Threads : " + std::to_string(d_updater_running_thread_count));
    }

//    logger->BAASInfo("End Running Threads : " + std::to_string(d_updater_running_thread_count));
    assert(d_updater_running_thread_count == 0);
}

void AutoFight::submit_data_updater_task(AutoFight* self, unsigned char idx)
{
    auto start_t = BAASUtil::getCurrentTimeMS();
    try {
        self->d_updaters[idx]->update();
    }
    catch (const std::exception &e) {
        self->logger->BAASError("In [ " + self->d_updaters[idx]->data_name() + " ] update | Error: " + e.what());
    }

    self->notify_d_update_thread_end();

    auto end_t = BAASUtil::getCurrentTimeMS();
    self->logger->BAASInfo("[ " + self->d_updaters[idx]->data_name() + " ] "
                            "update | Time: " + std::to_string(end_t - start_t) + "ms");
}

void AutoFight::display_screenshot_extracted_data()
{
    for (int i = 0; i <= d_updaters.size(); ++i)
        if ((1LL << i) & d_updater_mask)
            d_updaters[i]->display_data();
}


AutoFight::~AutoFight()
{
    for (auto &updater : d_updaters) {
        updater.reset();
    }
    d_updaters.clear();
    d_update_thread_pool->shutdown();
}

void AutoFight::init_workflow(const std::filesystem::path &name)
{
    // load workflow json
    _init_d_fight(name);

    // all appeared skills
    _init_skills();

    // all conditions
    _init_all_cond();

    // data updaters
    _init_data_updaters();

    // all states
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

void AutoFight::set_skill_slot_possible_templates(
        int skill_idx,
        const std::vector<int> &possible_templates
)
{
    if (skill_idx < 0 || skill_idx >= d_auto_f.skills.size()) {
        logger->BAASWarn("Invalid skill index: " + std::to_string(skill_idx));
        return;
    }
    d_auto_f.each_slot_possible_templates[skill_idx] = possible_templates;
}

void AutoFight::set_slot_possible_skill_idx(
        int slot_idx,
        const std::vector<int> &possible_skill_idx
)
{
    if (slot_idx < 0 || slot_idx >= d_auto_f.skills.size()) {
        logger->BAASError("Invalid slot index : [ " + std::to_string(slot_idx) + " ]");
        return;
    }
    for (auto &idx : possible_skill_idx) {
        if (idx < 0 || idx >= d_auto_f.all_possible_skills.size()) {
            logger->BAASError("Invalid skill index : [ " + std::to_string(idx) + " ]");
            return;
        }
    }
    d_auto_f.each_slot_possible_templates[slot_idx] = possible_skill_idx;
}

std::optional<uint64_t> AutoFight::cond_appear()
{

    return std::nullopt;
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

void AutoFight::_init_cond_timeout()
{
    // cond with and cond, timeout is min(and cond)
    logger->sub_title("Init Condition Timeout.");
}

void AutoFight::_init_d_fight(const std::filesystem::path &name)
{
    // consider chinese name
    if(name.empty())  {
        workflow_name = config->getString("/auto_fight/workflow_name");
        workflow_path = config->getPath("/auto_fight/workflow_name").replace_extension(".json");
    }
    else {
        std::filesystem::path temp = name;
        workflow_name = name.string();
        workflow_path = temp.replace_extension(".json");
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
    // Skill
    logger->hr("Init Skills");
    auto skill_names = d_auto_f.d_fight.get<std::vector<std::string>>("/formation/all_appeared_skills");
    for (int i = 0; i < skill_names.size(); ++i) {
        d_auto_f.skill_name_to_index_map[skill_names[i]] = i;
        _init_single_skill_template(skill_names[i]);
    }
    d_auto_f.slot_count = d_auto_f.d_fight.getInt("/formation/slot_count", 3);
    logger->BAASInfo("Slot Count : [ " + std::to_string(d_auto_f.slot_count) + " ]");

    d_auto_f.skills.resize(d_auto_f.slot_count);
    d_auto_f.each_slot_possible_templates.resize(d_auto_f.slot_count);
}

void AutoFight::_init_all_cond()
{
    // Condition
    logger->hr("Init Conditions");

    _init_self_cond();

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
            break;
        case BaseCondition::ConditionType::ACC_PHASE:
            break;
        case BaseCondition::ConditionType::AUTO_STATE:
            break;
        case BaseCondition::COMBINED:
            break;
        case BaseCondition::SKILL_NAME:
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
    // action is not implemented now
    // TODO:: check action


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

    // "next" and "default_transition" int64_value should not be init here
    if(d_state.contains("default_transition"))  {
        if(d_state.value_type("default_transition") != nlohmann::detail::value_t::string) {
            logger->BAASError("In state [ " + name + " ] , value of [ default_transition ] must be string.");
            throw TypeError("Invalid [ default_transition ] type in state.");
        }
        _state_default_trans_name_recorder.emplace_back(d_state.getString("default_transition"));
    }
    else {
        // do not have default transition
        _state_default_trans_name_recorder.emplace_back(std::nullopt);
    }

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
        logger->BAASError("Undefined START state [ " + start_state_name + " ] .");
        throw ValueError("Start State Invalid.");
    }

    curr_state_idx = it->second;
    logger->sub_title("Start State");
    logger->BAASInfo("Name  : [ " + start_state_name + " ]");
    logger->BAASInfo("Index : [ " + std::to_string(curr_state_idx) + " ]");

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
            if(state_name_idx_map.find(_state_default_trans_name_recorder[i].value()) == state_name_idx_map.end()) {
                logger->BAASError("Undefined state [ " + _state_default_trans_name_recorder[i].value() + " ] found in default transitions.");
                throw ValueError("Invalid State Name.");
            }
            all_states[i].default_trans = _default_trans_state_name_it->second;
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

        logger->BAASInfo("Trans_Cnt : " + std::to_string(all_states[i].transitions.size()));

        if(all_states[i].default_trans.has_value())
        logger->BAASInfo("D_Tran_Idx: " + std::to_string(all_states[i].default_trans.value()));

        if(!all_states[i].desc.empty())
        logger->BAASInfo("Desc      : " + all_states[i].desc);

        if(!all_states[i].transitions.empty()) {
        logger->BAASInfo("CondIdx  NexTIdx");
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

BAAS_NAMESPACE_END

