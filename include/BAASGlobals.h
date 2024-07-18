//
// Created by pc on 2024/4/12.
//
#pragma once
#ifndef BAAS_BAASGLOBALS_H
#define BAAS_BAASGLOBALS_H
#include <filesystem>

#include "BAASLogger.h"
#include "BAASAdbUtils.h"
#include "BAASConfig.h"
#include "BAASUserConfig.h"

extern std::string BAAS_PROJECT_DIR;

extern std::string BAAS_CONFIG_DIR;

extern std::string scrcpyJarPath;

extern std::string scrcpyJarName;

extern std::string nemuDllPath;

extern std::string MuMuInstallPath;

extern std::string BAAS_OUTPUT_DIR;

extern std::string CURRENT_TIME_STRING;

/*
 *  BAAS_CPP --
 *           -- cmake-build-debug (this is BAAS_PROJECT_DIR)
 *              -- resource
 *           -- resource (developer need to modify resource in this directory)
 *
 */
extern std::string DEVELOPER_PROJECT_DIR;


static bool inited = false;

void initGlobals();



#endif //BAAS_BAASGLOBALS_H
