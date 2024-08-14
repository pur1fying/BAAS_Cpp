//
// Created by pc on 2024/8/9.
//

#ifndef BAAS_DEVICE_SCREENSHOT_BAASSCREENSHOT_H_
#define BAAS_DEVICE_SCREENSHOT_BAASSCREENSHOT_H_
#include "device/screenshot/BaseScreenshot.h"

class BAASScreenshot {
public:
    BAASScreenshot(const std::string &method, BAASConnection *connection, const double interval = 0.3);

    void init();

    void set_interval(const double value) noexcept;

    void screenshot(cv::Mat &img);

    void ensure_interval() const;

    void exit();

    bool is_lossy();

    void set_screenshot_method(const std::string& method, bool exit = true);

    double get_screen_ratio();

private:
    BaseScreenshot* screenshot_instance;

    std::string screenshot_method;

    BAASConnection* connection;

    BAASLogger* logger;

    int interval;

    long long last_screenshot_time = 0;

    static std::vector<std::string> available_methods;
};

#endif //BAAS_DEVICE_SCREENSHOT_BAASSCREENSHOT_H_
