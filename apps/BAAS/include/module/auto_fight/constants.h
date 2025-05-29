//
// Created by Administrator on 2025/5/29.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_CONSTANTS_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_CONSTANTS_H_


#include <string>
#include <vector>

#include <BAASLogger.h>
#include <core_defines.h>

BAAS_NAMESPACE_BEGIN

void _log_valid_op(
        const std::string& name,
        BAASLogger* logger,
        const std::vector<std::string>& _op_st_list
) noexcept;

void _get_valid_log_string(
        const std::string& name,
        const std::vector<std::string>& _op_st_list,
        std::string& out
) noexcept;

BAAS_NAMESPACE_END

#endif //BAAS_CPP_MODULE_AUTO_FIGHT_CONSTANTS_H_
