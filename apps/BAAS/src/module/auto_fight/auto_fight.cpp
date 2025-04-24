//
// Created by pc on 2025/4/23.
//

#include "module/auto_fight/auto_fight.h"

#include "module/auto_fight/screenshot_data/CostUpdater.h"

BAAS_NAMESPACE_BEGIN

AutoFight::AutoFight(BAAS *baas)
{
    this->baas = baas;
    this->config = baas->get_config();
    logger = baas->get_logger();
    pool = std::make_unique<ThreadPool>(config->getInt("/auto_fight/data_update_thread"));
}

// Add data updaters here
void AutoFight::init_data_updater()
{
    data_updaters.push_back(std::make_unique<CostUpdater>(baas, &latest_screenshot_data));
    data_updater_map["cost"] = 1LL << 0;
}

void AutoFight::update_data()
{
    std::vector<uint64_t> data_wait_to_update_idx;

    for (int i = 0; i < data_updaters.size(); ++i) {
        if((1LL << i) & data_updater_mask) {
            data_wait_to_update_idx.push_back(i);
        }
    }

    if (data_wait_to_update_idx.empty()) {
        return;
    }

    // sort by cost
    std::sort(
            data_wait_to_update_idx.begin(),
            data_wait_to_update_idx.end(),
              [&](uint64_t a, uint64_t b) {
                  return data_updaters[a]->estimated_time_cost(), data_updaters[b]->estimated_time_cost();
              });

    std::queue<uint64_t> data_updater_queue;
    for (auto idx : data_wait_to_update_idx) {
        data_updater_queue.push(idx);
    }

    // put updater into thread pool
    while (!data_updater_queue.empty()) {
        auto idx = data_updater_queue.front();
        data_updater_queue.pop();
        auto updater = data_updaters[idx].get();

    }


}

BAAS_NAMESPACE_END

