//
// Created by Administrator on 2025/5/14.
//

#ifndef BAAS_YOLO_YOLO_D_H_
#define BAAS_YOLO_YOLO_D_H_

#include <string>
#include <vector>
#include <filesystem>

#include "opencv2/opencv.hpp"

#include "core_defines.h"

BAAS_NAMESPACE_BEGIN

enum model_type
{
    //FLOAT32 MODEL
    YOLO_DETECT_V8,

    //FLOAT16 MODEL
    YOLO_DETECT_V8_HALF,
};

struct yolo_d
{
    std::filesystem::path model_path, yaml_path;
    model_type modelType = YOLO_DETECT_V8;

    std::pair<int, int> img_size = {640, 640 };
    float rect_threshold = 0.6;

    // ---------------
    int gpu_id = -1;
    int num_thread = 4;
    bool enable_cpu_memory_arena = false;

};


struct yolo_single_res
{
    int classId;
    float confidence;
    cv::Rect box;
    std::vector<cv::Point2f> keyPoints;
};

struct yolo_run_time_info {
    double pre_t;
    double infer_t;
    double post_t;
};

struct yolo_res
{
    std::vector<yolo_single_res> results;
    yolo_run_time_info time_info;
};
BAAS_NAMESPACE_END

#endif //BAAS_YOLO_YOLO_D_H_
