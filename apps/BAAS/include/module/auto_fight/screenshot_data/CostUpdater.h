//
// Created by pc on 2025/4/24.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_DATA_COSTUPDATER_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_DATA_COSTUPDATER_H_

#include "BaseDataUpdater.h"


BAAS_NAMESPACE_BEGIN

class CostUpdater : public BaseDataUpdater {

public:
    explicit CostUpdater(BAAS *baas, screenshot_data *data);

    bool update() override;

    double estimated_time_cost() override;

    std::string data_name();
};

BAAS_NAMESPACE_END

#endif //BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_DATA_COSTUPDATER_H_
