//
// Created by pc on 2024/4/19.
//
#define WIN32_LEAN_AND_MEAN
#ifndef BAAS_CXX_REFACTOR_BAASNEMU_H
#define BAAS_CXX_REFACTOR_BAASNEMU_H
#include <Windows.h>
#include "BAASGlobals.h"
#include "opencv2/opencv.hpp"
#include <map>
#include <filesystem>
#include <mutex>
#include <thread>
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
    BAASNemu(std::string& installPath);

    BAASNemu(std::string& installPath, std::string& port);

    bool connect(const std::string& installPath);

    bool connect(const std::string& installPath,const std::string& port);

    bool disconnect();

    bool screenshot(cv::Mat &image);

    static bool getResolution(int connectionId, int displayId, std::pair<int, int> &resolution);

    bool click(BAASPoint point);

    bool down(BAASPoint point) const;

    bool up() const;

    void convertXY(BAASPoint &point) const;
private:
    std::string installPath;

    int displayId = -1;

    int connectionId = -1;

    std::pair<int, int> resolution;

    std::vector<unsigned char> pixels;

    std::mutex imageOpMutex;

    bool alive = false;

    static std::map<int, BAASNemu*> connections;

    static HINSTANCE hDllInst;

    static void init();

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

    const char* what() const noexcept override {
        return message.c_str();
    }
private:
    std::string message;
};
#endif //BAAS_CXX_REFACTOR_BAASNEMU_H
