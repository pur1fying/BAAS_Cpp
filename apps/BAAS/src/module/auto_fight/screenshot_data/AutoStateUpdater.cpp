//
// Created by pc on 2025/4/26.
//

#include "module/auto_fight/screenshot_data/AutoStateUpdater.h"


BAAS_NAMESPACE_BEGIN

AutoStateUpdater::AutoStateUpdater(
        BAAS *baas,
        screenshot_data *data
) : BaseDataUpdater(baas, data)
{

}

void AutoStateUpdater::update()
{
    BaseDataUpdater::update();
}

double AutoStateUpdater::estimated_time_cost()
{
    return BaseDataUpdater::estimated_time_cost();
}

constexpr std::string AutoStateUpdater::data_name()
{
    return "Auto_phase";
}

void AutoStateUpdater::display_data()
{
    BaseDataUpdater::display_data();
}


BAAS_NAMESPACE_END