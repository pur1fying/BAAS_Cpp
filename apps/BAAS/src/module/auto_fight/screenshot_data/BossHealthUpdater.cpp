//
// Created by pc on 2025/4/26.
//

#include "module/auto_fight/screenshot_data/BossHealthUpdater.h"
#include "ocr/BAASOCR.h"

BAAS_NAMESPACE_BEGIN

BossHealthUpdater::BossHealthUpdater(
        BAAS *baas,
        auto_fight_d *data
) : BaseDataUpdater(baas, data)
{
    if (data->d_fight.contains("/BossHealth/current_ocr_region"))
        current_ocr_region = data->d_fight.get<BAASRectangle>("/BossHealth/current_ocr_region");
    else
        current_ocr_region = static_config->get<BAASRectangle>("/BAAS/auto_fight/BossHealth/current_ocr_region");

    if (data->d_fight.contains("/BossHealth/max_ocr_region"))
        max_ocr_region = data->d_fight.get<BAASRectangle>("/BossHealth/max_ocr_region");
    else
        max_ocr_region = static_config->get<BAASRectangle>("/BAAS/auto_fight/BossHealth/max_ocr_region");

    if (data->d_fight.contains("/BossHealth/ocr_region"))
        ocr_region = data->d_fight.get<BAASRectangle>("/BossHealth/ocr_region");
    else
        ocr_region = static_config->get<BAASRectangle>("/BAAS/auto_fight/BossHealth/ocr_region");

    if (data->d_fight.contains("/BossHealth/ocr_model_name"))
        ocr_model_name = data->d_fight.get<std::string>("/BossHealth/ocr_model_name");
    else
        ocr_model_name = static_config->get<std::string>("/BAAS/auto_fight/BossHealth/ocr_model_name");

    if (!BAASOCR::is_valid_language(ocr_model_name)) {
        logger->BAASInfo("BossHealth ocr_model_name [ " + ocr_model_name + " ] Invalid.");
        throw ValueError("Invalid ocr language");
    }

    logger->BAASInfo("BossHealth Ocr Model Name : [ " + ocr_model_name + " ].");
    baas_ocr->init({ocr_model_name});
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
            ocr_model_name,
            cropped_image,
            ocr_result,
            "c",
            logger,
            "0123456789"
    );
    data->boss_current_health = std::stoll(ocr_result.text);
}

void BossHealthUpdater::_update_max_health()
{
    cropped_image = BAASImageUtil::crop(origin_screenshot, max_ocr_region);
    baas_ocr->ocr_for_single_line(
            ocr_model_name,
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
            ocr_model_name,
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


