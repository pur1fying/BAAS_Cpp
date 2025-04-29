//
// Created by pc on 2025/4/29.
//

#include "module/auto_fight/screenshot_data/SkillCostUpdater.h"

BAAS_NAMESPACE_BEGIN



BAAS_NAMESPACE_END

void baas::SkillCostUpdater::display_data()
{
    BaseDataUpdater::display_data();
}

constexpr std::string baas::SkillCostUpdater::data_name()
{
    return BaseDataUpdater::data_name();
}

double baas::SkillCostUpdater::estimated_time_cost()
{
    return BaseDataUpdater::estimated_time_cost();
}

void baas::SkillCostUpdater::update()
{
    BaseDataUpdater::update();
}

baas::SkillCostUpdater::SkillCostUpdater(
        baas::BAAS *baas,
        baas::screenshot_data *data
) : BaseDataUpdater(baas, data)
{

}
