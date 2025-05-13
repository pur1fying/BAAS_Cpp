//
// Created by Administrator on 2025/5/14.
//

#include "module/auto_fight/yolo/BAAS_yolo.h"

BAAS_NAMESPACE_BEGIN

void BAAS_Yolo_v8::preprocess_input_image(const cv::Mat& In, cv::Mat& Out)
{
    if (In.channels() == 3)
    {
        Out = In.clone();
        cv::cvtColor(In, Out, cv::COLOR_BGR2RGB);
    }
    else
    {
        cv::cvtColor(In, Out, cv::COLOR_GRAY2RGB);
    }

    switch (type)
    {
        case YOLO_DETECT_V8:
        case YOLO_DETECT_V8_HALF:
        {
            if (In.cols >= In.rows)
            {
                resize_scales = In.cols / (float)img_size.first;
                cv::resize(Out, Out, cv::Size(img_size.first, int(In.rows / resize_scales)));
            }
            else
            {
                resize_scales = In.rows / (float)img_size.first
                cv::resize(Out, Out, cv::Size(int(In.cols / resize_scales), img_size.second));
            }
            cv::Mat tempImg = cv::Mat::zeros(img_size.first, img_size.second, CV_8UC3);
            Out.copyTo(tempImg(cv::Rect(0, 0, Out.cols, Out.rows)));
            Out = tempImg;
            break;
        }
    }
}


void BAAS_Yolo_v8::run_session(const cv::Mat& In, std::vector<yolo_res>& Out) {

    cv::Mat processed_img;



}


void BAAS_Yolo_v8::init_model()
{

}

void BAAS_Yolo_v8::warm_up()
{
    clock_t starttime_1 = clock();
    cv::Mat In = cv::Mat(cv::Size(img_size.first, img_size.second), CV_8UC3);
    cv::Mat processedImg;
    preprocess_input_image(In, img_size, processedImg);
    switch (type) {
        case YOLO_DETECT_V8
    {
        float* blob = new float[In.total() * 3];
        BlobFromImage(processedImg, blob);
        std::vector<int64_t> YOLO_input_node_dims = { 1, 3, imgSize.at(0), imgSize.at(1) };
        Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
                Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU), blob, 3 * imgSize.at(0) * imgSize.at(1),
                YOLO_input_node_dims.data(), YOLO_input_node_dims.size());
        auto output_tensors = session->Run(options, inputNodeNames.data(), &input_tensor, 1, outputNodeNames.data(),
                                           outputNodeNames.size());
        delete[] blob;
        clock_t starttime_4 = clock();
        double post_process_time = (double)(starttime_4 - starttime_1) / CLOCKS_PER_SEC * 1000;
        if (cudaEnable)
        {
            std::cout << "[YOLO_V8(CUDA)]: " << "Cuda warm-up cost " << post_process_time << " ms. " << std::endl;
        }
    }
    case YOLO_DETECT_V8_HALF:
    {
#ifdef __CUDA__
        half* blob = new half[iImg.total() * 3];
        BlobFromImage(processedImg, blob);
        std::vector<int64_t> YOLO_input_node_dims = { 1,3,imgSize.at(0),imgSize.at(1) };
        Ort::Value input_tensor = Ort::Value::CreateTensor<half>(Ort::MemoryInfo::CreateCpu(OrtDeviceAllocator, OrtMemTypeCPU), blob, 3 * imgSize.at(0) * imgSize.at(1), YOLO_input_node_dims.data(), YOLO_input_node_dims.size());
        auto output_tensors = session->Run(options, inputNodeNames.data(), &input_tensor, 1, outputNodeNames.data(), outputNodeNames.size());
        delete[] blob;
        clock_t starttime_4 = clock();
        double post_process_time = (double)(starttime_4 - starttime_1) / CLOCKS_PER_SEC * 1000;
        if (cudaEnable)
        {
            std::cout << "[YOLO_V8(CUDA)]: " << "Cuda warm-up cost " << post_process_time << " ms. " << std::endl;
        }
#endif
    }
    }
}

BAAS_NAMESPACE_END