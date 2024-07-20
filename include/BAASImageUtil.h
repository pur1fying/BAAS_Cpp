
#ifndef BAAS_BAASIMAGEUTIL_H_
#define BAAS_BAASIMAGEUTIL_H_
#include <map>
#include <string>
#include <filesystem>

#include <opencv2/opencv.hpp>

#include "BAASLogger.h"

struct BAASPoint {
    int x;

    int y;

    BAASPoint();

    BAASPoint(int xx, int yy);

};

BAASPoint operator*(const BAASPoint &p, int i);

BAASPoint operator*(int i, const BAASPoint &p);

BAASPoint operator*(const BAASPoint &p, float i);

BAASPoint operator*(float i, const BAASPoint &p);

BAASPoint operator*(double i, const BAASPoint &p);

BAASPoint operator*(const BAASPoint &p, double i);

BAASPoint &operator*=(BAASPoint &p1, int i);

BAASPoint operator/(const BAASPoint &p1, int i);

BAASPoint operator/(const BAASPoint &p1, float i);

BAASPoint operator/(const BAASPoint &p1, double i);

BAASPoint &operator/=(BAASPoint &p1, int i);

BAASPoint operator+(const BAASPoint &p1, const BAASPoint &p2);

bool operator!=(const BAASPoint &p1, const BAASPoint &p2);

bool operator==(const BAASPoint &p1, const BAASPoint &p2);

bool operator<(const BAASPoint &p1, const BAASPoint &p2);

bool operator<=(const BAASPoint &p1, const BAASPoint &p2);

bool operator>(const BAASPoint &p1, const BAASPoint &p2);

bool operator>=(const BAASPoint &p1, const BAASPoint &p2);

BAASPoint operator-(const BAASPoint &p1,const BAASPoint &P2);

BAASPoint &operator-=(BAASPoint &p1,const BAASPoint &P2);

BAASPoint operator+(const BAASPoint &p1,const BAASPoint &P2);

BAASPoint &operator+=(BAASPoint &p1,const BAASPoint &P2);

std::ostream &operator<<(std::ostream &os, const BAASPoint &point);

std::istream &operator>>(std::istream &is, BAASPoint &point);


struct BAASRectangle {
    BAASPoint ul;               // upper left

    BAASPoint lr;               // lower right

    BAASRectangle();

    BAASRectangle(int x1, int y1, int x2, int y2);

    BAASRectangle(BAASPoint p1, BAASPoint p2);

    bool contains(BAASPoint p) const;

    bool empty() const;
};

bool operator==(const BAASRectangle &r1, const BAASRectangle &r2);      // same rect

bool operator<= (const BAASPoint &point, const BAASRectangle &rect);    // point in rect or on the edge

bool operator< (const BAASRectangle &r1, const BAASRectangle &r2);      // r1 in r2

std::ostream &operator<<(std::ostream &os, const BAASRectangle &rect);

std::istream &operator>>(std::istream &is, BAASRectangle &rect);

class BAASImageUtil {
public:
    static bool loadImage(const std::string& path, cv::Mat& dst);

    static bool saveImage(const cv::Mat& image, const std::string& imageName, const std::string& path, const bool& check=true);

    static cv::Mat imageCrop(const cv::Mat& src,BAASRectangle region);

    static cv::Mat imageCrop(const cv::Mat& src, int x1, int y1, int x2, int y2);

    static bool imageResize(const cv::Mat& src, cv::Mat& dst, double ratio = 1.0);

    static std::pair<int, int> imageSize(const cv::Mat& src);

    static void imagePaste(cv::Mat& src, const cv::Mat& dst, const BAASPoint& point = BAASPoint(0, 0));

    static BAASPoint imageSearch(const cv::Mat& screenshot, const cv::Mat& templateImage, BAASRectangle region={-1,-1,-1,-1}, double threshold=0.7, bool toGrey=false);

    static int pointDistance(const BAASPoint &p1, const BAASPoint &p2);
    
    static bool judgeRGBRange(cv::Vec3b target, cv::Vec3b min, cv::Vec3b max);
    
    static bool judgeRGBRange(cv::Mat &target, BAASPoint position, cv::Vec3b min, cv::Vec3b max);

    static bool judgeRGBRange(cv::Mat &target, BAASPoint position, cv::Vec3b min, cv::Vec3b max, bool checkAround, int aroundRange=1);

    static cv::Vec3b getRegionMeanRGB(cv::Mat &target, BAASRectangle region);

    static cv::Vec3b getRegionMeanRGB(cv::Mat &target, cv::Rect region);
    
    static bool isLargerRGB(const cv::Vec3b &a, const cv::Vec3b &b);
    
    static bool isSmallerRGB(const cv::Vec3b &a, const cv::Vec3b &b);
};

#endif //BAAS_BAASIMAGEUTIL_H_
