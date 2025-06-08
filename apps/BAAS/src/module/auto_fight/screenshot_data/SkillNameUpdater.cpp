//
// Created by pc on 2025/4/28.
//

#include "module/auto_fight/screenshot_data/SkillNameUpdater.h"

#include <config/BAASStaticConfig.h>

BAAS_NAMESPACE_BEGIN

SkillNameUpdater::SkillNameUpdater(
        BAAS *baas,
        auto_fight_d *data
) : BaseDataUpdater(baas, data)
{
    each_skill_match_template_region = static_config->get<std::vector<BAASRectangle>>(
            "/BAAS/auto_fight/Skill/slot/match_template_regions"
    );
    if (each_skill_match_template_region.size() != data->each_slot_possible_templates.size()) {
        logger->BAASError("SkillNameUpdater: each_skill_match_template_region size not match");
        return;
    }
    _threshold = static_config->getDouble("/BAAS/auto_fight/Skill/threshold");
    _rgb_diff = static_config->getUInt("/BAAS/auto_fight/Skill/rgb_diff");
    _init_skill_estimated_match_template_cost();
}

void SkillNameUpdater::update()
{
    appeared_skill_idx.clear();
    for (const auto& skill: data->skills)
        if (skill.index.has_value())
            appeared_skill_idx.insert(skill.index.value()); // insert existing skill into appeared

    baas->get_latest_screenshot(origin_screenshot);
    for (int i = 0; i < data->each_slot_possible_templates.size(); i++) {
        if (data->skills[i].index.has_value()) continue;    // skip slot with skill
        screenshot_crop_img = BAASImageUtil::crop(origin_screenshot, each_skill_match_template_region[i]);
        _find = false;
        for (auto& _tmp_idx : data->each_slot_possible_templates[i]) {
            if (appeared_skill_idx.contains(_tmp_idx)) continue;

            for (auto& _active_template : data->all_possible_skills[_tmp_idx].skill_active_templates) {
                if(_template_appear(_active_template)) {
                    appeared_skill_idx.insert(_tmp_idx);
                    data->skills[i].index = _tmp_idx;
                    data->skills[i].is_active = true;
                    _find = true;
                    break;
                }
            }
            if (_find) {
                for (int j = 0 ; j < data->slot_count; ++j) {
                    if (data->skill_last_detect[j].index.has_value()
                     && data->skill_last_detect[j].index.value() == _tmp_idx) {
                        data->skill_last_detect[j].reset();
                        break;
                    }
                }
                data->skill_last_detect[i].index = _tmp_idx;
                data->skill_last_detect[i].is_active = true;
                break;
            }

            for (auto& _inactive_template : data->all_possible_skills[_tmp_idx].skill_inactive_templates) {
                if(_template_appear(_inactive_template)) {
                    appeared_skill_idx.insert(_tmp_idx);
                    data->skills[i].index = _tmp_idx;
                    data->skills[i].is_active = false;
                    _find = true;
                    break;
                }
            }

            if (_find) {
                for (int j = 0 ; j < data->slot_count; ++j) {
                    if (data->skill_last_detect[j].index.has_value()
                     && data->skill_last_detect[j].index.value() == _tmp_idx) {
                        data->skill_last_detect[j].reset();
                        break;
                    }
                }
                data->skill_last_detect[i].index = _tmp_idx;
                data->skill_last_detect[i].is_active = false;
                break;
            }
        }
    }
}

double SkillNameUpdater::estimated_time_cost()
{
    double cost = 0.0;
    size_t tmp_cnt = 0;
    double d_tmp_cnt;
    for (int i = 0; i < data->each_slot_possible_templates.size(); i++) {
        for (auto& _template : data->each_slot_possible_templates[i]) {
            tmp_cnt = data->all_possible_skills[_template].skill_active_templates.size() +
                      data->all_possible_skills[_template].skill_inactive_templates.size();
        }
        d_tmp_cnt = double(tmp_cnt);
        for (int j = 0; j < data->each_slot_possible_templates[i].size(); j++) {
            for (auto _template : data->all_possible_skills[data->each_slot_possible_templates[i][j]].skill_active_templates)
                cost += _template.estimated_match_template_time_cost[i] * (d_tmp_cnt - j) / d_tmp_cnt;
            for (auto _template : data->all_possible_skills[data->each_slot_possible_templates[i][j]].skill_inactive_templates)
                cost += _template.estimated_match_template_time_cost[i] * (d_tmp_cnt - j) / d_tmp_cnt;
        }
    }
    return cost;
}

constexpr std::string SkillNameUpdater::data_name()
{
    return "SkillName";
}

void SkillNameUpdater::display_data()
{
    logger->sub_title("Slot Skill Name State");
    for (auto & skill : data->skills) {
        if (!skill.index.has_value()) {
            logger->BAASInfo("[ No Value ]");
            continue;
        }
        if (skill.is_active.value())
            logger->BAASInfo("[  Active  ] " + data->all_possible_skills[skill.index.value()].name);
        else
            logger->BAASInfo("[ InActive ] " + data->all_possible_skills[skill.index.value()].name);
    }
}

void SkillNameUpdater::_init_skill_estimated_match_template_cost()
{
    for (auto&_skill : data->all_possible_skills) {
        for(auto& _template : _skill.skill_active_templates) {
            _template.estimated_match_template_time_cost.resize(data->slot_count);
            for (int i = 0; i < data->slot_count; i++)
                _template.estimated_match_template_time_cost[i] = _calc_single_match_template_cost(
                        _template.template_image,
                        each_skill_match_template_region[i]
                );
        }

        for(auto& _template : _skill.skill_inactive_templates) {
            _template.estimated_match_template_time_cost.resize(data->slot_count);
            for (int i = 0; i < data->slot_count; i++)
                _template.estimated_match_template_time_cost[i] = _calc_single_match_template_cost(
                        _template.template_image,
                        each_skill_match_template_region[i]
                );
        }
    }
}

// search _tmp in screenshot_crop_img

bool SkillNameUpdater::_template_appear(const template_info& _tmp)
{
    cv::matchTemplate(
            screenshot_crop_img,
            _tmp.template_image,
            match_result,
            cv::TM_CCOEFF_NORMED
    );

    cv::minMaxLoc(
            match_result,
            &_minVal,
            &_maxVal,
            &_minLoc,
            &_maxLoc
    );
    if (_maxVal < _threshold) return false;

    // check mean rgb
    matched_region_roi = cv::Rect(_maxLoc.x, _maxLoc.y, _tmp.template_image.cols, _tmp.template_image.rows);
    cv::Vec3b matched_region_mean_rgb = BAASImageUtil::get_region_mean_rgb(screenshot_crop_img, matched_region_roi);
    if(abs(matched_region_mean_rgb[0] - _tmp.mean_rgb[0]) > _rgb_diff ||
       abs(matched_region_mean_rgb[1] - _tmp.mean_rgb[1]) > _rgb_diff ||
       abs(matched_region_mean_rgb[2] - _tmp.mean_rgb[2]) > _rgb_diff  )  return false;

    return true;
}

BAAS_NAMESPACE_END