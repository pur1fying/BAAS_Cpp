//
// Created by pc on 2025/4/24.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_DATA_AUTOSTATEUPDATER_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_DATA_AUTOSTATEUPDATER_H_
#include <feature/BAASFeature.h>

#include "BaseDataUpdater.h"

BAAS_NAMESPACE_BEGIN

class AutoStateUpdater : public BaseDataUpdater {

public:

    explicit AutoStateUpdater(BAAS *baas, auto_fight_d *data);

    void update() override;

    double estimated_time_cost() override;

    constexpr std::string data_name() override;

    void display_data() override;

private:

    BAASConfig feature_appear_output;

    double _time_cost;

    std::vector<BaseFeature*> auto_feature_ptrs;

    cv::Mat origin_screenshot;

};


BAAS_NAMESPACE_END


#endif //BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_DATA_AUTOSTATEUPDATER_H_
