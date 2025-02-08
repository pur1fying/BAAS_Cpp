//
// Created by pc on 2024/8/9.
//
#include "device/screenshot/BAASScreenshot.h"

#include "device/screenshot/AscreenCap.h"
#include "device/screenshot/AdbScreenshot.h"
#include "device/screenshot/ScrcpyScreenshot.h"
#include "device/screenshot/NemuScreenshot.h"
#include "device/screenshot/LdopenglScreenshot.h"

using namespace std;

vector<string> BAASScreenshot::available_methods;

BAASScreenshot::BAASScreenshot(const std::string& method, BAASConnection *connection, const double interval) {
    assert(connection != nullptr);
    this->connection = connection;
    logger = this->connection->get_logger();

    available_methods = static_config->get<std::vector<std::string>>("available_screenshot_methods");
    logger->BAASInfo("Available screenshot methods : ");
    logger->BAASInfo(available_methods);

    last_screenshot_time = BAASUtil::getCurrentTimeMS();
    screenshot_instance = nullptr;
    set_screenshot_method(method);
    set_interval(interval);
}

void BAASScreenshot::init() {
    screenshot_instance->init();
}

void BAASScreenshot::screenshot(cv::Mat &img) {
    ensure_interval();
    screenshot_instance->screenshot(img);
    last_screenshot_time = BAASUtil::getCurrentTimeMS();
}

void BAASScreenshot::immediate_screenshot(cv::Mat &img) {
    screenshot_instance->screenshot(img);
    last_screenshot_time = BAASUtil::getCurrentTimeMS();
}

void BAASScreenshot::ensure_interval() const {
    long long current_time = BAASUtil::getCurrentTimeMS();
    int difference = interval - int(current_time - last_screenshot_time);
    if (difference > 0) {
        BAASUtil::sleepMS(difference);
    }
}

void BAASScreenshot::set_interval(const double value) noexcept {
    interval = int(value * 1000);
    if(interval < 0) {
        logger->BAASWarn("Interval should be positive, set to default value 0.3");
        interval = 300;
    }
    logger->BAASInfo("Screenshot interval set to " + std::to_string(interval) + "ms");
}

void BAASScreenshot::exit() {
    screenshot_instance->exit();
}

bool BAASScreenshot::is_lossy() {
    return screenshot_instance->is_lossy();
}

void BAASScreenshot::set_screenshot_method(const std::string &method, bool exit) {
    if(std::find(available_methods.begin(), available_methods.end(), method) == available_methods.end()) {
        logger->BAASCritical("Unsupported screenshot method : [ " + method + " ]");
        throw RequestHumanTakeOver("Unsupported screenshot method: " + method);
    }

    if(method == screenshot_method) {
        logger->BAASWarn("Screenshot method already set to " + method);
        return;
    }

    if(screenshot_instance != nullptr) {
        if(exit) {
            logger->BAASInfo("Exiting current screenshot method : [" + screenshot_method + "]");
            screenshot_instance->exit();
        }
        delete screenshot_instance;
    }

    logger->BAASInfo("Screenshot method : [ " + method + " ]");
    screenshot_method = method;
    if (method == "nemu") {
        screenshot_instance = new NemuScreenshot(connection);
    } else if (method == "scrcpy") {
        screenshot_instance = new ScrcpyScreenshot(connection);
    } else if (method == "adb") {
        screenshot_instance = new AdbScreenshot(connection);
    } else if (method == "ascreencap") {
        screenshot_instance = new AScreenCap(connection);
    } else if (method == "ldopengl") {
        screenshot_instance = new LDOpenGLScreenshot(connection);
    }
    init();
}

double BAASScreenshot::get_screen_ratio() {
    double benchmark = 1280;
    cv::Mat img;
    screenshot(img);
    logger->BAASInfo("Screenshot size : " + std::to_string(img.cols) + "x" + std::to_string(img.rows));
    double long_edge = max(img.cols, img.rows);
    double short_edge = min(img.cols, img.rows);
    double ls_ratio = 9.0 / 16.0;
    if(short_edge/long_edge != ls_ratio) {
        logger->BAASWarn("Screen ratio is not 16:9");
        throw RequestHumanTakeOver("Screen ratio incorrect");
    }
    double ratio = long_edge / benchmark;
    logger->BAASInfo("Screen ratio : " + std::to_string(ratio));
    return ratio;
}




