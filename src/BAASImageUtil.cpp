#include "BAASImageUtil.h"

using namespace std;
using namespace cv;

inline BAASPoint operator*(const BAASPoint &p, int i) {
    return {p.x * i, p.y * i};
}

inline BAASPoint operator*(int i, const BAASPoint &p) {
    return {p.x * i, p.y * i};
}

inline BAASPoint operator*(const BAASPoint &p, float i) {
    return {int(p.x * i), int(p.y * i)};
}

inline bool operator!=(const BAASPoint &p1, const BAASPoint &p2) {
    return p1.x != p2.x || p1.y != p2.y;
}

inline bool operator==(const BAASPoint &p1, const BAASPoint &p2) {
    return p1.x == p2.x && p1.y == p2.y;
}

inline bool operator<(const BAASPoint &p1, const BAASPoint &p2) {
    return p1.x < p2.x && p1.y < p2.y;
}

inline bool operator<=(const BAASPoint &p1, const BAASPoint &p2) {
    return p1.x <= p2.x && p1.y <= p2.y;
}

inline bool operator>(const BAASPoint &p1, const BAASPoint &p2) {
    return p1.x > p2.x && p1.y > p2.y;
}

inline bool operator>=(const BAASPoint &p1, const BAASPoint &p2) {
    return p1.x >= p2.x && p1.y >= p2.y;
}

inline BAASPoint operator-(const BAASPoint &p1,const BAASPoint &p2) {
    return {p1.x - p2.x, p1.y - p2.y};
}

inline BAASPoint &operator-=(BAASPoint &p1,const BAASPoint &p2) {
    p1.x -= p2.x;
    p1.y -= p2.y;
    return p1;
}

inline BAASPoint operator+(const BAASPoint &p1,const BAASPoint &p2) {
    return {p1.x + p2.x, p1.y + p2.y};
}

inline BAASPoint &operator+=(BAASPoint &p1,const BAASPoint &p2) {
    p1.x += p2.x;
    p1.y += p2.y;
    return p1;
}

inline ostream &operator<<(ostream &os, const BAASPoint &point) {
    os << "(" << setw(4) << setfill(' ') << point.x << ", " << setw(4) << setfill(' ') << point.y << ")";
    return os;
}

inline istream &operator>>(istream &is, BAASPoint &point) {
    is >> point.x >> point.y;
    return is;
}


inline bool operator<= (const BAASPoint &point, const BAASRectangle &rect) {
    return (point.x >= rect.ul.x && point.x <= rect.lr.x) && (point.y >= rect.ul.y && point.y <= rect.lr.y);
}

inline bool operator< (const BAASRectangle &r1, const BAASRectangle &r2){
    return r2.contains(r1.ul) && r2.contains(r1.lr);
}

inline bool operator==(const BAASRectangle &r1, const BAASRectangle &r2) {
    return r1.ul == r2.ul && r1.lr == r2.lr;
}

inline ostream &operator<<(ostream &os, const BAASRectangle &rect) {
    os << "[" << rect.ul << ", \t" << rect.lr << ")";
    return os;
}

inline istream &operator >> (istream &is, BAASRectangle &rect) {
    is >> rect.ul >> rect.lr;
    return is;
}

bool BAASImageUtil::loadImage(const string& path, Mat& dst) {
    dst = imread(path);
    return !dst.empty();
}

Mat BAASImageUtil::imageCrop(const Mat& src, const BAASRectangle region) {
	return src(Range(region.ul.y, region.lr.y), Range(region.ul.x, region.lr.x));
}

Mat BAASImageUtil::imageCrop(const Mat &src, int x1, int y1, int x2, int y2) {
    return src(Range(y1, y2), Range(x1, x2));
}

bool BAASImageUtil::imageResize(const Mat& src, Mat &dst, const double ratio) {
	resize(src, dst, Size(), ratio, ratio, INTER_CUBIC);
    return true;
}

bool BAASImageUtil::saveImage(const Mat& image, const string& imageName, const string& path, const bool& check) {
    if (!filesystem::exists(path)) {
        filesystem::create_directories(path);
    }
    string savePath = path + "/" + imageName + ".png";
    imwrite(savePath, image);
    if (check) {
        if(!filesystem::exists(path)) {
            BAASGlobalLogger->BAASError("FAIL Save image : [ " + savePath + " ]");
            return false;
        }
        BAASGlobalLogger->BAASInfo("SUCCESS Save image : [ " + savePath + " ]");
        return true;
    }
    else return true;
}

pair<int, int> BAASImageUtil::imageSize(const Mat &src) {
    return make_pair(src.cols, src.rows);
}

void BAASImageUtil::imagePaste(Mat &src, const Mat &dst, const BAASPoint &point) {
    dst.copyTo(src(Rect(point.x, point.y, dst.cols, dst.rows)));
}

BAASPoint BAASImageUtil::imageSearch(const Mat &screenshot, const Mat &templateImage, BAASRectangle region, double threshold, bool toGrey) {
    Mat inputCopy= screenshot , templateCopy = templateImage;
    if(toGrey || region.ul.x != -1) {
        inputCopy = screenshot.clone();
    }
    if (region.ul.x != -1) {
        inputCopy = imageCrop(inputCopy, region);
    }
    if(toGrey) {
        templateCopy = templateImage.clone();
        cvtColor(inputCopy, inputCopy, COLOR_BGR2GRAY);
        cvtColor(templateCopy, templateCopy, COLOR_BGR2GRAY);
        // don't use src directly to avoid changing the original image
    }

    int result_cols = inputCopy.cols - templateCopy.cols + 1;
    int result_rows = inputCopy.rows - templateCopy.rows + 1;
    Mat result(result_rows, result_cols, CV_32FC1);
    matchTemplate(inputCopy, templateCopy, result, TM_CCOEFF_NORMED);
    double minVal, maxVal;
    Point minLoc, maxLoc;
    minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc);
    if (maxVal < threshold) {
        return {-1, -1};
    }
    if(region.ul.x != -1) {
        maxLoc.x += region.ul.x;
        maxLoc.y += region.ul.y;
    }
    return {maxLoc.x + templateImage.cols / 2, maxLoc.y + templateImage.rows / 2};
}

int BAASImageUtil::pointDistance(const BAASPoint &p1, const BAASPoint &p2) {
    return (p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y);
}

bool BAASImageUtil::judgeRGBRange(Vec3b target, Vec3b min, Vec3b max) {
    if(target[0] >= min[0] && target[0] <= max[0] && target[1] >= min[1] && target[1] <= max[1] && target[2] >= min[2] && target[2] <= max[2])
        return true;
    return false;
}

bool BAASImageUtil::judgeRGBRange(Mat &target, BAASPoint position, Vec3b min, Vec3b max) {
    Vec3b pixel = target.at<Vec3b>(position.y, position.x);
    if(pixel[0] >= min[0] && pixel[0] <= max[0] && pixel[1] >= min[1] && pixel[1] <= max[1] && pixel[2] >= min[2] && pixel[2] <= max[2])
        return true;
    return false;
}

bool BAASImageUtil::judgeRGBRange(Mat &target, BAASPoint position, Vec3b min, Vec3b max, bool checkAround, int aroundRange) {
    if(judgeRGBRange(target, position, min, max))return true;
    if (checkAround)
        for (int i = -aroundRange; i <= aroundRange; i++)
            for (int j = -aroundRange; j <= aroundRange; j++)
                if (judgeRGBRange(target, {position.x + i, position.y + j}, min, max))
                    return true;
    return false;
}

Vec3b BAASImageUtil::getRegionMeanRGB(Mat &target, BAASRectangle region) {
    return getRegionMeanRGB(target, Rect(region.ul.x, region.ul.y, region.lr.x - region.ul.x, region.lr.y - region.ul.y));
}

Vec3b BAASImageUtil::getRegionMeanRGB(Mat &target, Rect region) {
    Scalar m = mean(target(region));
    return Vec3b {(unsigned char)m[2], (unsigned char)m[1], (unsigned char)m[0]};
}

bool BAASImageUtil::isLargerRGB(const Vec3b &a, const Vec3b &b) {
    if (a[0] > b[0] && a[1] > b[1] && a[2] > b[2]) {
        return true;
    }
    return false;
}

bool BAASImageUtil::isSmallerRGB(const Vec3b &a, const Vec3b &b) {
    if (a[0] < b[0] && a[1] < b[1] && a[2] < b[2]) {
        return true;
    }
    return false;
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

BAASRectangle::BAASRectangle(BAASPoint p1, BAASPoint p2) {
    ul = p1;
    lr = p2;
}

inline bool BAASRectangle::contains(BAASPoint p) const {
    return (p.x > ul.x && p.x < lr.x) && (p.y > ul.y && p.y < lr.y);
}

bool BAASRectangle::empty() const {
    return ul.x >= lr.x || ul.y >= lr.y;
}

