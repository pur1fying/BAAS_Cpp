//
// Created by pc on 2024/8/13.
//

#ifndef BAAS_FEATURE_JUDGEPOINTRGBRANGEFEATURE_H_
#define BAAS_FEATURE_JUDGEPOINTRGBRANGEFEATURE_H_

#include "feature/BaseFeature.h"
#include "BAASImageResource.h"

#define BAAS_JUDGE_POINT_RGB_RANGE_FEATURE 2

BAAS_NAMESPACE_BEGIN

class JudgePointRGBRangeFeature : public BaseFeature {

private:

    struct RGBInfo{
        BAASPoint p;
        cv::Vec3b _min, _max;

        static constexpr auto _p_format = "({:>4},{:>4})";
        static constexpr auto _rgb_format = "({:>3},{:>3},{:>3},{:>3},{:>3},{:>3})";

        [[nodiscard]] std::string get_position() const {
            return std::format(_p_format, p.x, p.y);
        }
        [[nodiscard]] std::string get_rgb_range() const {
            return std::format(_rgb_format, _min[0], _max[0], _min[1], _max[1], _min[2], _max[2]);
        }

    };

    static void _decode_single_rgb_info(const nlohmann::json& j, std::vector<RGBInfo>& out);

public:

    enum Op {
        ALL,
        ANY,
        ALL_NOT,
        ANY_NOT
    };

    static const std::map<int, Op> op_map;

    explicit JudgePointRGBRangeFeature(BAASConfig* config);

    void show() override;

    bool appear(
            const BAAS* baas,
            BAASConfig& output
    ) override;

    [[nodiscard]] double self_average_cost(
            const BAAS* baas
    ) override;

private:

    Op _op;

    bool check_around;

    int around_range;

    int feature_direction;

    std::map<std::string, std::vector<RGBInfo>> rgb_info;
};

class JudgePointRGBRangeFeatureError : public std::exception {

public:

    explicit JudgePointRGBRangeFeatureError(const std::string& message) : message(message) {}

    [[nodiscard]] const char* what() const noexcept override
    {
        return message.c_str();
    }

private:

    std::string message;
};

BAAS_NAMESPACE_END
#endif //BAAS_FEATURE_JUDGEPOINTRGBRANGEFEATURE_H_
