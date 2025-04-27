//
// Created by pc on 2025/4/23.
//

#include "module/auto_fight/auto_fight.h"

#include "module/auto_fight/screenshot_data/CostUpdater.h"
#include "module/auto_fight/screenshot_data/BossHealthUpdater.h"

BAAS_NAMESPACE_BEGIN

AutoFight::AutoFight(BAAS *baas)
{
    this->baas = baas;
    this->config = baas->get_config();
    logger = baas->get_logger();
    d_update_max_thread = config->getInt("/auto_fight/d_update_max_thread");
    d_update_thread_pool = std::make_unique<ThreadPool>(d_update_max_thread);
    d_update_thread_pool->init();
    init_data_updater();
}

// Add data updaters here
void AutoFight::init_data_updater()
{
    // cost
    d_updaters.push_back(std::make_unique<CostUpdater>(baas, &latest_screenshot_d));
    d_updater_map["cost"] = 1LL << 0;

    // boss health
    d_updaters.push_back(std::make_unique<BossHealthUpdater>(baas, &latest_screenshot_d));
    d_updater_map["boss_health"] = 1LL << 1;
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
        std::unique_lock<std::mutex> d_update_thread_lock(d_update_thread_mutex);
        d_update_thread_finish_notifier.wait(d_update_thread_lock, [&]() {
            return d_updater_running_thread_count < d_update_max_thread;
        });
        ++d_updater_running_thread_count;
        d_update_thread_pool->submit([this]() {
            submit_data_updater_task(this);
        });
    }

    // wait for all threads to finish
    std::unique_lock<std::mutex> d_update_thread_lock(d_update_thread_mutex);
    d_update_thread_finish_notifier.wait(d_update_thread_lock, [&]() {
        return d_updater_running_thread_count == 0;
    });
}

void AutoFight::submit_data_updater_task(AutoFight *self)
{
    auto start_t = BAASUtil::getCurrentTimeMS();

    auto idx = self->d_updater_queue.front();
    self->d_updater_queue.pop();
    try {
        self->d_updaters[idx]->update();
    }
    catch (const std::exception &e) {
        self->logger->BAASError("In [ " + self->d_updaters[idx]->data_name() + " ] update | Error: " + e.what());
    }

    self->notify_d_update_thread_end();

    auto end_t = BAASUtil::getCurrentTimeMS();
    self->logger->BAASInfo("[ " + self->d_updaters[idx]->data_name() + " ] update | Time: " +
                                                                    std::to_string(end_t - start_t) + "ms");
}

void AutoFight::display_data()
{
    for (auto &updater : d_updaters) {
        updater->display_data();
    }
}


AutoFight::~AutoFight()
{
    for (auto &updater : d_updaters) {
        updater.reset();
    }
    d_updaters.clear();
    d_update_thread_pool->shutdown();
}


BAAS_NAMESPACE_END

