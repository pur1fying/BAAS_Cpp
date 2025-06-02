//
// Created by Administrator on 2025/5/7.
//

#include "module/auto_fight/conditions/CostCondition.h"

#include "module/auto_fight/constants.h"

BAAS_NAMESPACE_BEGIN

const std::map<std::string, CostCondition::Op> CostCondition::op_map = {
        {"over", OVER},
        {"below", BELOW},
        {"in_range", IN_RANGE},
        {"increase", INCREASE},
        {"decrease", DECREASE}
};

const std::vector<std::string> CostCondition::op_st_list = {
        "over",
        "below",
        "in_range",
        "increase",
        "decrease"
};

CostCondition::CostCondition(
        BAAS* baas,
        auto_fight_d* data,
        const BAASConfig& config
) : BaseCondition(baas, data, config)
{
    type = COST;
    _parse_op();
}

CostCondition::CostCondition(
        BAAS* baas,
        auto_fight_d* data,
        CostCondition::Op op,
        double value,
        uint64_t timeout
) : BaseCondition(baas, data, timeout)
{
    type = COST;
    _op = op;
    _value = value;
}

std::optional<bool> CostCondition::try_match()
{
    if (!data->cost.has_value()) return std::nullopt;

    std::optional<bool> ret;
    switch (_op) {
        case OVER:
            if(data->cost > _value) ret = true;
            break;
        case BELOW:
            if(data->cost < _value) ret = true;
            break;
        case IN_RANGE:
            if(data->cost > _range_min && data->cost < _range_max) ret = true;
            break;
        case INCREASE:
            if (_last_recorded_cost.has_value() && (data->cost.value() > _last_recorded_cost.value())) {
                _cost_increment = _cost_increment + (data->cost.value() - _last_recorded_cost.value());
                if (_cost_increment > _value) ret = true;
            }
            break;
        case DECREASE:
            if (_last_recorded_cost.has_value() && (data->cost.value() < _last_recorded_cost.value())) {
                _cost_increment = _cost_increment + (_last_recorded_cost.value() - data->cost.value());
                if (_cost_increment > _value) ret = true;
            }
            break;
    }

    _last_recorded_cost = data->cost;
    return ret;
}

void CostCondition::reset_state()
{
    _cost_increment = 0;
}

void CostCondition::_parse_op()
{
    auto _it = this->config.find("op");
    if (_it == this->config.end()) {
        logger->BAASError("[ CostCondition ] config must contain [ op ].");
        _log_valid_op("[ CostCondition ] [ op ]", logger, op_st_list);
        throw ValueError("[ Cost ] [ op ] not found.");
    }
    if (!_it->is_string()) {
        logger->BAASError("[ CostCondition ] [ op ] must be a string.");
        _log_valid_op("[ CostCondition ] [ op ]", logger, op_st_list);
        throw TypeError("Invalid [ CostCondition ] [ op ] Config Type.");
    }
    std::string op_str = *_it;
    auto it = op_map.find(op_str);
    if (it == op_map.end()) {
        logger->BAASError("Invalid [ CostCondition ] [ op ] : " + op_str);
        _log_valid_op("[ CostCondition ] [ op ]", logger, op_st_list);
        throw ValueError("Invalid [ CostCondition ] [ op ].");
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

void CostCondition::_parse_config_value()
{
    auto  _it = this->config.find("value");
    if (_it == this->config.end()) {
        logger->BAASError("[ CostCondition ] [ op ] : \"" + op_st_list[_op] + "\" requires [ value ].");
        throw ValueError("[ CostCondition ] [ value ] not found.");
    }
    if (!_it->is_number()) {
        logger->BAASError("[ CostCondition ] [ value ] must be a number.");
        throw TypeError("[ CostCondition ] [ value ] TypeError");
    }

    _value = _it->get<double>();
}

void CostCondition::_parse_config_range()
{
    auto _it = this->config.find("range");
    if (_it == this->config.end()) {
        logger->BAASError("[ CostCondition ] [ op ] : \"" + op_st_list[_op] + "\" requires [ range ].");
        throw ValueError("[ CostCondition ] [ range ] not found.");
    }
    if (!_it->is_array()) {
        logger->BAASError("[ CostCondition ] [ range ] must be an array.");
        throw TypeError("[ CostCondition ] [ range ] TypeError");
    }
    if (_it->size() != 2) {
        logger->BAASError("[ CostCondition ] [ range ] array size must be 2.");
        throw ValueError("[ CostCondition ] [ range ] Size Error");
    }

    if(!(*_it)[0].is_number() || !(*_it)[1].is_number()) {
        logger->BAASError("[ CostCondition ] [ range ] element must be number.");
        throw TypeError("[ CostCondition ] [ range ] Element Type Error");
    }
    _range_min =  (*_it)[0].get<double>();
    _range_max =  (*_it)[1].get<double>();
}

void CostCondition::display() const noexcept
{
    _display_basic_info();
    logger->BAASInfo("Op      : [ " + op_st_list[_op] + " ]");
    switch (_op) {
        case IN_RANGE:
            logger->BAASInfo("Range   : [ " + std::to_string(_range_min) + ", " + std::to_string(_range_max) + " ]");
            break;
        case OVER:
        case BELOW:
        case INCREASE:
        case DECREASE:
            logger->BAASInfo("Value   : [ " + std::to_string(_value) + " ]");
            break;
    }
}

void CostCondition::set_d_update_flag()
{
    data->d_updater_mask |= (1LL << 0);
}


BAAS_NAMESPACE_END