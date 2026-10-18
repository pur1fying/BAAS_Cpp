#include "module/auto_fight/actions/ActionFactory.h"

#include <memory>
#include <stdexcept>

#include "module/auto_fight/workflow/ActionConfig.h"
#include "module/auto_fight/actions/acc_handler.h"
#include "module/auto_fight/actions/auto_handler.h"
#include "module/auto_fight/actions/restart_handler.h"
#include "module/auto_fight/actions/skill_handler.h"
#include "module/auto_fight/actions/skip_ani_handler.h"

BAAS_NAMESPACE_BEGIN

std::unique_ptr<base_handler> create_baas_action_handler(
        BAAS* baas,
        auto_fight_d* data,
        const ActionStepConfig& config
)
{
    switch (config.type) {
        case ActionType::ACC:
            return std::make_unique<acc_handler>(baas, data, config);
        case ActionType::AUTO:
            return std::make_unique<auto_handler>(baas, data, config);
        case ActionType::RESTART:
            return std::make_unique<restart_handler>(baas, data, config);
        case ActionType::SKIP_ANIMATION:
            return std::make_unique<skip_ani_handler>(baas, data, config);
        case ActionType::SKILL:
            return std::make_unique<skill_handler>(baas, data, config);
    }
    throw std::logic_error("Invalid parsed action type");
}

BAAS_NAMESPACE_END
