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
string BAAS_PROJECT_DIR;

string BAAS_CONFIG_DIR;

string BAAS_IMAGE_RESOURCE_DIR;

string BAAS_FEATURE_DIR;

string BAAS_PROCEDURE_DIR;

string BAAS_OCR_MODEL_DIR;

string scrcpyJarPath;

string scrcpyJarName;

string nemuDllPath;

string DEVELOPER_PROJECT_DIR;

string MuMuInstallPath;

string BAAS_OUTPUT_DIR;

string CURRENT_TIME_STRING;

string ASCREENCAP_BIN_DIR;

string ASCREENCAP_REMOTE_DIR;

void init_globals() {
    if (inited) {
        return;
    }
    inited = true;
    BAASUtil::initWinsock();
    path curr = current_path();

    BAAS_PROJECT_DIR = curr.string();
    BAAS_OUTPUT_DIR = BAAS_PROJECT_DIR + R"(\output)";
    BAAS_CONFIG_DIR = BAAS_PROJECT_DIR + R"(\config)";
    BAAS_IMAGE_RESOURCE_DIR = BAAS_PROJECT_DIR + R"(\resource\image)";
    BAAS_FEATURE_DIR = BAAS_PROJECT_DIR + R"(\resource\features)";
    BAAS_PROCEDURE_DIR = BAAS_PROJECT_DIR + R"(\resource\procedure)";
    BAAS_OCR_MODEL_DIR = BAAS_PROJECT_DIR + R"(\resource\ocr_models)";

    BAASGlobalLogger = GlobalLogger::getGlobalLogger();
    BAASGlobalLogger->BAASInfo("BAAS VERSION : " + string(BAAS_VERSION));
    BAASGlobalLogger->BAASInfo(BAAS_PROJECT_DIR);

    scrcpyJarName = "scrcpy-server.jar";
    scrcpyJarPath = BAAS_PROJECT_DIR + R"(\resource\bin\scrcpy\scrcpy-server.jar)";

    ASCREENCAP_BIN_DIR = BAAS_PROJECT_DIR + R"(\resource\bin\ascreencap\)";
    ASCREENCAP_REMOTE_DIR = "/data/local/tmp/ascreencap";

    nemuDllPath = BAAS_PROJECT_DIR + R"(\resource\nemu_dll\external_renderer_ipc.dll)";

    baas_ocr = BAASOCR::get_instance();


    DEVELOPER_PROJECT_DIR = curr.parent_path().parent_path().string();

    static_config = BAASStaticConfig::getStaticConfig();

    baas_features = BAASFeature::get_instance();
    baas_procedures = BAASProcedure::get_instance();
    resource = BAASImageResource::get_instance();

    Server::init();
    BAAS::init_implement_funcs();

    config_name_change = new BAASConfig(CONFIG_TYPE_CONFIG_NAME_CHANGE);
    config_template = new BAASUserConfig(CONFIG_TYPE_DEFAULT_CONFIG);
    default_global_setting = new BAASConfig(CONFIG_TYPE_DEFAULT_GLOBAL_SETTING);
    BAASGlobalSetting::check_global_setting_exist();
    global_setting = BAASGlobalSetting::getGlobalSetting();
}