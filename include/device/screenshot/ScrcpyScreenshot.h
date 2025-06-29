//
// Created by pc on 2024/7/27.
//

#ifndef BAAS_DEVICE_SCREENSHOT_SCRCPYSCREENSHOT_H_
#define BAAS_DEVICE_SCREENSHOT_SCRCPYSCREENSHOT_H_

#include "device/screenshot/BaseScreenshot.h"
#include "device/BAASScrcpyClient.h"
#include "BaseScreenshot.h"

BAAS_NAMESPACE_BEGIN
class ScrcpyScreenshot : public BaseScreenshot {

public:

    explicit ScrcpyScreenshot(BAASConnection* connection);

    void init() override;

    void screenshot(cv::Mat& img) override;

    void exit() override;

    bool is_lossy() override;

private:

    BAASConnection* connection;

    BAASScrcpyClient* client;

};

BAAS_NAMESPACE_END


#endif //CONFIG_JSON_SCRCPYSCREENSHOT_H
