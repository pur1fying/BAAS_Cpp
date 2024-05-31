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

//    BAASDevelopUtils::shotStudentSkill("a", SKILL1_LEFT, SKILL_LEFT);
//    BAASDevelopUtils::shotStudentSkill("b", SKILL2_LEFT, SKILL_LEFT);
//    BAASDevelopUtils::shotStudentSkill("c", SKILL3_LEFT, SKILL_LEFT);
//    BAASDevelopUtils::shotStudentSkill("d", SKILL1_RIGHT, SKILL_RIGHT);
//    BAASDevelopUtils::shotStudentSkill("e", SKILL2_RIGHT, SKILL_RIGHT);
//    BAASDevelopUtils::shotStudentSkill("f", SKILL3_RIGHT, SKILL_RIGHT);

//    return 0;
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
    BAASAutoFight autoFight;
    autoFight.procedure = {
            {"Akane", BOSS_POSITION, 6.5},
            {"Ako", {380, 235}, 9.5},
            {"Maki", BOSS_POSITION, 6.5},
            {"Chinatsu (Hot Spring)", {380, 235}, 2.0},
            {"Akane", BOSS_POSITION, 7.5},
            {"Ako", {380, 235}, 9.0},
            {"Maki", BOSS_POSITION, 6.5},
            {"Karin", BOSS_POSITION, 4.1},
            {"Ako", {985,  621}, 9.5},
            {"Maki", BOSS_POSITION, 6.5},
            {"Chinatsu (Hot Spring)", {985,  621}, 2},
            {"Akane", BOSS_POSITION, 3},
            {"Ako", {985,  621}, 5.5},
            {"Karin", BOSS_POSITION, 7.5},
            {"Maki", BOSS_POSITION, 5},
    };
    BAASImageResource SkillIconResource;
    SkillIconResource.loadDirectoryImage("resource/image/CN/skill_icon_bright", "", "_bright");
    SkillIconResource.loadDirectoryImage("resource/image/CN/skill_icon_left_black", "", "_left");
    SkillIconResource.loadDirectoryImage("resource/image/CN/skill_icon_right_grey", "", "_right");
    SkillIconResource.showResource();
    autoFight.setImageResource(&SkillIconResource);
    autoFight.setSkills({"Maki", "Akane", "Karin", "Ako", "Cherino", "Chinatsu (Hot Spring)"});
    autoFight.startLoop();

//        autoFight.refreshSkillPosition();
//        long long t2 = BAASUtil::getCurrentTimeMS();
//        cout<<"Time : "<<t2 - t1<<endl;

    cout<<"exit"<<endl;
    return 0;
//    BAASUtil::executeCommandWithoutOutPut("adb forward tcp:27183 tcp:27183");
}
/*
    y -->   601 ,  662
    x -->   847 , 932
            946, 1038
            1048, 1130

            883, 688
            985, 621
            1087, 622


*/