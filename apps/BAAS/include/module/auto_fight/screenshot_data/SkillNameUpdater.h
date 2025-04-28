//
// Created by pc on 2025/4/28.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_SKILLNAMEUPDATER_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_SKILLNAMEUPDATER_H_

#include "BaseDataUpdater.h"

BAAS_NAMESPACE_BEGIN

class SkillNameUpdater : public BaseDataUpdater {
public:
    explicit SkillNameUpdater(BAAS *baas, screenshot_data *data);

    void update() override;

    double estimated_time_cost() override;

    constexpr std::string data_name() override;

    void display_data() override;

private:
    cv::Mat screenshot_cropped_template_img;
};

BAAS_NAMESPACE_END

#endif //BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_SKILLNAMEUPDATER_H_
