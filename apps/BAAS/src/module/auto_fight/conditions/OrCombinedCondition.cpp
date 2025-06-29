//
// Created by Administrator on 2025/5/22.
//

#include "module/auto_fight/conditions/OrCombinedCondition.h"

BAAS_NAMESPACE_BEGIN

OrCombinedCondition::OrCombinedCondition(
        BAAS* baas, 
        auto_fight_d* data,
const BAASConfig& config
) : BaseCondition(baas, data, config)
{
    type = O_COMBINED;
}

void OrCombinedCondition::reset_state()
{

}

void OrCombinedCondition::display() const noexcept
{
    _display_basic_info();
}

void OrCombinedCondition::set_d_update_flag()
{

}

std::optional<bool> OrCombinedCondition::try_match()
{
    return false;
}

BAAS_NAMESPACE_END


