//
// Created by pc on 2025/4/23.
//

#ifndef BAAS_MODULE_AUTO_FIGHT_AUTO_FIGHT_H_
#define BAAS_MODULE_AUTO_FIGHT_AUTO_FIGHT_H_

#define BEGIN_AUTO_FIGHT_DATA_UPDATE
#define END_AUTO_FIGHT_DATA_UPDATE


#include "ThreadPool.h"

#include "BAAS.h"
#include "screenshot_data/screenshot_data_recoder.h"
#include "screenshot_data/BaseDataUpdater.h"

BAAS_NAMESPACE_BEGIN

class AutoFight{
public:
    explicit AutoFight(BAAS* baas);

    ~AutoFight();

    void update_data();

    void display_data();

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

private:

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

    void init_data_updater();

    static void submit_data_updater_task(AutoFight* self);

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

    std::string workflow_name;

    BAAS* baas;

};

BAAS_NAMESPACE_END


#endif //BAAS_MODULE_AUTO_FIGHT_AUTO_FIGHT_H_
