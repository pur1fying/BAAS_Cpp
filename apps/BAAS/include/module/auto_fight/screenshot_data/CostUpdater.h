//
// Created by pc on 2025/4/24.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_DATA_COSTUPDATER_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_DATA_COSTUPDATER_H_

#include "BaseDataUpdater.h"

BAAS_NAMESPACE_BEGIN

class CostUpdater : public BaseDataUpdater {

public:
    explicit CostUpdater(BAAS *baas, screenshot_data *data);

    void update() override;

    double estimated_time_cost() override;

    constexpr std::string data_name() override;

    void display_data() override;

private:
    void _init_static_value();

    // static value
    cv::Vec3b  cost_pixel_min_rgb, cost_pixel_max_rgb;
    BAASRectangle cost_recognize_region;
    double cost_increase_1_dealt_x ;
    double average_cost;

    // change in runtime
    double current_cost;
    cv::Mat screenshot;
};

BAAS_NAMESPACE_END

#endif //BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_DATA_COSTUPDATER_H_
