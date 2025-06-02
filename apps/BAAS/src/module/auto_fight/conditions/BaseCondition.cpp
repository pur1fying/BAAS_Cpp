//
// Created by Administrator on 2025/5/6.
//

#include "module/auto_fight/conditions/BaseCondition.h"

BAAS_NAMESPACE_BEGIN

BaseCondition::BaseCondition(BAAS* baas, auto_fight_d* data, const BAASConfig& config)
{
    this->config = BAASConfig(config.get_config(), baas->get_logger());
    this->data = data;
    this->timeout = config.getLLong("timeout", BAAS_AUTO_FIGHT_CONDITION_DEFAULT_TIMEOUT);
    this->baas = baas;
    this->logger = baas->get_logger();
    this->desc = config.getString("desc", "");
}

BaseCondition::BaseCondition(BAAS* baas, auto_fight_d* data, long long _timeout)
{
    this->data = data;
    this->timeout = _timeout;
    this->baas = baas;
    this->logger = baas->get_logger();
    this->desc = "";
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
    logger->BAASInfo("Type    : [ " + cond_type_st_list[type] + " ]");
    logger->BAASInfo("Timeout : [ " + std::to_string(timeout) + " ]");
    if (has_and_cond())logger->BAASInfo("A C Cnt : [ " + std::to_string(and_conditions.size()) + " ]");
    if (has_or_cond()) logger->BAASInfo("O C Cnt : [ " + std::to_string(or_conditions.size()) + " ]");
    if(!desc.empty())logger->BAASInfo("Desc    : " + desc);
}


BAAS_NAMESPACE_END
