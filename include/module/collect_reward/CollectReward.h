//
// Created by pc on 2024/8/16.
//

#ifndef BAAS_MODULE_COLLECT_REWARD_COLLECT_REWARD_H_
#define BAAS_MODULE_COLLECT_REWARD_COLLECT_REWARD_H_

#include "BAAS.h"

namespace ISA{
    class CollectReward {
    public:
        static bool implement(BAAS* baas);

        static void collect_daily_reward(BAAS* baas);

        static void collect_weekly_reward(BAAS* baas);

        static void collect_achievement_reward(BAAS* baas);

        static void collect_pass_reward(BAAS* baas);

        static void get_pass_reward_position(BAAS* baas, std::vector<BAASPoint> &points);

        static void collect(BAAS* baas);
    };

}



#endif //BAAS_MODULE_COLLECT_REWARD_COLLECT_REWARD_H_
