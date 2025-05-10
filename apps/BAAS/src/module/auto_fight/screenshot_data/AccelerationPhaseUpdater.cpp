//
// Created by pc on 2025/4/26.
//


#include "module/auto_fight/screenshot_data/AccelerationPhaseUpdater.h"

BAAS_NAMESPACE_BEGIN

AccelerationPhaseUpdater::AccelerationPhaseUpdater(
        BAAS *baas,
        auto_fight_d *data
) : BaseDataUpdater(baas, data)
{
    acc_feature_ptrs.push_back(nullptr);

    _time_cost = 0.0;
    std::string feature_name;
    for (int i = 1; i <= 3; ++i) {
        feature_name = "fight_acc_phase" + std::to_string(i);
        if (!BAASFeature::contains(feature_name)) {
            logger->BAASError("AccelerationPhaseUpdater Required Feature [ " + feature_name + " ] not exist.");
            throw ValueError("Required Feature [ " + feature_name + " ] not exist." );
        }
        acc_feature_ptrs.push_back(BAASFeature::get_feature_ptr(feature_name));
        _time_cost += acc_feature_ptrs.back()->self_average_cost(baas);
    }
}

void AccelerationPhaseUpdater::update()
{
    for (uint8_t i = 1; i <= 3; ++i) {
        feature_appear_output.clear();
        if (acc_feature_ptrs[i]->appear(baas, feature_appear_output)) {
            data->acceleration_state = i;
            return;
        }
    }
    data->acceleration_state = std::nullopt;
}

double AccelerationPhaseUpdater::estimated_time_cost()
{
    return _time_cost;
}

constexpr std::string AccelerationPhaseUpdater::data_name()
{
    return "Acc_State";
}

void AccelerationPhaseUpdater::display_data()
{
    if (!data->acceleration_state.has_value()) logger->BAASInfo("Acc_State : [  Unknown ]");
    else  logger->BAASInfo("Acc_State : [    " + std::string(1, '0' + data->acceleration_state.value()) + "     ]");
}



BAAS_NAMESPACE_END