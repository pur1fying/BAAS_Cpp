//
// Created by pc on 2024/4/12.
//

#include "BAASGlobals.h"
using namespace std::filesystem;
using namespace std;
string BAAS_PROJECT_DIR;

string BAAS_CONFIG_DIR;

string scrcpyJarPath;

string scrcpyJarName;

string nemuDllPath;

string DEVELOPER_PROJECT_DIR;

string MuMuInstallPath;

string BAAS_OUTPUT_DIR;

string CURRENT_TIME_STRING;

void initGlobals() {
    if (inited) {
        return;
    }
    inited = true;
    BAASUtil::initWinsock();
    path curr = current_path();

    BAAS_PROJECT_DIR = curr.string();
    BAAS_OUTPUT_DIR = BAAS_PROJECT_DIR + "\\output";
    BAAS_CONFIG_DIR = BAAS_PROJECT_DIR + "\\config";

    BAASGlobalLogger = GlobalLogger::getGlobalLogger();

    scrcpyJarName = "scrcpy-server.jar";
    scrcpyJarPath = BAAS_PROJECT_DIR + "\\resource\\scrcpy-server.jar";

    nemuDllPath = BAAS_PROJECT_DIR + "\\resource\\nemu_dll\\external_renderer_ipc.dll";

    BAASGlobalLogger->BAASInfo(BAAS_PROJECT_DIR);

    DEVELOPER_PROJECT_DIR = curr.parent_path().parent_path().string();

    static_config = new BAASConfig(CONFIG_TYPE_STATIC_CONFIG);

    config_name_change = new BAASConfig(CONFIG_TYPE_CONFIG_NAME_CHANGE);

    config_template = new UserConfig(CONFIG_TYPE_DEFAULT_CONFIG);


}