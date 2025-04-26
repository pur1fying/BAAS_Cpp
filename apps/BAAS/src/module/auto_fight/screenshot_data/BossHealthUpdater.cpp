//
// Created by pc on 2025/4/26.
//

#include "module/auto_fight/screenshot_data/BossHealthUpdater.h"

BAAS_NAMESPACE_BEGIN

BossHealthUpdater::BossHealthUpdater(
        BAAS *baas,
        screenshot_data *data
) : BaseDataUpdater(baas, data)
{
    ocr_region = static_config->get_rect("/BAAS/auto_fight/BossHealth/ocr_region");
}

void BossHealthUpdater::update()
{
    baas->get_latest_screenshot(origin_screenshot);
    cropped_image = BAASImageUtil::crop(origin_screenshot, ocr_region);

    baas_ocr->ocr_for_single_line(
            "zh-cn",
            cropped_image,
            ocr_result,
            "BossHealth",
            logger,
            "0123456789/"
    );
    ocr_result_to_boss_health();
}

double BossHealthUpdater::estimated_time_cost()
{
    return 1e8;
}

void BossHealthUpdater::display_data()
{
    if(!data->boss_current_health.has_value()) logger->BAASInfo("Boss_C_Health: No Value.");
    else logger->BAASInfo("Boss_C_Health : [ " + std::to_string(data->boss_current_health.value()) + " ]");

    if(!data->boss_max_health.has_value()) logger->BAASInfo("Boss_M_Health: No Value.");
    else logger->BAASInfo("Boss_M_Health : [ " + std::to_string(data->boss_max_health.value()) + " ]");
}

constexpr std::string BossHealthUpdater::data_name()
{
    return "BossHealth";
}

void BossHealthUpdater::ocr_result_to_boss_health()
{
    size_t pos = ocr_result.text.find('/');
    if (pos != std::string::npos) {
        std::string first_part = ocr_result.text.substr(0, pos);
        std::string second_part = ocr_result.text.substr(pos + 1);

        data->boss_current_health = std::stoll(first_part);
        data->boss_max_health  = std::stoll(second_part);

    } else {
        data->boss_current_health = std::nullopt;
        data->boss_max_health = std::nullopt;
    }
}

BAAS_NAMESPACE_END


