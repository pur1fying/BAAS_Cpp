//
// Created by pc on 2024/4/12.
//
#include "BAASGlobals.h"
#include "version.h"

using namespace std::filesystem;
using namespace std;

BAAS_NAMESPACE_BEGIN

std::filesystem::path BAAS_PROJECT_DIR;

std::filesystem::path BAAS_CONFIG_DIR;

std::filesystem::path BAAS_RESOURCE_DIR;

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

void log_git_info() {
    BAASGlobalLogger->BAASInfo("BAAS     VERSION : " + std::string(BAAS_VERSION));
    BAASGlobalLogger->sub_title("Build Info");
    BAASGlobalLogger->BAASInfo("Branch      : " + std::string(GIT_BRANCH));
    BAASGlobalLogger->BAASInfo("Hash        : " + std::string(GIT_HASH));
    BAASGlobalLogger->BAASInfo("Author      : " + std::string(GIT_COMMIT_DATE) +
                               " <" + std::string(GIT_COMMIT_AUTHOR) + ">");
    BAASGlobalLogger->BAASInfo("Commit      : " + std::string(GIT_COMMIT_TITLE));
    BAASGlobalLogger->BAASInfo("Build time  : " + std::string(BUILD_TIME));
}

void init_path() {
    BAAS_PROJECT_DIR = std::filesystem::current_path();
    BAAS_OUTPUT_DIR = BAAS_PROJECT_DIR / "output";
    BAAS_CONFIG_DIR = BAAS_PROJECT_DIR / "config";
    BAAS_RESOURCE_DIR = BAAS_PROJECT_DIR / "resource";

    BAAS_IMAGE_RESOURCE_DIR = BAAS_RESOURCE_DIR / "image";

    BAAS_FEATURE_DIR = BAAS_RESOURCE_DIR / "feature";
    BAAS_PROCEDURE_DIR = BAAS_RESOURCE_DIR / "procedure";
    BAAS_OCR_MODEL_DIR = BAAS_RESOURCE_DIR / "ocr_models";
    DEVELOPER_PROJECT_DIR = BAAS_PROJECT_DIR.parent_path().parent_path().string();
    scrcpyJarName = "scrcpy-server.jar";
    scrcpyJarPath = BAAS_RESOURCE_DIR / "bin" / "scrcpy" / scrcpyJarName;
    scrcpyJar_REMOTE_DIR = "/data/local/tmp" / scrcpyJarName;

    ASCREENCAP_BIN_DIR = BAAS_RESOURCE_DIR / "bin" / "ascreencap";
    ASCREENCAP_REMOTE_DIR = "/data/local/tmp/ascreencap";
}

BAAS_NAMESPACE_END