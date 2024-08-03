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

typedef int (*nemuConnect)(const wchar_t* , int);
typedef void (*nemuDisconnect)(int);
typedef int (*nemuCaptureDisplay)(int, unsigned int, int, int*, int*, unsigned char*);
typedef int (*nemuInputText)(int, int, const char*);
typedef int (*nemuInputEventTouchDown)(int, int, int, int);
typedef int (*nemuInputEventTouchUp)(int, int);
typedef int (*nemuInputEventKeyDown)(int , int , int);
typedef int (*nemuInputEventKeyUp)(int, int, int);

class BAASNemu {
public:
    static BAASNemu* get_instance(BAASConnection* connection);

    explicit BAASNemu(BAASConnection* connection);

    explicit BAASNemu(std::string& installPath);

    void reconnect();

    void connect();

    bool disconnect();

    bool screenshot(cv::Mat &image);

    static int get_resolution(int connectionId, int displayId, std::pair<int, int> &resolution);

    bool click(BAASPoint point);

    bool down(BAASPoint point) const;

    bool up() const;

    void convertXY(BAASPoint &point) const;

private:
    void init_dll();

    BAASLogger* logger;

    std::string mumu_install_path;

    int instance_id = -1;

    int display_id = -1;

    int connection_id = -1;

    std::pair<int, int> resolution;

    std::vector<unsigned char> pixels;

    std::mutex imageOpMutex;

    bool alive = false;

    static std::map<int, BAASNemu*> connections;

    static HINSTANCE hDllInst;

    static bool dllLoaded;

    static nemuConnect nemu_connect;

    static nemuDisconnect nemu_disconnect;

    static nemuCaptureDisplay nemu_capture_display;

    static nemuInputText nemu_input_text;

    static nemuInputEventTouchDown nemu_input_event_touch_down;

    static nemuInputEventTouchUp nemu_input_event_touch_up;

    static nemuInputEventKeyDown nemu_input_event_key_down;

    static nemuInputEventKeyUp nemu_input_event_key_up;
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

class NemuIpcError : public std::exception {
public:
    explicit NemuIpcError(const std::string& message) : message(message) {}

    [[nodiscard]] const char* what() const noexcept override {
        return message.c_str();
    }
private:
    std::string message;
};
#endif //BAAS_DEVICE_BAASNEMU_H_
