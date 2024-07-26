//
// Created by pc on 2024/7/26.
//

#include "device/screenshot/BaseScreenshot.h"

BaseScreenshot::BaseScreenshot(BAASConnection *connection) {
    this->connection = connection;
}

void BaseScreenshot::init() {
    throw std::runtime_error("Base screenshot class init function should not be called.");
}

void BaseScreenshot::screenshot(cv::Mat &img) {
    throw std::runtime_error("Base screenshot class screenshot function should not be called.");
}
