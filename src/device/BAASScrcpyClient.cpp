//
// Created by pc on 2024/4/19.



#include "device/BAASScrcpyClient.h"

using namespace std;
using namespace std::filesystem;

map<BAASConnection*, BAASScrcpyClient*> BAASScrcpyClient::clients;

BAASScrcpyClient::BAASScrcpyClient(BAASConnection *connection) {
    this->connection = connection;
    logger = connection->get_logger();
}

bool BAASScrcpyClient::deploy_server() {
    try{
        connection->adb_push(scrcpyJarPath, "/data/local/tmp/" + scrcpyJarName);
    }catch (AdbError &e) {
        string msg(e.what());
        logger->BAASError("Fail to push scrcpy-server : " + msg);
        return false;
    }
    vector<string> cmd = {
            "CLASSPATH=/data/local/tmp/" + scrcpyJarName,
            "app_process",
            "/",
            "com.genymobile.scrcpy.Server",
            "1.20",                                 // Server version
            "info",                                 // Log level
            fmt::format("{}", maxWidth),            // Max screen width
            fmt::format("{}", bitrate),             // Bit rate
            fmt::format("{}", maxFPS),              // Max frame rate
            "-1",                                   // Lock video orientation
            "true",                                 // Tunnel forward
            "-",                                    // Crop screen
            "false",                                // Send frame rate to Client
            "true",                                 // Control enabled
            "0",                                    // Display id
            "false",                                // Show touches
            stayAwake ? "true" : "false",           // Stay awake
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
        logger->BAASError("Fail to start scrcpy server : " + msg);
        return false;
    }
    return true;
}

bool BAASScrcpyClient::init_socket() {
    BAASAdbConnection *video_stream, *control_stream;
    logger->BAASInfo("-- Try to connect to scrcpy server --");
    for (int i = 1; i <= 30; ++i) {
        try{
            video_stream = device->createConnection(Network::LOCAL_ABSTRACT, "scrcpy");
            break;
        }catch (AdbError &e) {
            logger->BAASInfo(e.what());
            this_thread::sleep_for(chrono::milliseconds(100));
        }
    }
    if(video_stream == nullptr) {
        logger->BAASError("Can't connect to Scrcpy server after 30 attempts");
        throw ScrcpyError("Server Connect Error");
    }
    string buffer = video_stream->readFully(1);
    if(buffer.size() != 1 || buffer[0] != 0) {
        logger->BAASError("Invalid scrcpy server response");
        delete video_stream;
        return false;
    }
    control_stream = device->createConnection(Network::LOCAL_ABSTRACT, "scrcpy");
    delete device;
    buffer = video_stream->readFully(64);
    logger->BAASInfo("Device name : " + buffer);
    buffer = video_stream->readFully(4);

    resolution.first = BAASUtil::unsignedBinary2int(buffer.substr(0, 2), 2);
    resolution.second = BAASUtil::unsignedBinary2int(buffer.substr(2, 2), 2);

    logger->BAASInfo("Resolution : " + to_string(resolution.first) + "x" + to_string(resolution.second));
    u_long mode = 1;

    logger->BAASInfo("Set video socket blocking : " + to_string(ioctlsocket(video_stream->getConnection(), FIONBIO, &mode)));

    video_stream->setCloseSocketWhenDestruct(false);
    control_stream->setCloseSocketWhenDestruct(false);

    videoSocket = video_stream->getConnection();
    controlSocket = control_stream->getConnection();
    delete video_stream;
    delete control_stream;

    return true;
}



bool BAASScrcpyClient::screenshot_loop() {
    try {
        ffmpeg_init();
        int ret;
        while(get_alive()) {
            rawH264 = ret_buffer;
            int dataSize = recv(videoSocket, rawH264, 1<<16, 0);
            if (dataSize <= 0) {
                this_thread::sleep_for(chrono::milliseconds(1));
                continue;
            }
            while (dataSize > 0) {
                if(packet == nullptr || frame == nullptr) {
                    ffmpeg_release_resource();
                    throw ScrcpyError("Cannot allocate packet or frame");
                }
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
                    frame_mutex.lock();
                    int width = frame->width, height = frame->height;
                    if(last_frame.rows != height || last_frame.cols != width || last_frame.type() != CV_8UC3) {
                        last_frame = cv::Mat(height, width, CV_8UC3);
                    }
                    int cv_line_size[1];
                    cv_line_size[0] = int(last_frame.step1());
                    SwsContext* conversion = sws_getContext(width, height, (AVPixelFormat)frame->format, width, height, AV_PIX_FMT_BGR24, SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
                    if(conversion == nullptr) {
                        ffmpeg_release_resource();
                        throw ScrcpyError("Cannot create SwsContext");
                    }
                    sws_scale(conversion, frame->data, frame->linesize, 0, height, &last_frame.data, cv_line_size);
                    sws_freeContext(conversion);
                    frame_mutex.unlock();
                    set_last_frame_arrive_time();
                }
            }
        }
        ffmpeg_release_resource();
    } catch (AdbError &e) {
        logger->BAASError(e.what());
        return false;
    }catch(ScrcpyError &e) {
        logger->BAASError(e.what());
        return false;
    }
    catch (...) {
        logger->BAASError("Unknown Error");
        return false;
    }
    return true;
}

bool BAASScrcpyClient::screenshot(cv::Mat &output) {
    long long currentTime = BAASUtil::getCurrentTimeMS();

    while (get_last_frame_arrive_time() < currentTime) {
        if(!alive)throw RuntimeError("Scrcpy Client is not alive");
        BAASUtil::sleepMS(1);
    }

    frame_mutex.lock();
    output = last_frame.clone();
    frame_mutex.unlock();
    return true;
}

bool BAASScrcpyClient::start() {
    device = connection->adb_device();
    if (!deploy_server()) {
        logger->BAASError("Fail to deploy scrcpy server.");
        return false;
    }
    if (!init_socket()) {
        logger->BAASError("Cannot connect to scrcpy server.");
        return false;
    }

    set_alive(true);

    screenshotThread = thread(&BAASScrcpyClient::screenshot_loop, this);
    return true;
}

bool BAASScrcpyClient::stop() {
    set_alive(false);
    screenshotThread.join();
    delete serverStream;
    closesocket(videoSocket);
    closesocket(controlSocket);
    return true;
}

BAASScrcpyClient *BAASScrcpyClient::get_client(BAASConnection *connection) {
    if(clients.find(connection) == clients.end()) {
        clients[connection] = new BAASScrcpyClient(connection);
    }
    return clients[connection];
}

void BAASScrcpyClient::release_client(BAASConnection *connection) {
    auto it = clients.find(connection);
    if(it != clients.end()) {
        it->second->stop();
        delete it->second;
        clients.erase(connection);
    }
}




