#include "module/auto_fight/conditions/ConditionFactory.h"

#include <memory>
#include <stdexcept>

#include "module/auto_fight/workflow/ConditionConfig.h"
#include "module/auto_fight/conditions/AndCombinedCondition.h"
#include "module/auto_fight/conditions/BattleTimeCondition.h"
#include "module/auto_fight/conditions/BossHealthCondition.h"
#include "module/auto_fight/conditions/CostCondition.h"
#include "module/auto_fight/conditions/OrCombinedCondition.h"
#include "module/auto_fight/conditions/SkillNameCondition.h"

BAAS_NAMESPACE_BEGIN

std::unique_ptr<BaseCondition> create_baas_condition(
        BAAS* baas,
        auto_fight_d* data,
        const ConditionConfig& config,
        const ConditionIndex& indices
)
{
    switch (config.type) {
        case ConditionType::AND_COMBINED:
            return std::make_unique<AndCombinedCondition>(baas, data, config, indices);
        case ConditionType::OR_COMBINED:
            return std::make_unique<OrCombinedCondition>(baas, data, config, indices);
        case ConditionType::COST:
            return std::make_unique<CostCondition>(baas, data, config, indices);
        case ConditionType::SKILL_NAME:
            return std::make_unique<SkillNameCondition>(baas, data, config, indices);
        case ConditionType::BOSS_HEALTH:
            return std::make_unique<BossHealthCondition>(baas, data, config, indices);
        case ConditionType::BATTLE_TIME:
            return std::make_unique<BattleTimeCondition>(baas, data, config, indices);
    }
    throw std::logic_error("Invalid parsed condition type");
}

BAAS_NAMESPACE_END
