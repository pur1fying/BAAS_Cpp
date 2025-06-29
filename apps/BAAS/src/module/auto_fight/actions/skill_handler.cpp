//
// Created by Administrator on 2025/5/17.
//

#include "module/auto_fight/actions/skill_handler.h"

#include <utils/BAASChronoUtil.h>

#include "module/auto_fight/constants.h"
#include "module/auto_fight/screenshot_data/BaseDataUpdater.h"

BAAS_NAMESPACE_BEGIN

skill_handler::skill_handler(
        BAAS* baas,
        auto_fight_d* data,
        const BAASConfig& config
) : base_handler(baas, data, config)
{
    _parse_op();
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

    _execute_release_check();
    _execute_postprocess();
    return true;
}

void skill_handler::display() noexcept
{

}

void skill_handler::_parse_op()
{
    _parse_release_op();
    _parse_check_op();
}

void skill_handler::_parse_release_op()
{
    auto _it = this->config.find("op");
    if (_it == this->config.end()) {
        logger->BAASError("[ Action Skill ] config must contain [ op ].");
        _log_valid_op("[ Action Skill ] [ op ]", logger, release_op_st_list);
        throw ValueError("[ Action Skill ] [ op ] not found.");
    }
    if (!_it->is_string()) {
        logger->BAASError("[ Action Skill ] [ op ] must be a string.");
        _log_valid_op("[ Action Skill ] [ op ]", logger, release_op_st_list);
        throw TypeError("Invalid [ Action Skill ] [ op ] Config Type.");
    }

    std::string _op_st = *_it;
    auto it = release_op_map.find(_op_st);
    if (it == release_op_map.end()) {
        logger->BAASError("Invalid [ Action Skill ] [ op ] : " + _op_st);
        _log_valid_op("[ Action Skill ] [ op ]", logger, release_op_st_list);
        throw ValueError("Invalid [ Action Skill ] [ op ].");
    }

    _r_op = it->second;
    switch (_r_op) {
        case AUTO:
            break;
        case NAME:
            _parse_skill_n_idx();
            _parse_target_op();
            break;
        case LAST_REL_IDX:
            _parse_l_r_idx();
            _parse_target_op();
            break;
    }

}

void skill_handler::_parse_target_op()
{
    auto _target_it = this->config.find("target");
    if (_target_it == this->config.end()) {
        logger->BAASError("[ Action skill ] [ op ] : \"" + release_op_st_list[_r_op] + "\" requires [ target ].");
        throw ValueError("[ Action skill ] [ target ] not found.");
    }
    if (!_target_it->is_object()) {
        logger->BAASError("[ Action skill ] [ /target ] must be an object.");
        throw TypeError("[ Action skill ] [ /target ] TypeError");
    }

    auto _op_it = _target_it->find("op");
    if (_op_it == _target_it->end()) {
        logger->BAASError("[ Action skill ] [ op ] : \"" + release_op_st_list[_r_op] + "\" requires [ /target/op ].");
        _log_valid_op("[ Action skill ] [ /target/op ]", logger, target_op_st_list);
        throw ValueError("[ Action skill ] [ /target/op ] not found.");
    }
    if (!_op_it->is_string()) {
        logger->BAASError("[ Action skill ] [ /target/op ] must be a string.");
        _log_valid_op("[ Action skill ] [ /target/op ]", logger, target_op_st_list);
        throw TypeError("[ Action skill ] [ /target/op ] TypeError");
    }
    std::string _t_op_st = *_op_it;
    auto it = target_op_map.find(_t_op_st);
    if (it == target_op_map.end()) {
        logger->BAASError("Invalid [ Action skill ] [ /target/op ] : " + _t_op_st);
        _log_valid_op("[ Action skill ] [ /target/op ]", logger, target_op_st_list);
        throw ValueError("Invalid [ Action skill ] [ /target/op ].");
    }

    _t_op = it->second;
    switch(_t_op) {
        case FIXED: {
            _parse_fixed_x_y(_target_it);
            break;
        }
        case YOLO_T_P_MEAN:
        case YOLO_C_P_MEAN:
        case YOLO_G_P_MEAN:
        case YOLO_L_P_MEAN:
        case YOLO_R_P_MEAN:{
            _parse_yolo_obj(_target_it);
            break;
        }
    }
}

void skill_handler::_parse_check_op()
{
    auto _check_it = this->config.find("check");
    if (_check_it == this->config.end()) {
        _c_op = NO_CHECK;
        return;
    }
    if (!_check_it->is_object()) {
        logger->BAASError("[ Action skill ] [ check ] must be an object.");
        throw TypeError("[ Action skill ] [ check ] TypeError");
    }

    auto _check_op_it = _check_it->find("op");
    if (_check_op_it == _check_it->end()) {
        logger->BAASError("[ Action skill ] [ check ] must contain [ /check/op ].");
        _log_valid_op("[ Action skill ] [ /check/op ]", logger, check_op_st_list);
        throw ValueError("[ Action skill ] [ /check/op ] not found.");
    }
    if (!_check_op_it->is_string()) {
        logger->BAASError("[ Action skill ] [ /check/op ] must be a string.");
        _log_valid_op("[ Action skill ] [ /check/op ]", logger, check_op_st_list);
        throw TypeError("[ Action skill ] [ /check/op ] TypeError");
    }
    std::string _c_op_st = *_check_op_it;
    auto it = check_op_map.find(_c_op_st);
    if (it == check_op_map.end()) {
        logger->BAASError("Invalid [ Action skill ] [ /check/op ] : " + _c_op_st);
        _log_valid_op("[ Action skill ] [ /check/op ]", logger, check_op_st_list);
        throw ValueError("Invalid check op.");
    }
    _c_op = it->second;
    switch (_c_op) {
        case C_DECREASE: {
            _parse_check_value(_check_it);
            _c_timeout = config.getLLong("/check/timeout", 5000);
            _cost_cond = std::make_unique<CostCondition>(
                baas,
                data,
                CostCondition::Op::DECREASE,
                _c_v,
                _c_timeout
            );
            break;
        }
        case NO_CHECK:
            break;
    }
}

void skill_handler::_parse_skill_n_idx()
{
    auto it = this->config.find("skill_n");

     if (it == this->config.end()) {
         logger->BAASError("[ Action Skill ] [ op ] : \"" + release_op_st_list[_r_op] + "\" requires [ skill_n ].");
         _log_valid_skill_names("[ Action Skill ] [ skill_n ]", logger, data);
         throw ValueError("[ Action Skill ] [ skill_n ] not found.");
     }
    if (!it->is_string()) {
        logger->BAASError("[ Action Skill ] [ skill_n ] must be a string.");
        _log_valid_skill_names("[ Action Skill ] [ skill_n ]", logger, data);
        throw TypeError("Invalid [ Action Skill ] [ skill_n ] Config Type.");
    }

    std::string _skill_n_st = *it;

    bool find = false;
    for (int i = 0; i < data->all_possible_skills.size(); ++i) {
        if (data->all_possible_skills[i].name == _skill_n_st) {
            _skill_n_idx = i;
            find = true;
            break;
        }
    }
    if (!find) {
        logger->BAASError("[ Action Skill ] [ skill_n ] : \"" + _skill_n_st + "\" not found in all possible skills.");
        _log_valid_skill_names("[ Action Skill ] [ skill_n ]", logger, data);
        throw ValueError("Invalid [ Action Skill ] [ skill_n ].");

    }
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

void skill_handler::_parse_l_r_idx()
{
    auto _it = this->config.find("l_rel_idx");
    if (_it == this->config.end()) {
        logger->BAASError("[ Action Skill ] [ op ] : \"" + release_op_st_list[_r_op] + "\" requires [ l_rel_idx ].");
        throw ValueError("[ Action Skill ] [ l_rel_idx ] not found.");
    }

    if (!_it->is_number_unsigned()) {
        logger->BAASError("[ Action Skill ] [ l_rel_idx ] must be an unsigned integer.");
        throw TypeError("[ Action Skill ] [ l_rel_idx ] TypeError");
    }

    _l_r_idx = config.getInt("l_rel_idx");
    if (_l_r_idx > 2 || _l_r_idx < 0) {
        logger->BAASError("[ Action Skill ] [ l_rel_idx ] must be in [ 0, 1, 2 ].");
        throw ValueError("[ Action Skill ] [ l_rel_idx ] out of range.");
    }
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

void skill_handler::_parse_fixed_x_y(const nlohmann::json::const_iterator& _target_it)
{
    auto _it = _target_it->find("x");
    if (_it == _target_it->end()) {
        logger->BAASError("[ Action Skill ] [ /target/op ] : \"" + target_op_st_list[_t_op] + "\" requires [ /target/x ].");
        throw ValueError("[ Action Skill ] [ /target/x ] not found.");
    }
    if (!_it->is_number()) {
        logger->BAASError("[ Action Skill ] [ /target/x ] must be a number.");
        throw TypeError("[ /target/x ] TypeError");
    }
    _fixed_x = _it->get<int>();

    _it = _target_it->find("y");
    if (_it == _target_it->end()) {
        logger->BAASError("[ Action Skill ] [ /target/op ] : \"" + target_op_st_list[_t_op] + "\" requires [ /target/y ].");
        throw ValueError("[ Action Skill ] [ /target/y ] not found.");
    }
    if (!_it->is_number()) {
        logger->BAASError("[ Action Skill ] [ /target/y ] must be a number.");
        throw TypeError("[ /target/y ] TypeError");
    }
    _fixed_y = _it->get<int>();
}

void skill_handler::_parse_yolo_obj(const nlohmann::json::const_iterator& _target_it)
{
    auto _obj_it = _target_it->find("obj");
    if (_obj_it == _target_it->end()) {
        logger->BAASError("[ Action Skill ] [ /target/op ] : \"" + target_op_st_list[_t_op] + "\" requires [ /target/obj ].");
        _log_valid_yolo_obj("[ Action Skill ] [ /target/obj ]", logger, data);
        throw ValueError("[ Action Skill ] [ /target/obj ] not found.");
    }

    if (!_obj_it->is_array()) {
        logger->BAASError("[ Action Skill ] [ /target/obj ] must be an array.");
        throw TypeError("[ Action Skill ] [ /target/obj ] TypeError");
    }

    for (const auto& obj : *_obj_it) {
        if (!obj.is_string()) {
            logger->BAASError("Element of [ Action Skill ] [ /target/obj ] must be string.");
            throw TypeError("Element of [ Action Skill ] [ /target/obj ] TypeError");
        }
    }

    std::vector<std::string> obj_names = *_obj_it;

    for (const auto &obj_name : obj_names) {
        auto  _it = data->obj_name_to_index_map.find(obj_name);
        if (_it == data->obj_name_to_index_map.end()) {
            logger->BAASError("Object [ " + obj_name + " ] is not detect in this workflow.");
            _log_valid_yolo_obj("[ Action Skill ] [ /target/obj ]", logger, data);
            throw ValueError("Invalid yolo object detected.");
        }
        else _obj_idx.push_back(_it->second);
    }

    if (_obj_idx.empty()) {
        logger->BAASError("You didn't specify any obj in [ Action Skill ] [ /target/obj ].");
        _log_valid_yolo_obj("[ Action Skill ] [ /target/obj ]", logger, data);
        throw ValueError("At least one obj must be specified.");
    }
}

void skill_handler::_parse_check_value(const nlohmann::json::const_iterator& _check_it)
{
    auto _value_it = _check_it->find("value");
    if (_value_it == _check_it->end()) {
        logger->BAASError("[ Action Skill ] [ /check/op ] : \"" + check_op_st_list[_c_op] + "\" requires [ /check/value ].");
        throw ValueError("[ Action Skill ] [ /check/value ] not found.");
    }
    if (!_value_it->is_number()) {
        logger->BAASError("[ Action Skill ] [ /check/value ] must be a number.");
        throw TypeError("[ Action Skill ] [ /check/value ] TypeError");
    }
    _c_v = _value_it->get<double>();
}


BAAS_NAMESPACE_END