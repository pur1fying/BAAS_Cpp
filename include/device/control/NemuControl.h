//
// Created by pc on 2024/8/3.
//

#ifndef BAAS_DEVICE_CONTROL_NEMUCONTROL_H_
#define BAAS_DEVICE_CONTROL_NEMUCONTROL_H_

#include <device/control/BaseControl.h>
#include <device/BAASNemu.h>

class NemuControl : public BaseControl {
public:
    explicit NemuControl(BAASConnection* connection);

    void init() override;

    void tap(int x, int y) override;

    void swipe(int x1, int y1, int x2, int y2, int duration) override;

    void keyevent(int keycode) override;

    void text(const std::string& text) override;

    void exit() override;
};

#endif //BAAS_DEVICE_CONTROL_NEMUCONTROL_H_
