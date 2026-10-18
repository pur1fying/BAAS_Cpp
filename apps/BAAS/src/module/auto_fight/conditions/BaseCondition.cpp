//
// Created by Administrator on 2025/5/6.
//

#include "module/auto_fight/conditions/BaseCondition.h"

#include <optional>
#include <string>

#include "module/auto_fight/workflow/ConditionConfig.h"
#include "module/auto_fight/workflow/WorkflowNames.h"

BAAS_NAMESPACE_BEGIN

BaseCondition::BaseCondition(
        BAAS* baas,
        auto_fight_d* data,
        const ConditionConfig& config,
        const ConditionIndex& indices
)
{
    this->data = data;
    this->timeout = config.timeout;
    this->baas = baas;
    this->logger = baas->get_logger();
    this->desc = config.desc;
    this->type = config.type;
    for (const auto& name : config.and_conditions) {
        and_conditions.push_back(indices.at(name));
    }
    for (const auto& name : config.or_conditions) {
        or_conditions.push_back(indices.at(name));
    }
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

void BaseCondition::display() const noexcept
{
    throw RuntimeError("BaseCondition class display should not be called.");
}

void BaseCondition::set_d_update_flag()
{
    throw RuntimeError("BaseCondition class set_d_update_flag should not be called.");
}

void BaseCondition::_display_basic_info() const noexcept
{
    logger->BAASInfo("Type    : " + std::string(wire_name(type)));
    logger->BAASInfo("Timeout : " + std::to_string(timeout));
    if (has_and_cond())logger->BAASInfo("A C Cnt : " + std::to_string(and_conditions.size()));
    if (has_or_cond()) logger->BAASInfo("O C Cnt : " + std::to_string(or_conditions.size()));
    if(!desc.empty())logger->BAASInfo("Desc    : " + desc);
}


BAAS_NAMESPACE_END
