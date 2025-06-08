#include "utils/BAASImageUtil.h"

#include <numbers>

using namespace std;
using namespace cv;

BAAS_NAMESPACE_BEGIN

bool BAASImageUtil::checkImageBroken(const std::string &path)
{
    if (!filesystem::exists(path)) {
        throw PathError("File : [ " + path + " ] not exists");
    }
    cv::Mat image = cv::imread(path);
    if (image.empty()) {
        BAASGlobalLogger->BAASError("Broken Image Path : " + path);
        return false;
    }
    return true;
}

pair<int, int> BAASImageUtil::deleteBrokenImage(const std::string &path)
{
    int totalFiles = 0, brokenFiles = 0;
    if (filesystem::is_directory(path)) {
        for (auto &p: filesystem::directory_iterator(path)) {
            totalFiles++;
            if (!checkImageBroken(
                    p.path()
                     .string())) {
                brokenFiles++;
                filesystem::remove(p.path());
            }
        }
    } else {
        totalFiles = 1;
        if (!filesystem::exists(path))throw ValueError("File not exists");
        if (!checkImageBroken(path)) {
            brokenFiles = 1;
            filesystem::remove(path);
        }
    }
    return make_pair(brokenFiles, totalFiles);
}

bool BAASImageUtil::load(
        const std::string &path,
        cv::Mat &dst
)
{
    if (!filesystem::exists(path)) return false;
    dst = imread(path);
    return !dst.empty();
}

Mat BAASImageUtil::crop(
        const Mat &src,
        const BAASRectangle region
)
{
    if (!(region.ul < region.lr)) {
        throw ValueError("Invalid Crop Image Region " + region.to_string()  + " , ul should be smaller than lr");
    }
    if (region.lr.x > src.cols || region.lr.y > src.rows) {
        throw ValueError("Invalid Crop Image Region " + region.to_string() + " , out of image size");
    }
    return src(Range(region.ul.y, region.lr.y), Range(region.ul.x, region.lr.x));
}

Mat BAASImageUtil::crop(
        const Mat &src,
        int x1,
        int y1,
        int x2,
        int y2
)
{
    return src(Range(y1, y2), Range(x1, x2));
}

void BAASImageUtil::resize(
        const Mat &src,
        Mat &dst,
        const double ratio
)
{
    cv::resize(src, dst, Size(), ratio, ratio, INTER_CUBIC);
}

void BAASImageUtil::resize(
        const Mat &src,
        Mat &dst,
        int width,
        int height
)
{
    cv::resize(src, dst, Size(width, height), 0, 0, INTER_CUBIC);
}

void BAASImageUtil::filter_rgb(
        Mat &src,
        const cv::Scalar &min_scalar,
        const cv::Scalar &max_scalar
)
{
    cv::Mat mask, out_put;
    Scalar min_scalar_ = min_scalar;
    Scalar max_scalar_ = max_scalar;
    swap(min_scalar_[0], min_scalar_[2]);
    swap(max_scalar_[0], max_scalar_[2]);
    cv::inRange(src, min_scalar_, max_scalar_, mask);
    src.copyTo(out_put, mask);
    src = out_put;
}

void BAASImageUtil::filter_rgb(
        Mat &src,
        const Scalar &min_scalar,
        const Scalar &max_scalar,
        Mat &dst
)
{
    cv::Mat mask, out_put;
    cv::inRange(src, min_scalar, max_scalar, mask);
    src.copyTo(out_put, mask);
    dst = out_put;
}

void BAASImageUtil::crop_edge(
        Mat &src,
        uint8_t enable,
        vector<int> &cnt,
        const cv::Scalar &min_scalar,
        const cv::Scalar &max_scalar,
        Mat &dst
)
{
    cnt.resize(4, 0);
    cv::Mat line, mask;
    if (enable & 0b1000) {
        for (int i = 0; i < src.rows; i++) {
            line = src.row(i);
            cv::inRange(line, min_scalar, max_scalar, mask);
            if (countNonZero(mask) == src.cols) {
                cnt[0]++;
            } else break;
        }
    }

    if (enable & 0b0100) {
        for (int i = src.rows - 1; i >= cnt[0]; i--) {
            line = src.row(i);
            cv::inRange(line, min_scalar, max_scalar, mask);
            if (countNonZero(mask) == src.cols) {
                cnt[1]++;
            } else break;
        }
    }

    if (enable & 0b0010) {
        for (int i = 0; i < src.cols; i++) {
            line = src.col(i);
            cv::inRange(line, min_scalar, max_scalar, mask);
            if (countNonZero(mask) == src.rows) {
                cnt[2]++;
            } else break;
        }
    }

    if (enable & 0b0001) {
        for (int i = src.cols - 1; i >= cnt[2]; i--) {
            line = src.col(i);
            cv::inRange(line, min_scalar, max_scalar, mask);
            if (countNonZero(mask) == src.rows) {
                cnt[3]++;
            } else break;
        }
    }

    crop(src, cnt[2], cnt[0], src.cols - cnt[3], src.rows - cnt[1]).copyTo(dst);
}

void BAASImageUtil::crop_edge(
        Mat &src,
        uint8_t enable,
        vector<int> &cnt,
        const Scalar &color,
        Mat &dst
)
{
    crop_edge(src, enable, cnt, color, color, dst);
}

bool BAASImageUtil::save(
        const Mat &image,
        const string &imageName,
        const string &path,
        const bool &check
)
{
    if (!filesystem::exists(path)) {
        filesystem::create_directories(path);
    }
    string savePath = path + "/" + imageName + ".png";
    imwrite(savePath, image);
    if (check) {
        if (!filesystem::exists(path)) {
            BAASGlobalLogger->BAASError("FAIL Save image : [ " + savePath + " ]");
            return false;
        }
        BAASGlobalLogger->BAASInfo("SUCCESS Save image : [ " + savePath + " ]");
        return true;
    } else return true;
}

void BAASImageUtil::imagePaste(
        Mat &src,
        const Mat &dst,
        const BAASPoint &point
)
{
    dst.copyTo(src(Rect(point.x, point.y, dst.cols, dst.rows)));
}

int BAASImageUtil::pointDistance(
        const BAASPoint &p1,
        const BAASPoint &p2
)
{
    return (p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y);
}

bool BAASImageUtil::judge_rgb_range(
        const Vec3b &target,
        const Vec3b &min,
        const Vec3b &max
)
{
    if (target[0] >= min[2] && target[0] <= max[2] && target[1] >= min[1] && target[1] <= max[1] &&
        target[2] >= min[0] && target[2] <= max[0])
        return true;
    return false;
}

bool BAASImageUtil::judge_rgb_range(
        const Mat &target,
        const BAASPoint &position,
        const Vec3b &min,   // rgb
        const Vec3b &max
)
{
    Vec3b pixel = target.at<Vec3b>(position.y, position.x);     // bgr
    if (pixel[0] >= min[2] && pixel[0] <= max[2] && pixel[1] >= min[1] && pixel[1] <= max[1] && pixel[2] >= min[0] &&
        pixel[2] <= max[0])
        return true;
    return false;
}

bool BAASImageUtil::judge_rgb_range(
        const Mat &target,
        const BAASPoint &position,
        const Vec3b &min,
        const Vec3b &max,
        double ratio
)
{
    BAASPoint temp = {int(position.x * ratio), int(position.y * ratio)};
    return judge_rgb_range(target, temp, min, max);
}

// any pixel in the region satisfy the condition will return true
bool BAASImageUtil::judge_rgb_range(
        const Mat &target,
        const BAASPoint &position,
        const Vec3b &min,
        const Vec3b &max,
        bool checkAround,
        int aroundRange
)
{
    if (checkAround) {
        for (int i = -aroundRange; i <= aroundRange; i++)
            for (int j = -aroundRange; j <= aroundRange; j++)
                if (judge_rgb_range(target, {position.x + i, position.y + j}, min, max))
                    return true;
        return false;
    }
    if (judge_rgb_range(target, position, min, max)) return true;
    return false;
}

bool BAASImageUtil::judge_rgb_range(
        const Mat &target,
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
        int aroundRange
)
{
    BAASPoint temp = {int(x * ratio), int(y * ratio)};
    return judge_rgb_range(target, temp, {r_min, g_min, b_min}, {r_max, g_max, b_max}, checkAround, aroundRange);
}

Vec3b BAASImageUtil::get_region_mean_rgb(
        const Mat &target,
        const BAASRectangle &region
)
{
    return get_region_mean_rgb(
            target,
            Rect(
                    region.ul.x,
                    region.ul.y,
                    region.lr.x - region.ul.x,
                    region.lr.y - region.ul.y
            ));
}

Vec3b BAASImageUtil::get_region_mean_rgb(
        const Mat &target,
        const Rect &region
)
{
    Scalar m = mean(target(region));
    return Vec3b{(unsigned char) m[2], (unsigned char) m[1], (unsigned char) m[0]};
}

bool BAASImageUtil::isLargerRGB(
        const Vec3b &a,
        const Vec3b &b
)
{
    if (a[0] > b[0] && a[1] > b[1] && a[2] > b[2]) {
        return true;
    }
    return false;
}

bool BAASImageUtil::isSmallerRGB(
        const Vec3b &a,
        const Vec3b &b
)
{
    if (a[0] < b[0] && a[1] < b[1] && a[2] < b[2]) {
        return true;
    }
    return false;
}

void BAASImageUtil::filter_region_rgb(
        Mat &src,
        BAASRectangle region,
        const Scalar &min_scalar,
        const Scalar &max_scalar
)
{
    src = crop(src, region);
    filter_rgb(src, min_scalar, max_scalar);
}

cv::Vec3b BAASImageUtil::calc_abs_diff(
        const Vec3b &a,
        const Vec3b &b
)
{
    Vec3b diff;
    if (a[0] > b[0]) diff[0] = a[0] - b[0];
    else diff[0] = b[0] - a[0];

    if (a[1] > b[1]) diff[1] = a[1] - b[1];
    else diff[1] = b[1] - a[1];

    if (a[2] > b[2]) diff[2] = a[2] - b[2];
    else diff[2] = b[2] - a[2];

    return diff;
}

cv::Vec3b BAASImageUtil::get_region_not_black_mean_rgb(const Mat &target)
{
    vector<Mat> channels(3);
    split(target, channels);
    vector<double> sum(3, 0);
    int cnt;
    for (int i = 0; i < 3; ++i) {
        sum[i] = cv::sum(channels[i])[0];
        cnt = cv::countNonZero(channels[i]);
        if (sum[i] != 0)
            sum[i] /= cnt;
    }
    return Vec3b{(unsigned char) sum[2], (unsigned char) sum[1], (unsigned char) sum[0]};
}

cv::Vec3b BAASImageUtil::get_region_not_black_mean_rgb(
        const Mat &target,
        const BAASRectangle &region
)
{
    return get_region_not_black_mean_rgb(
            target,
            Rect(
                    region.ul.x,
                    region.ul.y,
                    region.lr.x - region.ul.x,
                    region.lr.y - region.ul.y
            )
   );
}

cv::Vec3b BAASImageUtil::get_region_not_black_mean_rgb(
        const Mat &target,
        const Rect &region
)
{
    Mat roi = target(region);
    vector<Mat> channels(3);
    split(roi, channels);
    vector<double> sum(3, 0);
    int cnt;
    for (int i = 0; i < 3; ++i) {
        sum[i] = cv::sum(channels[i])[0];
        cnt = cv::countNonZero(channels[i]);
        if (sum[i] != 0)
            sum[i] /= cnt;
    }
    return Vec3b{(unsigned char) sum[2], (unsigned char) sum[1], (unsigned char) sum[0]};
}

void BAASImageUtil::gen_not_black_region_mask(
        const Mat &src,
        Mat &mask,
        const BAASRectangle &region
)
{
    gen_not_black_region_mask(
            src, mask, Rect(
                    region.ul
                          .x, region.ul
                                    .y, region.lr
                                              .x - region.ul
                                                         .x, region.lr
                                                                   .y - region.ul
                                                                              .y
            ));
}

void BAASImageUtil::gen_not_black_region_mask(
        const Mat &src,
        Mat &mask,
        const Rect &region
)
{
    Mat roi = src(region);
    inRange(roi, Scalar(0, 0, 0), Scalar(0, 0, 0), mask);
    bitwise_not(mask, mask);
}

void BAASImageUtil::gen_not_black_region_mask(
        const Mat &src,
        Mat &mask
)
{
    inRange(src, Scalar(0, 0, 0), Scalar(0, 0, 0), mask);
    bitwise_not(mask, mask);
}

// [ r, g, b ]
void BAASImageUtil::pixel2string(
        const Vec3b &pixel,
        string &str
)
{
    str = "[ " + to_string(pixel[2]) + ", " + to_string(pixel[1]) + ", " + to_string(pixel[0]) + " ]";
}



BAASPoint::BAASPoint(
        int xx,
        int yy
)
{
    x = xx;
    y = yy;
}

BAASPoint::BAASPoint()
{
    x = 0;
    y = 0;
}

BAASPoint BAASPoint::rotate(
        int r,
        int angle
) const
{
    double radian = angle * numbers::pi / 180;
    return {int(x - r * sin(radian)), int(y - r * cos(radian))};
}

BAASRectangle::BAASRectangle()
{
    ul.x = 0;
    ul.y = 0;
    lr.x = 0;
    lr.y = 0;
}

BAASRectangle::BAASRectangle(
        int x1,
        int y1,
        int x2,
        int y2
)
{
    ul.x = x1;
    ul.y = y1;
    lr.x = x2;
    lr.y = y2;
}

BAASRectangle::BAASRectangle(
        BAASPoint p1,
        BAASPoint p2
)
{
    ul = p1;
    lr = p2;
}

BAAS_NAMESPACE_END