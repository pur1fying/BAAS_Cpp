//
// Created by pc on 2024/4/17.
//

#ifndef BAAS_BAASSCRCPYCORE_H
#define BAAS_BAASSCRCPYCORE_H
#include <thread>
#include <filesystem>
#include <format>

#include "BAASAdbUtils.h"
extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}

class BAASScrcpyCore {
public:
    class Client {
    public:
        Client(std::string serial);

        Client(std::string host, std::string port);

        bool start();

        bool stop();

        bool startLoop();

        bool screenshot(cv::Mat &output);

        std::string getSerial();
    private:
        cv::Mat lastFrame;

        long long lastFrameTime = LLONG_MIN;

        bool alive = false;

        int maxWidth = 2560;

        int maxFPS = 60;

        int bitrate = 1000000000;

        bool stayAwake = false;

        std::pair<int, int> resolution;

        std::string encoderName = "";

        SOCKET videoSocket;

        SOCKET controlSocket;

        std::string host = "";

        std::string port = "";

        std::string serial = "";

        BAASAdbDevice* device = nullptr;

        BAASAdbConnection* serverStream = nullptr;

        std::mutex frameMutex;

        bool deployServer();

        bool initServerSocketConnection();

        bool screenshotLoop();

        std::thread screenshotThread;
    };
private:
    friend class Client;

};

#endif //BAAS_BAASSCRCPYCORE_H
