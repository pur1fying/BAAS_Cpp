//
// Created by Administrator on 2025/5/22.
//

#include "module/auto_fight/conditions/AndCombinedCondition.h"

#include <optional>

#include "module/auto_fight/workflow/ConditionConfig.h"

BAAS_NAMESPACE_BEGIN

AndCombinedCondition::AndCombinedCondition(
        BAAS* baas,
        auto_fight_d* data,
        const ConditionConfig& config,
        const ConditionIndex& indices
) : BaseCondition(baas, data, config, indices)
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


