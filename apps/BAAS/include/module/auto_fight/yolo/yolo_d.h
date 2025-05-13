//
// Created by Administrator on 2025/5/14.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_YOLO_YOLO_D_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_YOLO_YOLO_D_H_

#include <string>
#include <vector>

#include <opencv2/opencv.hpp>

#include <core_defines.h>

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
    std::string modelPath;
    model_type modelType = YOLO_DETECT_V8;
    std::vector<int> imgSize = { 640, 640 };
    float rectConfidenceThreshold = 0.6;
    float iouThreshold = 0.5;
    int	keyPointsNum = 2;//Note:kpt number for pose
    bool cudaEnable = false;

    int logSeverityLevel = 3;
    int intraOpNumThreads = 1;

    // ---------------
    int gpu_id = -1;
    int num_thread = 4;
    bool enable_cpu_memory_arena = false;

};

struct yolo_res
{
    int classId;
    float confidence;
    cv::Rect box;
    std::vector<cv::Point2f> keyPoints;
};

BAAS_NAMESPACE_END

#endif //BAAS_CPP_MODULE_AUTO_FIGHT_YOLO_YOLO_D_H_
