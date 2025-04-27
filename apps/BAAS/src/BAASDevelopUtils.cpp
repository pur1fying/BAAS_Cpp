//
// Created by pc on 2024/5/31.
//
#include "BAASDevelopUtils.h"
#include "BAASGlobals.h"

using namespace std;
using namespace cv;

BAAS_NAMESPACE_BEGIN

void BAASDevelopUtils::shotStudentSkill(
        const string &name,
        const BAASRectangle &r,
        const int type
)
{
    string MuMuPlayerPath = "H:\\MuMuPlayer-12.0";
    auto *nemu = new BAASNemu(MuMuPlayerPath);
    std::filesystem::path temp = BAAS_PROJECT_DIR / "resource" / "image" / "CN";
    if (type == SKILL_FULL) {
        temp = temp / "skill_icon_bright";
    } else if (type == SKILL_LEFT) {
        temp = temp / "skill_icon_left_black";
    } else if (type == SKILL_RIGHT) {
        temp = temp / "skill_icon_right_grey";
    }
    Mat image;
    nemu->screenshot(image);
    Mat im = BAASImageUtil::crop(image, r);
    BAASImageUtil::save(im, name, temp.string(), true);
    delete nemu;
}

void BAASDevelopUtils::extract_image_rgb_range(
        const cv::Mat &img,
        const string &name,
        const BAASRectangle &r,
        const Scalar &min_,
        const Scalar &max_,
        const uint8_t cut_edge
)
{
    Mat im = img.clone();
    BAASImageUtil::filter_region_rgb(im, r, min_, max_);
    BAASRectangle region = r;
    if (cut_edge) {
        vector<int> rg;
        BAASImageUtil::crop_edge(im, 0b1111, rg, {0, 0, 0}, {0, 0, 0}, im);
        BAASGlobalLogger->BAASInfo(
                "crop region: " + to_string(rg[0]) + " " + to_string(rg[1]) + " " + to_string(rg[2]) + " " +
                to_string(rg[3]));
        region.ul
              .x += rg[2];
        region.ul
              .y += rg[0];
        region.lr
              .x -= rg[3];
        region.lr
              .y -= rg[1];
    }
    BAASGlobalLogger->BAASInfo("Region: " + region.to_string());
    BAASGlobalLogger->BAASInfo("Shape: " + to_string(im.cols) + "x" + to_string(im.rows));
    imwrite(name + ".png", im);
}

BAAS_NAMESPACE_END
