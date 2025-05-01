//
// Created by pc on 2025/4/29.
//

#include <ocr/BAASOcr.h>

#include "module/auto_fight/screenshot_data/SkillCostUpdater.h"


BAAS_NAMESPACE_BEGIN

SkillCostUpdater::SkillCostUpdater(
        BAAS *baas,
        screenshot_data *data
) : BaseDataUpdater(baas, data)
{
    skill_cost_ocr_region = static_config->get<std::vector<BAASRectangle>>("/BAAS/auto_fight/Skill/slot/cost_ocr/regions");
    if (skill_cost_ocr_region.size() != data->slot_count) {
        logger->BAASInfo("Skill Cost Ocr Region count : [ " + std::to_string(skill_cost_ocr_region.size())
                          + " ] do not match slot count [ " + std::to_string(data->slot_count) + " ]." );
        throw ValueError("Skill Cost Ocr Region count mismatch");
    }
    ocr_model_name = static_config->getString("/BAAS/auto_fight/Skill/slot/cost_ocr/model_name");
    if (!BAASOCR::is_valid_language(ocr_model_name)) {
        logger->BAASInfo("Skill Cost ocr_model_name [ " + ocr_model_name + " ] Invalid.");
        throw ValueError("Invalid ocr language");
    }
    logger->BAASInfo("Skill Cost Ocr Model Name : [ " + ocr_model_name + " ].");
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
    return double(BAASUtil::count_bit(data->skill_cost_update_flag)) * 1.0 * 1e8;
}

void SkillCostUpdater::update()
{
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

        if (filtered_text.empty()) data->skills[i].cost = std::nullopt;
        else                       data->skills[i].cost = std::stoi(filtered_text);
    }
}


BAAS_NAMESPACE_END