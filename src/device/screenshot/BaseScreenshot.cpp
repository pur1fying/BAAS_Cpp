//
// Created by pc on 2024/7/26.
//

#include "device/screenshot/BaseScreenshot.h"

BaseScreenshot::BaseScreenshot(BAASConnection *connection) {
    this->connection = connection;
    logger = connection->get_logger();
}

void BaseScreenshot::init() {
    throw std::runtime_error("Base screenshot class init function should not be called.");
}

void BaseScreenshot::screenshot(cv::Mat &img) {
    throw std::runtime_error("Base screenshot class screenshot function should not be called.");
}

void BaseScreenshot::exit() {
    throw std::runtime_error("Base screenshot class exit function should not be called.");
}

bool BaseScreenshot::is_lossy() {
    throw std::runtime_error("Base screenshot class is_lossy function should not be called.");
}
