//
// Created by pc on 2024/8/14.
//

#ifdef _WIN32

#include "device/screenshot/LDOpenGLScreenshot.h"

BAAS_NAMESPACE_BEGIN

LDOpenGLScreenshot::LDOpenGLScreenshot(BAASConnection* connection) : BaseScreenshot(connection)
{

}

void LDOpenGLScreenshot::init()
{
    ldopengl_connection = BAASLdopengl::get_instance(connection);
    instance_id = ldopengl_connection->get_instance_id();
}

void LDOpenGLScreenshot::screenshot(cv::Mat& output)
{
    ldopengl_connection->screenshot(output);
}

void LDOpenGLScreenshot::exit()
{
    BAASLdopengl::release(instance_id);
}

bool LDOpenGLScreenshot::is_lossy()
{
    return false;
}

BAAS_NAMESPACE_END

#endif // _WIN32
