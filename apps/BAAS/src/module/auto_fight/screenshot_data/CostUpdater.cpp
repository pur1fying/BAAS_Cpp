//
// Created by pc on 2025/4/24.
//

#include "module/auto_fight/screenshot_data/CostUpdater.h"

BAAS_NAMESPACE_BEGIN

CostUpdater::CostUpdater(BAAS *baas, screenshot_data *data) : BaseDataUpdater(baas, data)
{

}

bool CostUpdater::update()
{
//    instance->
    return true;
}

double CostUpdater::estimated_time_cost()
{
    return BaseDataUpdater::estimated_time_cost();
}

std::string CostUpdater::data_name()
{
    return "Cost";
}

BAAS_NAMESPACE_END