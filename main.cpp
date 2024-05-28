#include <opencv2/opencv.hpp>
#include <iostream>
#include <winsock2.h>
#include <BAAS.h>

#include <cstdint>
#include <cstring>
#pragma comment(lib, "ws2_32.lib")
using namespace cv;
using namespace std;
int main() {
    initGlobals();

//    BAASAdbDevice* d = adb.iterDevice()[1];
//    // screencap
//    Mat im;
//    string buf;
//    d->shellBytes("screencap", buf);
//    uint32_t  width , height;
//    memcpy(&width, buf.c_str(), 4);
//    memcpy(&height, buf.c_str() + 4, 4);
//    cout<<"Width : "<<width<<" Height : "<<height<<endl;
//    int channels = 4;
//    im = Mat(height, width, CV_8UC4, (void*)(buf.c_str() + 8));
//    cvtColor(im, im, COLOR_RGBA2BGR);
//    // screencap
//    return 0;
//    int max_width = 1280;
    BAASScrcpyCore::Client client = BAASScrcpyCore::Client("127.0.0.1:16384");
    client.start();
    this_thread::sleep_for(chrono::seconds(1));
    Mat image;
    client.screenshot(image);
    imshow("Image", image);
    waitKey(0);
    try {
        BAASAdbDevice* conn = adb.iterDevice()[0];
        conn->push(scrcpyJarPath, "/data/local/tmp/" + scrcpyJarName, 493,true);
        string cmd = "CLASSPATH=/data/local/tmp/scrcpy-server.jar app_process / com.genymobile.scrcpy.Server 1.20 info 0 1000000000 0 -1 true - false true 0 false false - - false";
        BAASAdbConnection* serverStream = conn->shellStream(cmd, 3);
        serverStream->readFully(10);
        BAASAdbConnection* videoStream, *controlStream;
        for (int i = 1; i <= 30; ++i) {
            cout<<"Try to connect to scrcpy server: "<<i<<endl;
            try{
                videoStream = conn->createConnection(Network::LOCAL_ABSTRACT, "scrcpy");
                break;
            }catch (AdbError &e) {
                cout<<e.what()<<endl;
                this_thread::sleep_for(chrono::milliseconds(100));
            }
        }
        char buffer[64];
        recv(videoStream->getConnection(), buffer, 1, 0);
        controlStream = conn->createConnection(Network::LOCAL_ABSTRACT, "scrcpy");
        recv(videoStream->getConnection(), buffer, 64, 0);
        cout<<"Device Name : "<<buffer<<endl;
        recv(videoStream->getConnection(), buffer, 4, 0);
        pair<int, int> resolution;
        resolution.first = BAASUtil::unsignedBinary2int(buffer, 2);
        resolution.second = BAASUtil::unsignedBinary2int(buffer + 2, 2);
        cout<<"Resolution : "<<resolution.first<<"x"<<resolution.second<<endl;
        u_long mode = 1;
        cout << "Set blocking" << ioctlsocket(videoStream->getConnection(), FIONBIO, &mode);
        AVFormatContext* formatContext = avformat_alloc_context();
        const AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_H264);
        AVCodecParserContext *parser = av_parser_init(codec->id);
        AVCodecContext *codecContext = avcodec_alloc_context3(codec);
        int ret = avcodec_open2(codecContext, codec, nullptr);
        Mat pCvMat;
        int frameCNT = 0;
        int totalFrame = 0;
        int startTime = chrono::duration_cast<chrono::seconds>(chrono::system_clock::now().time_since_epoch()).count();
        while(true) {
            char* rawH264 = new char[1<<16];
            int dataSize = recv(videoStream->getConnection(), rawH264, 1<<16, 0);
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
                    int width = frame->width, height = frame->height;
                    if(pCvMat.rows != height || pCvMat.cols != width || pCvMat.type() != CV_8UC3) {
                        pCvMat = Mat(height, width, CV_8UC3);
                    }
                    int cvLinesize[1];
                    cvLinesize[0] = pCvMat.step1();
                    SwsContext* conversion = sws_getContext(width, height, (AVPixelFormat)frame->format, width, height, AV_PIX_FMT_BGR24, SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
                    sws_scale(conversion, frame->data, frame->linesize, 0, height, &pCvMat.data, cvLinesize);
                    sws_freeContext(conversion);
                    frameCNT++;
                    int currentTime =   chrono::duration_cast<chrono::seconds>(chrono::system_clock::now().time_since_epoch()).count();
                    if(currentTime - startTime >= 1) {
                        cout<<"FPS : "<<frameCNT<<endl;
                        frameCNT = 0;
                        startTime = currentTime;
                    }
                    totalFrame++;
                    imshow("Frame", pCvMat);
                    waitKey(1);
                }
            }
        }

    } catch (AdbError &e) {
        cout<<e.what()<<endl;
    } catch (...) {
        cout<<"Unknown error"<<endl;
    }
//    BAASNemu* nemu = BAASNemu::getInstance();
//    int conn = nemu->connect("H:\\MuMuPlayer-12.0", "127.0.0.1:16384");
//    Mat image;
//    nemu->screenshot(conn, image);
//    imshow("Image", image);
//    waitKey(0);
    int mm;
    cin>>mm;
    return 0;
//    BAASUtil::executeCommandWithoutOutPut("adb forward tcp:27183 tcp:27183");
}
