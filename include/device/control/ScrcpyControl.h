//
// Created by pc on 2024/7/27.
//

#ifndef BAAS_DEVICE_CONTROL_SCRCPYCONTROL_H_
#define BAAS_DEVICE_CONTROL_SCRCPYCONTROL_H_

#include "device/control/BaseControl.h"
#include "device/BAASScrcpyClient.h"

class ScrcpyControl : public BaseControl {
public:
    explicit ScrcpyControl(BAASConnection *connection);

    void init() override;

    void click(int x, int y) override;

    void long_click(int x, int y, double duration) override;

    void swipe(int x1, int y1, int x2, int y2, double duration) override;

    void exit() override;


private:
    BAASConnection *connection;

    BAASScrcpyClient *client;
};

#endif //BAAS_DEVICE_CONTROL_SCRCPYCONTROL_H_
