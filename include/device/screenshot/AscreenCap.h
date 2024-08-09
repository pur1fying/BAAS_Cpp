//
// Created by pc on 2024/7/26.
//

#ifndef BAAS_DEVICE_SCREENSHOT_ASCREENCAP_H_
#define BAAS_DEVICE_SCREENSHOT_ASCREENCAP_H_

#include "device/screenshot/BaseScreenshot.h"
class AScreenCap : public BaseScreenshot {
public:
    explicit AScreenCap(BAASConnection *connection);

    void init() override;

    void screenshot(cv::Mat &img) override;

    void exit() override;

    bool is_lossy() override;

private:
    void uninstall();

    static std::string shot_cmd;

    void decompress();

    BAASConnection* connection;

    std::string adb_path;

    long long byte_pointer;

    bool available;

    std::string ret_buffer;

    char* decompress_buffer;

    void reposition_byte_pointer();

    int decompress_buffer_size;
};

class AScreenCapError : public std::exception {
public:
    explicit AScreenCapError(const std::string &msg) {
        message = msg;
    }

    [[nodiscard]] const char *what() const noexcept override {
        return message.c_str();
    }
private:
    std::string message;


};


#endif //BAAS_DEVICE_SCREENSHOT_ASCREENCAP_H_
