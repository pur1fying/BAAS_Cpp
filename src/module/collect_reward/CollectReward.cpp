//
// Created by pc on 2024/8/16.
//

#include "module/collect_reward/CollectReward.h"

bool ISA::CollectReward::implement(BAAS *baas) {
    BAASConfig output;
    baas->solve_procedure("UI-GO-TO_collect_reward_menu", output, true);
    collect_daily_reward(baas);
    collect_weekly_reward(baas);
    collect_achievement_reward(baas);
    collect_pass_reward(baas);
//    baas->solve_procedure("UI-GO-TO_collect_reward_menu-limited-reward", output, true);
    return true;
}

void ISA::CollectReward::collect_daily_reward(BAAS *baas) {
    baas->get_logger()->sub_title("Daily Reward");
    baas->solve_procedure("UI-GO-TO_collect_reward_menu-daily-reward", true);
    collect(baas);
}

void ISA::CollectReward::collect_weekly_reward(BAAS *baas) {
    baas->get_logger()->sub_title("Weekly Reward");
    baas->solve_procedure("UI-GO-TO_collect_reward_menu-weekly-reward", true);
    collect(baas);
}

void ISA::CollectReward::collect_achievement_reward(BAAS *baas) {
    baas->get_logger()->sub_title("Achievement Reward");
    baas->solve_procedure("UI-GO-TO_collect_reward_menu-achievement-reward", true);
    collect(baas);
}

void ISA::CollectReward::collect_pass_reward(BAAS *baas) {
    BAASLogger *logger = baas->get_logger();
    logger->sub_title("Pass Reward");
    baas->solve_procedure("UI-GO-TO_collect-reward-pass", true);
    std::vector<BAASPoint> points;
    nlohmann::json patch;
    patch["/possibles/0/2"] = 0;
    BAASConfig output;
    bool first_round = true;
    while (baas->is_running()) {
        get_pass_reward_position(baas, points);
        if(points.empty()) {
            if(first_round) {
                logger->BAASInfo("No Reward Found In First Round, Exit");
                return;
            }
            else {
                logger->sub_title("Recheck Pass Reward.");
                baas->solve_procedure("UI-GO-TO_collect_reward_menu", true);
                baas->solve_procedure("UI-GO-TO_collect-reward-pass", true);
                first_round = true;
                continue;
            }
        }
        first_round = false;
        logger->BAASInfo("[ " + std::to_string(points.size()) + " ] Pass Reward Found.");
        for(auto &p : points) {
            logger->BAASInfo("Collect Pass Reward At : ( " + std::to_string(p.x) + ", " + std::to_string(p.y) + " )");
            patch["/possibles/0/2"] = p.y;
            baas->solve_procedure("COLLECT_PASS_REWARD", patch, true);
            baas->solve_procedure("UI-GO-TO_collect-reward-pass", true);
        }
    }
}

void ISA::CollectReward::collect(BAAS *baas) {
    BAASLogger* logger = baas->get_config()->get_logger();
    if(baas->reset_then_feature_appear("collect_reward_collect-reward-bright_appear")) {
        logger->BAASInfo("Collect Reward [ Bright ]");
        baas->solve_procedure("COLLECT_REWARD", true);
    }
    else if(baas->reset_then_feature_appear("collect_reward_collect-reward-grey_appear")) {
        logger->BAASInfo("Collect Reward [ Grey ]");
    }
    else {
        logger->BAASError("Collect Reward Status Unknown");
    }
}

void ISA::CollectReward::get_pass_reward_position(BAAS *baas, std::vector<BAASPoint> &points) {
    points.clear();
    BAASPoint p(272, 326);
    cv::Mat screenshot;
    cv::Vec3b min_ = {245, 8, 63}, max_ = {255, 28, 83};
    while (p.y <= 872) {
        if(baas->judge_rgb_range(p , min_, max_)) {
            points.emplace_back(p.x-192, p.y+54);
            p.y += 50;
        }
        else p.y += 1;
    }
}


