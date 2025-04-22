//
// Created by pc on 2024/8/16.
//

#ifndef BAAS_APP_MODULE_COLLECT_REWARD_COLLECT_REWARD_H_
#define BAAS_APP_MODULE_COLLECT_REWARD_COLLECT_REWARD_H_

#include "ISA.h"

BAAS_NAMESPACE_BEGIN

class CollectReward {
public:
    static bool implement(BAAS *baas);

    static void collect_daily_reward(BAAS *baas);

    static void collect_weekly_reward(BAAS *baas);

    static void collect_achievement_reward(BAAS *baas);

    static void collect_pass_reward(BAAS *baas);

    static void get_pass_reward_position(
            BAAS *baas,
            std::vector<BAASPoint> &points
    );

    static void collect(BAAS *baas);
};

BAAS_NAMESPACE_END

#endif //BAAS_APP_MODULE_COLLECT_REWARD_COLLECT_REWARD_H_
