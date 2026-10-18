//
// Created by pc on 2025/4/26.
//

#include "module/auto_fight/screenshot_data/BossHealthUpdater.h"

#include <string>

#include "config/BAASStaticConfig.h"
#include "ocr/BAASOCR.h"
#include "utils/BAASImageUtil.h"

BAAS_NAMESPACE_BEGIN

BossHealthUpdater::BossHealthUpdater(
        BAAS* baas,
        auto_fight_d* data,
        const RecognitionConfig& config
) : BaseDataUpdater(baas, data)
{
    ocr_model_name = config.model;
    ocr_region = BAASRectangle(config.region[0], config.region[1], config.region[2], config.region[3]);
    current_ocr_region = BAASRectangle(config.current_region[0], config.current_region[1],
            config.current_region[2], config.current_region[3]);
    max_ocr_region = BAASRectangle(config.max_region[0], config.max_region[1],
            config.max_region[2], config.max_region[3]);
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
    result_current_health = std::stoll(ocr_result.text);
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
    result_max_health = std::stoll(ocr_result.text);
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
        result_current_health = std::nullopt;
        result_max_health = std::nullopt;
    } else {
        result_current_health = std::stoll(text.substr(0, p));
        result_max_health = std::stoll(text.substr(p + 1));
    }
}

void BossHealthUpdater::write_result_into_data()
{
    data->boss_current_health = result_current_health;
    data->boss_max_health = result_max_health;
}

BAAS_NAMESPACE_END


