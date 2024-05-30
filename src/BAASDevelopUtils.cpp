//
// Created by pc on 2024/5/31.
//
#include "BAASDevelopUtils.h"


using namespace std;
using namespace cv;
void BAASDevelopUtils::getThreeStudentSkillScreenshot(const std::string& name1, const std::string& name2, const std::string& name3) {
    BAASNemu* nemu = BAASNemu::getInstance();
    int conn = nemu->connect("H:\\MuMuPlayer-12.0", "127.0.0.1:16384");
    Mat image;
    nemu->screenshot(conn, image);
    BAASRectangle r1(847, 601, 922, 662), r2(946, 601, 1028, 662), r3(1048, 601, 1128, 662);
    Mat im1 = BAASImageUtil::imageCrop(image, r1);
    Mat im2 = BAASImageUtil::imageCrop(image, r2);
    Mat im3 = BAASImageUtil::imageCrop(image, r3);

    string temp = BAAS_PROJECT_DIR + R"(\resource\image\CN\student_skill_icon)";
    BAASImageUtil::saveImage(im1, name1, temp, true);
    BAASImageUtil::saveImage(im2, name2, temp, true);
    BAASImageUtil::saveImage(im3, name3, temp, true);
}