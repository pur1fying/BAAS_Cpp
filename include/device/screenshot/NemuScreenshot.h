//
// Created by pc on 2024/8/3.
//

#ifndef BAAS_DEVICE_SCREENSHOT_NEMUSCREENSHOT_H_
#define BAAS_DEVICE_SCREENSHOT_NEMUSCREENSHOT_H_

#include "device/BAASNemu.h"
#include "device/screenshot/BaseScreenshot.h"

BAAS_NAMESPACE_BEGIN
class NemuScreenshot : public BaseScreenshot {

public:

    NemuScreenshot(BAASConnection* connection);

    void init() override;

    void screenshot(cv::Mat& output) override;

    void exit() override;

    bool is_lossy() override;

private:

    std::shared_ptr<BAASNemu> nemu_connection;

};

BAAS_NAMESPACE_END

#endif //BAAS_DEVICE_SCREENSHOT_NEMUSCREENSHOT_H_
