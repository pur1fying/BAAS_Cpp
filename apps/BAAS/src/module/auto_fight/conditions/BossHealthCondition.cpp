//
// Created by Administrator on 2025/5/15.
//

#include <optional>
#include <string>

#include "module/auto_fight/workflow/ConditionConfig.h"
#include "module/auto_fight/workflow/WorkflowNames.h"
#include "module/auto_fight/conditions/BossHealthCondition.h"

BAAS_NAMESPACE_BEGIN

BossHealthCondition::BossHealthCondition(
        BAAS* baas,
        auto_fight_d* data,
        const ConditionConfig& config,
        const ConditionIndex& indices
) : BaseCondition(baas, data, config, indices)
{
    type = ConditionType::BOSS_HEALTH;
    _op = config.health_op;
    _value = config.value;
    _range_min = config.range_min;
    _range_max = config.range_max;
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
            if(data->boss_current_health >= _value) ret = true;
            break;
        case C_BELOW:
            if(data->boss_current_health <= _value) ret = true;
            break;
        case C_IN_RANGE:
            if(data->boss_current_health >= _range_min && data->boss_current_health <= _range_max) ret = true;
            break;
        case C_INCREASE:
            if (_last_recorded_health.has_value() &&
               (data->boss_current_health.value() > _last_recorded_health.value()))
               {
                _health_increment =_health_increment + (data->boss_current_health.value() - _last_recorded_health.value());
                if (_health_increment >= _value) ret = true;
            }
            break;
        case C_DECREASE:
            if (_last_recorded_health.has_value() &&
               (data->boss_current_health.value() < _last_recorded_health.value()))
               {
                _health_increment =_health_increment + (_last_recorded_health.value() - data->boss_current_health.value());
                if (_health_increment >= _value) ret = true;
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
    logger->BAASInfo("Op      : " + std::string(wire_name(_op)));
    switch (_op) {
        case C_IN_RANGE:
            logger->BAASInfo("Range   : [ " + std::to_string(_range_min) + ", " + std::to_string(_range_max) + " ]");
            break;
        case C_OVER:
        case C_BELOW:
        case C_INCREASE:
        case C_DECREASE:
        case M_EQUAL:
            logger->BAASInfo("Value   : " + std::to_string(_value));
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
