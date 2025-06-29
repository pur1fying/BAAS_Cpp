//
// Created by pc on 2024/7/28.
//

#include "device/screenshot/ScrcpyScreenshot.h"

BAAS_NAMESPACE_BEGIN

ScrcpyScreenshot::ScrcpyScreenshot(BAASConnection* connection) : BaseScreenshot(connection)
{
    client = BAASScrcpyClient::get_client(connection);
}

void ScrcpyScreenshot::init()
{
    logger->hr("Screenshot Method ScrcpyScreenshot init.");
    client->start();
}

void ScrcpyScreenshot::screenshot(cv::Mat& img)
{
    std::lock_guard<std::mutex> lock(screenshot_mtx);
    client->screenshot(img);
}

void ScrcpyScreenshot::exit()
{
    logger->BAASInfo("ScrcpyScreenshot exit.");
    client->stop();
}

// H.264 is a lossy compression method
bool ScrcpyScreenshot::is_lossy()
{
    return true;
}

BAAS_NAMESPACE_END

