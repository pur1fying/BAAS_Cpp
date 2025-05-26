//
// Created by Administrator on 2025/5/17.
//

#include "module/auto_fight/actions/skill_handler.h"

#include "module/auto_fight/screenshot_data/BaseDataUpdater.h"

BAAS_NAMESPACE_BEGIN

const std::map<std::string, skill_handler::Release_Op> skill_handler::release_op_map = {
        {"auto", AUTO},
        {"name", NAME},
        {"l_rel_p", LAST_REL_IDX}
};

const std::map<std::string, skill_handler::Check_Op> skill_handler::check_op_map = {
        {"C_decrease", C_DECREASE},
        {"", NO_CHECK}
};

const std::map<std::string, skill_handler::Target_Op> skill_handler::target_op_map = {
        {"fixed", FIXED},
        {"yolo_t_p", YOLO_T_P_MEAN},
        {"yolo_c_p", YOLO_C_P_MEAN},
        {"yolo_g_p", YOLO_G_P_MEAN},
        {"yolo_l_p", YOLO_L_P_MEAN},
        {"yolo_r_p", YOLO_R_P_MEAN}
};

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
    _parse_target_op();
    _parse_check_op();
}

void skill_handler::_parse_release_op()
{
    if (!config.contains("op")) {
        logger->BAASError("If you want to do action [ skill ], you must fill [ op ] which indicates the skill release method type.");
        _display_valid_release_op();
        throw ValueError("[ /op ] must be specified.");
    }
    if (config.value_type("op") != nlohmann::detail::value_t::string) {
        logger->BAASError("[ /op ] must be a string.");
        throw TypeError("[ /op ] TypeError");
    }

    std::string _rel_op_st = this->config.getString("op");
    if (release_op_map.find(_rel_op_st) == release_op_map.end()) {
        logger->BAASError("Invalid skill_handler release op : [ " + _rel_op_st + " ]");
        _display_valid_release_op();
        throw ValueError("Invalid skill_handler release op.");
    }

    _r_op = release_op_map.at(_rel_op_st);

    switch (_r_op) {
        case AUTO:
            break;
        case NAME:
            _parse_skill_n_idx();
            break;
        case LAST_REL_IDX:
            _parse_l_r_idx();
            break;
    }

}

void skill_handler::_parse_target_op()
{
    switch (_r_op) {
        // no target
        case AUTO:
            break;
        case NAME:
        case LAST_REL_IDX:{
            if (!config.contains("/target/op")) {
                logger->BAASError("If you want to release skill with a target, you must fill [ /target/op ]"
                                  " which indicates the target type.");
                _display_valid_target_op();
                throw ValueError("[ /target/op ] must be specified.");
            }
            if (config.value_type("/target/op") != nlohmann::detail::value_t::string) {
                logger->BAASError("[ /target/op ] must be a string.");
                throw TypeError("[ /target/op ] TypeError");
            }
            std::string _t_op_st = this->config.getString("/target/op");
            if (target_op_map.find(_t_op_st) == target_op_map.end()) {
                logger->BAASError("Invalid skill_handler target op : [ " + _t_op_st + " ]");
                _display_valid_target_op();
                throw ValueError("Invalid skill_handler target op.");
            }
            _t_op = target_op_map.at(_t_op_st);
            switch(_t_op) {
                case FIXED: {
                    if (!config.contains("/target/x")) {
                        logger->BAASError("When target is fixed, you must fill [ /target/x ].");
                        throw ValueError("Fixed position [ x ] must be specified.");
                    }
                    if (!config.contains("/target/y")) {
                        logger->BAASError("When target is fixed, you must fill [ /target/y ].");
                        throw ValueError("Fixed position [ y ] must be specified.");
                    }
                    if (!config.getJson("/target/x").is_number()) {
                        logger->BAASError("When target is fixed [ /target/x ] must be a number.");
                        throw TypeError("[ /target/x ] TypeError");
                    }
                    if (!config.getJson("/target/y").is_number()) {
                        logger->BAASError("When target is fixed [ /target/y ] must be a number.");
                        throw TypeError("[ /target/y ] TypeError");
                    }
                    _fixed_x = config.getInt("/target/x");
                    _fixed_y = config.getInt("/target/y");
                    break;
                }
                case YOLO_T_P_MEAN:
                case YOLO_C_P_MEAN:
                case YOLO_G_P_MEAN:
                case YOLO_L_P_MEAN:
                case YOLO_R_P_MEAN:{
                    if (!config.contains("/target/obj")) {
                        logger->BAASError("skill_handler target op [ " + std::to_string(_t_op) + " ] requires [ /target/obj ].");
                        throw ValueError("Yolo object name must be specified.");
                    }
                    auto obj_names = config.get<std::vector<std::string>>("/target/obj");
                    if (obj_names.size() == 0) {
                        logger->BAASError("skill_handler target op [ " + std::to_string(_t_op) + " ] requires [ /target/obj ] length > 0.");
                        throw ValueError("At least one object name is required.");
                    }
                    for (const auto &obj_name : obj_names) {
                        auto it = data->obj_name_to_index_map.find(obj_name);
                        if (it == data->obj_name_to_index_map.end()) {
                            logger->BAASError("Object [ " + obj_name + " ] is not detect in this workflow.");
                            throw ValueError("Invalid yolo object detected.");
                        }
                        else _obj_idx.push_back(it->second);
                    }
                    break;
                }
            }
            break;
        }
    }
}

void skill_handler::_parse_check_op()
{
    std::string _c_op_st = this->config.getString("/check/op", "");
    if (check_op_map.find(_c_op_st) == check_op_map.end()) {
        logger->BAASError("Invalid check op : [ " + _c_op_st + " ]");
        _display_valid_check_op();
        throw ValueError("Invalid check op.");
    }
    _c_op = check_op_map.at(_c_op_st);

    switch (_c_op) {
        case C_DECREASE: {
            if (!config.contains("/check/value")) {
                logger->BAASError("If you want to check skill release by [ Cost Decrease ], "
                                  "you must fill [ /check/value ] which indicates the cost decrease threshold.");
                throw ValueError("[ /check/value ] must be specified.");
            }
            if (!config.getJson("/check/value").is_number()) {
                logger->BAASError("[ /check/value ] must be a number.");
                throw TypeError("[ /check/value ] TypeError");
            }
            _c_v = config.getDouble("/check/value");
            _c_timeout = config.getUInt("/check/timeout", 5000);

            BAASConfig _c_c;
            _c_c.insert("op", "decrease");
            _c_c.insert("value", _c_v);
            _c_c.insert("timeout", _c_timeout);
            _cost_cond = std::make_unique<CostCondition>(baas, data, _c_c);
            break;
        }
        case NO_CHECK:
            break;
    }
}

void skill_handler::_parse_skill_n_idx()
{
    switch (_r_op) {
        case AUTO:
            break;
        case NAME: {
            if (!config.contains("skill_n")) {
                logger->BAASError("skill_handler op [ " + std::to_string(_r_op) + " ] requires [ skill_n ].");
                throw ValueError("skill_handler op [ " + std::to_string(_r_op) + " ] requires [ skill_n ].");
            }
            if (config.value_type("skill_n") != nlohmann::detail::value_t::string) {
                logger->BAASError("skill_handler config [ skill_n ] must be string.");
                throw TypeError("skill_handler [ skill_n ] TypeError");
            }
            std::string _skill_n_st = this->config.getString("skill_n");

            bool find = false;
            for (int i = 0; i < data->all_possible_skills.size(); ++i) {
                if (data->all_possible_skills[i].name == _skill_n_st) {
                    _skill_n_idx = i;
                    find = true;
                    break;
                }
            }
            if (!find) {
                logger->BAASError("Skill Name [ " + _skill_n_st + " ] not found in all_possible_skills.");
                throw ValueError("Invalid skill_handler skill_n");
            }
            break;
        }
        case LAST_REL_IDX:
            break;
    }
}

bool skill_handler::_C_decrease_check_loop()
{
    long long _start_t = BAASUtil::getCurrentTimeMS();

    while (true) {
        long long _time_elapsed = BAASUtil::getCurrentTimeMS() - _start_t;
        if (_time_elapsed >= _cost_cond->get_timeout()) {
            logger->BAASInfo("Skill Release Cost Decrease Check Timeout.");
            return false;
        }
        baas->i_update_screenshot_array();
        data->d_updaters[0]->update();
        if (_cost_cond->try_match()) {
            logger->BAASInfo("C_decrease Check Passed.");
            logger->BAASInfo("C After Skill_Rel : [ " + std::to_string(data->cost.value()) + " ]");
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
    _patch.update("/tentative_click", nlohmann::json::array({true, x, y, 1.0}));
    baas->solve_procedure("fight_release_skill_slot_" + std::to_string(slot_idx), _out, _patch, true);
}

void skill_handler::_parse_l_r_idx()
{
    if (!config.contains("l_rel_idx")) {
        logger->BAASError("skill_handler op [ " + std::to_string(_r_op) + " ] requires [ l_rel_idx ].");
        throw ValueError("skill_handler op [ " + std::to_string(_r_op) + " ] requires [ l_rel_idx ].");
    }
    if (config.value_type("l_rel_idx") != nlohmann::detail::value_t::number_unsigned) {
        logger->BAASError("skill_handler config [ l_rel_idx ] must be unsigned.");
        throw TypeError("skill_handler [ l_rel_idx ] TypeError");
    }
    _l_r_idx = config.getInt("l_rel_idx");
    if (_l_r_idx > 2 || _l_r_idx < 0) {
        logger->BAASError("skill_handler config [ l_rel_idx ] must be in [ 0, 1, 2 ].");
        throw ValueError("skill_handler config [ l_rel_idx ] must be in [ 0, 1, 2 ].");
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


BAAS_NAMESPACE_END