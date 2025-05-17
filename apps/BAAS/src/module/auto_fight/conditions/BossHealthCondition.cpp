//
// Created by Administrator on 2025/5/15.
//

#include "module/auto_fight/conditions/BossHealthCondition.h"

BAAS_NAMESPACE_BEGIN

const std::map<std::string, BossHealthCondition::Op> BossHealthCondition::op_map = {
        {"C_over", C_OVER},
        {"C_below", C_BELOW},
        {"C_in_range", C_IN_RANGE},
        {"C_increase", C_INCREASE},
        {"C_decrease", C_DECREASE},
        {"M_equal", M_EQUAL}
};

const std::vector<std::string> BossHealthCondition::op_to_st = {
        "C_over",
        "C_below",
        "C_in_range",
        "C_increase",
        "C_decrease",
        "M_equal"
};

BossHealthCondition::BossHealthCondition(
        BAAS* baas,
        auto_fight_d* data,
        const BAASConfig& config
) : BaseCondition(baas, data, config)
{
    
    type = BaseCondition::ConditionType::BOSS_HEALTH;
    _parse_op();
}

std::optional<bool> BossHealthCondition::try_match()
{
    switch (_op) {
        case C_OVER:
        case C_BELOW:
        case C_IN_RANGE:
        case C_INCREASE:
        case C_DECREASE:
            if (!data->boss_current_health.has_value()) return std::nullopt;
            break;
        case M_EQUAL:
            if (!data->boss_max_health.has_value())     return std::nullopt;
            break;
    }

    std::optional<bool> ret = std::nullopt;
    switch (_op) {
        case C_OVER:
            if(data->boss_current_health > _value) ret = true;
            break;
        case C_BELOW:
            if(data->boss_current_health < _value) ret = true;
            break;
        case C_IN_RANGE:
            if(data->boss_current_health > _range_min && data->boss_current_health < _range_max) ret = true;
            break;
        case C_INCREASE:
            if (_last_recorded_health.has_value() &&
               (data->boss_current_health.value() > _last_recorded_health.value())) {
                _health_increment =
                        _health_increment + (data->boss_current_health.value() - _last_recorded_health.value());
                if (_health_increment > _value) ret = true;
            }
            break;
        case C_DECREASE:
            if (_last_recorded_health.has_value() &&
               (data->boss_current_health.value() < _last_recorded_health.value())) {
                _health_increment =
                        _health_increment + (_last_recorded_health.value() - data->boss_current_health.value());
                if (_health_increment > _value) ret = true;
            }
            break;
        case M_EQUAL:
            if (data->boss_max_health == _value) ret = true;
            break;
    }

    _last_recorded_health = data->boss_current_health;

    return ret;
}


void BossHealthCondition::display() const noexcept
{
    _display_basic_info();
    logger->BAASInfo("Op      : [ " + op_to_st[_op] + " ]");
    switch (_op) {
        case C_IN_RANGE:
            logger->BAASInfo("Range   : [ " + std::to_string(_range_min) + ", " + std::to_string(_range_max) + " ]");
            break;
        case C_OVER:
        case C_BELOW:
        case C_INCREASE:
        case C_DECREASE:
        case M_EQUAL:
            logger->BAASInfo("Value   : [ " + std::to_string(_value) + " ]");
            break;
    }
}

void BossHealthCondition::_parse_config_value()
{
    if(!config.contains("value"))   {
        logger->BAASError("BossHealthCondition op " + std::to_string(_op) + " requires [ value ].");
        throw ValueError("BossHealthCondition op " + std::to_string(_op) + " requires [ value ].");
    }
    if(!config.get<nlohmann::json>("value").is_number()) {
        logger->BAASError("BossHealthCondition config [ value ] must be number.");
        throw TypeError("BossHealthCondition [ value ] TypeError");
    }
    _value = config.getDouble("value");
}

void BossHealthCondition::_parse_config_range()
{
    if(!config.contains("range")) {
        logger->BAASError("BossHealthCondition op " + std::to_string(_op) + " requires [ range ].");
        throw ValueError("BossHealthCondition op " + std::to_string(_op) + " requires [ range ].");
    }
    if(config.get_array_size("range") != 2) {
        logger->BAASError("BossHealthCondition op " + std::to_string(_op) + " requires [ range ] to be a 2-element array.");
        throw ValueError("BossHealthCondition op " + std::to_string(_op) + " requires [ range ] to be a 2-element array.");
    }
    nlohmann::json j = config.getJson("range");
    if(!j[0].is_number() || !j[1].is_number()) {
        logger->BAASError("BossHealthCondition config [ range ] element must be number.");
        throw TypeError("BossHealthCondition [ range ] element TypeError");
    }
    _range_min = j[0];
    _range_max = j[1];
}

void BossHealthCondition::_parse_op()
{
    std::string op = this->config.getString("op");
    if (op_map.find(op) == op_map.end()) {
        logger->BAASError("Invalid BossHealthCondition op: " + op);
        throw ValueError("Invalid BossHealthCondition op.");
    }
    _op = op_map.at(op);
    switch (_op) {
        case C_OVER:

        case C_BELOW:

        case C_INCREASE:

        case C_DECREASE:

        case M_EQUAL:
            _parse_config_value();
            break;
        case C_IN_RANGE:
            _parse_config_range();
            break;
    }
}

void BossHealthCondition::reset_state()
{
    _last_recorded_health = std::nullopt;
    _health_increment = 0;
}

void BossHealthCondition::set_d_update_flag()
{
    data->d_updater_mask |= (1LL << 1);

    switch (_op) {
        case C_OVER:
        case C_BELOW:
        case C_IN_RANGE:
        case C_INCREASE:
        case C_DECREASE:
            if(data->boss_health_update_flag == 0b100) break;
            if(data->boss_health_update_flag & 0b001)  data->boss_health_update_flag = 0b100;
            else                                       data->boss_health_update_flag = 0b010;
            break;
        case M_EQUAL:
            if(data->boss_health_update_flag == 0b100) break;
            if(data->boss_health_update_flag & 0b010)  data->boss_health_update_flag = 0b100;
            else                                       data->boss_health_update_flag = 0b001;
            break;
    }
}


BAAS_NAMESPACE_END