//
// Created by Administrator on 2025/5/17.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_ACTIONS_SKILL_HANDLER_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_ACTIONS_SKILL_HANDLER_H_

#include <cstdint>
#include <memory>
#include <vector>

#include "base_handler.h"
#include "module/auto_fight/workflow/ActionConfig.h"
#include "module/auto_fight/conditions/CostCondition.h"

BAAS_NAMESPACE_BEGIN

class skill_handler : public base_handler {

public:

    static constexpr auto AUTO = SkillReleaseOp::AUTO;
    static constexpr auto NAME = SkillReleaseOp::NAME;
    static constexpr auto LAST_REL_IDX = SkillReleaseOp::LAST_RELEASE_POSITION;
    static constexpr auto FIXED = SkillTargetOp::FIXED;
    static constexpr auto YOLO_T_P_MEAN = SkillTargetOp::YOLO_TOP_MEAN;
    static constexpr auto YOLO_C_P_MEAN = SkillTargetOp::YOLO_CENTER_MEAN;
    static constexpr auto YOLO_G_P_MEAN = SkillTargetOp::YOLO_GROUND_MEAN;
    static constexpr auto YOLO_L_P_MEAN = SkillTargetOp::YOLO_LEFT_MEAN;
    static constexpr auto YOLO_R_P_MEAN = SkillTargetOp::YOLO_RIGHT_MEAN;
    static constexpr auto C_DECREASE = SkillCheckOp::COST_DECREASE;
    static constexpr auto NO_CHECK = SkillCheckOp::NONE;

    skill_handler(
            BAAS* baas,
            auto_fight_d* data,
            const ActionStepConfig& config
    );

    bool execute() noexcept override;

    void display() noexcept override;

private:

    void _get_l_r_slot_idx(int& _slot_idx);

    void _rel_skill(int slot_idx, int x, int y);

    void _get_skill_n_slot_idx(int& _slot_idx);

    void _calc_target_p(int& _x, int& _y);

    void _execute_prepare();

    bool _execute_release_check();

    void _execute_postprocess();

    bool _C_decrease_check_loop();

    uint64_t _skill_n_idx;

    int _l_r_idx;

    SkillReleaseOp _r_op;

    SkillTargetOp _t_op;

    std::vector<int> _obj_idx;

    int _fixed_x, _fixed_y;

    SkillCheckOp _c_op;

    double _c_v;

    long long _c_timeout;

    std::unique_ptr<BaseCondition> _cost_cond;

};


BAAS_NAMESPACE_END

#endif //BAAS_CPP_MODULE_AUTO_FIGHT_ACTIONS_SKILL_HANDLER_H_
