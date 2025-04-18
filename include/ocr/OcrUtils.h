#ifndef BAAS_OCR_UTILS_H_
#define BAAS_OCR_UTILS_H_

#include <numeric>
#include <sys/stat.h>

#include <opencv2/core.hpp>
#include <nlohmann/json.hpp>
#include <onnxruntime/onnxruntime_cxx_api.h>

#include "OcrStruct.h"

BAAS_NAMESPACE_BEGIN

void to_json(
        nlohmann::json &j,
        const cv::Point &point
);

void from_json(
        const nlohmann::json &j,
        cv::Point &point
);

void to_json(
        nlohmann::json &j,
        const TextBox &box
);

void from_json(
        const nlohmann::json &j,
        TextBox &box
);


template<typename T, typename... Ts>
static std::unique_ptr<T> makeUnique(Ts &&... params)
{
    return std::unique_ptr<T>(new T(std::forward<Ts>(params)...));
}

template<typename T>
static double getMean(std::vector<T> &input)
{
    auto sum = accumulate(input.begin(), input.end(), 0.0);
    return sum / input.size();
}

template<typename T>
static double getStdev(
        std::vector<T> &input,
        double mean
)
{
    if (input.size() <= 1) return 0;
    double accum = 0.0;
    for_each(
            input.begin(), input.end(), [&](const double d) {
                accum += (d - mean) * (d - mean);
            }
    );
    double stdev = sqrt(accum / (input.size() - 1));
    return stdev;
}

template<class T>
inline T clamp(
        T x,
        T min,
        T max
)
{
    if (x > max)
        return max;
    if (x < min)
        return min;
    return x;
}

double getCurrentTime();

inline bool isFileExists(const std::string &name)
{
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

std::wstring strToWstr(std::string str);

ScaleParam getScaleParam(
        cv::Mat &src,
        const float scale
);

ScaleParam getScaleParam(
        cv::Mat &src,
        const int targetSize
);

std::vector<cv::Point2f> getBox(const cv::RotatedRect &rect);

cv::Mat matRotateClockWise180(cv::Mat src);

cv::Mat matRotateClockWise90(cv::Mat src);

cv::Mat getRotateCropImage(
        const cv::Mat &src,
        std::vector<cv::Point> box
);

cv::Mat adjustTargetImg(
        cv::Mat &src,
        int dstWidth,
        int dstHeight
);

std::vector<cv::Point2f> getMinBoxes(
        const cv::RotatedRect &boxRect,
        float &maxSideLen
);

float boxScoreFast(
        const std::vector<cv::Point2f> &boxes,
        const cv::Mat &pred
);

cv::RotatedRect unClip(
        std::vector<cv::Point2f> box,
        float unClipRatio
);

std::vector<float> substractMeanNormalize(
        cv::Mat &src,
        const float *meanVals,
        const float *normVals
);

std::vector<int> getAngleIndexes(std::vector<Angle> &angles);

std::vector<Ort::AllocatedStringPtr> getInputNames(const std::unique_ptr<Ort::Session>& session);

std::vector<Ort::AllocatedStringPtr> getOutputNames(const std::unique_ptr<Ort::Session>& session);

BAAS_NAMESPACE_END

#endif //BAAS_OCR_UTILS_H_
