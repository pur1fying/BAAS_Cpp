#ifndef BAAS_OCR_CRNNNET_H_
#define BAAS_OCR_CRNNNET_H_

#include <filesystem>

#include "OcrStruct.h"
#include "onnxruntime/onnxruntime_cxx_api.h"
#include "opencv2/opencv.hpp"

BAAS_NAMESPACE_BEGIN

class CrnnNet {
public:
    static CrnnNet *get_net(
            const std::filesystem::path &model_path,
            const std::filesystem::path &keys_path
    );

    static bool release_net(
            const std::filesystem::path &model_path,
            const std::filesystem::path &keys_path
    );

    static inline std::string model_key_joined_path(
            const std::filesystem::path &model_path,
            const std::filesystem::path &keys_path
    );

    static void release_all();

    ~CrnnNet();

    void set_num_thread(int num_thread);

    void set_gpu_id(int gpu_id);

    void initModel();

    std::vector<TextLine> getTextLines(
            std::vector<cv::Mat> &partImg,
            const char *path,
            const char *imgName
    );

    std::vector<TextLine> getTextLines(
            std::vector<cv::Mat> &partImg,
            const char *path,
            const char *imgName,
            const std::vector<std::string> &candidates
    );

    TextLine getTextLine(const cv::Mat &src);

    TextLine getTextLine(
            const cv::Mat &src,
            const std::vector<size_t> &enabledIndexes
    );

    void getTextIndexes(
            const std::vector<std::string> &characters,
            std::vector<size_t> &enabledIndexes
    );

    inline const std::filesystem::path getModelPath() const { return model_key_joined_path(modelPath, keyDictPath); }
private:
    void initModel(
            const std::filesystem::path &pathStr,
            const std::filesystem::path &keysPath
    );

    std::filesystem::path modelPath, keyDictPath;

    static std::map<std::string, CrnnNet *> nets;

    bool isOutputDebugImg = false;
    Ort::Session *session;
    Ort::Env env = Ort::Env(ORT_LOGGING_LEVEL_ERROR, "CrnnNet");
    Ort::SessionOptions sessionOptions = Ort::SessionOptions();
    int numThread = 0;

    std::vector<Ort::AllocatedStringPtr> inputNamesPtr;
    std::vector<Ort::AllocatedStringPtr> outputNamesPtr;

    const float meanValues[3] = {127.5, 127.5, 127.5};
    const float normValues[3] = {1.0 / 127.5, 1.0 / 127.5, 1.0 / 127.5};
    const int dstHeight = 48;

    std::vector<std::string> keys;

    std::map<std::string, size_t> character2Index;

    TextLine scoreToTextLine(
            const std::vector<float> &outputData,
            size_t h,
            size_t w
    );

    TextLine scoreToTextLine(
            const std::vector<float> &outputData,
            size_t h,
            size_t w,
            const std::vector<size_t> &enabledIndexes
    );


};

BAAS_NAMESPACE_END


#endif //BAAS_OCR_CRNNNET_H_
