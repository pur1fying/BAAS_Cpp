//
// Created by pc on 2024/4/12.
//

#include "BAASGlobals.h"
using namespace std::filesystem;
using namespace std;
string BAAS_PROJECT_DIR = "";
string scrcpyJarPath = "";
string scrcpyJarName = "";
string nemuDllPath = "";

void initGlobals() {
    BAASUtil::initWinsock();
    BAAS_PROJECT_DIR = current_path().string();
    BAASLoggerInstance = BAASLogger::getInstance();
    scrcpyJarName = "scrcpy-server.jar";
    scrcpyJarPath = BAAS_PROJECT_DIR + "//resource//scrcpy-server.jar";
    nemuDllPath = BAAS_PROJECT_DIR + "//resource//nemu_dll//external_renderer_ipc.dll";
    BAASLoggerInstance->BAASInfo(BAAS_PROJECT_DIR.c_str());
    BAASAdbClient adb;
}