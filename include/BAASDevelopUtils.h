//
// Created by pc on 2024/5/31.
//

#ifndef BAAS_BAASDEVELOPUTILS_H_
#define BAAS_BAASDEVELOPUTILS_H_
#include "BAAS.h"
#define SKILL1_FULL BAASRectangle(845, 601, 923, 662)
#define SKILL1_LEFT BAASRectangle(845, 601, 886, 662)
#define SKILL1_RIGHT BAASRectangle(886, 601, 923, 662)

#define SKILL2_FULL BAASRectangle(947, 601, 1025, 662)
#define SKILL2_LEFT BAASRectangle(947, 601, 987, 662)
#define SKILL2_RIGHT BAASRectangle(987, 601, 1025, 662)

#define SKILL3_FULL BAASRectangle(1049, 601, 1127, 662)
#define SKILL3_LEFT BAASRectangle(1049, 601, 1089, 662)
#define SKILL3_RIGHT BAASRectangle(1089, 601, 1127, 662)
enum {
    SKILL_FULL = 0,
    SKILL_LEFT = 1,
    SKILL_RIGHT = 2
};
class BAASDevelopUtils {
public:
    static void shotStudentSkill
    (
            const cv::Mat &image,
            const std::string &name1 = "student1",
            const std::string &name2 = "student2",
            const std::string &name3 = "student3",
            const int type = SKILL_FULL
    );

    static void extract_image_rgb_range(const cv::Mat& img, const std::string &name, const BAASRectangle &r, const cv::Scalar &min_ , const cv::Scalar &max_, const uint8_t cut_edge = true);
};
#endif //BAAS_BAASDEVELOPUTILS_H_
