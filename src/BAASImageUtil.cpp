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

bool BAASImageUtil::load(const std::string &path, cv::Mat &dst) {
    if(!filesystem::exists(path)) return false;
    dst = imread(path);
    return !dst.empty();
}

Mat BAASImageUtil::crop(const Mat& src, const BAASRectangle region) {
    if (!(region.ul < region.lr)) {
        throw ValueError("Invalid Crop Image Region, ul should be smaller than lr");
    }
    if (region.lr.x > src.cols || region.lr.y > src.rows) {
        throw ValueError("Invalid Crop Image Region, out of bound");
    }
	return src(Range(region.ul.y, region.lr.y), Range(region.ul.x, region.lr.x));
}

Mat BAASImageUtil::crop(const Mat &src, int x1, int y1, int x2, int y2) {
    return src(Range(y1, y2), Range(x1, x2));
}

bool BAASImageUtil::resize(const Mat& src, Mat &dst, const double ratio) {
	cv::resize(src, dst, Size(), ratio, ratio, INTER_CUBIC);
    return true;
}

void BAASImageUtil::filter_rgb(Mat &src,const cv::Scalar& min_scalar,const cv::Scalar& max_scalar) {
    cv::Mat mask, out_put;
    Scalar min_scalar_ = min_scalar;
    Scalar max_scalar_ = max_scalar;
    swap(min_scalar_[0], min_scalar_[2]);
    swap(max_scalar_[0], max_scalar_[2]);
    cv::inRange(src, min_scalar_, max_scalar_, mask);
    src.copyTo(out_put, mask);
    src = out_put;
}

void BAASImageUtil::filter_rgb(Mat &src, const Scalar &min_scalar, const Scalar &max_scalar, Mat &dst) {
    cv::Mat mask, out_put;
    cv::inRange(src, min_scalar, max_scalar, mask);
    src.copyTo(out_put, mask);
    dst = out_put;
}

void BAASImageUtil::crop_edge(Mat &src, uint8_t enable, vector<int>& cnt, const cv::Scalar& min_scalar, const cv::Scalar& max_scalar, Mat &dst) {
    cnt.resize(4, 0);
    cv::Mat line, mask;
    if(enable & 0b1000) {
        for (int i = 0; i < src.rows; i++) {
            line = src.row(i);
            cv::inRange(line, min_scalar, max_scalar, mask);
            if (countNonZero(mask) == src.cols) {
                cnt[0]++;
            } else break;
        }
    }

    if(enable & 0b0100) {
        for (int i = src.rows - 1; i >= cnt[0]; i--) {
            line = src.row(i);
            cv::inRange(line, min_scalar, max_scalar, mask);
            if (countNonZero(mask) == src.cols) {
                cnt[1]++;
            } else break;
        }
    }

    if(enable & 0b0010) {
        for (int i = 0; i < src.cols; i++) {
            line = src.col(i);
            cv::inRange(line, min_scalar, max_scalar, mask);
            if (countNonZero(mask) == src.rows) {
                cnt[2]++;
            } else break;
        }
    }

    if(enable & 0b0001) {
        for (int i = src.cols - 1; i >= cnt[2]; i--) {
            line = src.col(i);
            cv::inRange(line, min_scalar, max_scalar, mask);
            if (countNonZero(mask) == src.rows) {
                cnt[3]++;
            } else break;
        }
    }

    dst = crop(src, cnt[2], cnt[0], src.cols - cnt[3], src.rows - cnt[1]);
}

void BAASImageUtil::crop_edge(Mat &src, uint8_t enable, vector<int> &cnt, const Scalar &color, Mat &dst) {
    crop_edge(src, enable, cnt, color, color, dst);
}

bool BAASImageUtil::save(const Mat& image, const string& imageName, const string& path, const bool& check) {
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

void BAASImageUtil::imagePaste(Mat &src, const Mat &dst, const BAASPoint &point) {
    dst.copyTo(src(Rect(point.x, point.y, dst.cols, dst.rows)));
}

BAASPoint BAASImageUtil::imageSearch(const Mat &screenshot, const Mat &templateImage, BAASRectangle region, double threshold, bool toGrey) {
    Mat inputCopy= screenshot , templateCopy = templateImage;
    if(toGrey || region.ul.x != -1) {
        inputCopy = screenshot.clone();
    }
    if (region.ul.x != -1) {
        inputCopy = crop(inputCopy, region);
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

bool BAASImageUtil::judgeRGBRange(const Vec3b& target,const Vec3b& min,const Vec3b& max) {
    if(target[0] >= min[0] && target[0] <= max[0] && target[1] >= min[1] && target[1] <= max[1] && target[2] >= min[2] && target[2] <= max[2])
        return true;
    return false;
}

bool BAASImageUtil::judgeRGBRange(const Mat &target,const BAASPoint& position,const Vec3b& min,const Vec3b& max) {
    Vec3b pixel = target.at<Vec3b>(position.y, position.x);
    if(pixel[0] >= min[0] && pixel[0] <= max[0] && pixel[1] >= min[1] && pixel[1] <= max[1] && pixel[2] >= min[2] && pixel[2] <= max[2])
        return true;
    return false;
}

bool BAASImageUtil::judgeRGBRange(const Mat &target,const BAASPoint& position,const Vec3b& min,const Vec3b& max, bool checkAround, int aroundRange) {
    if(judgeRGBRange(target, position, min, max))return true;
    if (checkAround)
        for (int i = -aroundRange; i <= aroundRange; i++)
            for (int j = -aroundRange; j <= aroundRange; j++)
                if (judgeRGBRange(target, {position.x + i, position.y + j}, min, max))
                    return true;
    return false;
}

Vec3b BAASImageUtil::getRegionMeanRGB(const Mat &target,const BAASRectangle &region) {
    return getRegionMeanRGB(target, Rect(region.ul.x, region.ul.y, region.lr.x - region.ul.x, region.lr.y - region.ul.y));
}

Vec3b BAASImageUtil::getRegionMeanRGB(const Mat &target,const Rect &region) {
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

void BAASImageUtil::filter_region_rgb(Mat &src, BAASRectangle region, const Scalar &min_scalar, const Scalar &max_scalar) {
    src = crop(src, region);
    filter_rgb(src, min_scalar, max_scalar);
}

cv::Vec3b BAASImageUtil::getRegionMeanRGB(const Mat &target) {
    Scalar m = mean(target);
    return Vec3b {(unsigned char)m[2], (unsigned char)m[1], (unsigned char)m[0]};
}

cv::Vec3b BAASImageUtil::calc_abs_diff(const Vec3b &a, const Vec3b &b) {
    Vec3b diff;
    if(a[0] > b[0]) diff[0] = a[0] - b[0];
    else diff[0] = b[0] - a[0];

    if(a[1] > b[1]) diff[1] = a[1] - b[1];
    else diff[1] = b[1] - a[1];

    if(a[2] > b[2]) diff[2] = a[2] - b[2];
    else diff[2] = b[2] - a[2];

    return diff;
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

