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
        YOLO_L_P_MEAN,
        YOLO_R_P_MEAN,
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

    bool execute() noexcept override;

    void display() noexcept override;

    const static std::map<std::string, Release_Op> release_op_map;

    const static std::vector<std::string> release_op_st_list;

    const static std::map<std::string, Target_Op> target_op_map;

    const static std::vector<std::string> target_op_st_list;

    const static std::map<std::string, Check_Op> check_op_map;

    const static std::vector<std::string> check_op_st_list;

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

    inline void _display_valid_release_op() {
        logger->BAASInfo("Valid release op are displayed as follow : ");
        int cnt = 0;
        for (auto& op : release_op_map)
            logger->BAASInfo(std::to_string(++cnt) + " : \"" + op.first + "\"");
    }

    void _parse_target_op();

    inline void _display_valid_target_op() {
        logger->BAASInfo("Valid target op are displayed as follow : ");
        int cnt = 0;
        for (auto& op : target_op_map)
            logger->BAASInfo(std::to_string(++cnt) + " : \"" + op.first + "\"");
    }

    void _parse_check_op();

    void _parse_check_value(const nlohmann::json::const_iterator& _check_it);

    uint64_t _skill_n_idx;

    int _l_r_idx;

    Release_Op _r_op;

    Target_Op _t_op;

    std::vector<int> _obj_idx;

    int _fixed_x, _fixed_y;

    Check_Op _c_op;

    double _c_v;

    long long _c_timeout;

    std::unique_ptr<BaseCondition> _cost_cond;

    void _parse_fixed_x_y(const nlohmann::json::const_iterator& _target_it);

    void _parse_yolo_obj(const nlohmann::json::const_iterator& _target_it);
};



BAAS_NAMESPACE_END

#endif //BAAS_CPP_MODULE_AUTO_FIGHT_ACTIONS_SKILL_HANDLER_H_
