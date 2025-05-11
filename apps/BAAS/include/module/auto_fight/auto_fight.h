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

#include <ThreadPool.h>
#include <BAAS.h>

#include "auto_fight_d.h"
#include "screenshot_data/BaseDataUpdater.h"
#include "conditions/BaseCondition.h"

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

    std::atomic<bool> flag_run;

    BAASUserConfig* config;

    BAASLogger* logger;

BEGIN_AUTO_FIGHT_DATA_UPDATE

public:

    void set_slot_possible_skill_idx(
            int slot_idx,
            const std::vector<int>& possible_skill_idx
    );

    void set_skill_slot_possible_templates(
            int skill_idx,
            const std::vector<int>& possible_templates
    );

    const static std::vector<std::string> default_active_skill_template, default_inactive_skill_template;

    const static std::string template_j_ptr_prefix;

    void update_data();

    void display_screenshot_extracted_data();

    inline void set_data_updater_mask(uint64_t mask) {
        d_updater_mask = mask;
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
    std::vector<std::unique_ptr<BaseDataUpdater>> d_updaters;

    std::vector<uint8_t> d_wait_to_update_idx;

    std::queue<uint8_t> d_updater_queue;

    std::map<std::string, uint64_t> d_updater_map;

    uint64_t d_updater_mask = 0;

    auto_fight_d d_auto_f;

END_AUTO_FIGHT_DATA_UPDATE

BEGIN_AUTO_FIGHT_CONDITIONS

public:

    void display_all_cond_info() const noexcept;

    std::optional<uint64_t> cond_appear();

    void display_cond_idx_name_map() const noexcept;

private:

    void _init_all_cond();

    void _init_self_cond();

    bool _init_single_cond(const BAASConfig& d_cond);

    void _init_cond_and_or_idx();

    void _init_cond_timeout();

    // currently judging condition idx
    std::vector<uint64_t> cond_wait_to_judge_idx;

    std::string _cond_type;

    std::vector<std::unique_ptr<BaseCondition>> all_cond;

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

    void _init_all_state();

    void _init_start_state();

    void _init_self_state();

    bool _init_single_state(const BAASConfig& d_state, const std::string& state_name);

    void _conv_tans_state_name_st_to_idx();

    std::vector<state_info> all_states;

    std::vector<std::vector<std::string>> _state_trans_name_recorder;
    std::vector<std::optional<std::string>> _state_default_trans_name_recorder;

    std::map<std::string, uint64_t> state_name_idx_map;

    uint64_t curr_state_idx;

    std::string start_state_name;

END_AUTO_FIGHT_STATES

    std::filesystem::path workflow_path;

    std::string workflow_name;

    BAAS* baas;

};

BAAS_NAMESPACE_END


#endif //BAAS_CPP_MODULE_AUTO_FIGHT_AUTO_FIGHT_H_
