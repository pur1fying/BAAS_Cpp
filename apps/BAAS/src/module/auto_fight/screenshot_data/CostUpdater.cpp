//
// Created by pc on 2025/4/24.
//

#include "module/auto_fight/screenshot_data/CostUpdater.h"

#include <utils/BAASImageUtil.h>
#include <config/BAASStaticConfig.h>

BAAS_NAMESPACE_BEGIN

CostUpdater::CostUpdater(
        BAAS *baas,
        auto_fight_d *data
) : BaseDataUpdater(baas, data)
{
    _init_static_value();
}

void CostUpdater::update()
{
    current_cost = 0;
    baas->get_latest_screenshot(origin_screenshot);
    for(int i = cost_recognize_region.lr.x; i >= cost_recognize_region.ul.x; --i){
        for(int j = cost_recognize_region.ul.y; j <= cost_recognize_region.lr.y; ++j){
            if(BAASImageUtil::judge_rgb_range(origin_screenshot, {i, j}, cost_pixel_min_rgb, cost_pixel_max_rgb)){
                int x = i - (cost_recognize_region.ul.x + (cost_recognize_region.lr.y - j) / 5);
                _t_x = double(x);
                int integer = int(_t_x / cost_increase_1_dealt_x);
                double decimal = (_t_x - double(integer) * cost_increase_1_dealt_x) / 28;
                if(decimal < 0) decimal = 0;
                current_cost = integer + decimal;
                current_cost = current_cost > 10.0 ? 10.0 : current_cost;
                data->cost = current_cost;
                return;
            }
        }
    }
}

double CostUpdater::estimated_time_cost()
{
    return average_cost;
}

constexpr std::string CostUpdater::data_name()
{
    return "Cost";
}

void CostUpdater::_init_static_value()
{
    cost_increase_1_dealt_x = static_config->getDouble("/BAAS/auto_fight/cost/increase_1_dealt_x");
    cost_pixel_min_rgb = static_config->getVec3b("/BAAS/auto_fight/cost/target_pixel_rgb/min");
    cost_pixel_max_rgb = static_config->getVec3b("/BAAS/auto_fight/cost/target_pixel_rgb/max");
    cost_recognize_region = static_config->get_rect("/BAAS/auto_fight/cost/recognize_region");

    average_cost = 1.0 * double(cost_recognize_region.size()) * 0.5 * 6;
}

void CostUpdater::display_data()
{
    if(!data->cost.has_value()) logger->BAASInfo("Cost: No Value.");
    else logger->BAASInfo("Cost : [ " + std::to_string(data->cost.value()) + " ]");
}

BAAS_NAMESPACE_END