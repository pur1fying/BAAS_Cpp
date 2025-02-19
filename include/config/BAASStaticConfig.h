//
// Created by pc on 2024/7/20.
//

#ifndef BAAS_CONFIG_BAASSTATICCONFIG_H_
#define BAAS_CONFIG_BAASSTATICCONFIG_H_

#include "config/BAASConfig.h"

BAAS_NAMESPACE_BEGIN

class BAASStaticConfig : public BAASConfig {
public:
    static BAASStaticConfig *getStaticConfig();

    inline std::vector<std::string> adb_binary_dirs() noexcept
    {
        return get("adb_binary_dirs", std::vector<std::string>{});
    }

    inline std::vector<std::string> valid_packages() noexcept
    {
        return get("valid_packages", std::vector<std::string>{});
    }

private:
    BAASStaticConfig();

    static BAASStaticConfig *staticConfig;
};

extern BAASStaticConfig *static_config;

BAAS_NAMESPACE_END

#endif //BAAS_CONFIG_BAASSTATICCONFIG_H_
