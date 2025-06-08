//
// Created by pc on 2024/7/27.
//

#ifndef BAAS_DEVICE_CONTROL_BASECONTROL_H_
#define BAAS_DEVICE_CONTROL_BASECONTROL_H_

#include "device/BAASConnection.h"

BAAS_NAMESPACE_BEGIN

class BaseControl {

public:

    explicit BaseControl(BAASConnection* connection);

    virtual void init();

    virtual void click(
            int x,
            int y
    );

    virtual void long_click(
            int x,
            int y,
            double duration
    );

    virtual void swipe(
            int x1,
            int y1,
            int x2,
            int y2,
            double duration
    );

    virtual void exit();

protected:
    BAASConnection* connection;

    std::mutex control_mtx;

    BAASLogger* logger;
};

BAAS_NAMESPACE_END

#endif //BAAS_DEVICE_CONTROL_BASECONTROL_H_

