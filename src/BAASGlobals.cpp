//
// Created by pc on 2024/4/12.
//

#include "BAASGlobals.h"
using namespace std::filesystem;
using namespace std;
string BAAS_PROJECT_DIR;

string scrcpyJarPath;

string scrcpyJarName;

string nemuDllPath;

string DEVELOPER_PROJECT_DIR;

string MuMuInstallPath;
void initGlobals() {
    if (inited) {
        return;
    }
    inited = true;
    BAASUtil::initWinsock();
    path curr = current_path();
    BAAS_PROJECT_DIR = curr.string();
    BAASLoggerInstance = BAASLogger::getInstance();
    scrcpyJarName = "scrcpy-server.jar";
    scrcpyJarPath = BAAS_PROJECT_DIR + "//resource//scrcpy-server.jar";
    nemuDllPath = BAAS_PROJECT_DIR + "//resource//nemu_dll//external_renderer_ipc.dll";
    BAASLoggerInstance->BAASInfo(BAAS_PROJECT_DIR);
    DEVELOPER_PROJECT_DIR = curr.parent_path().parent_path().string();
    BAASAdbClient adb;
}