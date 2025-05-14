//
// Created by Administrator on 2025/5/15.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_DATA_OBJECTPOSITIONUPDATER_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_DATA_OBJECTPOSITIONUPDATER_H_

#include "BaseDataUpdater.h"

BAAS_NAMESPACE_BEGIN

class ObjectPositionUpdater : public BaseDataUpdater {

public:
    explicit ObjectPositionUpdater(
            BAAS* baas,
            auto_fight_d* data
    );

    void update() override;

    double estimated_time_cost() override;

    constexpr std::string data_name() override;

    void display_data() override;

private:
    std::string ocr_model_name, filtered_text;

    cv::Mat origin_screenshot, cropped_image;

    std::vector<BAASRectangle> skill_cost_ocr_region;

    TextLine ocr_result;

};


BAAS_NAMESPACE_END

#endif //BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_DATA_OBJECTPOSITIONUPDATER_H_
