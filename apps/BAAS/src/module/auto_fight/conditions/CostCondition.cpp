//
// Created by Administrator on 2025/5/7.
//

#include "module/auto_fight/conditions/CostCondition.h"

BAAS_NAMESPACE_BEGIN

const std::map<std::string, CostCondition::Op> CostCondition::op_map = {
        {"over", OVER},
        {"below", BELOW},
        {"in_range", IN_RANGE},
        {"increase", INCREASE},
        {"decrease", DECREASE}
};

const std::vector<std::string> CostCondition::op_to_st = {
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
    if (_reset_cost) _last_recorded_cost = std::nullopt;
    _cost_increment = 0;
}

void CostCondition::_parse_op()
{
    std::string op = this->config.getString("op");
    if (op_map.find(op) == op_map.end()) {
        logger->BAASError("Invalid CostCondition op: " + op);
        throw ValueError("Invalid CostCondition op.");
    }
    _op = op_map.at(op);
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
    _reset_cost = this->config.getBool("reset", false);

}

void CostCondition::_parse_config_value()
{
    if(!config.contains("value"))   {
        logger->BAASError("CostCondition op " + std::to_string(_op) + " requires [ value ].");
        throw ValueError("CostCondition op " + std::to_string(_op) + " requires [ value ].");
    }
    if(!config.get<nlohmann::json>("value").is_number()) {
        logger->BAASError("CostCondition config [ value ] must be number.");
        throw TypeError("CostCondition [ value ] TypeError");
    }
    _value = config.getDouble("value");
}

void CostCondition::_parse_config_range()
{
    if(!config.contains("range")) {
        logger->BAASError("CostCondition op " + std::to_string(_op) + " requires [ range ].");
        throw ValueError("CostCondition op " + std::to_string(_op) + " requires [ range ].");
    }
    if(config.get_array_size("range") != 2) {
        logger->BAASError("CostCondition op " + std::to_string(_op) + " requires [ range ] to be a 2-element array.");
        throw ValueError("CostCondition op " + std::to_string(_op) + " requires [ range ] to be a 2-element array.");
    }
    nlohmann::json j = config.getJson("range");
    if(!j[0].is_number() || !j[1].is_number()) {
        logger->BAASError("CostCondition config [ range ] element must be number.");
        throw TypeError("CostCondition [ range ] element TypeError");
    }
    _range_min = j[0];
    _range_max = j[1];
}

void CostCondition::display() const noexcept
{
    _display_basic_info();
    logger->BAASInfo("Op      : [ " + op_to_st[_op] + " ]");
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