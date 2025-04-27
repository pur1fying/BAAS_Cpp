//
// Created by pc on 2025/4/24.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_BOSSHEALTHUPDATER_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_BOSSHEALTHUPDATER_H_

#include "BaseDataUpdater.h"

BAAS_NAMESPACE_BEGIN

class BossHealthUpdater : public BaseDataUpdater {

public:
    explicit BossHealthUpdater(BAAS *baas, screenshot_data *data);

    void update() override;

    double estimated_time_cost() override;

    constexpr std::string data_name() override;

    void display_data() override;

private:
    void _update_all();

    void _update_current_health();

    void _update_max_health();

    BAASRectangle ocr_region, current_ocr_region, max_ocr_region;

    cv::Mat origin_screenshot, cropped_image;

    TextLine ocr_result;
};

BAAS_NAMESPACE_END

#endif //BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_BOSSHEALTHUPDATER_H_
