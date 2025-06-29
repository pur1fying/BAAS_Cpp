//
// Created by pc on 2024/4/12.
//
#include "BAASGlobals.h"

#include <iostream>

#include "config.h"
#include "BAASLogger.h"
#include "ocr/BAASOCR.h"
#include "BAAS_version.h"
#include "feature/BAASFeature.h"
#include "procedure/BAASProcedure.h"

using namespace std::filesystem;
using namespace std;

BAAS_NAMESPACE_BEGIN

std::filesystem::path BAAS_PROJECT_DIR;

std::filesystem::path BAAS_CONFIG_DIR;

std::filesystem::path BAAS_IMAGE_RESOURCE_DIR;

std::filesystem::path BAAS_FEATURE_DIR;

std::filesystem::path BAAS_PROCEDURE_DIR;

std::filesystem::path BAAS_OCR_MODEL_DIR;

std::filesystem::path scrcpyJarPath;

std::filesystem::path scrcpyJar_REMOTE_DIR;

std::filesystem::path scrcpyJarName;

std::filesystem::path MuMuInstallPath;

std::filesystem::path BAAS_OUTPUT_DIR;

std::filesystem::path ASCREENCAP_BIN_DIR;

std::filesystem::path ASCREENCAP_REMOTE_DIR;

std::filesystem::path DEVELOPER_PROJECT_DIR;

std::string CURRENT_TIME_STRING;

void init_globals()
{
    if (inited) {
        return;
    }
    inited = true;
    BAASUtil::initWinsock();
    init_path();

    BAASGlobalLogger = GlobalLogger::getGlobalLogger();
    BAASGlobalLogger->BAASInfo("BAAS VERSION : " + std::string(BAAS_VERSION));
    BAASGlobalLogger->Path(BAAS_PROJECT_DIR);
    baas_ocr = BAASOCR::get_instance();
    static_config = BAASStaticConfig::getStaticConfig();
#ifdef BAAS_APP_BUILD_FEATURE
    baas_features = BAASFeature::get_instance();
#endif // BAAS_APP_BUILD_FEATURE

#if defined(BAAS_APP_BUILD_FEATURE) || defined(BAAS_APP_BUILD_PROCEDURE)
    baas_procedures = BAASProcedure::get_instance();
#endif // defined(BAAS_APP_BUILD_FEATURE) || defined(BAAS_APP_BUILD_PROCEDURE)

    GameServer::init();

    config_name_change = new BAASConfig(CONFIG_TYPE_CONFIG_NAME_CHANGE);
    config_template = new BAASUserConfig(CONFIG_TYPE_DEFAULT_CONFIG);
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
}

void init_path() {
    BAAS_PROJECT_DIR = std::filesystem::current_path();
    BAAS_OUTPUT_DIR = BAAS_PROJECT_DIR / "output";
    BAAS_CONFIG_DIR = BAAS_PROJECT_DIR / "config";
    BAAS_IMAGE_RESOURCE_DIR = BAAS_PROJECT_DIR / "resource" / "image";

    BAAS_FEATURE_DIR = BAAS_PROJECT_DIR / "resource" / "features";
    BAAS_PROCEDURE_DIR = BAAS_PROJECT_DIR / "resource" / "procedure";
    BAAS_OCR_MODEL_DIR = BAAS_PROJECT_DIR / "resource" / "ocr_models";
    DEVELOPER_PROJECT_DIR = BAAS_PROJECT_DIR.parent_path()
                                            .parent_path()
                                            .string();
    scrcpyJarName = "scrcpy-server.jar";
    scrcpyJarPath = BAAS_PROJECT_DIR / "resource" / "bin" / "scrcpy" / scrcpyJarName;
    scrcpyJar_REMOTE_DIR = "/data/local/tmp" / scrcpyJarName;

    ASCREENCAP_BIN_DIR = BAAS_PROJECT_DIR / "resource" / "bin" / "ascreencap";
    ASCREENCAP_REMOTE_DIR = "/data/local/tmp/ascreencap";
}

BAAS_NAMESPACE_END