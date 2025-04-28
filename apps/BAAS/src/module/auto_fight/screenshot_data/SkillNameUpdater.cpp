//
// Created by pc on 2025/4/28.
//

#include "module/auto_fight/screenshot_data/SkillNameUpdater.h"

baas::SkillNameUpdater::SkillNameUpdater(
        baas::BAAS *baas,
        baas::screenshot_data *data
) : BaseDataUpdater(baas, data)
{

}

void baas::SkillNameUpdater::update()
{
    BaseDataUpdater::update();
}

double baas::SkillNameUpdater::estimated_time_cost()
{
    return BaseDataUpdater::estimated_time_cost();
}

constexpr std::string baas::SkillNameUpdater::data_name()
{
    return BaseDataUpdater::data_name();
}

void baas::SkillNameUpdater::display_data()
{
    BaseDataUpdater::display_data();
}
