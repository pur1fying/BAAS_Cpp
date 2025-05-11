//
// Created by pc on 2024/8/10.
//

#ifndef BAAS_PROCEDURE_BASEPROCEDURE_H_
#define BAAS_PROCEDURE_BASEPROCEDURE_H_

#include "feature/BAASFeature.h"
#include "device/screenshot/BAASScreenshot.h"
#include "device/control/BAASControl.h"

#define BAAS_ACTION_TYPE_DO_NOTHING 0
#define BAAS_ACTION_TYPE_CLICK 1
#define BAAS_ACTION_TYPE_LONG_CLICK 2
#define BAAS_ACTION_TYPE_SWIPE 3

BAAS_NAMESPACE_BEGIN

class BAAS;

class BaseProcedure {
public:
    explicit BaseProcedure(BAAS* baas, const BAASConfig& possible_feature);

    virtual void implement(
            BAASConfig& output,
            bool skip_first_screenshot = false
    );

    virtual ~BaseProcedure();

    virtual void clear_resource();

    inline const BAASConfig& get_config()
    {
        return possible_feature;
    }

protected:
    BAASConfig possible_feature;

    BAAS* baas;

    BAASLogger* logger;

    bool show_log;
};

BAAS_NAMESPACE_END

#endif //BAAS_PROCEDURE_BASEPROCEDURE_H_
