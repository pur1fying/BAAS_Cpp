//
// Created by pc on 2025/4/24.
//

#include "module/auto_fight/screenshot_data/BaseDataUpdater.h"
#include "module/auto_fight/BAASAutoFight.h"

BAAS_NAMESPACE_BEGIN

BaseDataUpdater::BaseDataUpdater(BAAS *baas, auto_fight_d *data)
{
    this->baas = baas;
    this->data = data;
    this->logger = baas->get_logger();
}

void BaseDataUpdater::update()
{
    throw RuntimeError("BaseDataUpdater class estimated_time_cost should not be called.");
}

double BaseDataUpdater::estimated_time_cost()
{
    throw RuntimeError("BaseDataUpdater class estimated_time_cost should not be called.");
}

constexpr std::string BaseDataUpdater::data_name()
{
    return "BaseData";
}

void BaseDataUpdater::display_data()
{
    throw RuntimeError("BaseDataUpdater class display_screenshot_extracted_data should not be called.");
}

bool BaseDataUpdater::at_fight_page()
{
    if(!baas->feature_appear("fight_pause-button_appear")) return false;
    return true;
}

BAAS_NAMESPACE_END