//
// Created by pc on 2025/4/23.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_DATA_SCREENSHOT_DATA_RECORDER_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_DATA_SCREENSHOT_DATA_RECORDER_H_
#include <optional>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

#include <config/BAASConfig.h>

#include "core_defines.h"

BAAS_NAMESPACE_BEGIN

struct slot_skill{
    std::optional<int>         index;     // skill index
    std::optional<int>         cost;      // skill real cost
    std::optional<bool>        is_active; // skill active state

    void reset() noexcept {
        index.reset();
        cost.reset();
        is_active.reset();
    }
};

struct template_info {
    cv::Mat template_image;
    cv::Vec3b mean_rgb;
    // time cost in each slot
    std::vector<double> estimated_match_template_time_cost;
};

struct skill_template {
    std::string name;    // skill name

    std::vector<template_info> skill_active_templates;
    std::vector<template_info> skill_inactive_templates;
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

    // use vector since there is fight with 6 skills
    std::vector<slot_skill>  skills;
    std::vector<std::vector<int>> each_slot_possible_templates; 
    std::vector<skill_template> all_possible_skills;
    uint32_t skill_cost_update_flag = 0b000000; // max skill is 6
    std::map<std::string, int> skill_name_to_index_map;
    int slot_count;

    // fight auto over time
    std::optional<double>    fight_left_time;

    // room close time
    std::optional<int>       room_left_time;

    BAASConfig d_fight;

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
