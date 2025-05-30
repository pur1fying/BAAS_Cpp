//
// Created by pc on 2025/2/24.
//
#include "ISA.h"

#include "module/competition/Competition.h"
#include "module/work/Work.h"
#include "module/collect_activity_fee/CollectActivityFee.h"
#include "module/restart/Restart.h"
#include "module/collect_reward/CollectReward.h"
#include "module/mail/Mail.h"

BAAS_NAMESPACE_BEGIN

std::map<std::string, bool (*)(BAAS *)> ISA::implement_funcs;

bool ISA::solve(const std::string &task)
{
    auto it = implement_funcs.find(task);
    if (it == implement_funcs.end()) {
        logger->BAASError("Task implement not found : [ " + task + " ]");
        return false;
    }
    try {
        logger->hr(task);
        return it->second(this->baas);
    } catch (std::exception &e) {
        logger->BAASError("Error in solve task: [ " + task + " ] " + e.what());
        return false;
    }
}

void ISA::init_implement_funcs()
{
    implement_funcs["competition"] = Competition::implement;
    implement_funcs["work"] = Work::implement;
    implement_funcs["collect_activity_fee"] = CollectActivityFee::implement;
    implement_funcs["restart"] = Restart::implement;
    implement_funcs["collect_reward"] = CollectReward::implement;
    implement_funcs["mail"] = Mail::implement;
}

ISA::ISA(BAAS *baas)
{
    this->baas = baas;
    this->logger = baas->get_logger();
}

BAAS_NAMESPACE_END
