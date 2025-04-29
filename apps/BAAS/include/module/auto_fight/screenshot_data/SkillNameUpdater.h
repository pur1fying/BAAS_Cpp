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

    bool _template_appear(const template_info& _tmp);

    void _init_skill_estimated_match_template_cost();

    inline static double _calc_single_match_template_cost(const cv::Mat& _tmp, const BAASRectangle& region) {
        int _rows = region.height(), _cols = region.width();
        return double(_tmp.cols * _tmp.rows * (_cols - _tmp.cols + 1) * (_rows - _tmp.rows + 1));
    }

    bool _find{};

    double _threshold, _minVal{}, _maxVal{};

    cv::Rect matched_region_roi;

    cv::Point _minLoc, _maxLoc;

    uint8_t _rgb_diff;

    // each skill appear one time
    std::set<int> appeared_skill_idx;

    std::vector<BAASRectangle> each_skill_match_template_region;

    cv::Mat origin_screenshot, screenshot_crop_img;

    cv::Mat match_result;
};

BAAS_NAMESPACE_END

#endif //BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_SKILLNAMEUPDATER_H_
