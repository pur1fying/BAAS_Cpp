//
// Created by Administrator on 2025/5/15.
//

#include <memory>
#include "module/auto_fight/conditions/BossHealthCondition.h"

#include "module/auto_fight/constants.h"

BAAS_NAMESPACE_BEGIN

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
    logger->BAASInfo("Op      : [ " + op_st_list[_op] + " ]");
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
    auto  _it = this->config.find("value");
    if (_it == this->config.end()) {
        logger->BAASError("[ BossHealthCondition ] [ op ] : \"" + op_st_list[_op] + "\" requires [ value ].");
        throw ValueError("[ BossHealthCondition ] [ value ] not found.");
    }
    if (!_it->is_number()) {
        logger->BAASError("[ BossHealthCondition ] [ value ] must be a number.");
        throw TypeError("[ BossHealthCondition ] [ value ] TypeError");
    }

    _value = _it->get<double>();
}

void BossHealthCondition::_parse_config_range()
{
    auto _it = this->config.find("range");
    if (_it == this->config.end()) {
        logger->BAASError("[ BossHealthCondition ] [ op ] : \"" + op_st_list[_op] + "\" requires [ range ].");
        throw ValueError("[ BossHealthCondition ] [ range ] not found.");
    }
    if (!_it->is_array()) {
        logger->BAASError("[ BossHealthCondition ] [ range ] must be an array.");
        throw TypeError("[ BossHealthCondition ] [ range ] TypeError");
    }
    if (_it->size() != 2) {
        logger->BAASError("[ BossHealthCondition ] [ range ] array size must be 2.");
        throw ValueError("[ BossHealthCondition ] [ range ] Size Error");
    }

    if(!(*_it)[0].is_number() || !(*_it)[1].is_number()) {
        logger->BAASError("[ BossHealthCondition ] [ range ] element must be number.");
        throw TypeError("[ BossHealthCondition ] [ range ] Element Type Error");
    }
    _range_min =  (*_it)[0].get<double>();
    _range_max =  (*_it)[1].get<double>();
}

void BossHealthCondition::_parse_op()
{
    auto it = config.find("op");
    if (it == config.end()) {
        logger->BAASError("[ BossHealthCondition ] config must contain [ op ].");
        _log_valid_op("[ BossHealthCondition ] [ op ]", logger, op_st_list);
        throw ValueError("[ BossHealthCondition ] [ op ] not found.");
    }
    if (!it->is_string()) {
        logger->BAASError("[ BossHealthCondition ] [ op ] must be a string.");
        _log_valid_op("[ BossHealthCondition ] [ op ]", logger, op_st_list);
        throw TypeError("[ BossHealthCondition ] [ op ] TypeError");
    }
    std::string op_str = *it;
    auto _it = op_map.find(op_str);
    if (_it == op_map.end()) {
        logger->BAASError("Invalid [ BossHealthCondition ] [ op ] : " + op_str);
        _log_valid_op("[ BossHealthCondition ] [ op ]", logger, op_st_list);
        throw ValueError("Invalid [ BossHealthCondition ] [ op ].");
    }
    _op = _it->second;
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