//
// Created by pc on 2024/7/27.
//

#ifndef BAAS_DEVICE_SCREENSHOT_ADBSCREENSHOT_H_
#define BAAS_DEVICE_SCREENSHOT_ADBSCREENSHOT_H_

#include "device/screenshot/BaseScreenshot.h"

class AdbScreenshot : public BaseScreenshot {
public:

    explicit AdbScreenshot(BAASConnection *connection);

    void init() override;

    void screenshot(cv::Mat &img) override;

    void exit() override;

    bool is_lossy() override;
private:

};

#endif //BAAS_DEVICE_SCREENSHOT_ADBSCREENSHOT_H_
