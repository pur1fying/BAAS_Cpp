#include <opencv2/opencv.hpp>
#include <iostream>
#include <winsock2.h>
#include "BAAS.h"

#include <cstdint>
#include <cstring>
#pragma comment(lib, "ws2_32.lib")
using namespace cv;
using namespace std;
int main() {
    initGlobals();
    if(filesystem::exists("output")){
        filesystem::remove_all("output");
    }
    filesystem::create_directory("output");

    //adb.connect("127.0.0.1:16384");
    //BAASScrcpyCore::Client client("127.0.0.1:16384");
    //client.start();
    //Mat im;
    //client.screenshot(im);
    //cout<<im.cols<<" "<<im.rows<<endl;
    //imwrite("output\\im.png", im);
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
//    adb.connect("127.0.0.1:16512");
//    BAASScrcpyCore::Client client = BAASScrcpyCore::Client("127.0.0.1:16512");
    BAASImageResource IconResource;
    BAASDevelopUtils::getThreeStudentSkillScreenshot("1", "2", "3");
    int mm;
    cin>>mm;
    return 0;
//    BAASUtil::executeCommandWithoutOutPut("adb forward tcp:27183 tcp:27183");
}
/*
    y -->   601 ,  662
    x -->   847 , 932
            946, 1038
            1048, 1130
*/