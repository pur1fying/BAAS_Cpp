//
// Created by pc on 2024/8/3.
//

#include "device/screenshot/NemuScreenshot.h"

BAAS_NAMESPACE_BEGIN

NemuScreenshot::NemuScreenshot(BAASConnection *connection) : BaseScreenshot(connection)
{

}

void NemuScreenshot::init()
{
    logger->hr("Screenshot Method NemuScreenshot init");
    nemu_connection = BAASNemu::get_instance(connection);
}

void NemuScreenshot::screenshot(cv::Mat &output)
{
    std::lock_guard<std::mutex> lock(screenshot_mtx);
    auto t1 = std::chrono::high_resolution_clock::now();
    nemu_connection->screenshot(image);
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
    logger->BAASInfo("Screenshot Time : " + std::to_string(duration) + "us");
    output = image.clone();
}

void NemuScreenshot::exit()
{
    logger->BAASInfo("Screenshot Method NemuScreenshot exit");
    nemu_connection->disconnect();
}

bool NemuScreenshot::is_lossy()
{
    return false;
}

BAAS_NAMESPACE_END
