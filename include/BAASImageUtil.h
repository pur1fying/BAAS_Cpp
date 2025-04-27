
#ifndef BAAS_BAASIMAGEUTIL_H_
#define BAAS_BAASIMAGEUTIL_H_

#include <map>
#include <string>
#include <filesystem>

#include <opencv2/opencv.hpp>

#include "BAASLogger.h"
#include "BAASExceptions.h"
#include "core_defines.h"

BAAS_NAMESPACE_BEGIN

struct BAASPoint {
    int x;

    int y;

    BAASPoint();

    BAASPoint(
            int xx,
            int yy
    );

    [[nodiscard]] std::string to_string() const {
        std::ostringstream oss;
        oss << "(" << std::setw(4) << std::setfill(' ') << x << ", " << std::setw(4) << std::setfill(' ') << y<< ")";
        return oss.str();
    }

    // circle point with center : point , radius : r
    [[nodiscard]] BAASPoint rotate(
            int r,
            int angle
    ) const;
};

inline BAASPoint operator*(
        const BAASPoint &p,
        int i
)
{
    return {p.x * i, p.y * i};
}

inline BAASPoint operator*(
        int i,
        const BAASPoint &p
)
{
    return {p.x * i, p.y * i};
}

inline BAASPoint operator*(
        const BAASPoint &p,
        float i
)
{
    return {int(float(p.x) * 1.0 * i), int(float(p.y) * 1.0 * i)};
}

inline BAASPoint operator*(
        float i,
        const BAASPoint &p
)
{
    return {int(float(p.x) * 1.0 * i), int(float(p.y) * 1.0 * i)};
}

inline BAASPoint operator*(
        double i,
        const BAASPoint &p
)
{
    return {int(double(p.x) * 1.0 * i), int(double(p.y) * 1.0 * i)};
}

inline BAASPoint operator*(
        const BAASPoint &p,
        double i
)
{
    return {int(double(p.x) * 1.0 * i), int(double(p.y) * 1.0 * i)};
}

inline BAASPoint &operator*=(
        BAASPoint &p1,
        int i
)
{
    p1.x *= i;
    p1.y *= i;
    return p1;
}

inline BAASPoint operator/(
        const BAASPoint &p1,
        int i
)
{
    return {p1.x / i, p1.y / i};
}

inline BAASPoint operator/(
        const BAASPoint &p1,
        float i
)
{
    return {int(float(p1.x) / i), int(float(p1.y) / i)};
}

inline BAASPoint operator/(
        const BAASPoint &p1,
        double i
)
{
    return {int(double(p1.x) / i), int(double(p1.y) / i)};
}

inline BAASPoint &operator/=(
        BAASPoint &p1,
        int i
)
{
    p1.x /= i;
    p1.y /= i;
    return p1;
}

inline BAASPoint operator+(
        const BAASPoint &p1,
        const BAASPoint &p2
)
{
    return {p1.x + p2.x, p1.y + p2.y};
}

inline bool operator!=(
        const BAASPoint &p1,
        const BAASPoint &p2
)
{
    return p1.x != p2.x || p1.y != p2.y;
}

inline bool operator==(
        const BAASPoint &p1,
        const BAASPoint &p2
)
{
    return p1.x == p2.x && p1.y == p2.y;
}

inline bool operator<(
        const BAASPoint &p1,
        const BAASPoint &p2
)
{
    return p1.x < p2.x && p1.y < p2.y;
}

inline bool operator<=(
        const BAASPoint &p1,
        const BAASPoint &p2
)
{
    return p1.x <= p2.x && p1.y <= p2.y;
}

inline bool operator>(
        const BAASPoint &p1,
        const BAASPoint &p2
)
{
    return p1.x > p2.x && p1.y > p2.y;
}

inline bool operator>=(
        const BAASPoint &p1,
        const BAASPoint &p2
)
{
    return p1.x >= p2.x && p1.y >= p2.y;
}

inline BAASPoint operator-(
        const BAASPoint &p1,
        const BAASPoint &p2
)
{
    return {p1.x - p2.x, p1.y - p2.y};
}

inline BAASPoint &operator-=(
        BAASPoint &p1,
        const BAASPoint &p2
)
{
    p1.x -= p2.x;
    p1.y -= p2.y;
    return p1;
}

inline BAASPoint &operator+=(
        BAASPoint &p1,
        const BAASPoint &p2
)
{
    p1.x += p2.x;
    p1.y += p2.y;
    return p1;
}

inline std::ostream &operator<<(
        std::ostream &os,
        const BAASPoint &point
)
{
    os << "(" << std::setw(4) << std::setfill(' ') << point.x << ", " <<
                std::setw(4) << std::setfill(' ') << point.y << ")";
    return os;
}

inline std::istream &operator>>(
        std::istream &is,
        BAASPoint &point
)
{
    is >> point.x >> point.y;
    return is;
}


struct BAASRectangle {
    BAASPoint ul;               // upper left

    BAASPoint lr;               // lower right

    BAASRectangle();

    BAASRectangle(
            int x1,
            int y1,
            int x2,
            int y2
    );

    BAASRectangle(
            BAASPoint p1,
            BAASPoint p2
    );

    [[nodiscard]] std::string to_string() const {
        std::ostringstream oss;
        oss << "[" << ul << ", \t" << lr << "]";
        return oss.str();
    }

    [[nodiscard]] inline bool contains(BAASPoint p) const {
        return (p.x > ul.x && p.x < lr.x) && (p.y > ul.y && p.y < lr.y);
    }

    [[nodiscard]] inline bool empty() const{
        return ul.x >= lr.x || ul.y >= lr.y;
    }

    [[nodiscard]] inline int width() const{
        return lr.x - ul.x;
    }

    [[nodiscard]] inline int height() const{
        return lr.y - ul.y;
    }

    [[nodiscard]] inline int size() const {
        return (lr.x - ul.x) * (lr.y - ul.y);
    }

    [[nodiscard]] inline BAASPoint center() const {
        return {(ul.x + lr.x) / 2, (ul.y + lr.y) / 2};
    }
};

inline bool operator<=(
        const BAASPoint &point,
        const BAASRectangle &rect
)
{
    return (point.x >= rect.ul.x && point.x <= rect.lr.x) && (point.y >= rect.ul.y && point.y <= rect.lr.y);
}

inline bool operator<(
        const BAASRectangle &r1,
        const BAASRectangle &r2
)
{
    return r2.contains(r1.ul) && r2.contains(r1.lr);
}

inline bool operator==(
        const BAASRectangle &r1,
        const BAASRectangle &r2
)
{
    return r1.ul == r2.ul && r1.lr == r2.lr;
}

inline std::ostream &operator<<(
        std::ostream &os,
        const BAASRectangle &rect
)
{
    os << "[" << rect.ul << ", " << rect.lr << ")";
    return os;
}

inline std::istream &operator>>(
        std::istream &is,
        BAASRectangle &rect
)
{
    is >> rect.ul >> rect.lr;
    return is;
}

class   BAASImageUtil {
public:
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

    static BAASPoint imageSearch(
            const cv::Mat &screenshot,
            const cv::Mat &templateImage,
            BAASRectangle region = {-1, -1, -1, -1},
            double threshold = 0.7,
            bool toGrey = false
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
            u_char r_min,
            u_char r_max,
            u_char g_min,
            u_char g_max,
            u_char b_min,
            u_char b_max,
            double ratio,
            bool checkAround,
            int aroundRange = 1
    );

    static cv::Vec3b getRegionMeanRGB(
            const cv::Mat &target,
            const BAASRectangle &region
    );

    static cv::Vec3b getRegionMeanRGB(
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

    static cv::Vec3b getRegionMeanRGB(const cv::Mat &target);

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

#endif //BAAS_BAASIMAGEUTIL_H_
