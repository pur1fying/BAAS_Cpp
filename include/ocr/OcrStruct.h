#ifndef BAAS_OCR_STRUCT_H_
#define BAAS_OCR_STRUCT_H_

#include "opencv2/core.hpp"
#include <vector>

#include "core_defines.h"

BAAS_NAMESPACE_BEGIN

struct ScaleParam {
    int srcWidth;
    int srcHeight;
    int dstWidth;
    int dstHeight;
    float ratioWidth;
    float ratioHeight;
};

struct TextBox {
    std::vector<cv::Point> boxPoint;
    float score;
};

struct Angle {
    int index;
    float score;
    double time;
};

struct TextLine {
    std::string text;
    std::vector<float> charScores;
    double time;
};

struct TextBlock {
    std::vector<cv::Point> boxPoint;
    float boxScore;
    int angleIndex;
    float angleScore;
    double angleTime;
    std::string text;
    std::vector<float> charScores;
    double crnnTime;
};

struct OcrResult {
    /*
     * strRes : pure text result, every single block separated by '\n'
     */
    double dbNetTime;
    std::vector<TextBlock> textBlocks;
    cv::Mat boxImg;
    double detectTime;
    std::string strRes;
};

BAAS_NAMESPACE_END

#endif //BAAS_OCR_STRUCT_H_
