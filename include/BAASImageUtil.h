
#ifndef BAAS_BAASIMAGEUTIL_H_
#define BAAS_BAASIMAGEUTIL_H_
#include <map>
#include <string>
#include <filesystem>

#include <opencv2/opencv.hpp>

#include "BAASLogger.h"
#include "BAASExceptions.h"

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

    [[nodiscard]] bool contains(BAASPoint p) const;

    [[nodiscard]] bool empty() const;
};

bool operator==(const BAASRectangle &r1, const BAASRectangle &r2);      // same rect

bool operator<= (const BAASPoint &point, const BAASRectangle &rect);    // point in rect or on the edge

bool operator< (const BAASRectangle &r1, const BAASRectangle &r2);      // r1 in r2

std::ostream &operator<<(std::ostream &os, const BAASRectangle &rect);

std::istream &operator>>(std::istream &is, BAASRectangle &rect);

class BAASImageUtil {
public:
    static bool load(const std::string& path, cv::Mat& dst);

    static bool save(const cv::Mat& image, const std::string& imageName, const std::string& path, const bool& check=true);

    static cv::Mat crop(const cv::Mat& src, BAASRectangle region);

    static cv::Mat crop(const cv::Mat& src, int x1, int y1, int x2, int y2);

    static bool resize(const cv::Mat& src, cv::Mat& dst, double ratio = 1.0);

    static void filter_region_rgb(cv::Mat &src, BAASRectangle region, const cv::Scalar& min_scalar, const cv::Scalar& max_scalar);

    static void filter_rgb(cv::Mat &src,const cv::Scalar& min_scalar,const cv::Scalar& max_scalar);

    static void filter_rgb(cv::Mat &src, const cv::Scalar& min_scalar, const cv::Scalar& max_scalar, cv::Mat &dst);

    /*
     *  crop image edge with color min_scalar and max_scalar
     *  enable: 0b0000  1<<4 top 1<<3 bottom 1<<2 left 1<<1 right
     *          example 0b1111 means crop all edge 0x1010 means crop top and left edge
     */
    static void crop_edge(cv::Mat &src, uint8_t enable ,std::vector<int>& cnt, const cv::Scalar& min_scalar, const cv::Scalar& max_scalar,cv::Mat &dst);

    static void crop_edge(cv::Mat &src, uint8_t enable ,std::vector<int>& cnt, const cv::Scalar& color, cv::Mat &dst);

    static void imagePaste(cv::Mat& src, const cv::Mat& dst, const BAASPoint& point = BAASPoint(0, 0));

    static BAASPoint imageSearch(const cv::Mat& screenshot, const cv::Mat& templateImage, BAASRectangle region={-1,-1,-1,-1}, double threshold=0.7, bool toGrey=false);

    static int pointDistance(const BAASPoint &p1, const BAASPoint &p2);
    
    static bool judgeRGBRange(const cv::Vec3b& target,const cv::Vec3b& min,const cv::Vec3b& max);
    
    static bool judgeRGBRange(const cv::Mat &target,const BAASPoint& position,const cv::Vec3b& min,const cv::Vec3b& max);

    static bool judgeRGBRange(const cv::Mat &target,const BAASPoint& position,const cv::Vec3b& min,const cv::Vec3b& max,bool checkAround,int aroundRange=1);

    static cv::Vec3b getRegionMeanRGB(const cv::Mat &target,const BAASRectangle& region);

    static cv::Vec3b getRegionMeanRGB(const cv::Mat &target,const cv::Rect& region);

    static cv::Vec3b calc_abs_diff(const cv::Vec3b &a, const cv::Vec3b &b);

    static cv::Vec3b getRegionMeanRGB(const cv::Mat &target);

    static bool isLargerRGB(const cv::Vec3b &a, const cv::Vec3b &b);
    
    static bool isSmallerRGB(const cv::Vec3b &a, const cv::Vec3b &b);
};

#endif //BAAS_BAASIMAGEUTIL_H_
