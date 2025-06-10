//
// Created by pc on 2024/8/9.
//

#include "device/screenshot/BAASScreenshot.h"

#include "config/BAASStaticConfig.h"
#include "device/screenshot/AscreenCap.h"
#include "device/screenshot/AdbScreenshot.h"
#include "device/screenshot/ScrcpyScreenshot.h"
#ifdef _WIN32
#include "device/screenshot/NemuScreenshot.h"
#include "device/screenshot/LdopenglScreenshot.h"
#endif // _WIN32

using namespace std;

BAAS_NAMESPACE_BEGIN
const set<string> BAASScreenshot::available_methods = {
         "scrcpy"
        ,"adb"
        ,"ascreencap"
#ifdef _WIN32
        ,"nemu"
        ,"ldopengl"
#endif // _WIN32
};

BAASScreenshot::BAASScreenshot(
        const std::string& method,
        BAASConnection* connection,
        const double interval
)
{
    assert(connection != nullptr);
    this->connection = connection;
    logger = this->connection->get_logger();

    logger->BAASInfo("Available screenshot methods : ");
    int cnt = 0;
    for (const auto& m: available_methods) {
        logger->BAASInfo("[" + to_string(++cnt) + "] : " + m);
    }

    last_screenshot_time = BAASChronoUtil::getCurrentTimeMS();
    screenshot_instance = nullptr;
    set_screenshot_method(method);
    set_interval(interval);
}

void BAASScreenshot::init()
{
    screenshot_instance->init();
}

void BAASScreenshot::screenshot(cv::Mat& img)
{
    ensure_interval();
    screenshot_instance->screenshot(img);
    last_screenshot_time = BAASChronoUtil::getCurrentTimeMS();
}

void BAASScreenshot::immediate_screenshot(cv::Mat& img)
{
    screenshot_instance->screenshot(img);
}

void BAASScreenshot::ensure_interval() const
{
    long long current_time = BAASChronoUtil::getCurrentTimeMS();
    int difference = interval - int(current_time - last_screenshot_time);
    if (difference > 0) {
        BAASChronoUtil::sleepMS(difference);
    }
}

void BAASScreenshot::set_interval(const double value) noexcept
{
    interval = int(value * 1000);
    if (interval < 0) {
        logger->BAASWarn("Interval should be positive, set to default value 0.3");
        interval = 300;
    }
    logger->BAASInfo("Screenshot interval set to " + std::to_string(interval) + "ms");
}

void BAASScreenshot::exit()
{
    screenshot_instance->exit();
}

bool BAASScreenshot::is_lossy()
{
    return screenshot_instance->is_lossy();
}

void BAASScreenshot::set_screenshot_method(
        const std::string& method,
        bool exit
)
{
    if (available_methods.find(method) == available_methods.end()) {
        logger->BAASCritical("Unsupported screenshot method : [ " + method + " ]");
        throw RequestHumanTakeOver("Unsupported screenshot method: " + method);
    }

    if (method == screenshot_method) {
        logger->BAASWarn("Screenshot method already set to " + method);
        return;
    }

    if (screenshot_instance != nullptr) {
        if (exit) {
            logger->BAASInfo("Exiting current screenshot method : [" + screenshot_method + "]");
            screenshot_instance->exit();
        }
        delete screenshot_instance;
    }

    logger->BAASInfo("Screenshot method : [ " + method + " ]");
    screenshot_method = method;
    if (method == "ascreencap") {
        screenshot_instance = new AScreenCap(connection);
    }
    else if (method == "scrcpy") {
        screenshot_instance = new ScrcpyScreenshot(connection);
    }
    else if (method == "adb") {
        screenshot_instance = new AdbScreenshot(connection);
    }
#ifdef _WIN32
    else if (method == "nemu") {
        screenshot_instance = new NemuScreenshot(connection);
    }
    else if (method == "ldopengl") {
        screenshot_instance = new LDOpenGLScreenshot(connection);
    }
#endif // _WIN32
    init();
}

double BAASScreenshot::get_screen_ratio()
{
    double benchmark = 1280;
    cv::Mat img;
    screenshot(img);
    logger->BAASInfo("Screenshot size : " + std::to_string(img.cols) + "x" + std::to_string(img.rows));
    double long_edge = max(img.cols, img.rows);
    double short_edge = min(img.cols, img.rows);
    double ls_ratio = 9.0 / 16.0;
    if (short_edge / long_edge != ls_ratio) {
        logger->BAASWarn("Screen ratio is not 16:9");
        throw RequestHumanTakeOver("Screen ratio incorrect");
    }
    double ratio = long_edge / benchmark;
    logger->BAASInfo("Screen ratio : " + std::to_string(ratio));
    return ratio;
}

BAAS_NAMESPACE_END
