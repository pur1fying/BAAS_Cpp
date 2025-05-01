//
// Created by pc on 2025/4/29.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_SKILLCOSTUPDATER_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_SKILLCOSTUPDATER_H_

#include "BaseDataUpdater.h"

BAAS_NAMESPACE_BEGIN

class SkillCostUpdater : public BaseDataUpdater {

public:
    explicit SkillCostUpdater(
            BAAS* baas,
            screenshot_data* data
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

#endif //BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_SKILLCOSTUPDATER_H_
