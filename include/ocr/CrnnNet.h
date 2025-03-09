#ifndef BAAS_OCR_CRNNNET_H_
#define BAAS_OCR_CRNNNET_H_

#include <filesystem>

#include "OcrStruct.h"
#include "onnxruntime/onnxruntime_cxx_api.h"
#include "opencv2/opencv.hpp"

BAAS_NAMESPACE_BEGIN

class CrnnNet {
public:
    static std::shared_ptr<CrnnNet> get_net(
            const std::filesystem::path &model_path,
            const std::filesystem::path &keys_path,
            int gpu_id=-1,
            int num_thread=4
    );

    static bool try_release_net(
            const std::string& joined_path
    );

    static inline std::string model_key_joined_path(
            const std::filesystem::path &model_path,
            const std::filesystem::path &keys_path
    )
    {
        return model_path.string() + " | " + keys_path.string();
    }

    static void try_release_all();

    ~CrnnNet();

    void set_num_thread(int num_thread);

    void set_gpu_id(int gpu_id);

    void initModel();

    std::vector<TextLine> getTextLines(
            std::vector<cv::Mat> &partImg
    );

    std::vector<TextLine> getTextLines(
            std::vector<cv::Mat> &partImg,
            const std::vector<std::string> &candidates
    );

    TextLine getTextLine(const cv::Mat &src);

    TextLine getTextLine(
            const cv::Mat &src,
            const std::vector<size_t> &enabledIndexes
    );

    inline std::vector<Ort::Value> session_run(const Ort::Value* inputTensors)
    {
        return session->Run(
                Ort::RunOptions{nullptr},
                inputNames.data(),
                inputTensors,
                inputNames.size(),
                outputNames.data(),
                outputNames.size()
        );
    }

    void getTextIndexes(
            const std::vector<std::string> &characters,
            std::vector<size_t> &enabledIndexes
    );

    inline const std::filesystem::path get_joined_path() const { return model_key_joined_path(modelPath, keyDictPath); }
private:
    void initModel(
            const std::filesystem::path &pathStr,
            const std::filesystem::path &keysPath
    );
    std::filesystem::path modelPath, keyDictPath;

    static std::map<std::string, std::shared_ptr<CrnnNet>> nets;

    std::unique_ptr<Ort::Session> session;
    Ort::Env env = Ort::Env(ORT_LOGGING_LEVEL_ERROR, "CrnnNet");
    Ort::SessionOptions sessionOptions = Ort::SessionOptions();

    int numThread = 0;

    std::vector<Ort::AllocatedStringPtr> inputNamesPtr;
    std::vector<Ort::AllocatedStringPtr> outputNamesPtr;
    std::vector<const char *> inputNames;
    std::vector<const char *> outputNames;

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
