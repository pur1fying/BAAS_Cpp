//
// Created by Administrator on 2026/2/18.
//

//
// Created by Administrator on 2025/5/7.
//

#include "module/auto_fight/conditions/BattleTimeCondition.h"

#include <optional>
#include <string>

#include "module/auto_fight/workflow/ConditionConfig.h"
#include "module/auto_fight/workflow/WorkflowNames.h"
#include "module/auto_fight/screenshot_data/BattleTimeUpdater.h"

BAAS_NAMESPACE_BEGIN


BattleTimeCondition::BattleTimeCondition(
        BAAS* baas,
        auto_fight_d* data,
        const ConditionConfig& config,
        const ConditionIndex& indices
) : BaseCondition(baas, data, config, indices)
{
    type = BATTLE_TIME;
    _op = config.numeric_op;
    _value = config.time_value;
    _range_min = config.time_range_min;
    _range_max = config.time_range_max;
    if (_op == IN_RANGE) {
        _time_str = BAASStringUtil::format_battle_time_ms(_range_min) + " - "
                + BAASStringUtil::format_battle_time_ms(_range_max);
    } else {
        _time_str = BAASStringUtil::format_battle_time_ms(_value);
    }
}

std::optional<bool> BattleTimeCondition::try_match()
{
    if (!data->fight_left_time_ms.has_value()) return std::nullopt;
    if (is_first_frame) {
        _last_recorded_time = data->fight_left_time_ms;
        is_first_frame = false;
        return std::nullopt;
    }

    std::optional<bool> ret;
    switch (_op) {
        case OVER:
            if(data->fight_left_time_ms >= _value) ret = true;
            break;
        case BELOW:
            if(data->fight_left_time_ms <= _value) ret = true;
            break;
        case IN_RANGE:
            if(data->fight_left_time_ms >= _range_min && data->fight_left_time_ms <= _range_max) ret = true;
            break;
        case INCREASE:
            if (_last_recorded_time.has_value() && (data->fight_left_time_ms.value() > data->fight_left_time_ms.value())) {
                _time_increment = _time_increment + (data->fight_left_time_ms.value() - data->fight_left_time_ms.value());
                if (_time_increment >= _value) ret = true;
            }
            break;
        case DECREASE:
            if (_last_recorded_time.has_value() && (data->cost.value() < _last_recorded_time.value())) {
                _time_increment = _time_increment + (_last_recorded_time.value() - data->cost.value());
                if (_time_increment >= _value) ret = true;
            }
            break;
    }

    _last_recorded_time = data->fight_left_time_ms;
    return ret;
}

void BattleTimeCondition::reset_state()
{
    _time_increment = 0;
    is_first_frame = true;
}

void BattleTimeCondition::display() const noexcept
{
    _display_basic_info();
    logger->BAASInfo("Op      : " + std::string(wire_name(_op)));
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
