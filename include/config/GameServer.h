//
// Created by pc on 2024/7/22.
//

#ifndef BAAS_CONFIG_GAMESERVER_H_
#define BAAS_CONFIG_GAMESERVER_H_

#include <map>
#include <string>
#include <vector>

#include "core_defines.h"

BAAS_NAMESPACE_BEGIN

class GameServer {

public:

    static void init();

    static inline std::string package2server(const std::string &package) noexcept
    {
        auto it = package2serverMap.find(package);
        if (it != package2serverMap.end()) {
            return it->second;
        }
        return "CN";
    }

    static std::vector<std::string> valid_servers;

    static std::map<std::string, std::string> package2serverMap;

};

BAAS_NAMESPACE_END

#endif //BAAS_CONFIG_GAMESERVER_H_
