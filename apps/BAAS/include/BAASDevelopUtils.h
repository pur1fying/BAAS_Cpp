//
// Created by pc on 2024/5/31.
//

#ifndef BAAS_BAASDEVELOPUTILS_H_
#define BAAS_BAASDEVELOPUTILS_H_

#include "BAASTypes.h"

#include <opencv2/opencv.hpp>

#define SKILL1_FULL BAASRectangle(857, 601, 935, 652)
#define SKILL1_LEFT BAASRectangle(857, 601, 898, 652)
#define SKILL1_RIGHT BAASRectangle(899, 601, 935, 652)

#define SKILL2_FULL BAASRectangle(957, 601, 1037, 662)
#define SKILL2_LEFT BAASRectangle(957, 601, 999, 662)
#define SKILL2_RIGHT BAASRectangle(1000, 601, 1037, 662)

#define SKILL3_FULL BAASRectangle(1060, 601, 1139, 662)
#define SKILL3_LEFT BAASRectangle(1060, 601, 1101, 662)
#define SKILL3_RIGHT BAASRectangle(1102, 601, 1139, 662)

BAAS_NAMESPACE_BEGIN

class BAAS;

enum {
    SKILL_FULL = 0,
    SKILL_LEFT = 1,
    SKILL_RIGHT = 2
};

struct screenshot_extract_params {
    std::filesystem::path img_folder;
    double pre_wait = 0.0;
    double interval = 0.5;
    int img_count = 10;
    bool random = false;
    double duration = 60;

    screenshot_extract_params(
            const std::filesystem::path& img_folder,
            double pre_wait = 0.0,
            double interval = 0.5,
            int img_count = 10,
            bool random = false,
            double duration = 60
    ) : img_folder(img_folder), pre_wait(pre_wait), interval(interval), img_count(img_count), random(random), duration(duration) {}

};

class BAASDevelopUtils {

public:

    static void shotStudentSkill(
            BAAS* baas,
            const std::string &name = "student",
            const BAASRectangle &r = SKILL1_FULL,
            const int type = SKILL_FULL
    );

    static void extract_image_rgb_range(
            const cv::Mat &img,
            const std::string &name,
            const BAASRectangle &r,
            const cv::Scalar &min_,
            const cv::Scalar &max_,
            const uint8_t cut_edge = true
    );

    static void fight_screenshot_extract(
            BAAS* baas,
            const screenshot_extract_params& params
    );

    static int get_next_image_index(const std::filesystem::path& folder);
};

BAAS_NAMESPACE_END
#endif //BAAS_BAASDEVELOPUTILS_H_
