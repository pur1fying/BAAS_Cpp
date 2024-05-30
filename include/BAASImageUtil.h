
#include <map>
#include <string>
#include <opencv2/opencv.hpp>
#include <filesystem>
#include "BAASGlobals.h"
#ifndef BAAS_CPP_BAASIMAGEUTIL_H
#define BAAS_CPP_BAASIMAGEUTIL_H

struct BAASPoint {
    int x;
    int y;
    BAASPoint();
    BAASPoint(int xx, int yy);
};

struct BAASRectangle {
    BAASPoint ul;       // upper left
    BAASPoint lr;       // lower right
    BAASRectangle();
    BAASRectangle(int x1, int y1, int x2, int y2);
};

class BAASImageUtil {
public:
    static bool loadImage(const std::string& path, cv::Mat& dst);

    static bool saveImage(const cv::Mat& image, const std::string& imageName, const std::string& path, const bool& check=true);

    static cv::Mat imageCrop(const cv::Mat& src,BAASRectangle region);

    static cv::Mat imageCrop(const cv::Mat& src, int x1, int y1, int x2, int y2);

    static bool imageResize(const cv::Mat& src, cv::Mat& dst, double ratio = 1.0);

    static std::pair<int, int> imageSize(const cv::Mat& src);

    static void imagePaste(cv::Mat& src, const cv::Mat& dst, const BAASPoint& point = BAASPoint(0, 0));

    static BAASPoint imageSearch(const cv::Mat& screenshot, const cv::Mat& templateImage, double threshold=0.8, bool toGrey=false);

    static BAASPoint imageSearch(const cv::Mat& screenshot, const cv::Mat& templateImage, BAASRectangle& region ,double threshold=0.8, bool toGrey=false);
};

#endif //BAAS_CPP_BAASIMAGEUTIL_H
