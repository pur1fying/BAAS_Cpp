//
// Created by Administrator on 2026/2/17.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_DATA_BATTLETIMEUPDATER_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_DATA_BATTLETIMEUPDATER_H_

#include "BaseDataUpdater.h"

BAAS_NAMESPACE_BEGIN

class BattleTimeUpdater : public BaseDataUpdater {

public:

    explicit BattleTimeUpdater(BAAS* baas, auto_fight_d* data);

    void update() override;

    double estimated_time_cost() override;

    constexpr std::string data_name() override;

    void display_data() override;

private:

    std::optional<int> ocr_result_to_int(int start, int end);

    static const std::vector<std::string> supported_time_formats;

    uint8_t time_format;

    TextLine ocr_result;

    BAASRectangle ocr_region;

    std::string ocr_model_name;

    long long last_recognize_timestamp;

    cv::Mat origin_screenshot, cropped_image;

};

BAAS_NAMESPACE_END

#endif //BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_DATA_BATTLETIMEUPDATER_H_
