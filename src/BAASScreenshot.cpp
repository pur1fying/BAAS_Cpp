//
// Created by pc on 2024/5/27.
//
#include "BAASScreenshot.h"

BAASScreenshot::BAASScreenshot() {

}

BAASScreenshot::BAASScreenshot(const int Method, double interval) {
    this->screenshotMethod = Method;
    this->interval = interval;
    this->lastScreenshotTime = 0;
}

