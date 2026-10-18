//
// Created by Administrator on 2025/5/17.
//

#include "module/auto_fight/actions/skill_handler.h"

#include <memory>

#include "module/auto_fight/workflow/ActionConfig.h"
#include "module/auto_fight/workflow/ConditionConfig.h"
#include "module/auto_fight/screenshot_data/BaseDataUpdater.h"
#include "utils/BAASChronoUtil.h"

BAAS_NAMESPACE_BEGIN

skill_handler::skill_handler(
        BAAS* baas,
        auto_fight_d* data,
        const ActionStepConfig& config
) : base_handler(baas, data, config.desc),
    _skill_n_idx(0),
    _l_r_idx(config.last_release_index),
    _r_op(config.release_op),
    _t_op(config.target_op),
    _fixed_x(config.fixed_x),
    _fixed_y(config.fixed_y),
    _c_op(config.check_op),
    _c_v(config.check_value),
    _c_timeout(config.check_timeout)
{
    if (_r_op == NAME) {
        _skill_n_idx = data->skill_name_to_index_map.at(config.skill_id);
    }
    for (const auto& name : config.target_objects) {
        _obj_idx.push_back(data->obj_name_to_index_map.at(name));
    }
    if (_c_op == C_DECREASE) {
        ConditionConfig condition;
        condition.type = ConditionType::COST;
        condition.timeout = _c_timeout;
        condition.numeric_op = NumericConditionOp::DECREASE;
        condition.value = _c_v;
        _cost_cond = std::make_unique<CostCondition>(baas, data, condition);
    }
}

bool skill_handler::execute() noexcept
{
    _execute_prepare();
    switch (_r_op) {
        case AUTO : {
            baas->solve_procedure("fight_set_auto_on", true);
            break;
        }
        case NAME : {
            int x, y, slot_idx;
            _get_skill_n_slot_idx(slot_idx);
            _calc_target_p(x, y);
            _rel_skill(slot_idx, x, y);
            break;
        }
        case LAST_REL_IDX : {
            int x, y, slot_idx;
            _get_l_r_slot_idx(slot_idx);
            _calc_target_p(x, y);
            _rel_skill(slot_idx, x, y);
            break;
        }
    }

    const bool release_check_passed = _execute_release_check();
    _execute_postprocess();
    // Preserve historical behavior: a failed post-release check is logged only.
    (void)release_check_passed;
    return true;
}

void skill_handler::display() noexcept
{

}

bool skill_handler::_C_decrease_check_loop()
{
    long long _start_t = BAASChronoUtil::getCurrentTimeMS();

    while (true) {
        long long _time_elapsed = BAASChronoUtil::getCurrentTimeMS() - _start_t;
        if (_time_elapsed >= _cost_cond->get_timeout()) {
            logger->sub_title("Skill Action C_decrease Check Timeout.");
            return false;
        }
        baas->i_update_screenshot_array();
        data->d_updaters[0]->update();
        data->d_updaters[0]->write_result_into_data();
        if (_cost_cond->try_match()) {
            logger->sub_title("Skill Action C_decrease Check Passed.");
            logger->BAASInfo("C After Skill Rel : [ " + std::to_string(data->cost.value()) + " ]");
            return true;
        }
    }
}

void skill_handler::_execute_prepare()
{
    switch (_c_op) {
        case C_DECREASE : {
            _cost_cond->reset_state();
            _cost_cond->try_match();
            BAASGlobalLogger->BAASInfo("C Before Skill Rel : [ " + std::to_string(data->cost.value()) + " ]");
            break;
        }
        case NO_CHECK : {
            break;
        }
    }
}

bool skill_handler::_execute_release_check()
{
    switch (_c_op) {
        case C_DECREASE : return _C_decrease_check_loop();
        case NO_CHECK   : return true;
    }
}

void skill_handler::_execute_postprocess()
{
    switch (_r_op) {
        case AUTO : {
            baas->solve_procedure("fight_set_auto_off", true);
            break;
        }
        case NAME : {
            break;
        }
        case LAST_REL_IDX : {
            break;
        }
    }
}

void skill_handler::_get_skill_n_slot_idx(int& _slot_idx)
{
    for(int i = 0; i < data->slot_count; ++i) {
        if(data->skill_last_detect[i].index.has_value() &&
           data->skill_last_detect[i].index.value() == _skill_n_idx) {
            _slot_idx = i;
            return;
        }
    }
    logger->BAASError("Skill [ " + std::to_string(_skill_n_idx) + " ] not found in skills slot.");
    throw RuntimeError("Skill [ " + std::to_string(_skill_n_idx) + " ] not found in skills slot.");
}

void skill_handler::_calc_target_p(int& _x, int& _y)
{
    switch (_t_op) {
        case FIXED: {
            _x = _fixed_x;
            _y = _fixed_y;
            break;
        }
        case YOLO_T_P_MEAN:
            _x = 0;
            _y = 0;
            for (const auto& obj_idx : _obj_idx) {
                if (data->obj_last_appeared_pos[obj_idx].has_value()) {
                    int top_x = data->obj_last_appeared_pos[obj_idx].value().box.x +
                                data->obj_last_appeared_pos[obj_idx].value().box.width / 2;
                    int top_y = data->obj_last_appeared_pos[obj_idx].value().box.y;
                    _x += top_x;
                    _y += top_y;
                }
            }
            _x /= int(_obj_idx.size());
            _y /= int(_obj_idx.size());
            break;
        case YOLO_C_P_MEAN:
            _x = 0;
            _y = 0;
            for (const auto& obj_idx : _obj_idx) {
                if (data->obj_last_appeared_pos[obj_idx].has_value()) {
                    int center_x = data->obj_last_appeared_pos[obj_idx].value().box.x +
                                   data->obj_last_appeared_pos[obj_idx].value().box.width / 2;
                    int center_y = data->obj_last_appeared_pos[obj_idx].value().box.y +
                                   data->obj_last_appeared_pos[obj_idx].value().box.height / 2;
                    _x += center_x;
                    _y += center_y;
                }
            }
            _x /= int(_obj_idx.size());
            _y /= int(_obj_idx.size());
            break;
        case YOLO_G_P_MEAN: {
            _x = 0;
            _y = 0;
            for (const auto& obj_idx : _obj_idx) {
                if (data->obj_last_appeared_pos[obj_idx].has_value()) {
                    int ground_x = data->obj_last_appeared_pos[obj_idx].value().box.x +
                                   data->obj_last_appeared_pos[obj_idx].value().box.width / 2;
                    int ground_y = data->obj_last_appeared_pos[obj_idx].value().box.y +
                                   data->obj_last_appeared_pos[obj_idx].value().box.height;
                    _x += ground_x;
                    _y += ground_y;
                }
            }
            _x /= int(_obj_idx.size());
            _y /= int(_obj_idx.size());
            break;
        }
        case YOLO_L_P_MEAN: {
            _x = 0;
            _y = 0;
            for (const auto& obj_idx : _obj_idx) {
                if (data->obj_last_appeared_pos[obj_idx].has_value()) {
                    _x += data->obj_last_appeared_pos[obj_idx].value().box.x;
                    int left_y = data->obj_last_appeared_pos[obj_idx].value().box.y +
                                 data->obj_last_appeared_pos[obj_idx].value().box.height / 2;
                    _y += left_y;
                }
            }
            _x /= int(_obj_idx.size());
            _y /= int(_obj_idx.size());
            break;
        }
        case YOLO_R_P_MEAN: {
            _x = 0;
            _y = 0;
            for (const auto& obj_idx : _obj_idx) {
                if (data->obj_last_appeared_pos[obj_idx].has_value()) {
                    int right_x = data->obj_last_appeared_pos[obj_idx].value().box.x +
                                  data->obj_last_appeared_pos[obj_idx].value().box.width;
                    int right_y = data->obj_last_appeared_pos[obj_idx].value().box.y +
                                  data->obj_last_appeared_pos[obj_idx].value().box.height / 2;
                    _x += right_x;
                    _y += right_y;
                }
            }
            _x /= int(_obj_idx.size());
            _y /= int(_obj_idx.size());
            break;
        }
    }

}

void skill_handler::_rel_skill(int slot_idx, int x, int y)
{
    data->last_rel_skill_slot_idx.insert(data->last_rel_skill_slot_idx.begin(), slot_idx);
    data->last_rel_skill_slot_idx.resize(3);

    BAASConfig _patch, _out;
    _patch.insert("/tentative_click", nlohmann::json::array({true, 891, 621, 1.0}));
    baas->solve_procedure("fight_select_slot_" + std::to_string(slot_idx), _out, _patch, true);
    _patch.insert("/possibles/0/1", x);
    _patch.insert("/possibles/0/2", y);
    _patch.update("/tentative_click", nlohmann::json::array({true, x, y, 0.5}));
    baas->solve_procedure("fight_release_skill_slot_" + std::to_string(slot_idx), _out, _patch, true);
}

void skill_handler::_get_l_r_slot_idx(int& _slot_idx)
{
    if (data->last_rel_skill_slot_idx.size() <= _l_r_idx) {
        logger->BAASError("[ l_rel_idx ] out of range.");
        throw ValueError("[ l_rel_idx ] out of range.");
    }
    _slot_idx = data->last_rel_skill_slot_idx[_l_r_idx];
    BAASGlobalLogger->BAASInfo("l_rel_idx : " + std::to_string(_l_r_idx) +
            " slot idx : " + std::to_string(_slot_idx));
}

BAAS_NAMESPACE_END
