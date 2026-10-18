#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOW_CONDITIONCONFIG_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOW_CONDITIONCONFIG_H_

#include <cstdint>
#include <string>
#include <vector>

#include "core_defines.h"

BAAS_NAMESPACE_BEGIN

enum class ConditionType {
    AND_COMBINED, OR_COMBINED, COST, SKILL_NAME, BOSS_HEALTH, BATTLE_TIME
};
enum class NumericConditionOp { OVER, BELOW, IN_RANGE, INCREASE, DECREASE };
enum class SkillNameConditionOp { APPEAR, AT, APPEAR_READY, AT_READY };
enum class BossHealthConditionOp {
    CURRENT_OVER, CURRENT_BELOW, CURRENT_IN_RANGE,
    CURRENT_INCREASE, CURRENT_DECREASE, MAX_EQUAL
};

struct ConditionConfig {
    ConditionType type = ConditionType::AND_COMBINED;
    std::int64_t timeout = 5000;
    std::string desc;
    std::vector<std::string> and_conditions;
    std::vector<std::string> or_conditions;
    NumericConditionOp numeric_op = NumericConditionOp::OVER;
    BossHealthConditionOp health_op = BossHealthConditionOp::CURRENT_OVER;
    SkillNameConditionOp skill_op = SkillNameConditionOp::APPEAR;
    double value = 0.0;
    double range_min = 0.0;
    double range_max = 0.0;
    std::int32_t time_value = 0;
    std::int32_t time_range_min = 0;
    std::int32_t time_range_max = 0;
    std::string skill_name;
    std::uint64_t position = 0;
};

BAAS_NAMESPACE_END

#endif // BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOW_CONDITIONCONFIG_H_
