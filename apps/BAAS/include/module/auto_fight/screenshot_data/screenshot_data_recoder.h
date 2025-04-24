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
};

struct screenshot_data {

    std::optional<long long> boss_current_health;
    std::optional<long long> boss_max_health;

    std::optional<uint8_t>   acceleration_state;  // phase 1, 2, 3
    std::optional<bool>      auto_state;          // auto on / off

    std::optional<double>    cost;                // cost able to release skill
    std::vector<slot_skill>  skills;              // use vector since has skill with count 6

    std::optional<double>    fight_left_time;     // fight auto over time
    std::optional<int>       room_left_time;      // room close time

};

BAAS_NAMESPACE_END


#endif //BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_DATA_SCREENSHOT_DATA_RECORDER_H_
