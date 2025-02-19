//
// Created by pc on 2024/4/12.
//

#include "BAASGlobals.h"

#include "config.h"
#include "version.h"
#include "BAASImageResource.h"
#include "feature/BAASFeature.h"
#include "procedure/BAASProcedure.h"

using namespace std::filesystem;
using namespace std;

BAAS_NAMESPACE_BEGIN

std::string BAAS_PROJECT_DIR;

std::string BAAS_CONFIG_DIR;

std::string BAAS_IMAGE_RESOURCE_DIR;

std::string BAAS_FEATURE_DIR;

std::string BAAS_PROCEDURE_DIR;

std::string BAAS_OCR_MODEL_DIR;

std::string scrcpyJarPath;

std::string scrcpyJarName;

std::string MuMuInstallPath;

std::string BAAS_OUTPUT_DIR;

std::string CURRENT_TIME_STRING;

std::string ASCREENCAP_BIN_DIR;

std::string ASCREENCAP_REMOTE_DIR;

std::string DEVELOPER_PROJECT_DIR;

void init_globals()
{
    if (inited) {
        return;
    }
    inited = true;
    BAASUtil::initWinsock();
    std::filesystem::path curr = std::filesystem::current_path();

    BAAS_PROJECT_DIR = curr.string();
    BAAS_OUTPUT_DIR = BAAS_PROJECT_DIR + R"(\output)";
    BAAS_CONFIG_DIR = BAAS_PROJECT_DIR + R"(\config)";
    BAAS_IMAGE_RESOURCE_DIR = BAAS_PROJECT_DIR + R"(\resource\image)";
    BAAS_FEATURE_DIR = BAAS_PROJECT_DIR + R"(\resource\features)";
    BAAS_PROCEDURE_DIR = BAAS_PROJECT_DIR + R"(\resource\procedure)";
    BAAS_OCR_MODEL_DIR = BAAS_PROJECT_DIR + R"(\resource\ocr_models)";

    BAASGlobalLogger = GlobalLogger::getGlobalLogger();
    BAASGlobalLogger->BAASInfo("BAAS VERSION : " + std::string(BAAS_VERSION));
    BAASGlobalLogger->BAASInfo(BAAS_PROJECT_DIR);

    scrcpyJarName = "scrcpy-server.jar";
    scrcpyJarPath = BAAS_PROJECT_DIR + R"(\resource\bin\scrcpy\scrcpy-server.jar)";

    ASCREENCAP_BIN_DIR = BAAS_PROJECT_DIR + R"(\resource\bin\ascreencap\)";
    ASCREENCAP_REMOTE_DIR = "/data/local/tmp/ascreencap";

    baas_ocr = BAASOCR::get_instance();


    DEVELOPER_PROJECT_DIR = curr.parent_path()
                                .parent_path()
                                .string();

    static_config = BAASStaticConfig::getStaticConfig();

    baas_features = BAASFeature::get_instance();
    baas_procedures = BAASProcedure::get_instance();
    resource = BAASImageResource::get_instance();

    GameServer::init();
    BAAS::init_implement_funcs();

    config_name_change = new BAASConfig(CONFIG_TYPE_CONFIG_NAME_CHANGE);
    config_template = new BAASUserConfig(CONFIG_TYPE_DEFAULT_CONFIG);
    default_global_setting = new BAASConfig(CONFIG_TYPE_DEFAULT_GLOBAL_SETTING);
    BAASGlobalSetting::check_global_setting_exist();
    global_setting = BAASGlobalSetting::getGlobalSetting();
}

BAAS_NAMESPACE_END