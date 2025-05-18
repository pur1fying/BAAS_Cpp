//
// Created by pc on 2025/4/23.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_AUTO_FIGHT_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_AUTO_FIGHT_H_

#define BEGIN_AUTO_FIGHT_DATA_UPDATE
#define END_AUTO_FIGHT_DATA_UPDATE

#define BEGIN_AUTO_FIGHT_CONDITIONS
#define END_AUTO_FIGHT_CONDITIONS

#define BEGIN_AUTO_FIGHT_STATES
#define END_AUTO_FIGHT_STATES

#define BEGIN_AUTO_FIGHT_ACTIONS
#define END_AUTO_FIGHT_ACTIONS

#include <ThreadPool.h>
#include <BAAS.h>

#include "auto_fight_d.h"
#include "conditions/BaseCondition.h"
#include "actions/auto_fight_act.h"

BAAS_NAMESPACE_BEGIN

class AutoFight{

public:

    explicit AutoFight(BAAS* baas);

    void init_workflow(const std::filesystem::path& p="");

    ~AutoFight();

private:

    void _init_d_fight(const std::filesystem::path& name);

    void _init_skills();

    void _init_single_skill_template(std::string &skill_name);

    const static std::vector<std::string> default_active_skill_template, default_inactive_skill_template;

    const static std::string template_j_ptr_prefix;

    BAAS* baas;

    BAASLogger* logger;

    std::filesystem::path workflow_path;

    BAASUserConfig* config;

    std::string workflow_name;

BEGIN_AUTO_FIGHT_DATA_UPDATE

public:

    void update_data();

    void display_screenshot_extracted_data();

    inline void set_data_updater_mask(uint64_t mask) {
        d_auto_f.d_updater_mask = mask;
    }
    inline void reset_data() {
        d_auto_f.reset_all();
    }

    inline void update_screenshot(){
        auto t_start = BAASUtil::getCurrentTimeMS();
        baas->i_update_screenshot_array();
        auto t_end = BAASUtil::getCurrentTimeMS();
        logger->BAASInfo("[ Screenshot ] update | Time: " + std::to_string(t_end - t_start) + "ms");
    }

    inline void set_boss_health_update_flag(uint8_t flag) {
        d_auto_f.boss_health_update_flag = flag;
    }

    inline void set_skill_cost_update_flag(uint32_t flag) {
        d_auto_f.skill_cost_update_flag = flag;
    }

private:

    void _init_data_updaters();

    int d_update_max_thread;

    // data update occurs in multiple threads
    std::unique_ptr<ThreadPool> d_update_thread_pool;

    std::mutex d_update_thread_mutex;

    std::atomic<int> d_updater_running_thread_count = 0;

    std::condition_variable d_update_thread_finish_notifier;

    static void submit_data_updater_task(AutoFight* self, unsigned char idx);

    inline void notify_d_update_thread_end() {
        std::lock_guard<std::mutex> lock(d_update_thread_mutex);
        --d_updater_running_thread_count;
        d_update_thread_finish_notifier.notify_all();
    }

    // max len : 64 ( 2^6 )

    std::vector<uint8_t> d_wait_to_update_idx;

    std::queue<uint8_t> d_updater_queue;

    std::map<std::string, uint64_t> d_updater_map;

    auto_fight_d d_auto_f;

END_AUTO_FIGHT_DATA_UPDATE

BEGIN_AUTO_FIGHT_CONDITIONS

public:

    void display_all_cond_info() const noexcept;

    void display_cond_idx_name_map() const noexcept;

private:

    void _init_all_cond();

    void _init_self_cond();

    bool _init_single_cond(const BAASConfig& d_cond);

    void _init_cond_and_or_idx();

    std::string _cond_type;

    std::vector<std::unique_ptr<BaseCondition>> all_cond;

    std::vector<std::optional<bool>> _cond_is_matched_recorder;

    std::vector<std::optional<bool>> _cond_self_matched;

    std::vector<bool> _cond_checked;

    // name to index in all_conditions
    std::map<std::string, uint64_t> cond_name_idx_map;

END_AUTO_FIGHT_CONDITIONS

BEGIN_AUTO_FIGHT_STATES

public:

    void enter_fight();

    void restart_fight(bool update_room_left_time = false);

    void ensure_fighting_page();

    void start_state_transition();

    void display_all_state() const noexcept;

    void display_state_idx_name_map() const noexcept;

private:

    bool _state_start_trans_cond_j_loop();

    bool _try_default_trans();

    void _state_reset_all_cond();

    void _wait_d_update_running_thread_end();

    void _state_update_cond_j_loop_start_t();

    void _start_state_transition_loop();

    inline bool _cond_is_pending(uint64_t cond_idx) {
        return !_cond_is_matched_recorder[cond_idx].has_value();
    }

    inline bool _cond_is_satisfied(uint64_t cond_idx) {
        assert(_cond_is_matched_recorder[cond_idx].has_value());
        return _cond_is_matched_recorder[cond_idx].value();
    }

    inline bool _cond_is_dissatisfied(uint64_t cond_idx) {
        assert(_cond_is_matched_recorder[cond_idx].has_value());
        return !_cond_is_matched_recorder[cond_idx].value();
    }

    bool _state_cond_timeout_update();

    bool _recursive_check_cond_timeout(uint64_t cond_idx);

    void _state_cond_j();

    std::optional<bool> _recursive_cond_j(uint64_t cond_idx);

    void _state_set_d_update_flags();

    void _recursive_set_d_update_flags(uint64_t cond_idx);

    void _init_all_state();

    void _init_start_state();

    void _init_self_state();

    bool _init_single_state(const BAASConfig& d_state, const std::string& state_name);

    void _conv_tans_state_name_st_to_idx();

    std::vector<state_info> all_states;

    std::vector<std::vector<std::string>> _state_trans_name_recorder;
    std::vector<std::optional<std::string>> _state_default_trans_name_recorder;

    std::map<std::string, uint64_t> state_name_idx_map;

    uint64_t _curr_state_idx;

    // start time of condition judgement
    long long _state_cond_j_start_t;

    // start time of a single condition judgement loop
    long long _state_cond_j_loop_start_t;

    // time elapsed since the start of condition judgement
    long long _state_cond_j_elapsed_t;

    // index of matched transition
    std::optional<uint64_t> _state_trans_next_state_idx;

    bool _state_flg_all_trans_cond_dissatisfied;

    bool _state_cond_j_loop_running_flg;

    std::string start_state_name;

END_AUTO_FIGHT_STATES

BEGIN_AUTO_FIGHT_ACTIONS
public:


private:
    void _init_actions();

    std::unique_ptr<auto_fight_act> _actions;

END_AUTO_FIGHT_ACTIONS

};

BAAS_NAMESPACE_END


#endif //BAAS_CPP_MODULE_AUTO_FIGHT_AUTO_FIGHT_H_
