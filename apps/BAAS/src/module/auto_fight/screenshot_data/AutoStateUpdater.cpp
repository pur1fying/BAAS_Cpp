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
    std::vector<std::string> feature_names = {
            "fight_auto_off",
            "fight_auto_on",
            "fight_auto_off2on_r_white",
            "fight_auto_off2on_l_white"
    };
    _time_cost = 0.0;
    for (const auto &feature_name : feature_names) {
        if (!BAASFeature::contains(feature_name)) {
            logger->BAASError("AutoStateUpdater Required Feature [ " + feature_name + " ] not exist.");
            throw ValueError("Required Feature [ " + feature_name + " ] not exist." );
        }
        auto_feature_ptrs.push_back(BAASFeature::get_feature_ptr(feature_name));
        _time_cost += auto_feature_ptrs.back()->self_average_cost(baas);
    }
}

void AutoStateUpdater::update()
{
    feature_appear_output.clear();
    for (int i = 0; i < auto_feature_ptrs.size(); ++i) {
        if (auto_feature_ptrs[i]->appear(baas, feature_appear_output)) {
            logger->BAASInfo(auto_feature_ptrs[i]->get_name() + "  matched");
            data->auto_state = bool(i);
            return;
        }
    }
    data->auto_state = std::nullopt;
}

double AutoStateUpdater::estimated_time_cost()
{
    return _time_cost;
}

constexpr std::string AutoStateUpdater::data_name()
{
    return "Auto_State";
}

void AutoStateUpdater::display_data()
{
    if (!data->auto_state.has_value()) logger->BAASInfo("Auto      : [  UnKnown ]");
    else if (data->auto_state.value()) logger->BAASInfo("Auto      : [    On    ]");
    else                               logger->BAASInfo("Auto      : [    OFF   ]");
}


BAAS_NAMESPACE_END