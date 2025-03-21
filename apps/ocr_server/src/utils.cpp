//
// Created by pc on 2025/2/25.
//

#include <utils.h>
#include <BAASGlobals.h>
#include "config.h"
#include "ocr/BAASOCR.h"
#include "version.h"
#include "BAAS_OCR_Version.h"

using namespace baas;
OCR_NAMESPACE_BEGIN

void __init()
{
    init_path();
    BAASGlobalLogger = GlobalLogger::getGlobalLogger();
    BAASGlobalLogger->BAASInfo("BAAS     VERSION : " + std::string(BAAS_VERSION));
    BAASGlobalLogger->BAASInfo("BAAS OCR VERSION : " + std::string(BAAS_OCR_VERSION));
    BAASGlobalLogger->Path(BAAS_PROJECT_DIR);
    baas_ocr = BAASOCR::get_instance();

    static_config = BAASStaticConfig::getStaticConfig();
    default_global_setting = new BAASConfig(CONFIG_TYPE_DEFAULT_GLOBAL_SETTING);

    // check config dir
    BAASGlobalLogger->sub_title("Config Dir");
    BAASGlobalLogger->Path(BAAS_CONFIG_DIR);
    if (!std::filesystem::exists(BAAS_CONFIG_DIR)) {
        BAASGlobalLogger->sub_title("Create Config Dir");
        std::filesystem::create_directory(BAAS_CONFIG_DIR);
    }
    BAASGlobalSetting::check_global_setting_exist();
    global_setting = BAASGlobalSetting::getGlobalSetting();

    spdlog::flush_every(std::chrono::milliseconds(global_setting->getInt("/log/flush_inteval", 1000)));
}

OCR_NAMESPACE_END