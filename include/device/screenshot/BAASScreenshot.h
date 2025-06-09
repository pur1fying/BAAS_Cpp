//
// Created by pc on 2024/8/9.
//

#ifndef BAAS_DEVICE_SCREENSHOT_BAASSCREENSHOT_H_
#define BAAS_DEVICE_SCREENSHOT_BAASSCREENSHOT_H_

#include "device/screenshot/BaseScreenshot.h"

BAAS_NAMESPACE_BEGIN

class BAASScreenshot {

public:

    BAASScreenshot(
            const std::string& method,
            BAASConnection* connection,
            const double interval = 0.3
    );

    void init();

    void set_interval(double value) noexcept;

    void screenshot(cv::Mat& img);

    void immediate_screenshot(cv::Mat& img);

    void ensure_interval() const;

    void exit();

    bool is_lossy();

    void set_screenshot_method(
            const std::string& method,
            bool exit = true
    );

    double get_screen_ratio();

    [[nodiscard]] inline int get_interval() const
    {
        return interval;
    }

private:

    BaseScreenshot* screenshot_instance;

    std::string screenshot_method;

    BAASConnection* connection;

    BAASLogger* logger;

    int interval;

    long long last_screenshot_time = 0;

    const static std::set<std::string> available_methods;
};

BAAS_NAMESPACE_END
#endif //BAAS_DEVICE_SCREENSHOT_BAASSCREENSHOT_H_
