//
// Created by pc on 2024/7/27.
//

#ifndef BAAS_DEVICE_BAASSCRCPYCLIENT_H_
#define BAAS_DEVICE_BAASSCRCPYCLIENT_H_

#include <thread>
#include <filesystem>
#include <map>

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
}

#include "BAASConnection.h"


class BAASScrcpyClient {
public:
    static BAASScrcpyClient *get_client(BAASConnection *connection);

    static void release_client(BAASConnection *connection);

    bool start();

    bool stop();

    bool screenshot(cv::Mat &output);

    inline long long get_last_frame_arrive_time() {
        std::lock_guard<std::mutex> lock(time_mutex);
        return last_frame_arrive_time;
    }

    inline void set_last_frame_arrive_time() {
        std::lock_guard<std::mutex> lock(time_mutex);
        last_frame_arrive_time = BAASUtil::getCurrentTimeMS();
    }
private:
    inline bool ffmpeg_init() {
        codec = avcodec_find_decoder(AV_CODEC_ID_H264);
        parser = av_parser_init(codec->id);
        codecContext = avcodec_alloc_context3(codec);
        packet = av_packet_alloc();
        frame = av_frame_alloc();
        avcodec_open2(codecContext, codec, nullptr);

        return true;
    }

    inline bool ffmpeg_release_resource() {
        av_parser_close(parser);
        avcodec_free_context(&codecContext);
        av_packet_free(&packet);
        av_frame_free(&frame);

        return true;
    }

    inline void set_alive(bool state) {
        std::lock_guard<std::mutex> lock(alive_mutex);
        alive = state;
    }

    inline bool get_alive() {
        std::lock_guard<std::mutex> lock(alive_mutex);
        return alive;
    }

    explicit BAASScrcpyClient(BAASConnection *connection);

    static std::map<BAASConnection* , BAASScrcpyClient*> clients;

    BAASLogger *logger;

    cv::Mat last_frame;

    std::mutex frame_mutex;

    long long last_frame_arrive_time = LLONG_MIN;

    std::mutex time_mutex;

    bool alive = false;

    std::mutex alive_mutex;

    int maxWidth = 2560;

    int maxFPS = 60;

    int bitrate = 1000000000;

    bool stayAwake = false;

    std::pair<int, int> resolution;

    std::string encoderName;

    SOCKET videoSocket;

    SOCKET controlSocket;

    BAASConnection *connection;

    BAASAdbConnection* serverStream;

    bool deploy_server();

    bool init_socket();

    bool screenshot_loop();

    char ret_buffer[1 << 16];

    BAASAdbDevice* device = nullptr;

    char* rawH264;

    std::thread screenshotThread;

    const AVCodec *codec;

    AVCodecParserContext *parser;

    AVCodecContext *codecContext;

    AVPacket *packet;

    AVFrame *frame;

};


class ScrcpyError : public std::exception {
public:
    ScrcpyError() = default;

    explicit ScrcpyError(const char *msg) {
        message = msg;
    }

    [[nodiscard]] const char *what() const noexcept override {
        if (message.empty()) return "Scrcpy Error";
        return message.c_str();
    }

private:
    std::string message;
};

#endif //BAAS_DEVICE_BAASSCRCPYCLIENT_H_
