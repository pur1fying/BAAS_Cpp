//
// Created by pc on 2024/4/19.
//
#ifndef BAAS_DEVICE_BAASNEMU_H_
#define BAAS_DEVICE_BAASNEMU_H_

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <map>
#include <filesystem>
#include <mutex>
#include <thread>

#include "opencv2/opencv.hpp"

#include "BAASGlobals.h"
#include "BAASConnection.h"
#include "BAASImageUtil.h"

typedef int (*nemuConnect)(
        const wchar_t *,
        int
);

typedef void (*nemuDisconnect)(int);

typedef int (*nemuCaptureDisplay)(
        int,
        unsigned int,
        int,
        int *,
        int *,
        unsigned char *
);

typedef int (*nemuInputText)(
        int,
        int,
        const char *
);

typedef int (*nemuInputEventTouchDown)(
        int,
        int,
        int,
        int
);

typedef int (*nemuInputEventTouchUp)(
        int,
        int
);

typedef int (*nemuInputEventKeyDown)(
        int,
        int,
        int
);

typedef int (*nemuInputEventKeyUp)(
        int,
        int,
        int
);

BAAS_NAMESPACE_BEGIN

class NemuIpcError : public std::exception {
public:
    explicit NemuIpcError(const std::string &message) : message(message) {}

    [[nodiscard]] const char *what() const noexcept override
    {
        return message.c_str();
    }

private:
    std::string message;
};

class BAASNemu {
public:
    static BAASNemu *get_instance(BAASConnection *connection);

    static void release(int connectionId);

    explicit BAASNemu(BAASConnection *connection);

    explicit BAASNemu(std::string &installPath);

    void reconnect();

    void connect();

    void disconnect();

    void screenshot(cv::Mat &image);

    int get_resolution(
            int connectionId,
            int displayId,
            std::pair<int, int> &resolution
    );

    void click(
            int x,
            int y
    );

    void click(BAASPoint point);

    void long_click(
            int x,
            int y,
            double duration
    );

    void long_click(
            BAASPoint point,
            double duration
    );

    void swipe(
            int start_x,
            int start_y,
            int end_x,
            int end_y,
            int step_len = 5,
            double step_duration = 0.005
    );

    void down(
            int x,
            int y
    ) const;

    void down(BAASPoint point) const;

    void up() const;

    void convertXY(
            int &x,
            int &y
    ) const;

    void convertXY(BAASPoint &point) const;

    void update_resolution();

    inline std::pair<int, int> get_screen_resolution() const
    {
        return resolution;
    }

private:
    void init_dll();

    BAASLogger *logger;

    std::string mumu_install_path;

    int instance_id = -1;

    int display_id = -1;

    int connection_id = -1;

    std::pair<int, int> resolution;

    std::vector<unsigned char> pixels;

    bool alive = false;

    static std::map<int, BAASNemu *> connections;

    HINSTANCE hDllInst;

    nemuConnect nemu_connect;

    nemuDisconnect nemu_disconnect;

    nemuCaptureDisplay nemu_capture_display;

    nemuInputText nemu_input_text;

    nemuInputEventTouchDown nemu_input_event_touch_down;

    nemuInputEventTouchUp nemu_input_event_touch_up;

    nemuInputEventKeyDown nemu_input_event_key_down;

    nemuInputEventKeyUp nemu_input_event_key_up;
    //
    //    auto connection = nemu_connect(L"H:\\MuMuPlayer-12.0", 0);
    //        if (nemu_input_event_touch_down(connection, displayId, 500, 500) != 0) {
    //            break;
    //        }
    //        if (nemu_input_event_touch_up(connection, displayId)) {
    //            break;
    //        }
    //    }
    //    nemu_disconnect(connection);

    /*
    Contact down, continuous contact down will be considered as swipe
    */
};

BAAS_NAMESPACE_END


#endif //BAAS_DEVICE_BAASNEMU_H_
