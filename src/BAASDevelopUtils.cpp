//
// Created by pc on 2024/5/31.
//
#include "BAASDevelopUtils.h"

using namespace std;
using namespace cv;

void BAASDevelopUtils::shotStudentSkill
(
        const cv::Mat& image,
        const string &name1,
        const string &name2,
        const string &name3,
        const int type
)
{
    BAASRectangle r;
    Mat img;
    if(type == SKILL_FULL) {
        if (!name1.empty()) {
            img = BAASImageUtil::crop(image, SKILL1_FULL);
            BAASImageUtil::save(img, name1, string(), true);
        }
        if (!name2.empty()) {
            img = BAASImageUtil::crop(image, SKILL2_FULL);
            BAASImageUtil::save(img, name2, string(), true);
        }
        if (!name3.empty()) {
            img = BAASImageUtil::crop(image, SKILL3_FULL);
            BAASImageUtil::save(img, name3, string(), true);
        }
    }
    else if(type == SKILL_LEFT) {
        if(!name1.empty()) {
            img = BAASImageUtil::crop(image, SKILL1_LEFT);
            BAASImageUtil::save(img, name1, string(), true);
        }
        if(!name2.empty()) {
            img = BAASImageUtil::crop(image, SKILL2_LEFT);
            BAASImageUtil::save(img, name2, string(), true);
        }
        if(!name3.empty()) {
            img = BAASImageUtil::crop(image, SKILL3_LEFT);
            BAASImageUtil::save(img, name3, string(), true);
        }
    }
    else if(type == SKILL_RIGHT) {
        if(!name1.empty()) {
            img = BAASImageUtil::crop(image, SKILL1_RIGHT);
            BAASImageUtil::save(img, name1, string(), true);
        }
        if(!name2.empty()) {
            img = BAASImageUtil::crop(image, SKILL2_RIGHT);
            BAASImageUtil::save(img, name2, string(), true);
        }
        if(!name3.empty()) {
            img = BAASImageUtil::crop(image, SKILL3_RIGHT);
            BAASImageUtil::save(img, name3, string(), true);
        }
    }
}

void BAASDevelopUtils::extract_image_rgb_range(const cv::Mat& img, const string &name, const BAASRectangle &r,const Scalar &min_, const Scalar &max_, const uint8_t cut_edge) {
    Mat im = img.clone();
    BAASImageUtil::filter_region_rgb(im, r, min_, max_);
    BAASRectangle region = r;
    imshow("origin", im);
    cv::waitKey(0);
    if(cut_edge) {
        vector<int> rg;
        BAASImageUtil::crop_edge(im, 0b1111, rg, {0,0,0},  {0, 0, 0}, im);
        BAASGlobalLogger->BAASInfo("crop region: " + to_string(rg[0]) + " " + to_string(rg[1]) + " " + to_string(rg[2]) + " " + to_string(rg[3]));
        region.ul.x += rg[2];
        region.ul.y += rg[0];
        region.lr.x -= rg[3];
        region.lr.y -= rg[1];
    }
    BAASGlobalLogger->BAASInfo("region: " + to_string(region.ul.x) + ", " + to_string(region.ul.y) + ", " + to_string(region.lr.x) + ", " + to_string(region.lr.y));
    BAASGlobalLogger->BAASInfo("shape: " + to_string(im.cols) + "x" + to_string(im.rows));
    cv::imshow(name, im);
    imwrite(name + ".png", im);
    cv::waitKey(0);
}


