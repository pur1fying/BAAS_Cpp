//
// Created by pc on 2024/8/3.
//

#ifndef BAAS_DEVICE_SCREENSHOT_NEMUSCREENSHOT_H_
#define BAAS_DEVICE_SCREENSHOT_NEMUSCREENSHOT_H_

#include "device/screenshot/BaseScreenshot.h"
#include "device/BAASNemu.h"

class NemuScreenshot : public BaseScreenshot {
public:
    NemuScreenshot(BAASConnection *connection);

    void init() override;

    void screenshot(cv::Mat &output) override;

    void exit() override;

    bool is_lossy() override;

private:
    BAASNemu *nemu_connection;

};
#endif //BAAS_DEVICE_SCREENSHOT_NEMUSCREENSHOT_H_
