//
// Created by Administrator on 2025/5/7.
//

#include "module/auto_fight/conditions/CostCondition.h"

BAAS_NAMESPACE_BEGIN

const std::map<std::string, CostCondition::Op> CostCondition::op_map = {
        {"over", OVER},
        {"below", BELOW},
        {"in_range", IN_RANGE},
        {"increase", INCREASE},
        {"decrease", DECREASE}
};



BAAS_NAMESPACE_END