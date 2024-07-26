//
// Created by pc on 2024/7/26.
//

#ifndef BAAS_DEVICE_SCREENSHOT_BASESCREENSHOT_H_
#define BAAS_DEVICE_SCREENSHOT_BASESCREENSHOT_H_

#include "opencv2/opencv.hpp"

#include "device/BAASConnection.h"

class BaseScreenshot {
public:
    explicit BaseScreenshot(BAASConnection *connection);

    virtual void init();

    virtual void screenshot(cv::Mat &img);

protected:
    BAASConnection *connection;

    cv::Mat image;

    std::mutex screenshot_mtx;
};


#endif //BAAS_DEVICE_SCREENSHOT_BASESCREENSHOT_H_
