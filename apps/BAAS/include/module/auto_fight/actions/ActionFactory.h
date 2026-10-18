#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_ACTIONS_ACTIONFACTORY_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_ACTIONS_ACTIONFACTORY_H_

#include <memory>

#include "module/auto_fight/workflow/ActionConfig.h"
#include "module/auto_fight/actions/base_handler.h"

BAAS_NAMESPACE_BEGIN

std::unique_ptr<base_handler> create_baas_action_handler(
        BAAS* baas,
        auto_fight_d* data,
        const ActionStepConfig& config
);

BAAS_NAMESPACE_END

#endif // BAAS_CPP_MODULE_AUTO_FIGHT_ACTIONS_ACTIONFACTORY_H_
