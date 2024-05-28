//
// Created by pc on 2024/4/12.
//
#include "BAASConfig.h"
using namespace std;

BAASConfig *BAASConfig::instance = nullptr;
BAASConfig::BAASConfig() {}
BAASConfig *BAASConfig::getInstance() {
    if(instance == nullptr) {
        mutex m;
        m.lock();
        if(instance == nullptr) {
            instance = new BAASConfig();
        }
        m.unlock();
    }
    return instance;
}

vector<ConfigSet> BAASConfig::getConfigSet() {
    vector<ConfigSet> configSets;
    ConfigSet configSet;
    return configSets;
}
