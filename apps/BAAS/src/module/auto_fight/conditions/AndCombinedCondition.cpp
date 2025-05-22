//
// Created by Administrator on 2025/5/22.
//

#include "module/auto_fight/conditions/AndCombinedCondition.h"

BAAS_NAMESPACE_BEGIN

AndCombinedCondition::AndCombinedCondition(
        BAAS* baas,
        auto_fight_d* data,
        const BAASConfig& config
) : BaseCondition(baas, data, config)
{
    type = A_COMBINED;
}

void AndCombinedCondition::reset_state()
{

}

void AndCombinedCondition::display() const noexcept
{
    _display_basic_info();
}

void AndCombinedCondition::set_d_update_flag()
{

}

std::optional<bool> AndCombinedCondition::try_match()
{
    return true;
}

BAAS_NAMESPACE_END


