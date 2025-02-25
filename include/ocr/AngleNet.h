#ifndef BAAS_OCR_ANGLENET_H_
#define BAAS_OCR_ANGLENET_H_

#include <filesystem>
#include "OcrStruct.h"
#include "onnxruntime/onnxruntime_cxx_api.h"
#include "opencv2/opencv.hpp"

BAAS_NAMESPACE_BEGIN

class AngleNet {
public:
    static AngleNet *get_net(const std::filesystem::path &model_path);

    static bool release_net(const std::filesystem::path &model_path);

    static void release_all();

    ~AngleNet();

    void initModel();

    void setNumThread(int numOfThread);

    void set_gpu_id(int gpu_id);

    std::vector<Angle> getAngles(
            std::vector<cv::Mat> &partImgs,
            const char *path,
            const char *imgName,
            bool doAngle,
            bool mostAngle
    );

private:
    void initModel(const std::filesystem::path &path);

    static std::map<std::string, AngleNet *> nets;

    std::filesystem::path modelPath;

    bool isOutputAngleImg = false;

    Ort::Session *session;
    Ort::Env env = Ort::Env(ORT_LOGGING_LEVEL_ERROR, "AngleNet");
    Ort::SessionOptions sessionOptions = Ort::SessionOptions();
    int numThread = 0;

    std::vector<Ort::AllocatedStringPtr> inputNamesPtr;
    std::vector<Ort::AllocatedStringPtr> outputNamesPtr;

    const float meanValues[3] = {127.5, 127.5, 127.5};
    const float normValues[3] = {1.0 / 127.5, 1.0 / 127.5, 1.0 / 127.5};
    const int dstWidth = 192;
    const int dstHeight = 48;

    Angle getAngle(cv::Mat &src);
};

BAAS_NAMESPACE_END

#endif //BAAS_OCR_ANGLENET_H_
