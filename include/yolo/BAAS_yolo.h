//
// Created by Administrator on 2025/5/14.
//

#ifndef BAAS_YOLO_BAAS_YOLO_H_
#define BAAS_YOLO_BAAS_YOLO_H_

#include <string>
#include <filesystem>
#include "onnxruntime/onnxruntime_cxx_api.h"

#include "yolo_d.h"

BAAS_NAMESPACE_BEGIN

class BAAS_Yolo_v8 {

public:

    BAAS_Yolo_v8();

    ~BAAS_Yolo_v8();

    void preprocess_input_image(const cv::Mat& In, cv::Mat& Out);

    void init_model(const yolo_d& d);

    void run_session(
            const cv::Mat& In,
            yolo_res& Out,
            NMS_option nms_op = NO_NMS
    );

    void warm_up();

    const std::vector<std::string>& get_classes() const {
        return classes;
    }

private:

    void _init_yaml();

    void _get_group_num_Out(
            int strideNum,
            int signalResultNum,
            float* data,
            yolo_res& Out
    );

    void _get_whole_nms_Out(
            int strideNum,
            int signalResultNum,
            float* data,
            yolo_res& Out
    );

    void _get_no_nms_Out(
            int strideNum,
            int signalResultNum,
            float* data,
            yolo_res& Out
    );

    template<typename N>
    void _tensor_process(
            N& blob,
            yolo_res& Out,
            NMS_option nms_op
     );

    std::vector<std::string> classes;

    std::filesystem::path model_path, yaml_path;
    model_type type;

    void* blob_ptr;

    std::unique_ptr<Ort::Session> session;
    Ort::Env env = Ort::Env(ORT_LOGGING_LEVEL_ERROR, "Yolo");
    Ort::SessionOptions sessionOptions = Ort::SessionOptions();
    Ort::RunOptions runOptions = Ort::RunOptions();

    int num_thread, gpu_id;

    std::vector<Ort::AllocatedStringPtr> inputNamesPtr;
    std::vector<Ort::AllocatedStringPtr> outputNamesPtr;
    std::vector<const char*> inputNames;
    std::vector<const char*> outputNames;

    std::pair<int, int> img_size;
    std::vector<int64_t> inputNodeDims;
    unsigned long long tensor_size;
    float rect_confidence_threshold, iou_threshold;
    float resize_scales; //letterbox scale
};


BAAS_NAMESPACE_END

#endif //BAAS_YOLO_BAAS_YOLO_H_
