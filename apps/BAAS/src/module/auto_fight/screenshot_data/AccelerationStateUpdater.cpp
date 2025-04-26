//
// Created by pc on 2025/4/26.
//

#include "module/auto_fight/screenshot_data/AccelerationStateUpdater.h"

BAAS_NAMESPACE_BEGIN

AccelerationPhaseUpdater::AccelerationPhaseUpdater(
        BAAS *baas,
        screenshot_data *data
) : BaseDataUpdater(baas, data)
{

}

void AccelerationPhaseUpdater::update()
{


}

double AccelerationPhaseUpdater::estimated_time_cost()
{
    return BaseDataUpdater::estimated_time_cost();
}

constexpr std::string AccelerationPhaseUpdater::data_name()
{
    return "Acc_Phase";
}

void AccelerationPhaseUpdater::display_data()
{
    BaseDataUpdater::display_data();
}



BAAS_NAMESPACE_END