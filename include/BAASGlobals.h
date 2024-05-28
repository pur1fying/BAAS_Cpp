//
// Created by pc on 2024/4/12.
//
#pragma once
#ifndef BAAS_CXX_REFACTOR_BAASGLOBALS_H
#define BAAS_CXX_REFACTOR_BAASGLOBALS_H
#include <filesystem>
#include "BAASLogger.h"
#include "BAASAdbUtils.h"

extern std::string BAAS_PROJECT_DIR;

extern std::string scrcpyJarPath;

extern std::string scrcpyJarName;

extern std::string nemuDllPath;

void initGlobals();



#endif //BAAS_CXX_REFACTOR_BAASGLOBALS_H
