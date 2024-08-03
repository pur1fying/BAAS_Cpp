//
// Created by pc on 2024/8/3.
//

#include "device/screenshot/NemuScreenshot.h"

NemuScreenshot::NemuScreenshot(BAASConnection *connection) : BaseScreenshot(connection) {

}

void NemuScreenshot::init() {
    logger->hr("Screenshot Method NemuScreenshot init");
    nemu_connection = BAASNemu::get_instance(connection);
}

void NemuScreenshot::screenshot(cv::Mat &output) {
    std::lock_guard<std::mutex> lock(screenshot_mtx);
    nemu_connection->screenshot(image);
    output = image.clone();
}

void NemuScreenshot::exit() {
    logger->BAASInfo("Screenshot Method NemuScreenshot exit");
    nemu_connection->disconnect();
}

bool NemuScreenshot::is_lossy() {
    return false;
}
