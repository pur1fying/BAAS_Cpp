//
// Created by Administrator on 2025/5/22.
//

#include "module/auto_fight/conditions/OrCombinedCondition.h"

#include <optional>

#include "module/auto_fight/workflow/ConditionConfig.h"

BAAS_NAMESPACE_BEGIN

OrCombinedCondition::OrCombinedCondition(
        BAAS* baas,
        auto_fight_d* data,
        const ConditionConfig& config,
        const ConditionIndex& indices
) : BaseCondition(baas, data, config, indices)
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


