//
// Created by pc on 2025/4/23.
//

#ifndef BAAS_MODULE_AUTO_FIGHT_AUTO_FIGHT_H_
#define BAAS_MODULE_AUTO_FIGHT_AUTO_FIGHT_H_

#include <ThreadPool.h>

#include "BAAS.h"
#include "screenshot_data/screenshot_data_recoder.h"
#include "screenshot_data/BaseDataUpdater.h"

BAAS_NAMESPACE_BEGIN

class AutoFight{
public:
    explicit AutoFight(BAAS* baas);

    void update_data();

    void set_data_updater_mask(uint64_t mask) {
        data_updater_mask = mask;
    }
private:

    bool flag_run;

    BAASUserConfig* config;

    BAASLogger* logger;

    // data update occurs in multiple threads
    std::unique_ptr<ThreadPool> pool;

    int running_update_thread_count = 0;

    void init_data_updater();

    std::vector<std::unique_ptr<BaseDataUpdater>> data_updaters;

    std::map<std::string, uint64_t> data_updater_map;

    uint64_t data_updater_mask = 0;

    std::string workflow_name;

    BAAS* baas;

    screenshot_data latest_screenshot_data;
};

BAAS_NAMESPACE_END


#endif //BAAS_MODULE_AUTO_FIGHT_AUTO_FIGHT_H_
