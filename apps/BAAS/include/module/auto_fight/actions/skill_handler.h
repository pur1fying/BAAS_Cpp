//
// Created by Administrator on 2025/5/17.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_ACTIONS_SKILL_HANDLER_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_ACTIONS_SKILL_HANDLER_H_

#include "base_handler.h"
#include "module/auto_fight/conditions/CostCondition.h"

BAAS_NAMESPACE_BEGIN

class skill_handler : public base_handler {

public:

    enum Release_Op {
        AUTO,
        NAME,
        LAST_REL_IDX,
    };

    enum Target_Op {
        FIXED,
        YOLO_T_P_MEAN,
        YOLO_C_P_MEAN,
        YOLO_G_P_MEAN,
    };

    enum Check_Op {
        C_DECREASE,
        NO_CHECK
    };

    skill_handler(
            BAAS* baas,
            auto_fight_d* data,
            const BAASConfig& config
    );

    bool execute() override;

    void display() override;

    const static std::map<std::string, Release_Op> release_op_map;

    const static std::map<std::string, Target_Op> target_op_map;

    const static std::map<std::string, Check_Op> check_op_map;

private:

    void _get_l_r_slot_idx(int& _slot_idx);

    void _parse_l_r_idx();

    void _rel_skill(int slot_idx, int x, int y);

    void _get_skill_n_slot_idx(int& _slot_idx);

    void _calc_target_p(int& _x, int& _y);

    void _execute_prepare();

    bool _execute_release_check();

    void _execute_postprocess();

    bool _C_decrease_check_loop();

    void _parse_op();

    void _parse_skill_n_idx();

    void _parse_release_op();

    void _parse_target_op();

    void _parse_check_op();

    uint64_t _skill_n_idx;

    int _l_r_idx;

    Release_Op _r_op;

    Target_Op _t_op;

    std::vector<int> _obj_idx;

    int _fixed_x, _fixed_y;

    Check_Op _c_op;

    double _c_v;

    double _c_timeout;

    std::unique_ptr<BaseCondition> _cost_cond;

};



BAAS_NAMESPACE_END

#endif //BAAS_CPP_MODULE_AUTO_FIGHT_ACTIONS_SKILL_HANDLER_H_
