//
// Created by pc on 2025/4/23.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_DATA_SCREENSHOT_DATA_RECORDER_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_DATA_SCREENSHOT_DATA_RECORDER_H_
#include <optional>
#include <string>
#include <vector>

#include "core_defines.h"
BAAS_NAMESPACE_BEGIN

struct slot_skill{
    std::optional<std::string> name;    // skill name
    std::optional<int>         cost;    // skill real cost
    void reset() noexcept {
        name.reset();
        cost.reset();
    }
};

struct screenshot_data {

    // boss health
    std::optional<long long> boss_current_health;
    std::optional<long long> boss_max_health;

    std::uint8_t boss_health_update_flag = 0b010;
    /*
     * 0b010: update current health
     * 0b001: update max health
     * 0b100: update both
     */


    // phase 1, 2, 3
    std::optional<uint8_t>   acceleration_state;
    // auto on / off
    std::optional<bool>      auto_state;

    // cost able to release skill
    std::optional<double>    cost;

    // use vector since has skill with count 6
    std::vector<slot_skill>  skills;

    // fight auto over time
    std::optional<double>    fight_left_time;

    // room close time
    std::optional<int>       room_left_time;

    void reset_all() noexcept {
        cost.reset();
        auto_state.reset();
        room_left_time.reset();
        boss_max_health.reset();
        fight_left_time.reset();
        acceleration_state.reset();
        boss_current_health.reset();
        for (auto& skill : skills) {
            skill.reset();
        }
    }
};

BAAS_NAMESPACE_END


#endif //BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_DATA_SCREENSHOT_DATA_RECORDER_H_
