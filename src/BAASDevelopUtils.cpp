//
// Created by pc on 2024/5/31.
//
#include "BAASDevelopUtils.h"
using namespace std;
using namespace cv;

void BAASDevelopUtils::shotStudentSkill(const string &name, const BAASRectangle &r, const int type) {
    string MuMuPlayerPath = "H:\\MuMuPlayer-12.0";
    BAASNemu* nemu = new BAASNemu(MuMuPlayerPath);
    string temp = BAAS_PROJECT_DIR + R"(\resource\image\CN\)";
    if(type == SKILL_FULL) {
        temp = temp + "skill_icon_bright";
    }
    else if(type == SKILL_LEFT) {
        temp = temp + "skill_icon_left_black";
    }
    else if(type == SKILL_RIGHT) {
        temp = temp + "skill_icon_right_grey";
    }
    Mat image;
    nemu->screenshot(image);
    Mat im = BAASImageUtil::crop(image, r);
    BAASImageUtil::save(im, name, temp, true);
    delete nemu;
}
