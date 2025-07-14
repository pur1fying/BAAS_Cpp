//
// Created by pc on 2024/8/3.
//

#ifdef _WIN32

#include "device/screenshot/NemuScreenshot.h"

BAAS_NAMESPACE_BEGIN

NemuScreenshot::NemuScreenshot(BAASConnection* connection) : BaseScreenshot(connection)
{

}

void NemuScreenshot::init()
{
    logger->hr("Screenshot Method NemuScreenshot init");
    nemu_connection = BAASNemu::get_instance(connection);
}

void NemuScreenshot::screenshot(cv::Mat& output)
{
    std::lock_guard<std::mutex> lock(screenshot_mtx);
    nemu_connection->screenshot(image);
    output = image.clone();
}

void NemuScreenshot::exit()
{
    logger->BAASInfo("Screenshot Method NemuScreenshot exit");
    // maybe same instance nemu control still needs it, just try release, do not disconnect
    BAASNemu::try_release(nemu_connection, true);
}

bool NemuScreenshot::is_lossy()
{
    return false;
}

BAAS_NAMESPACE_END

#endif // _WIN32