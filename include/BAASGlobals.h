//
// Created by pc on 2024/4/12.
//

#ifndef BAAS_BAASGLOBALS_H_
#define BAAS_BAASGLOBALS_H_

#include <filesystem>

#include "core_defines.h"

#include <filesystem>

BAAS_NAMESPACE_BEGIN

extern std::filesystem::path BAAS_PROJECT_DIR;

extern std::filesystem::path BAAS_CONFIG_DIR;

extern std::filesystem::path BAAS_RESOURCE_DIR;

extern std::filesystem::path BAAS_IMAGE_RESOURCE_DIR;

extern std::filesystem::path BAAS_FEATURE_DIR;

extern std::filesystem::path BAAS_PROCEDURE_DIR;

extern std::filesystem::path BAAS_OCR_MODEL_DIR;

extern std::filesystem::path scrcpyJarPath;

extern std::filesystem::path scrcpyJar_REMOTE_DIR;

extern std::string scrcpyJarName;

extern std::filesystem::path BAAS_OUTPUT_DIR;

extern std::filesystem::path ASCREENCAP_BIN_DIR;

extern std::filesystem::path ASCREENCAP_REMOTE_DIR;

extern std::filesystem::path DEVELOPER_PROJECT_DIR;

extern std::string CURRENT_TIME_STRING;

void log_git_info();

void init_path();

BAAS_NAMESPACE_END

#endif //BAAS_BAASGLOBALS_H_
