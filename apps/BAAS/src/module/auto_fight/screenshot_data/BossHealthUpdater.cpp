//
// Created by pc on 2025/4/26.
//

#include "module/auto_fight/screenshot_data/BossHealthUpdater.h"
#include "ocr/BAASOCR.h"

BAAS_NAMESPACE_BEGIN

BossHealthUpdater::BossHealthUpdater(
        BAAS *baas,
        screenshot_data *data
) : BaseDataUpdater(baas, data)
{
    ocr_region = static_config->get_rect("/BAAS/auto_fight/BossHealth/ocr_region");
    current_ocr_region = static_config->get_rect("/BAAS/auto_fight/BossHealth/current_ocr_region");
    max_ocr_region = static_config->get_rect("/BAAS/auto_fight/BossHealth/max_ocr_region");
}

void BossHealthUpdater::update()
{
    baas->get_latest_screenshot(origin_screenshot);

    if (this->data->boss_health_update_flag & 0b100) {
        _update_all();
        return;
    }

    // current
    if (this->data->boss_health_update_flag & 0b10) _update_current_health();

    // max
    if (this->data->boss_health_update_flag & 0b01) _update_max_health();

}

double BossHealthUpdater::estimated_time_cost()
{
    return 1e8;
}

void BossHealthUpdater::display_data()
{
    if(!data->boss_current_health.has_value()) logger->BAASInfo("Boss_C_Health : No Value.");
    else logger->BAASInfo("Boss_C_Health : [ " + std::to_string(data->boss_current_health.value()) + " ]");

    if(!data->boss_max_health.has_value()) logger->BAASInfo("Boss_M_Health : No Value.");
    else logger->BAASInfo("Boss_M_Health : [ " + std::to_string(data->boss_max_health.value()) + " ]");
}

constexpr std::string BossHealthUpdater::data_name()
{
    return "BossHealth";
}

void BossHealthUpdater::_update_current_health()
{
    cropped_image = BAASImageUtil::crop(origin_screenshot, current_ocr_region);
    baas_ocr->ocr_for_single_line(
            "zh-cn",
            cropped_image,
            ocr_result,
            "",
            nullptr,
            "0123456789"
    );
    data->boss_current_health = std::stoll(ocr_result.text);
}

void BossHealthUpdater::_update_max_health()
{
    cropped_image = BAASImageUtil::crop(origin_screenshot, max_ocr_region);
    baas_ocr->ocr_for_single_line(
            "zh-cn",
            cropped_image,
            ocr_result,
            "",
            nullptr,
            "0123456789"
    );
    data->boss_max_health = std::stoll(ocr_result.text);
}

void BossHealthUpdater::_update_all()
{
    cropped_image = BAASImageUtil::crop(origin_screenshot, ocr_region);
    baas_ocr->ocr_for_single_line(
            "zh-cn",
            cropped_image,
            ocr_result,
            "",
            nullptr,
            "0123456789/"
    );
    std::string text = ocr_result.text;
    auto p = text.find('/');
    if (p == std::string::npos) {
        data->boss_current_health = std::nullopt;
        data->boss_max_health = std::nullopt;
    } else {
        data->boss_current_health = std::stoll(text.substr(0, p));
        data->boss_max_health = std::stoll(text.substr(p + 1));
    }
}

BAAS_NAMESPACE_END


