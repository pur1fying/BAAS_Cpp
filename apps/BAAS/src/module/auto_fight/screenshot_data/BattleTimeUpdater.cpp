//
// Created by Administrator on 2026/2/17.
//
//
// Created by pc on 2025/4/26.
//

#include "module/auto_fight/screenshot_data/BattleTimeUpdater.h"

#include <ocr/BAASOCR.h>
#include <utils/BAASImageUtil.h>
#include <config/BAASStaticConfig.h>

BAAS_NAMESPACE_BEGIN

const std::vector<std::string> BattleTimeUpdater::supported_time_formats = {
    "mm:ss.SSS",
    "mm:ss"
};


BattleTimeUpdater::BattleTimeUpdater(
        BAAS* baas,
        auto_fight_d* data
) : BaseDataUpdater(baas, data)
{
    if (data->d_fight.contains("/BattleTime/time_format"))
        time_format = data->d_fight.getUInt8("/BossHealth/current_ocr_region");
    else
        time_format = static_config->getUInt8("/BAAS/auto_fight/BossHealth/current_ocr_region");

    if (time_format >= supported_time_formats.size()) {
        logger->BAASError("Invalid Recognize BattleTime Format index : [ " + std::to_string(time_format) + " ]");
        logger->BAASInfo("Valid BattleTime Format : ");
        for (size_t i = 0; i < supported_time_formats.size(); ++i)
            logger->BAASInfo(std::to_string(i) + " : " + supported_time_formats[i]);
        throw ValueError("Invalid Recognize BattleTime Format index");
    }

    if (data->d_fight.contains("/BattleTime/ocr_region"))
        ocr_region = data->d_fight.get<BAASRectangle>("/BattleTime/ocr_region");
    else
        ocr_region = static_config->get<BAASRectangle>("/BAAS/auto_fight/BattleTime/ocr_region/" + std::to_string(time_format));

    if (data->d_fight.contains("/BattleTime/ocr_model_name"))
        ocr_model_name = data->d_fight.get<std::string>("/BattleTime/ocr_model_name");
    else
        ocr_model_name = static_config->get<std::string>("/BAAS/auto_fight/BattleTime/ocr_model_name");

    if (!BAASOCR::is_valid_language(ocr_model_name)) {
        logger->BAASInfo("BossHealth ocr_model_name [ " + ocr_model_name + " ] Invalid.");
        throw ValueError("Invalid ocr language");
    }

    logger->BAASInfo("BattleTime Ocr Model Name : [ " + ocr_model_name + " ].");
    baas_ocr->init({ocr_model_name});
}

void BattleTimeUpdater::update()
{
    baas->get_latest_screenshot(origin_screenshot);
    cropped_image = BAASImageUtil::crop(origin_screenshot, ocr_region);
    baas_ocr->ocr_for_single_line(
            ocr_model_name,
            cropped_image,
            ocr_result,
            "",
            nullptr,
            "0123456789:."
    );

    switch (time_format) {
        // 01:44.900
        case 0: {
            auto p_1 = ocr_result.text.find(':');
            auto p_2 = ocr_result.text.find('.', p_1);
            if (p_1 == std::string::npos || p_2 == std::string::npos) {
                result = std::nullopt;
                return;
            }
            auto minute = ocr_result_to_int(0, p_1);
            auto second = ocr_result_to_int(p_1 + 1, p_2);
            auto millisecond = ocr_result_to_int(p_2 + 1, ocr_result.text.size());
            if (!minute.has_value() || !second.has_value() || !millisecond.has_value()) {
                result = std::nullopt;
                return;
            }

            result = minute.value() * 60 * 1000 + second.value() * 1000 + millisecond.value();
            break;
        }

        // 02:15
        case 1: {
            auto p = ocr_result.text.find(':');
            if (p == std::string::npos) {
                result = std::nullopt;
                return;
            }

            auto minute = ocr_result_to_int(0, p);
            auto second = ocr_result_to_int(p + 1, ocr_result.text.size());
            if (!minute.has_value() || !second.has_value()) {
                result = std::nullopt;
                return;
            }

            result = minute.value() * 60 * 1000 + second.value() * 1000;
            break;
        }
    }

}

std::optional<int> BattleTimeUpdater::ocr_result_to_int(int start, int end)
{
    int value = 0;
    for (int i = start; i < end; ++i)  {
        if (ocr_result.text[i] >= '0' && ocr_result.text[i] <= '9')
            value = value * 10 + (ocr_result.text[i] - '0');
        else return std::nullopt;
    }
    return value;
}

double BattleTimeUpdater::estimated_time_cost()
{
    return 5e7;
}

void BattleTimeUpdater::display_data()
{
    if(!data->fight_left_time_ms.has_value()) logger->BAASInfo("BattleTime : No Value.");
    else logger->BAASInfo("BattleTime : [ " + BAASStringUtil::format_battle_time_ms(data->fight_left_time_ms.value()) + " ]");
}

constexpr std::string BattleTimeUpdater::data_name()
{
    return "BattleTime";
}

void BattleTimeUpdater::write_result_into_data()
{
    data->fight_left_time_ms = result;
}

BAAS_NAMESPACE_END


