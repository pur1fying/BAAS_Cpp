//
// Created by Administrator on 2025/5/29.
//

#include "module/auto_fight/constants.h"

#include "module/auto_fight/actions/acc_handler.h"
#include "module/auto_fight/actions/auto_handler.h"
#include "module/auto_fight/actions/skill_handler.h"

#include "module/auto_fight/conditions/BossHealthCondition.h"
#include "module/auto_fight/conditions/CostCondition.h"
#include "module/auto_fight/conditions/SkillNameCondition.h"

BAAS_NAMESPACE_BEGIN

void _log_valid_op(
        const std::string& name,
        BAASLogger* logger,
        const std::vector<std::string>& op_st_list
) noexcept
{
    BAASGlobalLogger->BAASInfo("Valid  <<< " + name + " >>> are listed below :");
    int cnt = 0;
    for (const auto &op: op_st_list)
        logger->BAASInfo(std::to_string(++cnt) + " : \"" + op + "\"");
}

void _get_valid_log_string(
        const std::string& name,
        const std::vector<std::string>& op_st_list,
        std::string& out
) noexcept
{
    out = "Valid  <<< " + name + " >>> are listed below :\n";
    int cnt = 0;
    for (const auto& op: op_st_list)
        out += std::to_string(++cnt) + " : \"" + op + "\"\n";
}

// constants of actions

const std::map<std::string, base_handler::ACTION_TYPE> base_handler::act_type_map = {
        {"skill", ACTION_TYPE::SKILL},
        {"auto", ACTION_TYPE::AUTO},
        {"acc", ACTION_TYPE::ACC},
        {"restart",  ACTION_TYPE::RESTART},
        {"skip_animation", ACTION_TYPE::SKIP_ANIMATION}
};

const std::vector<std::string> base_handler::act_type_st_list = {
        "skill",
        "auto",
        "acc",
        "restart",
        "skip_animation"
};

const std::map<std::string, acc_handler::Op> acc_handler::op_map = {
        {"1", PHASE_1},
        {"2", PHASE_2},
        {"3", PHASE_3}
};

const std::vector<std::string> acc_handler::op_st_list = {
        "1",
        "2",
        "3"
};

const std::map<std::string, auto_handler::Op> auto_handler::op_map = {
        {"on", ON},
        {"off", OFF},
        {"opposite", OPPOSITE}
};

const std::vector<std::string> auto_handler::op_st_list = {
        "on",
        "off",
        "opposite"
};

const std::map<std::string, skill_handler::Release_Op> skill_handler::release_op_map = {
        {"auto", AUTO},
        {"name", NAME},
        {"l_rel_p", LAST_REL_IDX}
};

const std::vector<std::string> skill_handler::release_op_st_list = {
        "auto",
        "name",
        "l_rel_p"
};

const std::map<std::string, skill_handler::Check_Op> skill_handler::check_op_map = {
        {"C_decrease", C_DECREASE},
        {"", NO_CHECK}
};

const std::vector<std::string> skill_handler::check_op_st_list = {
        "C_decrease",
        ""
};

const std::map<std::string, skill_handler::Target_Op> skill_handler::target_op_map = {
        {"fixed", FIXED},
        {"yolo_t_p", YOLO_T_P_MEAN},
        {"yolo_c_p", YOLO_C_P_MEAN},
        {"yolo_g_p", YOLO_G_P_MEAN},
        {"yolo_l_p", YOLO_L_P_MEAN},
        {"yolo_r_p", YOLO_R_P_MEAN}
};

const std::vector<std::string> skill_handler::target_op_st_list = {
        "fixed",
        "yolo_t_p",
        "yolo_c_p",
        "yolo_g_p",
        "yolo_l_p",
        "yolo_r_p"
};

// constants of conditions

const std::map<std::string, BaseCondition::ConditionType> BaseCondition::cond_type_map = {
        {"and_combined",  A_COMBINED},
        {"or_combined" ,  O_COMBINED},
        {"cost"        ,  COST},
        {"skill_name"  ,  SKILL_NAME},
        {"skill_cost"  ,  SKILL_COST},
        {"acc_phase"   ,  ACC_PHASE},
        {"auto_state"  ,  AUTO_STATE},
        {"boss_health" ,  BOSS_HEALTH}
};

const std::vector<std::string> BaseCondition::cond_type_st_list = {
        "and_combined",
        "or_combined",
        "cost",
        "skill_name",
        "skill_cost",
        "acc_phase",
        "auto_state",
        "boss_health"
};

const std::map<std::string, SkillNameCondition::Op> SkillNameCondition::op_map = {
        {"appear", APPEAR},
        {"at", AT}
};

const std::vector<std::string> SkillNameCondition::op_st_list = {
        "appear",
        "at"
};


const std::map<std::string, BossHealthCondition::Op> BossHealthCondition::op_map = {
        {"C_over", C_OVER},
        {"C_below", C_BELOW},
        {"C_in_range", C_IN_RANGE},
        {"C_increase", C_INCREASE},
        {"C_decrease", C_DECREASE},
        {"M_equal", M_EQUAL}
};

const std::vector<std::string> BossHealthCondition::op_st_list = {
        "C_over",
        "C_below",
        "C_in_range",
        "C_increase",
        "C_decrease",
        "M_equal"
};

BAAS_NAMESPACE_END