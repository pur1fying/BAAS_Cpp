//
// Created by pc on 2025/4/24.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_DATA_BASEDATAUPDATER_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_DATA_BASEDATAUPDATER_H_

#include "BAAS.h"
#include "module/auto_fight/auto_fight_d.h"

BAAS_NAMESPACE_BEGIN

class BaseDataUpdater {
public:
    explicit BaseDataUpdater(BAAS* baas, auto_fight_d* data);

    bool at_fight_page();

    virtual void update();

    virtual double estimated_time_cost();

    virtual constexpr std::string data_name();

    virtual void display_data();

protected:
    BAASLogger* logger;

    BAAS* baas;

    auto_fight_d* data;
};

BAAS_NAMESPACE_END


#endif //BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_DATA_BASEDATAUPDATER_H_
