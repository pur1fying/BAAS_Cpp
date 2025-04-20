//
// Created by pc on 2024/8/16.
//

#ifndef BAAS_APP_MODULE_COLLECT_REWARD_COLLECT_REWARD_H_
#define BAAS_APP_MODULE_COLLECT_REWARD_COLLECT_REWARD_H_

#include "ISA.h"

ISA_NAMESPACE_BEGIN

class CollectReward {
public:
    static bool implement(baas::BAAS *baas);

    static void collect_daily_reward(baas::BAAS *baas);

    static void collect_weekly_reward(baas::BAAS *baas);

    static void collect_achievement_reward(baas::BAAS *baas);

    static void collect_pass_reward(baas::BAAS *baas);

    static void get_pass_reward_position(
            baas::BAAS *baas,
            std::vector<baas::BAASPoint> &points
    );

    static void collect(baas::BAAS *baas);
};


ISA_NAMESPACE_END

#endif //BAAS_APP_MODULE_COLLECT_REWARD_COLLECT_REWARD_H_
