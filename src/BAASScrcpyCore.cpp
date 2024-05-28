//

#include "BAASScrcpyCore.h"
using namespace std;
using namespace std::filesystem;
bool BAASScrcpyCore::Client::deployServer() {
    try{
        device->push(scrcpyJarPath, "/data/local/tmp/" + scrcpyJarName, 493, true);
    }catch (AdbError &e) {
        string msg(e.what());
        BAASLoggerInstance->BAASError("Fail to push scrcpy-server : " + msg);
        return false;
    }
    vector<string> cmd = {
            "CLASSPATH=/data/local/tmp/" + scrcpyJarName,
            "app_process",
            "/",
            "com.genymobile.scrcpy.Server",
            "1.20",                                 // Server version
            "info",                                 // Log level
            format("{}", Client::maxWidth),         // Max screen width
            format("{}", Client::bitrate),          // Bit rate
            format("{}", Client::maxFPS),           // Max frame rate
            "-1",                                   // Lock video orientation
            "true",                                 // Tunnel forward
            "-",                                    // Crop screen
            "false",                                // Send frame rate to Client
            "true",                                 // Control enabled
            "0",                                    // Display id
            "false",                                // Show touches
            Client::stayAwake ? "true" : "false",   // Stay awake
            "-",                                    // Codec (video encoding) options
            "-",                                    // Encoder name
            "false"                                 // Power off screen after server closed
    };
    try{
        BAASAdbConnection* serverStream = device->shellStream(cmd, 3000.0);
        serverStream->readFully(10);
        delete serverStream;
    }
    catch (AdbError &e) {
        string msg(e.what());
        BAASLoggerInstance->BAASError("Fail to start scrcpy server : " + msg);
        return false;
    }
    return true;
}

bool BAASScrcpyCore::Client::initServerSocketConnection() {
    bool f = false;
    for (int i = 1; i <= 30; ++i) {
        BAASLoggerInstance->BAASInfo("Try to connect to scrcpy server");
        try{
            videoSocket = device->createConnection(Network::LOCAL_ABSTRACT, "scrcpy");
            f = true;
            break;
        }catch (AdbError &e) {
            BAASLoggerInstance->BAASError(e.what());
            this_thread::sleep_for(chrono::milliseconds(100));
        }
    }
    if(!f) {
        BAASLoggerInstance->BAASError("Failed to connect scrcpy-server after 3 seconds");
        return false;
    }
    string st = videoSocket->readFully(1);
    if(st.size() == 0 || int(st[0]) != 0) {
        BAASLoggerInstance->BAASError("Did not receive Dummy Byte!");
        return false;
    }
    controlSocket = device->createConnection(Network::LOCAL_ABSTRACT, "scrcpy");
    st = videoSocket->readFully(64);
    if(st.size() == 0) {
        BAASLoggerInstance->BAASError("Did not receive Device Name!");
        return false;
    }
    BAASLoggerInstance->BAASInfo("Device Name : " + st);
    st = videoSocket->readFully(4);
    Client::resolution.first = BAASUtil::unsignedBinary2int(st.substr(0, 2), 2);
    Client::resolution.second = BAASUtil::unsignedBinary2int(st.substr(2, 2), 2);
    BAASLoggerInstance->BAASInfo("Resolution : " + to_string(resolution.first) + "x" + to_string(resolution.second));
    u_long mode = 1;
    ioctlsocket(videoSocket->getConnection(), FIONBIO, &mode);
    return true;
}

//
// Created by pc on 2024/4/19.
BAASScrcpyCore::Client::Client(std::string serial) {
    this->serial = serial;
    pair<string, string> hostPort = BAASUtil::serialToHostPort(serial);
    if(hostPort.first == "" || hostPort.second == "") throw ValueError("Invalid serial : " + serial);
    this->serial = serial;
    this->host = hostPort.first;
    this->port = hostPort.second;
}

BAASScrcpyCore::Client::Client(string host, string port) {
    this->host = host;
    this->port = port;
    this->serial = host + ":" + port;
}

string BAASScrcpyCore::Client::getSerial() {
    return serial;
}

bool BAASScrcpyCore::Client::start() {
    device = new BAASAdbDevice(&adb, serial);
    if (!deployServer()) {
        BAASLoggerInstance->BAASError("Fail to deploy scrcpy server.");
        return false;
    }
    if (!initServerSocketConnection()) {
        BAASLoggerInstance->BAASError("Cannot connect to scrcpy server.");
        return false;
    }
    alive = true;
    screenshotThread = thread(&Client::screenshotLoop, this);
    return true;
}


bool BAASScrcpyCore::Client::screenshotLoop() {
    try {
        const AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_H264);
        AVCodecParserContext *parser = av_parser_init(codec->id);
        AVCodecContext *codecContext = avcodec_alloc_context3(codec);
        int ret = avcodec_open2(codecContext, codec, nullptr);
        SOCKET videoSocket = this->videoSocket->getConnection();
        while(alive) {
            char* rawH264 = new char[1<<16];
            int dataSize = recv(videoSocket, rawH264, 1<<16, 0);
            if (dataSize <= 0) {
                this_thread::sleep_for(chrono::milliseconds(100));
                continue;
            }
            while (dataSize > 0) {
                AVPacket *packet = av_packet_alloc();
                AVFrame *frame = av_frame_alloc();
                ret = av_parser_parse2(parser, codecContext, &packet->data, &packet->size, (uint8_t*)rawH264, dataSize, AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0);
                dataSize -= ret;
                rawH264 += ret;
                if(packet->size == 0) {
                    continue;
                }
                ret = avcodec_send_packet(codecContext, packet);
                if(ret != 0) {
                    cout<<"Error sending packet"<<endl;
                    continue;
                }
                while (avcodec_receive_frame(codecContext, frame) == 0) {
                    frameMutex.lock();
                    int width = frame->width, height = frame->height;
                    if(lastFrame.rows != height || lastFrame.cols != width || lastFrame.type() != CV_8UC3) {
                        lastFrame = cv::Mat(height, width, CV_8UC3);
                    }
                    int cvLinesize[1];
                    cvLinesize[0] = lastFrame.step1();
                    SwsContext* conversion = sws_getContext(width, height, (AVPixelFormat)frame->format, width, height, AV_PIX_FMT_BGR24, SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
                    sws_scale(conversion, frame->data, frame->linesize, 0, height, &lastFrame.data, cvLinesize);
                    frameMutex.unlock();
                    sws_freeContext(conversion);
                    lastFrameTime = BAASUtil::getCurrentTimeMS();
                }
            }
        }
    } catch (AdbError &e) {
        cout<<e.what()<<endl;
        return false;
    } catch (...) {
        cout<<"Unknown error"<<endl;
        return false;
    }
    return true;
}

bool BAASScrcpyCore::Client::screenshot(cv::Mat &output) {
    if(!alive) throw RuntimeError("Scrcpy Client is not alive");
    long long currentTime = BAASUtil::getCurrentTimeMS();
    while (lastFrameTime < currentTime) {
        if(!alive)throw RuntimeError("Scrcpy Client is not alive");
        BAASUtil::sleepMS(1);
    }
    frameMutex.lock();
    output = lastFrame.clone();
    frameMutex.unlock();
    return true;
}

bool BAASScrcpyCore::Client::stop() {
    alive = false;
    screenshotThread.join();
    delete videoSocket;
    delete controlSocket;
    return true;
}

