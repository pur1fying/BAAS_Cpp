//
// Created by Administrator on 2025/5/29.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_CONSTANTS_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_CONSTANTS_H_


#include <string>
#include <vector>

#include <BAASLogger.h>
#include <core_defines.h>

#include "module/auto_fight/auto_fight_d.h"

BAAS_NAMESPACE_BEGIN

void _log_valid_op(
        const std::string& name,
        BAASLogger* logger,
        const std::vector<std::string>& _op_st_list
) noexcept;

void _log_valid_skill_names(
        const std::string& name,
        BAASLogger* logger,
        const auto_fight_d* data
) noexcept;

void _log_valid_yolo_obj(
        const std::string& name,
        BAASLogger* logger,
        const auto_fight_d* data
) noexcept;

BAAS_NAMESPACE_END

#endif //BAAS_CPP_MODULE_AUTO_FIGHT_CONSTANTS_H_
