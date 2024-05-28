//
// Created by pc on 2024/4/12.
//

#ifndef BAAS_CXX_REFACTOR_BAASCONFIG_H
#define BAAS_CXX_REFACTOR_BAASCONFIG_H
#include <nlohmann/json.hpp>
#include <BAASGlobals.h>
#include <filesystem>
#include <vector>
#include <mutex>
struct ConfigSet {
    std::string configDir;
    nlohmann::json config, eventConfig, displayConfig, switchConfig;
};
class BAASConfig {
public:
    static BAASConfig* getInstance();
    std::vector<ConfigSet> getConfigSet();
private:
    BAASConfig();
    static BAASConfig* instance;
    static nlohmann::json staticConfig;
};


#endif //BAAS_CXX_REFACTOR_BAASCONFIG_H
