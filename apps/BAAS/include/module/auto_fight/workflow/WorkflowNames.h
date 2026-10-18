#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOW_WORKFLOWNAMES_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOW_WORKFLOWNAMES_H_

#include <array>
#include <cstddef>
#include <string>

#include "module/auto_fight/workflow/ActionConfig.h"
#include "module/auto_fight/workflow/ConditionConfig.h"

BAAS_NAMESPACE_BEGIN

inline const std::array<std::string, 5> ACTION_NAMES{
        "acc", "auto", "restart", "skip_animation", "skill"};
inline const std::array<std::string, 6> CONDITION_NAMES{
        "and_combined", "or_combined", "cost", "skill_name", "boss_health", "battle_time"};
inline const std::array<std::string, 3> ACC_OP_NAMES{"1", "2", "3"};
inline const std::array<std::string, 3> AUTO_OP_NAMES{"on", "off", "opposite"};
inline const std::array<std::string, 3> RELEASE_OP_NAMES{"auto", "name", "l_rel_p"};
inline const std::array<std::string, 6> TARGET_OP_NAMES{
        "fixed", "yolo_t_p", "yolo_c_p", "yolo_g_p", "yolo_l_p", "yolo_r_p"};
inline const std::array<std::string, 2> CHECK_OP_NAMES{"C_decrease", "none"};
inline const std::array<std::string, 5> NUMERIC_OP_NAMES{
        "over", "below", "in_range", "increase", "decrease"};
inline const std::array<std::string, 6> HEALTH_OP_NAMES{
        "C_over", "C_below", "C_in_range", "C_increase", "C_decrease", "M_equal"};
inline const std::array<std::string, 4> SKILL_NAME_OP_NAMES{
        "appear", "at", "appear_ready", "at_ready"};

inline const std::string& wire_name(ConditionType type)
{
    return CONDITION_NAMES.at(static_cast<std::size_t>(type));
}
inline const std::string& wire_name(NumericConditionOp op)
{
    return NUMERIC_OP_NAMES.at(static_cast<std::size_t>(op));
}
inline const std::string& wire_name(BossHealthConditionOp op)
{
    return HEALTH_OP_NAMES.at(static_cast<std::size_t>(op));
}
inline const std::string& wire_name(SkillNameConditionOp op)
{
    return SKILL_NAME_OP_NAMES.at(static_cast<std::size_t>(op));
}

BAAS_NAMESPACE_END

#endif // BAAS_CPP_MODULE_AUTO_FIGHT_WORKFLOW_WORKFLOWNAMES_H_
