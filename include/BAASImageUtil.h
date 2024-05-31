
#ifndef BAAS_CPP_BAASIMAGEUTIL_H
#define BAAS_CPP_BAASIMAGEUTIL_H
#include <map>
#include <string>
#include <opencv2/opencv.hpp>
#include <filesystem>
#include "BAASGlobals.h"

struct BAASPoint {
    int x;
    int y;
    BAASPoint();
    BAASPoint(int xx, int yy);
};

std::ostream &operator<<(std::ostream &os, const BAASPoint &point);

struct BAASRectangle {
    BAASPoint ul;       // upper left
    BAASPoint lr;       // lower right
    BAASRectangle();
    BAASRectangle(int x1, int y1, int x2, int y2);
};
std::ostream &operator<<(std::ostream &os, const BAASRectangle &rect);

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

#endif //BAAS_CPP_BAASIMAGEUTIL_H
