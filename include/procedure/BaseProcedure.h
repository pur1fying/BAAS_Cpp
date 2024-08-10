//
// Created by pc on 2024/8/10.
//

#ifndef BAAS_PROCEDURE_BASEPROCEDURE_H_
#define BAAS_PROCEDURE_BASEPROCEDURE_H_
#include "feature/BAASFeature.h"
#include "device/screenshot/BAASScreenshot.h"
#include "device/control/BAASControl.h"
#include "BAAS.h"

#define BAAS_ACTION_TYPE_DO_NOTHING 0
#define BAAS_ACTION_TYPE_CLICK 1
#define BAAS_ACTION_TYPE_LONG_CLICK 2
#define BAAS_ACTION_TYPE_SWIPE 3

class BaseProcedure {
public:
    explicit BaseProcedure(BAASConfig* possible_feature);

    virtual void implement(BAAS* baas, BAASConfig& output);

    virtual void clear_resource();
protected:
    BAASConfig* possible_feature;

    BAASLogger* logger;

    BAAS* baas;
};


#endif //BAAS_PROCEDURE_BASEPROCEDURE_H_
