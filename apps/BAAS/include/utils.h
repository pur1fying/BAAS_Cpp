//
// Created by pc on 2025/4/20.
//

#ifndef BAAS_APP_UTILS_H_
#define BAAS_APP_UTILS_H_

#include "BAAS.h"
#include "BAASLogger.h"
#include "core_defines.h"

BAAS_NAMESPACE_BEGIN

extern std::filesystem::path BAAS_AUTO_FIGHT_WORKFLOW_DIR;

extern std::filesystem::path BAAS_YOLO_MODEL_DIR;

void init_globals();

void register_baas_module(BAAS* baas);

BAAS_NAMESPACE_END

#endif //BAAS_APP_UTILS_H_
