//
// Created by pc on 2025/4/24.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_DATA_COSTUPDATER_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_DATA_COSTUPDATER_H_

#include "BaseDataUpdater.h"

BAAS_NAMESPACE_BEGIN

class CostUpdater : public BaseDataUpdater {

public:

    struct battle_type_info {
        std::vector<double> possible_cost;
        BAASRectangle roi;

        battle_type_info(const std::vector<double>& possible_cost,const BAASRectangle& roi) {
            this->possible_cost = possible_cost;
            this->roi = roi;
        }
    };

    explicit CostUpdater(BAAS* baas, auto_fight_d* data);

    void update() override;

    double estimated_time_cost() override;

    constexpr std::string data_name() override;

    void display_data() override;

    void detect_max_cost();

    inline const std::string& get_detect_max_cost_when() {
        return detect_max_cost_when;
    }

    inline std::string cost_value_to_mask_name(double cost) {
        return baas->get_image_resource_prefix() + "plot_cost_mask." + std::format("{:.1f}", cost);
    }

    void display_cost_info_map();

    void test_cost_detection(const std::filesystem::path& img_dir);

    double detect_max_cost(cv::Mat& image, bool is_positive, bool draw_rect);

    double detect_max_cost(cv::Mat& image, bool is_positive);

    virtual void write_result_into_data() override;

private:

    inline void _set_cost(double cost) {
        current_cost = cost;
    }

    double _calc_cost_from_block(int x, int block_idx);

    void _detect_positive_cost();

    void _detect_negative_cost();

    const static int negative_cost_check_block_count;

    int cost_is_positive();

    int count_block_color(cv::Mat& image, bool draw_lines = false);

    typedef std::pair<int, int> line;

    typedef std::vector<line> cost_block;

    struct cost_mask_info {
        std::vector<cost_block> blocks;  // record from roi[1] --> roi[3]
        std::vector<BAASRectangle> rectangles;
        int start_y = 0;
        int block_count = 0;
    };

    std::map<double, cost_mask_info> cost_mask_info_map;

    void _read_max_cost_mask_map(double cost);

    void _prepare_current_cost_recognize();

    void _init_static_value();

    void _read_max_cost_mask();

    // static values

    // Detect Max Cost
    std::string detect_max_cost_when;
    std::string max_cost_mask_name;
    static const std::map<std::string, battle_type_info> battle_type_info_map;

    double max_cost;
    bool last_block_is_half;
    double average_cost;
    cv::Mat max_cost_mask;
    int roi_y;
    cost_mask_info detect_info;
    int roi_y_idx;

    // change in runtime
    double current_cost; // [-5.0, 22.0]
    cv::Mat img;
};

BAAS_NAMESPACE_END

#endif //BAAS_CPP_MODULE_AUTO_FIGHT_SCREENSHOT_DATA_COSTUPDATER_H_
