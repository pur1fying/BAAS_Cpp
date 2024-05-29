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
    static BAASNemu* getInstance();

    bool connect(const std::string installPath,const std::string port);

    bool disconnect(const int connectionId);

    bool screenshot(const int connectionID, cv::Mat &image);

    bool getResolution(int connectionId, int displayId, std::pair<int, int> &resolution);

    bool click(int x, int y, int displayId);

private:
    struct MuMuDevice {
        std::string installPath;
        int displayId;
        int connectionId;
        std::pair<int, int> resolution;
        std::vector<unsigned char> pixels;
        cv::Mat image;
        std::mutex imageOpMutex;
        int alive;
    };
    static BAASNemu *instance;
    HINSTANCE hDllInst;
    std::map<int, MuMuDevice*> connections;     // connectionId -> connection
    BAASNemu();
    bool dllLoaded;
    nemuConnect nemu_connect;
    nemuDisconnect nemu_disconnect;
    nemuCaptureDisplay nemu_capture_display;
    nemuInputText nemu_input_text;
    nemuInputEventTouchDown nemu_input_event_touch_down;
    nemuInputEventTouchUp nemu_input_event_touch_up;
    nemuInputEventKeyDown nemu_input_event_key_down;
    nemuInputEventKeyUp nemu_input_event_key_up;
//    auto connection = nemu_connect(L"H:\\MuMuPlayer-12.0", 0);
//        if (nemu_input_event_touch_down(connection, displayId, 500, 500) != 0) {
//            break;
//        }
//        if (nemu_input_event_touch_up(connection, displayId)) {
//            break;
//        }
//    }
//    nemu_disconnect(connection);


};

#endif //BAAS_CXX_REFACTOR_BAASNEMU_H
