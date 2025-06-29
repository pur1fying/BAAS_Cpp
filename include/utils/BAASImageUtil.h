
#ifndef BAAS_UTILS_BAASIMAGEUTIL_H_
#define BAAS_UTILS_BAASIMAGEUTIL_H_

#include <map>
#include <string>
#include <filesystem>

#include <opencv2/opencv.hpp>
#include <nlohmann/detail/string_concat.hpp>

#include "BAASTypes.h"
#include "BAASLogger.h"
#include "BAASExceptions.h"

BAAS_NAMESPACE_BEGIN

class BAASImageUtil {

public:

    static bool checkImageBroken(const std::string &path);

    static std::pair<int, int> deleteBrokenImage(const std::string &path);

    static bool load(
            const std::string &path,
            cv::Mat &dst
    );

    static bool save(
            const cv::Mat &image,
            const std::string &imageName,
            const std::string &path,
            const bool &check = true
    );

    static cv::Mat crop(
            const cv::Mat &src,
            BAASRectangle region
    );

    static cv::Mat crop(
            const cv::Mat &src,
            int x1,
            int y1,
            int x2,
            int y2
    );

    static void resize(
            const cv::Mat &src,
            cv::Mat &dst,
            double ratio = 1.0
    );

    static void resize(
            const cv::Mat &src,
            cv::Mat &dst,
            int width,
            int height
    );

    static void filter_region_rgb(
            cv::Mat &src,
            BAASRectangle region,
            const cv::Scalar &min_scalar,
            const cv::Scalar &max_scalar
    );

    static void filter_rgb(
            cv::Mat &src,
            const cv::Scalar &min_scalar,
            const cv::Scalar &max_scalar
    );

    static void filter_rgb(
            cv::Mat &src,
            const cv::Scalar &min_scalar,
            const cv::Scalar &max_scalar,
            cv::Mat &dst
    );

    /*
     *  crop image edge with color min_scalar and max_scalar
     *  enable: 0b0000  1<<4 top 1<<3 bottom 1<<2 left 1<<1 right
     *          example 0b1111 means crop all edge 0x1010 means crop top and left edge
     */
    static void crop_edge(
            cv::Mat &src,
            uint8_t enable,
            std::vector<int> &cnt,
            const cv::Scalar &min_scalar,
            const cv::Scalar &max_scalar,
            cv::Mat &dst
    );

    static void crop_edge(
            cv::Mat &src,
            uint8_t enable,
            std::vector<int> &cnt,
            const cv::Scalar &color,
            cv::Mat &dst
    );

    static void imagePaste(
            cv::Mat &src,
            const cv::Mat &dst,
            const BAASPoint &point = BAASPoint(0, 0));

    static void gen_not_black_region_mask(
            const cv::Mat &src,
            cv::Mat &mask
    );

    static void gen_not_black_region_mask(
            const cv::Mat &src,
            cv::Mat &mask,
            const BAASRectangle &region
    );

    static void gen_not_black_region_mask(
            const cv::Mat &src,
            cv::Mat &mask,
            const cv::Rect &region
    );

    static void pixel2string(
            const cv::Vec3b &pixel,
            std::string &str
    );

    static int pointDistance(
            const BAASPoint &p1,
            const BAASPoint &p2
    );


    static bool judge_rgb_range(
            const cv::Vec3b &target,
            const cv::Vec3b &min,
            const cv::Vec3b &max
    );

    static bool judge_rgb_range(
            const cv::Mat &target,
            const BAASPoint &position,
            const cv::Vec3b &min,
            const cv::Vec3b &max
    );

    static bool judge_rgb_range(
            const cv::Mat &target,
            const BAASPoint &position,
            const cv::Vec3b &min,
            const cv::Vec3b &max,
            double ratio
    );

    static bool judge_rgb_range(
            const cv::Mat &target,
            const BAASPoint &position,
            const cv::Vec3b &min,
            const cv::Vec3b &max,
            bool checkAround,
            int aroundRange = 1
    );

    static bool judge_rgb_range(
            const cv::Mat &target,
            int x,
            int y,
            uint8_t r_min,
            uint8_t r_max,
            uint8_t g_min,
            uint8_t g_max,
            uint8_t b_min,
            uint8_t b_max,
            double ratio,
            bool checkAround,
            int aroundRange = 1
    );

    static cv::Vec3b get_region_mean_rgb(
            const cv::Mat &target,
            const BAASRectangle &region
    );

    static cv::Vec3b get_region_mean_rgb(
            const cv::Mat &target,
            const cv::Rect &region
    );

    static cv::Vec3b get_region_not_black_mean_rgb(const cv::Mat &target);

    static cv::Vec3b get_region_not_black_mean_rgb(
            const cv::Mat &target,
            const BAASRectangle &region
    );

    static cv::Vec3b get_region_not_black_mean_rgb(
            const cv::Mat &target,
            const cv::Rect &region
    );

    static cv::Vec3b calc_abs_diff(
            const cv::Vec3b &a,
            const cv::Vec3b &b
    );

    inline static cv::Vec3b get_mean_rgb(const cv::Mat &target) {
        cv::Scalar m = mean(target);
        return cv::Vec3b{(unsigned char) m[2], (unsigned char) m[1], (unsigned char) m[0]};
    }

    static bool isLargerRGB(
            const cv::Vec3b &a,
            const cv::Vec3b &b
    );

    static bool isSmallerRGB(
            const cv::Vec3b &a,
            const cv::Vec3b &b
    );
};

BAAS_NAMESPACE_END

#endif //BAAS_UIILS_BAASIMAGEUTIL_H_
