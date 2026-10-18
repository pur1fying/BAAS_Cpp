#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_CONDITIONFACTORY_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_CONDITIONFACTORY_H_

#include <memory>

#include "module/auto_fight/workflow/ConditionConfig.h"
#include "module/auto_fight/conditions/BaseCondition.h"

BAAS_NAMESPACE_BEGIN

std::unique_ptr<BaseCondition> create_baas_condition(
        BAAS* baas,
        auto_fight_d* data,
        const ConditionConfig& config,
        const ConditionIndex& indices
);

BAAS_NAMESPACE_END

#endif // BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_CONDITIONFACTORY_H_
