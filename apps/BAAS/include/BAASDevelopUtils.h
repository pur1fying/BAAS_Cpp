//
// Created by pc on 2024/5/31.
//

#ifndef BAAS_BAASDEVELOPUTILS_H_
#define BAAS_BAASDEVELOPUTILS_H_

#include "BAASTypes.h"

#include <opencv2/opencv.hpp>

#define SKILL1_FULL BAASRectangle(845, 601, 923, 662)
#define SKILL1_LEFT BAASRectangle(845, 601, 886, 662)
#define SKILL1_RIGHT BAASRectangle(886, 601, 923, 662)

#define SKILL2_FULL BAASRectangle(947, 601, 1025, 662)
#define SKILL2_LEFT BAASRectangle(947, 601, 987, 662)
#define SKILL2_RIGHT BAASRectangle(987, 601, 1025, 662)

#define SKILL3_FULL BAASRectangle(1049, 601, 1127, 662)
#define SKILL3_LEFT BAASRectangle(1049, 601, 1089, 662)
#define SKILL3_RIGHT BAASRectangle(1089, 601, 1127, 662)

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
