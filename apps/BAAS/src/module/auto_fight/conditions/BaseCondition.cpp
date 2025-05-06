//
// Created by Administrator on 2025/5/6.
//

#include "module/auto_fight/conditions/BaseCondition.h"

BAAS_NAMESPACE_BEGIN

const std::map<std::string, BaseCondition::ConditionType> BaseCondition::condition_type_map = {
        {"combined", COMBINED},
        {"cost", COST},
        {"skill_name", SKILL_NAME},
        {"skill_cost", SKILL_COST},
        {"acc_phase", ACC_PHASE},
        {"auto_state", AUTO_STATE},
        {"boss_health", BOSS_HEALTH}
};

BaseCondition::BaseCondition(BAAS *baas, screenshot_data *data, const BAASConfig &config)
{
    this->config = BAASConfig(config.get_config(), baas->get_logger());
    this->data = data;
}

BaseCondition::~BaseCondition()
{

}

std::optional<bool> BaseCondition::try_match()
{
    throw RuntimeError("BaseCondition class try_match should not be called.");

}

void BaseCondition::reset_state()
{
    throw RuntimeError("BaseCondition class reset_state should not be called.");
}

void BaseCondition::display()
{
    throw RuntimeError("BaseCondition class display should not be called.");
}


BAAS_NAMESPACE_END
