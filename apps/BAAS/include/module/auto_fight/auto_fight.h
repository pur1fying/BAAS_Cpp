//
// Created by pc on 2025/4/23.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_AUTO_FIGHT_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_AUTO_FIGHT_H_

#define BEGIN_AUTO_FIGHT_DATA_UPDATE
#define END_AUTO_FIGHT_DATA_UPDATE

#define BEGIN_AUTO_FIGHT_CONDITIONS
#define END_AUTO_FIGHT_CONDITIONS

#include <ThreadPool.h>
#include <BAAS.h>

#include "screenshot_data/screenshot_data_recoder.h"
#include "screenshot_data/BaseDataUpdater.h"
#include "conditions/BaseCondition.h"

BAAS_NAMESPACE_BEGIN

class AutoFight{
public:
    explicit AutoFight(BAAS* baas);

    void init_workflow(const std::filesystem::path& p="");

    ~AutoFight();

    void update_data();

    void display_data();

    void init_data_updaters();

    void set_slot_possible_skill_idx(
            int slot_idx,
            const std::vector<int>& possible_skill_idx
    );

    void set_skill_slot_possible_templates(
            int skill_idx,
            const std::vector<int>& possible_templates
    );

    inline void reset_data() {
        latest_screenshot_d.reset_all();
    }

    inline void set_data_updater_mask(uint64_t mask) {
        d_updater_mask = mask;
    }

    inline void update_screenshot(){
        auto t_start = BAASUtil::getCurrentTimeMS();
        baas->update_screenshot_array();
        auto t_end = BAASUtil::getCurrentTimeMS();
        logger->BAASInfo("[ Screenshot ] update | Time: " + std::to_string(t_end - t_start) + "ms");
    }

    inline void set_boss_health_update_flag(uint8_t flag) {
        latest_screenshot_d.boss_health_update_flag = flag;
    }

    inline void set_skill_cost_update_flag(uint32_t flag) {
        latest_screenshot_d.skill_cost_update_flag = flag;
    }

    std::optional<uint64_t> cond_appear();

    const static std::vector<std::string> default_active_skill_template, default_inactive_skill_template;

    const static std::string template_j_ptr_prefix;

private:

    void _init_d_fight(const std::filesystem::path& name);

    void _init_skills();

    void _init_all_cond();

    void _init_self_cond();

    bool _init_single_cond(const BAASConfig& d_cond);

    void _init_single_skill_template(std::string &skill_name);

    std::atomic<bool> flag_run;

    BAASUserConfig* config;

    BAASLogger* logger;

BEGIN_AUTO_FIGHT_DATA_UPDATE
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

    screenshot_data latest_screenshot_d;

END_AUTO_FIGHT_DATA_UPDATE

BEGIN_AUTO_FIGHT_CONDITIONS

    void _init_cond_ptr();

    void _init_cond_timeout();

    // currently judging condition idx
    std::vector<uint64_t> cond_wait_to_judge_idx;

    std::string _cond_type;

    std::vector<std::unique_ptr<BaseCondition>> all_cond;

    // name to index in all_conditions
    std::map<std::string, uint64_t> cond_name_map;

END_AUTO_FIGHT_CONDITIONS

    std::filesystem::path workflow_path;

    std::string workflow_name;

    BAAS* baas;

};

BAAS_NAMESPACE_END


#endif //BAAS_CPP_MODULE_AUTO_FIGHT_AUTO_FIGHT_H_
