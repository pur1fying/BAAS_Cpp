//
// Created by pc on 2024/5/25.
//

#ifndef BAAS_BAASSCREENSHOT_H_
#define BAAS_BAASSCREENSHOT_H_
#include <opencv2/opencv.hpp>

#include "device/BAASNemu.h"

#define screenshotAdb (1 << 0)
#define screenshotNemu (1 << 1)
#define screenshotScrcpy (1 << 2)

class BAASScreenshot {
public:
    BAASScreenshot();

    BAASScreenshot(const int Method = screenshotNemu, double interval = 0.01);

    void setScreenshotMethod(const int Method);

    void setInterval(double interval);

    bool screenshot(cv::Mat &output);

private:
    double interval;

    cv::Mat image;

    double lastScreenshotTime;

    int screenshotMethod;
};
#endif //BAAS_BAASSCREENSHOT_H_
