#include "BAASImageUtil.h"

using namespace std;
using namespace cv;

bool BAASImageUtil::loadImage(const std::string& path, cv::Mat& dst) {
    dst = imread(path);
    return !dst.empty();
}

cv::Mat BAASImageUtil::imageCrop(const cv::Mat& src, const BAASRectangle region) {
	return src(Range(region.ul.y, region.lr.y), Range(region.ul.x, region.lr.x));
}

cv::Mat BAASImageUtil::imageCrop(const Mat &src, int x1, int y1, int x2, int y2) {
    return src(Range(y1, y2), Range(x1, x2));
}

bool BAASImageUtil::imageResize(const cv::Mat& src, Mat &dst, const double ratio) {
	cv::resize(src, dst, Size(), ratio, ratio, INTER_CUBIC);
    return true;
}

bool BAASImageUtil::saveImage(const cv::Mat& image, const std::string& imageName, const std::string& path, const bool& check) {
    if (!filesystem::exists(path)) {
        filesystem::create_directories(path);
    }
    std::string savePath = path + "/" + imageName + ".png";
    imwrite(savePath, image);
    if (check) {
        if(!filesystem::exists(path)) {
            BAASLoggerInstance->BAASError("FAIL Save image : [ " + savePath + " ]");
            return false;
        }
        BAASLoggerInstance->BAASInfo("SUCCESS Save image : [ " + savePath + " ]");
        return true;
    }
    else return true;
}

std::pair<int, int> BAASImageUtil::imageSize(const Mat &src) {
    return make_pair(src.cols, src.rows);
}

void BAASImageUtil::imagePaste(Mat &src, const Mat &dst, const BAASPoint &point) {
    dst.copyTo(src(Rect(point.x, point.y, dst.cols, dst.rows)));
}

BAASPoint BAASImageUtil::imageSearch(const cv::Mat &screenshot, const cv::Mat &templateImage, double threshold, bool toGrey) {
    Mat input;
    if (toGrey) {
        cvtColor(screenshot, input, COLOR_BGR2GRAY);      // don't use src directly to avoid changing the original image
    } else {
        input = screenshot;
    }
    matchTemplate(input, templateImage, input, TM_CCOEFF_NORMED);
    double minVal, maxVal;
    Point minLoc, maxLoc;
    minMaxLoc(input, &minVal, &maxVal, &minLoc, &maxLoc);
    if (maxVal < threshold) {
        return {-1, -1};
    }
    return {maxLoc.x + templateImage.cols / 2, maxLoc.y + templateImage.rows / 2};
}

BAASPoint::BAASPoint(int xx, int yy) {
    x = xx;
    y = yy;
}

BAASPoint::BAASPoint() {
    x = 0;
    y = 0;
}

BAASRectangle::BAASRectangle() {
    ul.x = 0;
    ul.y = 0;
    lr.x = 0;
    lr.y = 0;
}

BAASRectangle::BAASRectangle(int x1, int y1, int x2, int y2) {
    ul.x = x1;
    ul.y = y1;
    lr.x = x2;
    lr.y = y2;
}
