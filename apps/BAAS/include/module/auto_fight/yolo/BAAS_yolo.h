//
// Created by Administrator on 2025/5/14.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_YOLO_BAAS_YOLO_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_YOLO_BAAS_YOLO_H_

#include <string>
#include <filesystem>
#include <onnxruntime/onnxruntime_cxx_api.h>

#include "yolo_d.h"

BAAS_NAMESPACE_BEGIN

class BAAS_Yolo_v8 {

private:

    BAAS_Yolo_v8(const yolo_d& d);

    void preprocess_input_image(const cv::Mat& In, cv::Mat& Out);

    void init_model();

    void run_session(const cv::Mat& In, std::vector<yolo_res>& Out);

    void warm_up();

private:
    std::vector<std::string> classes;

    std::filesystem::path model_path, yaml_path;
    model_type type;

    std::unique_ptr<Ort::Session> session;
    Ort::Env env = Ort::Env(ORT_LOGGING_LEVEL_ERROR, "Yolo");
    Ort::SessionOptions sessionOptions = Ort::SessionOptions();
    int numThread = 0;
    std::vector<Ort::AllocatedStringPtr> inputNamesPtr;
    std::vector<Ort::AllocatedStringPtr> outputNamesPtr;
    std::vector<const char*> inputNames;
    std::vector<const char*> outputNames;

    std::pair<int, int> img_size;
    float rect_confidence_threshold;
    float threshold;
    float resize_scales; //letterbox scale
};


BAAS_NAMESPACE_END

#endif //BAAS_CPP_MODULE_AUTO_FIGHT_YOLO_BAAS_YOLO_H_
