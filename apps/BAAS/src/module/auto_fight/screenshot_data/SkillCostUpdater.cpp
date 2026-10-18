//
// Created by pc on 2025/4/29.
//

#include "module/auto_fight/screenshot_data/SkillCostUpdater.h"

#include <string>
#include <vector>

#include "ocr/BAASOcr.h"
#include "utils/BAASImageUtil.h"

BAAS_NAMESPACE_BEGIN

SkillCostUpdater::SkillCostUpdater(
        BAAS* baas,
        auto_fight_d* data,
        const std::vector<OcrRegion>& regions,
        const std::string& model
) : BaseDataUpdater(baas, data)
{
    for (const auto& region : regions) {
        skill_cost_ocr_region.emplace_back(region[0], region[1], region[2], region[3]);
    }
    ocr_model_name = model;
    baas_ocr->init({ocr_model_name});
}

void SkillCostUpdater::display_data()
{
    logger->sub_title("Slot Skill Cost State");
    for (const auto& skill : data->skills)
        if (!skill.cost.has_value())    logger->BAASInfo("[ No Value ]");
        else                            logger->BAASInfo("Cost : [ " + std::to_string(skill.cost.value()) + " ]");
}

constexpr std::string SkillCostUpdater::data_name()
{
    return "SkillCost";
}

double SkillCostUpdater::estimated_time_cost()
{
    return double(BAASStringUtil::count_bit(data->skill_cost_update_flag)) * 1.0 * 1e8;
}

void SkillCostUpdater::update()
{
    result.clear();
    baas->get_latest_screenshot(origin_screenshot);
    for (int i = 0; i < skill_cost_ocr_region.size(); ++i) {
        if (!(data->skill_cost_update_flag & (1 << i))) continue;

        cropped_image = BAASImageUtil::crop(origin_screenshot, skill_cost_ocr_region[i]);
        baas_ocr->ocr_for_single_line(
                ocr_model_name,
                cropped_image,
                ocr_result,
                "",
                nullptr,
                "0123456789"
        );
        filtered_text.clear();

        // filter low score char
        for (int j = 0 ; j < ocr_result.text.size(); ++j) {
            if (ocr_result.charScores[j] < 0.3) continue;
            filtered_text.push_back(ocr_result.text[j]);
        }

        if (filtered_text.empty()) result.push_back({i, std::nullopt});
        else result.push_back({i, std::stoi(filtered_text)});
    }
}

void SkillCostUpdater::write_result_into_data()
{
    for (const auto& [slot_idx, cost] : result) {
        if (cost.has_value()) {
            data->skills[slot_idx].cost = cost.value();
            data->skill_last_detect[slot_idx].cost = cost.value();
        }
        else data->skills[slot_idx].cost = std::nullopt;
    }
}


BAAS_NAMESPACE_END
