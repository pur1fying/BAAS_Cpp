//
// Created by pc on 2024/5/22.
//

#include "BAASNemu.h"
using namespace std;

bool BAASNemu::dllLoaded = false;

nemuConnect BAASNemu::nemu_connect = nullptr;

nemuDisconnect BAASNemu::nemu_disconnect = nullptr;

nemuCaptureDisplay BAASNemu::nemu_capture_display = nullptr;

nemuInputText BAASNemu::nemu_input_text = nullptr;

nemuInputEventTouchDown BAASNemu::nemu_input_event_touch_down = nullptr;

nemuInputEventTouchUp BAASNemu::nemu_input_event_touch_up = nullptr;

nemuInputEventKeyDown BAASNemu::nemu_input_event_key_down = nullptr;

nemuInputEventKeyUp BAASNemu::nemu_input_event_key_up = nullptr;

HINSTANCE BAASNemu::hDllInst = nullptr;

std::map<int, BAASNemu*> BAASNemu::connections;

BAASNemu::BAASNemu(string &installPath) {
    if(!dllLoaded) {
        init();
        dllLoaded = true;
    }
    connect(installPath);
    getResolution(connectionId, displayId, resolution);
}

BAASNemu::BAASNemu(string &installPath, string &port) {
    if(!dllLoaded) {
        init();
        dllLoaded = true;
    }
    connect(installPath, port);
    if(!getResolution(connectionId, displayId, resolution)) {
        throw std::runtime_error("Nemu Connection Failed");
    }
}

bool BAASNemu::connect(const string& installPath,const string& serial) {
    int displayId = BAASUtil::MuMuSerialToDisplayId(serial);
    for(auto &connection: connections) {
        if(connection.second->installPath == installPath && connection.second->displayId == displayId) {
            BAASGlobalLogger->BAASError("Already connected");
            return true;
        }
    }
    if(displayId == -1) {
        BAASGlobalLogger->BAASError("Invalid serial");
        return false;
    }
    int connection = nemu_connect(BAASUtil::stringToWString(installPath).c_str(), displayId);
    if (connection == 0) {
        BAASGlobalLogger->BAASError("Nemu connect failed");
        return false;
    }
    this->installPath = installPath;
    this->displayId = displayId;
    connectionId = connection;
    connections[connection] = this;
    if(!getResolution(connection, displayId, resolution)) {
        BAASGlobalLogger->BAASError("Nemu get display resolution failed.");
        return false;
    }
    pixels.resize(resolution.first * resolution.second * 4);
    vector<string> msg = {
            "Nemu connected :   {",
            "                   Serial:\t" + serial,
            "                   MuMuPath:\t" + installPath,
            "                   DisplayID:\t" + to_string(displayId),
            "                   ConnID:\t" + to_string(connection),
            "                   Resolution:\t" + to_string(resolution.first) + "x" + to_string(resolution.second),
            "                   }"
    };
    BAASGlobalLogger->BAASInfo(msg);
    return true;
}

bool BAASNemu::disconnect() {
    if(connections.find(connectionId) == connections.end()) {
        BAASGlobalLogger->BAASError("Invalid connectionID");
        return false;
    }
    nemu_disconnect(connectionId);
    connections.erase(connectionId);
    return true;
}


bool BAASNemu::screenshot(cv::Mat &image) {
    if(nemu_capture_display(connectionId, 0, int(pixels.size()), &resolution.first, &resolution.second, pixels.data()) != 0) {
        throw std::runtime_error("Nemu capture display failed");
    }
    imageOpMutex.lock();
    image = cv::Mat(resolution.second, resolution.first, CV_8UC4, pixels.data());
    cv::cvtColor(image, image, cv::COLOR_BGRA2RGB);
    cv::flip(image, image, 0);
    imageOpMutex.unlock();
    return true;
}

bool BAASNemu::getResolution(int connectionId, int displayId, std::pair<int, int> &resolution) {
    if (nemu_capture_display(connectionId, displayId, 0, &resolution.first, &resolution.second, nullptr) != 0) {
        BAASGlobalLogger->BAASError("Nemu get resolution failed");
        return false;
    }
    return true;
}


bool BAASNemu::connect(const string &installPath) {
    return connect(installPath, "127.0.0.1:16384");
}

bool BAASNemu::click(BAASPoint point) {
    int t = BAASUtil::genRandInt(10, 20);
    down(point);
    BAASUtil::sleepMS(t);
    up();
    BAASUtil::sleepMS(50 - t);
    return true;
}

bool BAASNemu::down(BAASPoint point) const {
    convertXY(point);
    int ret = nemu_input_event_touch_down(connectionId, displayId, point.x, point.y);
    if(ret > 0)
        throw NemuIpcError("nemu_input_event_touch_down failed");
    return true;
}

void BAASNemu::convertXY(BAASPoint &point) const {
    int temp = point.x;
    point.x = resolution.second - point.y;
    point.y = temp;
}

void BAASNemu::init() {
    if(!filesystem::exists(nemuDllPath)) {
        BAASGlobalLogger->BAASError("Nemu dll not found : [ " + nemuDllPath + " ]");
        throw std::runtime_error("Nemu dll not found");
    }
    hDllInst = LoadLibrary(nemuDllPath.c_str());
    if (hDllInst == nullptr) {
        BAASGlobalLogger->BAASError("LoadLibrary : [ " + nemuDllPath + " ] failed");
        throw std::runtime_error("LoadLibrary : [ " + nemuDllPath + " ] failed");
    }
    nemu_connect = (nemuConnect)GetProcAddress(hDllInst, "nemu_connect");
    nemu_disconnect = (nemuDisconnect)GetProcAddress(hDllInst, "nemu_disconnect");
    nemu_capture_display = (nemuCaptureDisplay)GetProcAddress(hDllInst, "nemu_capture_display");
    nemu_input_text = (nemuInputText)GetProcAddress(hDllInst, "nemu_input_text");
    nemu_input_event_touch_down = (nemuInputEventTouchDown)GetProcAddress(hDllInst, "nemu_input_event_touch_down");
    nemu_input_event_touch_up = (nemuInputEventTouchUp)GetProcAddress(hDllInst, "nemu_input_event_touch_up");
    nemu_input_event_key_down = (nemuInputEventKeyDown)GetProcAddress(hDllInst, "nemu_input_event_key_down");
    nemu_input_event_key_up = (nemuInputEventKeyUp)GetProcAddress(hDllInst, "nemu_input_event_key_up");
}

bool BAASNemu::up() const {
    int ret = nemu_input_event_touch_up(connectionId, displayId);
    if(ret > 0)
        throw NemuIpcError("nemu_input_event_touch_up failed");
    return true;
}

