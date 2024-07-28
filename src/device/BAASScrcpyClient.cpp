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

// server stream
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
        logger->BAASInfo("Create Server Stream.");
        serverStream = device->shellStream(cmd, 3000.0);
        string ret = serverStream->readFully(10);
        logger->BAASInfo("Server response : " + ret);
        if(ret.find("Aborted") != string::npos)throw ScrcpyError("Aborted");
        else if(ret.find("[server] E") != string::npos) {
            string ret_err;
            serverStream->readUntilClose(ret_err);
            logger->BAASError(ret);
            if(ret.find("match the client") != string::npos)throw ScrcpyError("Server version does not match the client.");
            else throw ScrcpyError("Unknown Server Error");
        }
    }
    catch (AdbError &e) {
        string msg(e.what());
        logger->BAASError("Fail to start scrcpy server : " + msg);
        return false;
    }
    return true;
}

// control socket and video socket
bool BAASScrcpyClient::init_socket() {
    BAASAdbConnection *video_stream, *control_stream;
    logger->BAASInfo("Create Video Stream.");
    for (int i = 1; i <= 30; ++i) {
        try{
            video_stream = device->createConnection(Network::LOCAL_ABSTRACT, "scrcpy");
            break;
        }catch (AdbError &e) {
            video_stream = nullptr;
            logger->BAASInfo(e.what());
            BAASUtil::sleepMS(100);
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

    logger->BAASInfo("Create Control Stream.");
    control_stream = device->createConnection(Network::LOCAL_ABSTRACT, "scrcpy");

    delete device;
    buffer = video_stream->readFully(64);
    logger->BAASInfo("Device name : " + buffer);
    buffer = video_stream->readFully(4);

    set_resolution(uint16_t(BAASUtil::unsignedBinary2int(buffer.substr(0, 2), 2)),
                   uint16_t(BAASUtil::unsignedBinary2int(buffer.substr(2, 2), 2)));

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

        pair<uint16_t, uint16_t> resol;

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
                    resol = get_resolution();
                    if(uint16_t(width) != resol.first || uint16_t(height) != resol.second) {
                        logger->BAASInfo("Device Resolution changed : " + to_string(resol.first) + "x" + to_string(resol.second) + " --> " + to_string(width) + "x" + to_string(height));
                        set_resolution(uint16_t(width), uint16_t(height));
                    }
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
    if(get_alive()) return true;

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

    if(screenshotThread.joinable())screenshotThread.join();

    delete serverStream;

    if(videoSocket != INVALID_SOCKET)
        closesocket(videoSocket);

    if(controlSocket != INVALID_SOCKET)
        closesocket(controlSocket);
    logger->BAASInfo("Scrcpy Client stopped.");
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


void BAASScrcpyClient::touch(int x, int y, uint8_t action, unsigned long long touch_id)  {
    pair<uint16_t , uint16_t> resol = get_resolution();

    if(x < 0 || y < 0 || x >= resol.first || y >= resol.second) {
        logger->BAASError("Scrcpy Touch out of screen : ( " + to_string(x) + ", " + to_string(y) + " )");
        return;
    }
    uint8_t inject = ScrcpyConst::TYPE_INJECT_TOUCH_EVENT;
    uint16_t cst1 = 0xFFFF;
    uint32_t cst2 = 0x1;
    string msg;
    BAASUtil::append_big_endian(msg, inject);
    BAASUtil::append_big_endian(msg, action);
    BAASUtil::append_big_endian(msg, touch_id);
    BAASUtil::append_big_endian(msg, x);
    BAASUtil::append_big_endian(msg, y);
    BAASUtil::append_big_endian(msg, resol.first);
    BAASUtil::append_big_endian(msg, resol.second);
    BAASUtil::append_big_endian(msg, cst1);
    BAASUtil::append_big_endian(msg, cst2);

    control_socket_send(msg);
}

void BAASScrcpyClient::keycode(int keycode, uint8_t action, int repeat)  {
    uint8_t inject = ScrcpyConst::TYPE_INJECT_KEYCODE;
    uint32_t cst = 0;

    string msg;
    BAASUtil::append_big_endian(msg, inject);
    BAASUtil::append_big_endian(msg, action);
    BAASUtil::append_big_endian(msg, keycode);
    BAASUtil::append_big_endian(msg, repeat);
    BAASUtil::append_big_endian(msg, cst);

    control_socket_send(msg);
}

void BAASScrcpyClient::text(const string &text)  {
    uint8_t inject = ScrcpyConst::TYPE_INJECT_TEXT;

    string msg;
    BAASUtil::append_big_endian(msg, inject);
    BAASUtil::append_big_endian(msg, int(text.size()));

    msg += text;

    control_socket_send(msg);
}

void BAASScrcpyClient::scroll(int x, int y, int h, int v) {
    uint8_t inject = ScrcpyConst::TYPE_INJECT_SCROLL_EVENT;

    pair<uint16_t , uint16_t> resol = get_resolution();

    string msg;
    BAASUtil::append_big_endian(msg, inject);
    BAASUtil::append_big_endian(msg, x);
    BAASUtil::append_big_endian(msg, y);
    BAASUtil::append_big_endian(msg, resol.first);
    BAASUtil::append_big_endian(msg, resol.second);
    BAASUtil::append_big_endian(msg, h);
    BAASUtil::append_big_endian(msg, v);

    control_socket_send(msg);
}

void BAASScrcpyClient::back_or_turn_screen_on(uint8_t action) {
    uint8_t inject = ScrcpyConst::TYPE_BACK_OR_SCREEN_ON;

    string msg;
    BAASUtil::append_big_endian(msg, inject);
    BAASUtil::append_big_endian(msg, action);

    control_socket_send(msg);
}

void BAASScrcpyClient::expand_notification_panel() {
    uint8_t inject = ScrcpyConst::TYPE_EXPAND_NOTIFICATION_PANEL;

    string msg;
    BAASUtil::append_big_endian(msg, inject);

    control_socket_send(msg);
}

void BAASScrcpyClient::expand_settings_panel() {
    uint8_t inject = ScrcpyConst::TYPE_EXPAND_SETTINGS_PANEL;

    string msg;
    BAASUtil::append_big_endian(msg, inject);

    control_socket_send(msg);
}

void BAASScrcpyClient::collapse_panels() {
    uint8_t inject = ScrcpyConst::TYPE_COLLAPSE_PANELS;

    string msg;
    BAASUtil::append_big_endian(msg, inject);

    control_socket_send(msg);
}

std::string BAASScrcpyClient::get_clipboard() {
    // clear control socket
    u_long mode = 1;
    ioctlsocket(controlSocket, FIONBIO, &mode);
    char buffer[1024];
    int len;
    while(true) {
        len = recv(controlSocket, buffer, sizeof(buffer), 0);
        if (len == SOCKET_ERROR) {
            int error = WSAGetLastError();
            if (error == WSAEWOULDBLOCK || error == WSAENOTCONN) {
                break;
            }
        }
    }
    mode = 0;
    ioctlsocket(controlSocket, FIONBIO, &mode);

    // get clipboard
    uint8_t inject = ScrcpyConst::TYPE_GET_CLIPBOARD;
    string msg;
    BAASUtil::append_big_endian(msg, inject);

    control_socket_send(msg);

    recv(controlSocket, buffer, 1, 0);
    assert(buffer[0] == 0);
    recv(controlSocket, buffer, 4, 0);
    int size = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];
    recv(controlSocket, buffer, size, 0);
    msg = string(buffer, size);
    return msg;
}

void BAASScrcpyClient::set_clipboard(const string &text, const bool paste) {
    uint8_t inject = ScrcpyConst::TYPE_SET_CLIPBOARD;
    uint8_t paste_flag = paste ? 1 : 0;

    string msg;
    BAASUtil::append_big_endian(msg, inject);
    BAASUtil::append_big_endian(msg, paste_flag);
    BAASUtil::append_big_endian(msg, int(text.size()));
    msg += text;

    control_socket_send(msg);
}


void BAASScrcpyClient::set_screen_power_mode(int8_t mode) {
    uint8_t inject = ScrcpyConst::TYPE_SET_SCREEN_POWER_MODE;

    string msg;
    BAASUtil::append_big_endian(msg, inject);
    BAASUtil::append_big_endian(msg, mode);

    control_socket_send(msg);
}

void BAASScrcpyClient::rotate_device() {
    uint8_t inject = ScrcpyConst::TYPE_ROTATE_DEVICE;

    string msg;
    BAASUtil::append_big_endian(msg, inject);

    control_socket_send(msg);
}





