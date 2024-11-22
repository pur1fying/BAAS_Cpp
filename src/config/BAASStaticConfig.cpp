//
// Created by pc on 2024/7/20.
//
#include "config/BAASStaticConfig.h"


BAASStaticConfig *static_config = nullptr;

BAASStaticConfig* BAASStaticConfig::staticConfig = nullptr;

BAASStaticConfig *BAASStaticConfig::getStaticConfig() {
    if (staticConfig == nullptr) {
        staticConfig = new BAASStaticConfig();
    }
    return staticConfig;
}

BAASStaticConfig::BAASStaticConfig() : BAASConfig(CONFIG_TYPE_STATIC_CONFIG) {

}
