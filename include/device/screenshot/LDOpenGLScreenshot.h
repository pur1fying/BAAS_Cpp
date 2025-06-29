//
// Created by pc on 2024/8/14.
//

#ifndef BAAS_DEVICE_LDOPENGLSCREENSHOT_H_
#define BAAS_DEVICE_LDOPENGLSCREENSHOT_H_

#include "device/BAASLdopengl.h"
#include "device/screenshot/BaseScreenshot.h"

BAAS_NAMESPACE_BEGIN

class LDOpenGLScreenshot : public BaseScreenshot {

public:

    explicit LDOpenGLScreenshot(BAASConnection* connection);

    void init() override;

    void screenshot(cv::Mat& output) override;

    void exit() override;

    bool is_lossy() override;

private:

    BAASLdopengl* ldopengl_connection;

    int instance_id;
};

BAAS_NAMESPACE_END


#endif //BAAS_DEVICE_LDOPENGLSCREENSHOT_H_
