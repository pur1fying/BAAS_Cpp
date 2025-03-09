#ifndef BAAS_OCR_DBNET_H_
#define BAAS_OCR_DBNET_H_

#include <filesystem>

#include <onnxruntime/onnxruntime_cxx_api.h>
#include <opencv2/opencv.hpp>

#include "OcrStruct.h"
BAAS_NAMESPACE_BEGIN

class DbNet {
public:
    static std::shared_ptr<DbNet> get_net(
            const std::filesystem::path &model_path,
            int gpu_id=-1,
            int num_thread=4
            );


    void initModel();

    static bool try_release_net(const std::string& model_path);

    static void try_release_all();

    ~DbNet();

    void set_num_thread(int num_thread);

    void set_gpu_id(int gpu_id);

    std::vector<TextBox> getTextBoxes(
            cv::Mat &src,
            ScaleParam &s,
            float boxScoreThresh,
            float boxThresh,
            float unClipRatio
    );

    inline const std::filesystem::path getModelPath() const { return modelPath; }


private:
    void initModel(const std::filesystem::path &pathStr);

    static std::map<std::string, std::shared_ptr<DbNet>> nets;

    std::filesystem::path modelPath;

    std::unique_ptr<Ort::Session> session;

    Ort::Env env = Ort::Env(ORT_LOGGING_LEVEL_ERROR, "DbNet");
    Ort::SessionOptions sessionOptions = Ort::SessionOptions();
    int numThread = 0;

    std::vector<Ort::AllocatedStringPtr> inputNamesPtr;
    std::vector<Ort::AllocatedStringPtr> outputNamesPtr;
    std::vector<const char *> inputNames;
    std::vector<const char *> outputNames;

    const float meanValues[3] = {0.485 * 255, 0.456 * 255, 0.406 * 255};
    const float normValues[3] = {1.0 / 0.229 / 255.0, 1.0 / 0.224 / 255.0, 1.0 / 0.225 / 255.0};
};

BAAS_NAMESPACE_END
#endif //BAAS_OCR_DBNET_H_
