//
// Created by pc on 2025/4/24.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_DATA_BASEDATAUPDATER_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_DATA_BASEDATAUPDATER_H_

#include "BAAS.h"
#include "screenshot_data_recoder.h"

BAAS_NAMESPACE_BEGIN

class BaseDataUpdater {
public:
    explicit BaseDataUpdater(BAAS* baas, screenshot_data* data);

    virtual bool update();

    virtual double estimated_time_cost();

    virtual constexpr std::string data_name();

protected:
    BAAS* baas;

    screenshot_data* data;
};

BAAS_NAMESPACE_END


#endif //BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_DATA_BASEDATAUPDATER_H_
