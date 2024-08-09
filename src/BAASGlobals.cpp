//
// Created by pc on 2024/4/12.
//

#include "BAASGlobals.h"
#include "feature/BAASFeature.h"
#include "config.h"
#include "BAASImageResource.h"

using namespace std::filesystem;
using namespace std;
string BAAS_PROJECT_DIR;

string BAAS_CONFIG_DIR;

string BAAS_IMAGE_RESOURCE_DIR;

string BAAS_FEATURE_DIR;

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
    BAAS_OUTPUT_DIR = BAAS_PROJECT_DIR + "\\output";
    BAAS_CONFIG_DIR = BAAS_PROJECT_DIR + "\\config";
    BAAS_IMAGE_RESOURCE_DIR = BAAS_PROJECT_DIR + "\\resource\\image";
    BAAS_FEATURE_DIR = BAAS_PROJECT_DIR + "\\resource\\features";

    BAASGlobalLogger = GlobalLogger::getGlobalLogger();

    scrcpyJarName = "scrcpy-server.jar";
    scrcpyJarPath = BAAS_PROJECT_DIR + R"(\resource\bin\scrcpy\scrcpy-server.jar)";

    ASCREENCAP_BIN_DIR = BAAS_PROJECT_DIR + R"(\resource\bin\ascreencap\)";
    ASCREENCAP_REMOTE_DIR = "/data/local/tmp/ascreencap";

    nemuDllPath = BAAS_PROJECT_DIR + R"(\resource\nemu_dll\external_renderer_ipc.dll)";

    BAASGlobalLogger->BAASInfo(BAAS_PROJECT_DIR);

    DEVELOPER_PROJECT_DIR = curr.parent_path().parent_path().string();

    static_config = BAASStaticConfig::getStaticConfig();
    features = BAASFeature::get_instance();
    resource = BAASImageResource::get_instance();

    Server::init();

    config_name_change = new BAASConfig(CONFIG_TYPE_CONFIG_NAME_CHANGE);

    config_template = new BAASUserConfig(CONFIG_TYPE_DEFAULT_CONFIG);
}