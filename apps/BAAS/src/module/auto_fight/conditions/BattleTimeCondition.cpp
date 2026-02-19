//
// Created by Administrator on 2026/2/18.
//

//
// Created by Administrator on 2025/5/7.
//

#include "module/auto_fight/conditions/BattleTimeCondition.h"

#include "module/auto_fight/constants.h"
#include "module/auto_fight/screenshot_data/BattleTimeUpdater.h"

BAAS_NAMESPACE_BEGIN

const std::map<std::string, BattleTimeCondition::Op> BattleTimeCondition::op_map = {
        {"over", OVER},
        {"below", BELOW},
        {"in_range", IN_RANGE},
        {"increase", INCREASE},
        {"decrease", DECREASE}
};

const std::vector<std::string> BattleTimeCondition::op_st_list = {
        "over",
        "below",
        "in_range",
        "increase",
        "decrease"
};

BattleTimeCondition::BattleTimeCondition(
        BAAS* baas,
        auto_fight_d* data,
        const BAASConfig& config
) : BaseCondition(baas, data, config)
{
    type = BATTLE_TIME;
    _parse_op();
}

std::optional<bool> BattleTimeCondition::try_match()
{
    if (!data->fight_left_time_ms.has_value()) return std::nullopt;

    std::optional<bool> ret;
    switch (_op) {
        case OVER:
            if(data->fight_left_time_ms > _value) ret = true;
            break;
        case BELOW:
            if(data->fight_left_time_ms < _value) ret = true;
            break;
        case IN_RANGE:
            if(data->fight_left_time_ms > _range_min && data->fight_left_time_ms < _range_max) ret = true;
            break;
        case INCREASE:
            if (_last_recorded_time.has_value() && (data->fight_left_time_ms.value() > data->fight_left_time_ms.value())) {
                _time_increment = _time_increment + (data->fight_left_time_ms.value() - data->fight_left_time_ms.value());
                if (_time_increment > _value) ret = true;
            }
            break;
        case DECREASE:
            if (_last_recorded_time.has_value() && (data->cost.value() < _last_recorded_time.value())) {
                _time_increment = _time_increment + (_last_recorded_time.value() - data->cost.value());
                if (_time_increment > _value) ret = true;
            }
            break;
    }

    _last_recorded_time = data->cost;
    return ret;
}

void BattleTimeCondition::reset_state()
{
    _time_increment = 0;
}

void BattleTimeCondition::_parse_op()
{
    auto _it = this->config.find("op");
    if (_it == this->config.end()) {
        logger->BAASError("[ BattleTimeCondition ] config must contain [ op ].");
        _log_valid_op("[ BattleTimeCondition ] [ op ]", logger, op_st_list);
        throw ValueError("[ BattleTimeCondition ] [ op ] not found.");
    }
    if (!_it->is_string()) {
        logger->BAASError("[ BattleTimeCondition ] [ op ] must be a string.");
        _log_valid_op("[ BattleTimeCondition ] [ op ]", logger, op_st_list);
        throw TypeError("Invalid [ BattleTimeCondition ] [ op ] Config Type.");
    }
    std::string op_str = *_it;
    auto it = op_map.find(op_str);
    if (it == op_map.end()) {
        logger->BAASError("Invalid [ BattleTimeCondition ] [ op ] : " + op_str);
        _log_valid_op("[ BattleTimeCondition ] [ op ]", logger, op_st_list);
        throw ValueError("Invalid [ BattleTimeCondition ] [ op ].");
    }
    _op = it->second;
    switch (_op) {
        case OVER:

        case BELOW:

        case INCREASE:

        case DECREASE:
            _parse_config_value();
            break;
        case IN_RANGE:
            _parse_config_range();
            break;
    }
}

void BattleTimeCondition::_parse_config_value()
{
    auto  _it = this->config.find("value");
    if (_it == this->config.end()) {
        logger->BAASError("[ BattleTimeCondition ] [ op ] : \"" + op_st_list[_op] + "\" requires [ value ].");
        throw ValueError("[ BattleTimeCondition ] [ value ] not found.");
    }
    if (!_it->is_number()) {
        logger->BAASError("[ BattleTimeCondition ] [ value ] must be a number.");
        throw TypeError("[ BattleTimeCondition ] [ value ] TypeError");
    }

    _value = _it->get<int>();
    _time_str = BAASStringUtil::format_battle_time_ms(_value);
}

void BattleTimeCondition::_parse_config_range()
{
    auto _it = this->config.find("range");
    if (_it == this->config.end()) {
        logger->BAASError("[ BattleTimeCondition ] [ op ] : \"" + op_st_list[_op] + "\" requires [ range ].");
        throw ValueError("[ BattleTimeCondition ] [ range ] not found.");
    }
    if (!_it->is_array()) {
        logger->BAASError("[ BattleTimeCondition ] [ range ] must be an array.");
        throw TypeError("[ BattleTimeCondition ] [ range ] TypeError");
    }
    if (_it->size() != 2) {
        logger->BAASError("[ BattleTimeCondition ] [ range ] array size must be 2.");
        throw ValueError("[ BattleTimeCondition ] [ range ] Size Error");
    }

    if(!(*_it)[0].is_number() || !(*_it)[1].is_number()) {
        logger->BAASError("[ BattleTimeCondition ] [ range ] element must be number.");
        throw TypeError("[ BattleTimeCondition ] [ range ] Element Type Error");
    }
    _range_min =  (*_it)[0].get<int>();
    _range_max =  (*_it)[1].get<int>();

    _time_str = BAASStringUtil::format_battle_time_ms(_range_min) + " - " + BAASStringUtil::format_battle_time_ms(_range_max);

}

void BattleTimeCondition::display() const noexcept
{
    _display_basic_info();
    logger->BAASInfo("Op      : " + op_st_list[_op]);
    switch (_op) {
        case IN_RANGE:
            logger->BAASInfo("Range   : " + _time_str);
            break;
        case OVER:
        case BELOW:
        case INCREASE:
        case DECREASE:
            logger->BAASInfo("Value   : " + _time_str);
            break;
    }
}

void BattleTimeCondition::set_d_update_flag()
{
    data->d_updater_mask |= (1LL << 7);
}


BAAS_NAMESPACE_END