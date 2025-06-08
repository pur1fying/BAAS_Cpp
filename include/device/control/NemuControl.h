//
// Created by pc on 2024/8/3.
//

#ifndef BAAS_DEVICE_CONTROL_NEMUCONTROL_H_
#define BAAS_DEVICE_CONTROL_NEMUCONTROL_H_

#include "BaseControl.h"
#include "device/BAASNemu.h"

BAAS_NAMESPACE_BEGIN

class NemuControl : public BaseControl {

public:

    explicit NemuControl(BAASConnection* connection);

    void init() override;

    void click(
            int x,
            int y
    ) override;

    void long_click(
            int x,
            int y,
            double duration
    ) override;

    void swipe(
            int x1,
            int y1,
            int x2,
            int y2,
            double duration
    ) override;

    void exit() override;

private:

    BAASNemu* nemu_connection;
};

BAAS_NAMESPACE_END

#endif //BAAS_DEVICE_CONTROL_NEMUCONTROL_H_
