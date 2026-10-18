//
// Created by Administrator on 2025/5/7.
//

#include "module/auto_fight/conditions/CostCondition.h"

#include <optional>
#include <string>

#include "module/auto_fight/workflow/ConditionConfig.h"
#include "module/auto_fight/workflow/WorkflowNames.h"

BAAS_NAMESPACE_BEGIN


CostCondition::CostCondition(
        BAAS* baas,
        auto_fight_d* data,
        const ConditionConfig& config,
        const ConditionIndex& indices
) : BaseCondition(baas, data, config, indices)
{
    type = COST;
    _op = config.numeric_op;
    _value = config.value;
    _range_min = config.range_min;
    _range_max = config.range_max;
}

std::optional<bool> CostCondition::try_match()
{
    if (!data->cost.has_value()) return std::nullopt;
    if (is_first_frame) {
        _last_recorded_cost = data->cost;
        is_first_frame = false;
        return std::nullopt;
    }

    std::optional<bool> ret;
    switch (_op) {
        case OVER:
            if(data->cost >= _value) ret = true;
            break;
        case BELOW:
            if(data->cost <= _value) ret = true;
            break;
        case IN_RANGE:
            if(data->cost >= _range_min && data->cost <= _range_max) ret = true;
            break;
        case INCREASE:
            if (_last_recorded_cost.has_value() && (data->cost.value() > _last_recorded_cost.value())) {
                _cost_increment = _cost_increment + (data->cost.value() - _last_recorded_cost.value());
                if (_cost_increment >= _value) ret = true;
            }
            break;
        case DECREASE:
            if (_last_recorded_cost.has_value() && (data->cost.value() < _last_recorded_cost.value())) {
                _cost_increment = _cost_increment + (_last_recorded_cost.value() - data->cost.value());
                if (_cost_increment >= _value) ret = true;
            }
            break;
    }

    _last_recorded_cost = data->cost;
    return ret;
}

void CostCondition::reset_state()
{
    _cost_increment = 0;
    is_first_frame = true;
}

void CostCondition::display() const noexcept
{
    _display_basic_info();
    logger->BAASInfo("Op      : " + std::string(wire_name(_op)));
    switch (_op) {
        case IN_RANGE:
            logger->BAASInfo("Range   : [ " + std::to_string(_range_min) + ", " + std::to_string(_range_max) + " ]");
            break;
        case OVER:
        case BELOW:
        case INCREASE:
        case DECREASE:
            logger->BAASInfo("Value   : " + std::to_string(_value));
            break;
    }
}

void CostCondition::set_d_update_flag()
{
    data->d_updater_mask |= (1LL << 0);
}


BAAS_NAMESPACE_END
