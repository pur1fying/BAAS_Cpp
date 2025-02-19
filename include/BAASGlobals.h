//
// Created by pc on 2024/4/12.
//

#ifndef BAAS_BAASGLOBALS_H_
#define BAAS_BAASGLOBALS_H_

#include "core_defines.h"
#include "BAASLogger.h"

BAAS_NAMESPACE_BEGIN

extern std::string BAAS_PROJECT_DIR;

extern std::string BAAS_CONFIG_DIR;

extern std::string BAAS_IMAGE_RESOURCE_DIR;

extern std::string BAAS_FEATURE_DIR;

extern std::string BAAS_PROCEDURE_DIR;

extern std::string BAAS_OCR_MODEL_DIR;

extern std::string scrcpyJarPath;

extern std::string scrcpyJarName;

extern std::string MuMuInstallPath;

extern std::string BAAS_OUTPUT_DIR;

extern std::string CURRENT_TIME_STRING;

extern std::string ASCREENCAP_BIN_DIR;

extern std::string ASCREENCAP_REMOTE_DIR;

extern std::string DEVELOPER_PROJECT_DIR;

/*
 *  BAAS_CPP --
 *           -- cmake-build-debug (this is BAAS_PROJECT_DIR)
 *              -- resource
 *           -- resource (developer need to modify resource in this directory)
 *
 */

void init_globals();

static bool inited = false;

BAAS_NAMESPACE_END

#endif //BAAS_BAASGLOBALS_H_
