#ifndef BAAS_OCR_OCRLITE_H_
#define BAAS_OCR_OCRLITE_H_

#include "opencv2/core.hpp"
#include "onnxruntime/onnxruntime_cxx_api.h"
#include "OcrStruct.h"
#include "DbNet.h"
#include "AngleNet.h"
#include "CrnnNet.h"

#include "nlohmann/json.hpp"

BAAS_NAMESPACE_BEGIN

class OcrLite {
public:
    OcrLite();

    ~OcrLite();

    void set_num_thread(int num_thread);

    void initLogger(
            bool isConsole,
            bool isPartImg,
            bool isResultImg
    );

    void enableResultTxt(
            const char *path,
            const char *imgName
    );

    void set_gpu_id(int gpu_id);

    void get_net(
            const std::filesystem::path &detPath,
            const std::filesystem::path &clsPath,
            const std::filesystem::path &recPath,
            const std::filesystem::path &keysPath
    );

    bool initModels();

    void Logger(
            const char *format,
            ...
    );

    OcrResult detect(
            const char *path,
            const char *imgName,
            int padding,
            int maxSideLen,
            float boxScoreThresh,
            float boxThresh,
            float unClipRatio,
            bool doAngle,
            bool mostAngle,
            const std::vector<std::string> &candidates = std::vector<std::string>());

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


    int numThread = 4;
    int padding = 50;
    const int maxSideLen = 1024;
    float boxScoreThresh = 0.5f;
    float boxThresh = 0.3f;
    const float unClipRatio = 1.6f;
    const bool doAngle = true;
    const int flagDoAngle = 1;
    const bool mostAngle = true;
    const int flagMostAngle = 1;
    const int gpu_id = 0;
private:
    bool isOutputConsole = false;
    bool isOutputPartImg = false;
    bool isOutputResultTxt = false;
    bool isOutputResultImg = false;
    FILE *resultTxt;
    DbNet* dbNet;                // det
    AngleNet* angleNet;          // cls
    CrnnNet* crnnNet;            // rec


    std::vector<cv::Mat> getPartImages(
            cv::Mat &src,
            std::vector<TextBox> &textBoxes,
            const char *path,
            const char *imgName
    );

    OcrResult detect(
            const char *path,
            const char *imgName,
            cv::Mat &src,
            cv::Rect &originRect,
            ScaleParam &scale,
            float boxScoreThresh = 0.6f,
            float boxThresh = 0.3f,
            float unClipRatio = 2.0f,
            bool doAngle = true,
            bool mostAngle = true,
            const std::vector<std::string> &candidates = std::vector<std::string>());


};

BAAS_NAMESPACE_END

#endif //BAAS_OCR_OCRLITE_H_
