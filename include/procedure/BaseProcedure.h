//
// Created by pc on 2024/8/10.
//

#ifndef BAAS_PROCEDURE_BASEPROCEDURE_H_
#define BAAS_PROCEDURE_BASEPROCEDURE_H_
#include "feature/BAASFeature.h"
#include "device/screenshot/BAASScreenshot.h"
#include "device/control/BAASControl.h"
#include "BAAS.h"

class BaseProcedure {
public:
    BaseProcedure(BAAS* baas);


protected:

    BAASLogger* logger;

    BAAS* baas;


};


#endif //BAAS_PROCEDURE_BASEPROCEDURE_H_
