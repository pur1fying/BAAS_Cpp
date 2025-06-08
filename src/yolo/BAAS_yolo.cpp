//
// Created by Administrator on 2025/5/14.
//


#include "yolo/BAAS_yolo.h"

#include <fstream>

#ifdef __CUDA__
#include <cuda_fp16.h>
namespace Ort
{
    template<>
    struct TypeToTensorType<half> { static constexpr ONNXTensorElementDataType type = ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT16; };
}
#endif // __CUDA__

#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>

#include "BAASLogger.h"
#include "ocr/OcrUtils.h"
#include "BAASExceptions.h"

BAAS_NAMESPACE_BEGIN

BAAS_Yolo_v8::BAAS_Yolo_v8()
{

}

BAAS_Yolo_v8::~BAAS_Yolo_v8()
{
    session.reset();
    inputNamesPtr.clear();
    outputNamesPtr.clear();
    delete[] blob_ptr;
}

template<typename T>
void blob_from_image(cv::Mat& iImg, T& iBlob) {
    int channels = iImg.channels();
    int imgHeight = iImg.rows;
    int imgWidth = iImg.cols;

    for (int c = 0; c < channels; c++)
        for (int h = 0; h < imgHeight; h++)
            for (int w = 0; w < imgWidth; w++)
                iBlob[c * imgWidth * imgHeight + h * imgWidth + w] =
                        typename std::remove_pointer<T>::type((iImg.at<cv::Vec3b>(h, w)[c]) / 255.0f);
}


void BAAS_Yolo_v8::preprocess_input_image(const cv::Mat& In, cv::Mat& Out)
{
    if (In.channels() == 3) {
        Out = In.clone();
        cv::cvtColor(In, Out, cv::COLOR_BGR2RGB);
    }
    else cv::cvtColor(In, Out, cv::COLOR_GRAY2RGB);

    switch (type) {
        case YOLO_DETECT_V8:

        case YOLO_DETECT_V8_HALF: {
            if (In.cols >= In.rows) {
                resize_scales = In.cols / (float)img_size.first;
                cv::resize(Out, Out, cv::Size(img_size.first, int(In.rows / resize_scales)));
            }
            else {
                resize_scales = In.rows / (float)img_size.first;
                cv::resize(Out, Out, cv::Size(int(In.cols / resize_scales), img_size.second));
            }
            cv::Mat tempImg = cv::Mat::zeros(img_size.first, img_size.second, CV_8UC3);
            Out.copyTo(tempImg(cv::Rect(0, 0, Out.cols, Out.rows)));
            Out = tempImg;
            break;
        }
    }
}


void BAAS_Yolo_v8::run_session(const cv::Mat& In, yolo_res& Out, NMS_option nms_op)
{
    Out.results.clear();
    Out.time_info.pre_t = getCurrentTime();
    cv::Mat processed_img;
    preprocess_input_image(In, processed_img);
    switch (type) {

        case YOLO_DETECT_V8: {
            float* blob = (float*)blob_ptr;
            blob_from_image(processed_img, blob);
            _tensor_process(blob, Out, nms_op);
            break;
        }

        case YOLO_DETECT_V8_HALF: {
#ifdef __CUDA__
            half* blob = (half*)blob_ptr;
            blob_from_image(processed_img, blob);
            _tensor_process(blob, Out, nms_op);
            break;
#endif // __CUDA__
        }
    }
}


void BAAS_Yolo_v8::init_model(const yolo_d& d)
{
    try{
        if(d.modelType == YOLO_DETECT_V8_HALF && d.gpu_id < 0) {
            BAASGlobalLogger->BAASError("Fp16 model must use GPU Infer");
            throw RuntimeError("Fp16 model must use GPU Infer");
        }

        rect_confidence_threshold = d.rect_threshold;
        iou_threshold = d.iou_threshold;
        img_size = d.img_size;
        inputNodeDims = { 1, 3, img_size.first, img_size.second };
        tensor_size = 3 * img_size.first * img_size.second;

        type = d.modelType;
        gpu_id = d.gpu_id;
        num_thread = d.num_thread;
        model_path = d.model_path;
        yaml_path = d.yaml_path;

        _init_yaml();

        env = Ort::Env(ORT_LOGGING_LEVEL_WARNING, "Yolo");
#ifdef __CUDA__
        if (gpu_id >= 0) {
            OrtCUDAProviderOptions cudaOption;
            cudaOption.device_id = gpu_id;
            sessionOptions.AppendExecutionProvider_CUDA(cudaOption);
        }
#endif // __CUDA__

        if (d.enable_cpu_memory_arena)  sessionOptions.EnableCpuMemArena();
        else                            sessionOptions.DisableCpuMemArena();

        sessionOptions.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
        sessionOptions.SetInterOpNumThreads(num_thread);
        sessionOptions.SetIntraOpNumThreads(num_thread);

#ifdef _WIN32
        std::wstring modelPath = std::filesystem::path(d.model_path).wstring();
        session = std::make_unique<Ort::Session>(env, modelPath.c_str(), sessionOptions);
#else
        session = std::make_unique<Ort::Session>(env, d.model_path.c_str(), sessionOption);
#endif // _WIN32

        inputNamesPtr = getInputNames(session);
        outputNamesPtr = getOutputNames(session);
        inputNames = {inputNamesPtr.data()->get()};
        outputNames = {outputNamesPtr.data()->get()};
        runOptions = Ort::RunOptions{ nullptr };
    }
    catch (const std::exception& e)
    {
        BAASGlobalLogger->BAASError("Failed to Init YOLO Model.");
        BAASGlobalLogger->BAASError(e.what());
        throw RuntimeError("Failed to Init YOLO Model.");
    }

    switch (type) {
        case YOLO_DETECT_V8: {
            blob_ptr = new float[tensor_size];
            break;
        }
        case YOLO_DETECT_V8_HALF: {
#ifdef __CUDA__
            blob_ptr = new half[tensor_size];
            break;
#endif // __CUDA__
        }
    }
}

void BAAS_Yolo_v8::warm_up()
{
    cv::Mat In = cv::Mat(cv::Size(img_size.first, img_size.second), CV_8UC3);
    cv::Mat processedImg;
    preprocess_input_image(In, processedImg);
    switch (type) {
        case YOLO_DETECT_V8: {
            float* blob = (float*) blob_ptr;
            blob_from_image(processedImg, blob);
            std::vector<int64_t> YOLO_input_node_dims = {1, 3, img_size.first, img_size.second};
            Ort::Value input_tensor =
                    Ort::Value::CreateTensor<float>(
                            memory_info,
                            blob,
                            tensor_size,
                            inputNodeDims.data(),
                            inputNodeDims.size()
                    );

            auto output_tensors = session->Run(
                    runOptions,
                    inputNames.data(),
                    &input_tensor,
                    1,
                    outputNames.data(),
                    outputNames.size()
            );
            break;
        }
        case YOLO_DETECT_V8_HALF: {
#ifdef __CUDA__
            half* blob =  (half*) blob_ptr;
            blob_from_image(processedImg, blob);
            Ort::Value input_tensor =
                    Ort::Value::CreateTensor<half>(
                            memory_info,
                            blob,
                            tensor_size,
                            inputNodeDims.data(),
                            inputNodeDims.size()
                    );
            auto output_tensors = session->Run(
                    runOptions,
                    inputNames.data(),
                    &input_tensor,
                    1,
                    outputNames.data(),
                    outputNames.size()
            );
            break;
#endif
        }
    }
}



template<typename N>
void BAAS_Yolo_v8::_tensor_process(
        N& blob,
        yolo_res& Out,
        NMS_option nms_op
)
{
    Ort::Value inputTensor =
            Ort::Value::CreateTensor<typename std::remove_pointer<N>::type>(
                    memory_info,
                    blob,
                    tensor_size,
                    inputNodeDims.data(),
                    inputNodeDims.size()
            );
    Out.time_info.infer_t = getCurrentTime();

    auto outputTensor = session->Run(
            runOptions,
            inputNames.data(),
            &inputTensor,
            1,
            outputNames.data(),
            outputNames.size()
    );

    Out.time_info.post_t = getCurrentTime();

    Ort::TypeInfo typeInfo = outputTensor.front().GetTypeInfo();
    auto tensor_info = typeInfo.GetTensorTypeAndShapeInfo();
    std::vector<int64_t> outputNodeDims = tensor_info.GetShape();

    auto output = outputTensor.front().GetTensorMutableData<typename std::remove_pointer<N>::type>();
    switch (type) {
        case YOLO_DETECT_V8:
        case YOLO_DETECT_V8_HALF: {
            int signalResultNum = outputNodeDims[1];//84
            int strideNum = outputNodeDims[2];//8400
            cv::Mat rawData;
            if (type == YOLO_DETECT_V8) {
                // FP32
                rawData = cv::Mat(signalResultNum, strideNum, CV_32F, output);
            } else {
                // FP16
                rawData = cv::Mat(signalResultNum, strideNum, CV_16F, output);
                rawData.convertTo(rawData, CV_32F);
            }
            // Note:
            // ultralytics add transpose operator to the output of yolov8 model.which make yolov8/v5/v7 has same shape
            // https://github.com/ultralytics/assets/releases/download/v8.3.0/yolov8n.pt
            rawData = rawData.t();

            float* data = (float*) rawData.data;
            switch (nms_op) {
                case GROUP_NMS:
                    _get_group_num_Out(strideNum, signalResultNum, data, Out);
                    break;
                case WHOLE_NMS:
                    _get_whole_nms_Out(strideNum, signalResultNum, data, Out);
                    break;
                case NO_NMS:
                    _get_no_nms_Out(strideNum, signalResultNum, data, Out);
                    break;
            }

            double t_end = getCurrentTime();
            Out.time_info.pre_t = Out.time_info.infer_t - Out.time_info.pre_t;
            Out.time_info.infer_t = Out.time_info.post_t - Out.time_info.infer_t;
            Out.time_info.post_t = t_end - Out.time_info.post_t;
        }
    }
}

void BAAS_Yolo_v8::_init_yaml()
{
    // Open the YAML file
    std::ifstream file(yaml_path.c_str());
    if (!file.is_open()) throw PathError("Yolo Config File Not Found");

    // Read the file line by line
    std::string line;
    std::vector<std::string> lines;
    while (std::getline(file, line)) lines.push_back(line);

    // Find the start and end of the names section
    std::size_t start = 0;
    std::size_t end = 0;
    for (std::size_t i = 0; i < lines.size(); i++) {
        if (lines[i].find("names:") != std::string::npos) start = i + 1;
        else if ((start > 0 && lines[i].find(':') == std::string::npos)) {
            end = i;
            break;
        }
        if (i == lines.size() - 1) end = lines.size();
    }

    // Extract the names
    std::vector<std::string> names;
    for (std::size_t i = start; i < end; i++) {
        std::stringstream ss(lines[i]);
        std::string name;
        std::getline(ss, name, ':'); // Extract the number before the delimiter
        std::getline(ss, name); // Extract the string after the delimiter
        names.push_back(name.substr(1)); // remove space after ':'
    }

    classes = names;
}

void BAAS_Yolo_v8::_get_group_num_Out(
        int strideNum,
        int signalResultNum,
        float* data,
        yolo_res& Out
)
{
    std::map<int, std::vector<int>> m_class_ids;
    std::map<int, std::vector<float>> m_confidences;
    std::map<int, std::vector<cv::Rect>> m_boxes;

    for (int i = 0; i < strideNum; ++i) {
        float* classesScores = data + 4;
        cv::Mat scores(1, this->classes.size(), CV_32FC1, classesScores);
        cv::Point class_id;
        double maxClassScore;
        cv::minMaxLoc(scores, 0, &maxClassScore, 0, &class_id);
        if (maxClassScore > rect_confidence_threshold) {
            m_confidences[class_id.x].push_back(maxClassScore);
            m_class_ids[class_id.x].push_back(class_id.x);
            float x = data[0];
            float y = data[1];
            float w = data[2];
            float h = data[3];

            int left = int((x - 0.5 * w) * resize_scales);
            int top = int((y - 0.5 * h) * resize_scales);

            int width = int(w * resize_scales);
            int height = int(h * resize_scales);
            m_boxes[class_id.x].push_back(cv::Rect(left, top, width, height));
        }
        data += signalResultNum;
    }

    std::vector<int> nmsResult;
    for (auto& it : m_boxes) {
        cv::dnn::NMSBoxes(
                m_boxes[it.first],
                m_confidences[it.first],
                rect_confidence_threshold,
                iou_threshold,
                nmsResult
        );
        yolo_single_res res;
        for (int i = 0; i < nmsResult.size(); ++i) {
            int idx = nmsResult[i];
            res.class_id = it.first;
            res.confidence = m_confidences[it.first][idx];
            res.box = m_boxes[it.first][idx];
            Out.results.push_back(res);
        }
    }
}

void BAAS_Yolo_v8::_get_whole_nms_Out(
        int strideNum,
        int signalResultNum,
        float* data,
        yolo_res& Out
)
{
    std::vector<int> class_ids;
    std::vector<float> confidences;
    std::vector<cv::Rect> boxes;
    for (int i = 0; i < strideNum; ++i) {
        float* classesScores = data + 4;
        cv::Mat scores(1, this->classes.size(), CV_32FC1, classesScores);
        cv::Point class_id;
        double maxClassScore;
        cv::minMaxLoc(scores, 0, &maxClassScore, 0, &class_id);
        if (maxClassScore > rect_confidence_threshold) {
            confidences.push_back(maxClassScore);
            class_ids.push_back(class_id.x);
            float x = data[0];
            float y = data[1];
            float w = data[2];
            float h = data[3];

            int left = int((x - 0.5 * w) * resize_scales);
            int top = int((y - 0.5 * h) * resize_scales);

            int width = int(w * resize_scales);
            int height = int(h * resize_scales);
            boxes.push_back(cv::Rect(left, top, width, height));
        }
        data += signalResultNum;
    }

    std::vector<int> nmsResult;

    cv::dnn::NMSBoxes(
            boxes,
            confidences,
            rect_confidence_threshold,
            iou_threshold,
            nmsResult
    );

    yolo_single_res res;
    for (int i = 0; i < nmsResult.size(); ++i) {
        int idx = nmsResult[i];
        res.class_id = class_ids[idx];
        res.confidence = confidences[idx];
        res.box = boxes[idx];
        Out.results.push_back(res);
    }
}

void BAAS_Yolo_v8::_get_no_nms_Out(
        int strideNum,
        int signalResultNum,
        float* data,
        yolo_res& Out
)
{
    yolo_single_res res;

    for (int i = 0; i < strideNum; ++i) {
        float* classesScores = data + 4;
        cv::Mat scores(1, this->classes.size(), CV_32FC1, classesScores);
        cv::Point class_id;
        double maxClassScore;
        cv::minMaxLoc(scores, 0, &maxClassScore, 0, &class_id);
        if (maxClassScore > rect_confidence_threshold) {
            res.confidence = maxClassScore;
            res.class_id = class_id.x;
            float x = data[0];
            float y = data[1];
            float w = data[2];
            float h = data[3];

            int left = int((x - 0.5 * w) * resize_scales);
            int top = int((y - 0.5 * h) * resize_scales);

            int width = int(w * resize_scales);
            int height = int(h * resize_scales);
            res.box = cv::Rect(left, top, width, height);
            Out.results.push_back(res);
        }
        data += signalResultNum;
    }
}


BAAS_NAMESPACE_END