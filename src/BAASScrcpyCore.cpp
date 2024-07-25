//

#include "BAASScrcpyCore.h"
using namespace std;
using namespace std::filesystem;
bool BAASScrcpyCore::Client::deployServer() {
    try{
        device->push(scrcpyJarPath, "/data/local/tmp/" + scrcpyJarName, 493, true);
    }catch (AdbError &e) {
        string msg(e.what());
        BAASGlobalLogger->BAASError("Fail to push scrcpy-server : " + msg);
        return false;
    }
    vector<string> cmd = {
            "CLASSPATH=/data/local/tmp/" + scrcpyJarName,
            "app_process",
            "/",
            "com.genymobile.scrcpy.Server",
            "1.20",                                 // Server version
            "info",                                 // Log level
            fmt::format("{}", Client::maxWidth),         // Max screen width
            fmt::format("{}", Client::bitrate),          // Bit rate
            fmt::format("{}", Client::maxFPS),           // Max frame rate
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
        serverStream = device->shellStream(cmd, 3000.0);
        serverStream->readFully(10);
    }
    catch (AdbError &e) {
        string msg(e.what());
        BAASGlobalLogger->BAASError("Fail to start scrcpy server : " + msg);
        return false;
    }
    return true;
}

bool BAASScrcpyCore::Client::initServerSocketConnection() {
    BAASAdbConnection* videoStream, *controlStream;
    for (int i = 1; i <= 30; ++i) {
        cout<<"Try to connect to scrcpy server: "<<i<<endl;
        try{
            videoStream = device->createConnection(Network::LOCAL_ABSTRACT, "scrcpy");
            break;
        }catch (AdbError &e) {
            cout<<e.what()<<endl;
            this_thread::sleep_for(chrono::milliseconds(100));
        }
    }
    string buffer = videoStream->readFully(1);
    if(buffer.size() != 1 || buffer[0] != 0) {
        BAASGlobalLogger->BAASError("Invalid scrcpy server response");
        return false;
    }
    controlStream = device->createConnection(Network::LOCAL_ABSTRACT, "scrcpy");
    delete device;
    buffer = videoStream->readFully(64);
    cout<<"Device Name : "<<buffer<<endl;
    buffer = videoStream->readFully(4);
    pair<int, int> resolution;
    resolution.first = BAASUtil::unsignedBinary2int(buffer.substr(0, 2), 2);
    resolution.second = BAASUtil::unsignedBinary2int(buffer.substr(2, 2), 2);
    cout<<"Resolution : "<<resolution.first<<"x"<<resolution.second<<endl;
    u_long mode = 1;
    cout << "Set blocking" << ioctlsocket(videoStream->getConnection(), FIONBIO, &mode);
    AVFormatContext* formatContext = avformat_alloc_context();
    videoStream->setCloseSocketWhenDestruct(false);
    controlStream->setCloseSocketWhenDestruct(false);
    videoSocket = videoStream->getConnection();
    controlSocket = controlStream->getConnection();
    delete videoStream;
    delete controlStream;
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
        BAASGlobalLogger->BAASError("Fail to deploy scrcpy server.");
        return false;
    }
    if (!initServerSocketConnection()) {
        BAASGlobalLogger->BAASError("Cannot connect to scrcpy server.");
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
        while(alive) {
            char* rawH264 = new char[1<<16];
            int dataSize = recv(videoSocket, rawH264, 1<<16, 0);
            if (dataSize <= 0) {
                this_thread::sleep_for(chrono::milliseconds(1));
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
                    sws_freeContext(conversion);
                    frameMutex.unlock();
                    lastFrameTime = BAASUtil::getCurrentTimeMS();
                }
            }
        }
    } catch (AdbError &e) {
        BAASGlobalLogger->BAASError(e.what());
        return false;
    } catch (...) {
        BAASGlobalLogger->BAASError("Unknown Error");
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
        currentTime = BAASUtil::getCurrentTimeMS();
    }
//    frameMutex.lock();
    output = lastFrame.clone();
//    frameMutex.unlock();
    cout<<"lock freed";
    return true;
}

bool BAASScrcpyCore::Client::stop() {
    alive = false;
    screenshotThread.join();
    delete serverStream;
    closesocket(videoSocket);
    closesocket(controlSocket);
    return true;
}

