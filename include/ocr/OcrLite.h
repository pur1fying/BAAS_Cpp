#ifndef BAAS_OCR_OCRLITE_H_
#define BAAS_OCR_OCRLITE_H_

#include "onnxruntime/onnxruntime_cxx_api.h"
#include "OcrStruct.h"
#include "DbNet.h"
#include "AngleNet.h"
#include "CrnnNet.h"

BAAS_NAMESPACE_BEGIN

class OcrLite {
public:
    OcrLite();

    ~OcrLite();

    void get_net(
            const std::filesystem::path &detPath,
            const std::filesystem::path &clsPath,
            const std::filesystem::path &recPath,
            const std::filesystem::path &keysPath,
            int gpu_id=-1,
            int num_thread=4
    );

    OcrResult detect(
            const cv::Mat &mat,
            int padding,
            int maxSideLen,
            float boxScoreThresh,
            float boxThresh,
            float unClipRatio,
            bool doAngle,
            bool mostAngle,
            const std::vector<std::string> &candidates = std::vector<std::string>());

    void ocr_for_single_line(
            const cv::Mat &img,
            TextLine &text,
            const std::vector<std::string> &candidates = std::vector<std::string>());

    OcrResult detectImageBytes(
            const uint8_t *data,
            long dataLength,
            int grey,
            int padding,
            int maxSideLen,
            float boxScoreThresh,
            float boxThresh,
            float unClipRatio,
            bool doAngle,
            bool mostAngle
    );

    OcrResult detectBitmap(
            uint8_t *bitmapData,
            int width,
            int height,
            int channels,
            int padding,
            int maxSideLen,
            float boxScoreThresh,
            float boxThresh,
            float unClipRatio,
            bool doAngle,
            bool mostAngle
    );

    int padding = 50;
    const int maxSideLen = 1024;
    float boxScoreThresh = 0.5f;
    float boxThresh = 0.3f;
    const float unClipRatio = 1.6f;
    const bool doAngle = true;
    const int flagDoAngle = 1;
    const bool mostAngle = true;
    const int flagMostAngle = 1;
private:
    std::shared_ptr<DbNet> dbNet;                // det
    std::shared_ptr<AngleNet> angleNet;          // cls
    std::shared_ptr<CrnnNet> crnnNet;            // rec

    std::vector<cv::Mat> getPartImages(
            cv::Mat &src,
            std::vector<TextBox> &textBoxes
    );

    OcrResult detect(
            cv::Mat &src,
            cv::Rect &originRect,
            ScaleParam &scale,
            float boxScoreThresh = 0.6f,
            float boxThresh = 0.3f,
            float unClipRatio = 2.0f,
            bool doAngle = true,
            bool mostAngle = true,
            const std::vector<std::string> &candidates = std::vector<std::string>());

    inline static std::vector<TextBox> submit_getTextBoxes(
            std::shared_ptr<DbNet>& dbNet,
            cv::Mat &src,
            ScaleParam &s,
            float boxScoreThresh,
            float boxThresh,
            float unClipRatio
    )
    {
        return dbNet->getTextBoxes(src, s, boxScoreThresh, boxThresh, unClipRatio);
    }

    inline static std::vector<Angle> submit_getAngles(
            std::shared_ptr<AngleNet>& angleNet,
            std::vector<cv::Mat> &partImages,
            bool doAngle,
            bool mostAngle
    )
    {
        return angleNet->getAngles(partImages, doAngle, mostAngle);
    }

    inline static std::vector<TextLine> submit_getTextLines_withCandidates(
            std::shared_ptr<CrnnNet>& crnnNet,
            std::vector<cv::Mat> &partImages,
            const std::vector<std::string> &candidates
    )
    {
        return crnnNet->getTextLines(partImages, candidates);
    }

    inline static std::vector<TextLine> submit_getTextLines(
            std::shared_ptr<CrnnNet>& crnnNet,
            std::vector<cv::Mat> &partImages
    )
    {
        return crnnNet->getTextLines(partImages);
    }

    inline static TextLine submit_getTextLine_withEnabledIndexes(
            std::shared_ptr<CrnnNet>& crnnNet,
            const cv::Mat &src,
            const std::vector<size_t> &enabledIndexes
    )
    {
        return {crnnNet->getTextLine(src, enabledIndexes)};
    }

    inline static TextLine submit_getTextLine(
            std::shared_ptr<CrnnNet>& crnnNet,
            const cv::Mat &src
    )
    {
        return {crnnNet->getTextLine(src)};
    }

    inline static void submit_dbNet_initModel(DbNet* dbNet)
    {
        dbNet->initModel();
    }

    inline static void submit_angleNet_initModel(AngleNet* angleNet)
    {
        angleNet->initModel();
    }

    inline static void submit_crnnNet_initModel(CrnnNet* crnnNet)
    {
        crnnNet->initModel();
    }

    friend class BAASOCR;

};

BAAS_NAMESPACE_END

#endif //BAAS_OCR_OCRLITE_H_
