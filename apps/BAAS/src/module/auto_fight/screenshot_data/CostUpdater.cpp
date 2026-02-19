//
// Created by pc on 2025/4/24.
//

#include "module/auto_fight/screenshot_data/CostUpdater.h"

#include <utils/BAASImageUtil.h>
#include <config/BAASStaticConfig.h>
#include <BAASImageResource.h>

BAAS_NAMESPACE_BEGIN

const std::map<std::string, CostUpdater::battle_type_info> CostUpdater::battle_type_info_map = {
        {"normal", battle_type_info({10.0, 10.5, 11}, {824,682,1146,698})},
        {"infinite_assault", battle_type_info({20.0, 20.5, 21.0, 21.5, 22}, {752,687,1149,698})}
};

const int CostUpdater::negative_cost_check_block_count = 5;

CostUpdater::CostUpdater(
        BAAS* baas,
        auto_fight_d* data
) : BaseDataUpdater(baas, data)
{
    _init_static_value();
//    test_cost_detection("D:\\github\\datasets\\baas\\cost_recognition");
}

void CostUpdater::update()
{
    current_cost = 0;
    baas->get_latest_screenshot(img);
    int is_positive = cost_is_positive();
    if (is_positive == 2) return;

    if (is_positive == 0) _detect_positive_cost();

    else if (is_positive == 1) _detect_negative_cost();

//    for(int i = cost_recognize_y.lr.x; i >= cost_recognize_y.ul.x; --i){
//        for(int j = cost_recognize_y.ul.y; j <= cost_recognize_y.lr.y; ++j){
//            if(BAASImageUtil::judge_rgb_range(origin_screenshot, {i, j}, cost_pixel_min_rgb, cost_pixel_max_rgb)){
//                int x = i - (cost_recognize_y.ul.x + (cost_recognize_y.lr.y - j) / 5);
//                _t_x = double(x);
//                int integer = int(_t_x / cost_increase_1_dealt_x);
//                double decimal = (_t_x - double(integer) * cost_increase_1_dealt_x) / 28;
//                if(decimal < 0) decimal = 0;
//                current_cost = integer + decimal;
//                current_cost = current_cost > 10.0 ? 10.0 : current_cost;
//                data->cost = current_cost;
//                return;
//            }
//        }
//    }
}

double CostUpdater::estimated_time_cost()
{
    return average_cost;
}

constexpr std::string CostUpdater::data_name()
{
    return "Cost";
}

void CostUpdater::_init_static_value()
{
    detect_max_cost_when = baas->get_config()->getString("/auto_fight/detect_max_cost_when", "enter_fight");
    roi_y = static_config->getInt("/BAAS/auto_fight/cost/recognize_y/" + data->battle_type);

    if (detect_max_cost_when == "specify") {
        max_cost = baas->get_config()->getDouble("/auto_fight/max_cost", 10.0);
        _read_max_cost_mask();
        _prepare_current_cost_recognize();
    }
    else if (detect_max_cost_when == "enter_fight") max_cost = 0.0;
    else {
        logger->BAASError("Invalid detect_max_cost_when value [ " + detect_max_cost_when + " ].");
        throw ValueError("Invalid detect_max_cost_when value.");
    }

    average_cost = 100;
}

void CostUpdater::display_data()
{
    if(!data->cost.has_value()) logger->BAASInfo("Cost : No Value.");
    else logger->BAASInfo("Cost : [ " + std::to_string(data->cost.value()) + " ]");
}

void CostUpdater::detect_max_cost()
{
    logger->sub_title("Detect Max Cost");
    int is_positive = cost_is_positive();
    if (is_positive == 2) {
        logger->BAASError("Failed to detect cost is positive or negative");
        throw RuntimeError("AutoFight Begin Failed to Detect Max Cost");
    }
    logger->BAASInfo("Cost is positive --> " + std::to_string(is_positive == 0));
    baas->get_latest_screenshot(img);
//    cv::imshow("cost_detect", img);
//    cv::waitKey(0);
    max_cost = detect_max_cost(img, is_positive);
    logger->BAASInfo("Detected Max Cost : " + std::to_string(max_cost));
    _read_max_cost_mask();
    _prepare_current_cost_recognize();
}

void CostUpdater::_read_max_cost_mask()
{
    logger->sub_title("Cost Detection Mask");
    max_cost_mask_name = cost_value_to_mask_name(max_cost);
    logger->BAASInfo("Mask Image Name : " + max_cost_mask_name);
    if(!resource->is_loaded(max_cost_mask_name)) {
        logger->BAASInfo("Cost Mask [ " + max_cost_mask_name + " ] not loaded.");
        throw ValueError("Image Resource For Cost Mask Not Found.");
    }
    _read_max_cost_mask_map(max_cost);
}

void CostUpdater::_read_max_cost_mask_map(double cost)
{
    if (cost_mask_info_map.find(cost) != cost_mask_info_map.end()) return;

    std::string template_name = cost_value_to_mask_name(cost);
    logger->sub_title("Read Cost Mask");
    logger->BAASInfo("Image : " + template_name);
    cv::Mat mask;
    resource->get(template_name, mask);
    BAASRectangle roi = battle_type_info_map.at(data->battle_type).roi;
    cost_mask_info res;

    for(int y = roi.ul.y; y <= roi.lr.y; ++y) {
        cost_block block;
        int x = roi.ul.x;
        while (x <= roi.lr.x) {
            while(x <= roi.lr.x && !BAASImageUtil::judge_rgb_range(mask, {x, y}, {255, 255, 255}, {255, 255, 255})) ++x;
            if (x > roi.lr.x) break;

            int start = x++;
            while(x <= roi.lr.x && BAASImageUtil::judge_rgb_range(mask, {x, y}, {255, 255, 255}, {255, 255, 255})) ++x;

            block.emplace_back(start, x-1);
        }
        if (block.empty()) continue;
        if (res.block_count == 0) res.block_count = int(block.size());
        else if(res.block_count != int(res.block_count)) {
            logger->BAASInfo("Invalid Cost Mask, block count at y = " + std::to_string(y) + " is " + std::to_string(block.size()) + " , expected " + std::to_string(res.block_count));
            throw ValueError("Invalid Cost Mask");
        }
        if (res.start_y == 0) res.start_y = y;
        res.blocks.push_back(block);
    }

    res.rectangles.resize(res.block_count);
    int y_end = res.start_y + int(res.blocks.size()) - 1;

    for (int i = 0; i < res.block_count; ++i) {
        int x_st = roi.ul.x, x_end = roi.lr.x;
        for (const auto& block : res.blocks) {
            x_st = std::max(x_st, block[i].first);
            x_end = std::min(x_end, block[i].second);
        }
        res.rectangles[i] = {x_st, res.start_y, x_end, y_end};
    }

    cost_mask_info_map[cost] = res;
}

void CostUpdater::display_cost_info_map()
{
    logger->sub_title("Display Cost Info Map");
    for(const auto& [cost, info] : cost_mask_info_map) {
        logger->BAASInfo("Cost : " + std::to_string(cost));
        logger->BAASInfo("Block Count:" + std::to_string(info.block_count));
        logger->BAASInfo("Blocks : ");
        for(const auto& block : info.blocks) {
            if (block.empty()) continue;
            std::string tmp;
            for(const auto& [start, end] : block)
                tmp += std::format("({:4d} {:4d})", start, end) + " ";
            logger->BAASInfo(tmp);
        }
        logger->BAASInfo("Rectangles : ");
        std::string tmp;
        for(const auto& rect : info.rectangles) tmp += rect.to_string();
        logger->BAASInfo(tmp);
    }
}

void CostUpdater::test_cost_detection(const std::filesystem::path& img_dir)
{
    auto correct_p = img_dir / "correct.json";
    auto result_p = img_dir / "result.json";
    BAASConfig correct, result(result_p, logger, true);

    bool has_correct = false;
    if (std::filesystem::exists(correct_p)) {
        has_correct = true;
        correct = BAASConfig(correct_p, logger);
    }
    int cnt_all = 0;
    int cnt_max_cost_correct=0;
    int cnt_fail_battle_type=0;
    int cnt_fail_positive_negative=0;

    logger->BAASInfo(std::format("| {:<3} | {:<5} | {:<6} | {:<20} |", "Pos ", "M_C", "Cur_C", "Path"));

    for (const auto& entry : std::filesystem::recursive_directory_iterator(img_dir)) {
        if (!entry.is_regular_file() || entry.path().extension().string() != ".png") continue;

        cv::Mat img = cv::imread(entry.path().string());
        ++cnt_all;
        baas->set_latest_screenshot(img);
        baas->reset_all_feature();
        // battle type is dynamic when we test, but fixed in real detection
        std::string cost_name = entry.path().parent_path().filename().string();
        double real_max_cost = std::stod(cost_name);
        if (real_max_cost >= 10.0 && real_max_cost <= 11.0) data->battle_type = "normal";
        else if (real_max_cost >= 20.0 && real_max_cost <= 22.0) data->battle_type = "infinite_assault";
        else {
            ++cnt_fail_battle_type;
            logger->BAASError("Failed to detect battle type from image path : " + entry.path().string());
            continue;
        }
        nlohmann::json single_res = {
                {"is_positive", -1},
                {"max_cost", -1},
                {"cur_cost", -1}
        };
        const std::filesystem::path& full_path = entry.path();
        const std::filesystem::path rel_path = full_path.lexically_relative(img_dir);

        int is_positive = cost_is_positive();

        single_res["is_positive"] = is_positive;

        std::string display_pos = "UNK";
        std::string display_max_cost = "\\";
        std::string display_cur_cost = "\\";

        if (is_positive == 2) ++cnt_fail_positive_negative;
        else if (is_positive == 0) display_pos = "> 0";
        else display_pos = "< 0";

        if (is_positive != 2) {
            max_cost = detect_max_cost(img, is_positive, true);
            display_max_cost = std::format("{:.1f}", max_cost);
            if(max_cost == real_max_cost) ++cnt_max_cost_correct;
            else display_max_cost += "*";

            roi_y = static_config->getInt("/BAAS/auto_fight/cost/recognize_y/" + data->battle_type);
            max_cost = real_max_cost;
            _prepare_current_cost_recognize();
            update();
            display_cur_cost = std::format("{:.2f}", current_cost);
        }
        logger->BAASInfo(std::format("| {:<3} | {:<5} | {:<6} | {:<20} |", display_pos, display_max_cost, display_cur_cost, rel_path.string()));
    }

    logger->sub_title("Test Cost Detection General Result");
    logger->BAASInfo("Total Image Count      : " + std::to_string(cnt_all));
    logger->BAASInfo("Is_Pos Failed Count    : " + std::to_string(cnt_fail_positive_negative));
    logger->BAASInfo("Max Cost Correct Count : " + std::to_string(cnt_max_cost_correct));
}

int CostUpdater::count_block_color(cv::Mat& image, bool draw_lines)
{
//    cv::imshow("block", image);
    cv::Mat edges;
    cv::Canny(image, edges, 50, 150);
    std::vector<cv::Vec4i> lines;
    cv::HoughLinesP(
            edges,
            lines,
            1,
            CV_PI / 180,
            5,
            5,
            10
    );
//    cv::imshow("canny", edges);
    if (draw_lines)
        for (const auto& l : lines)
            cv::line(image, cv::Point(l[0], l[1]), cv::Point(l[2], l[3]), cv::Scalar(0, 0, 255), 1, cv::LINE_AA);


//    cv::imshow("lines", image);
//    cv::waitKey(0);
    return int(lines.size());
}

double CostUpdater::detect_max_cost(cv::Mat& image, bool is_positive)
{
    double result = 0.0;
    int _min = INT_MAX;
    int check_block_count = 0;
    if (is_positive == 1) check_block_count = negative_cost_check_block_count;

    auto possibles = battle_type_info_map.at(data->battle_type).possible_cost;
    for(const auto& cost : possibles) {
        _read_max_cost_mask_map(cost);
        const auto& mask_info = cost_mask_info_map[cost];
        if (is_positive == 0) check_block_count = mask_info.block_count;

        int cnt = 0;
        for(int i = 0; i < check_block_count; ++i) {
            cv::Mat img_roi = BAASImageUtil::crop(image, mask_info.rectangles[i]);
            cnt += count_block_color(img_roi, false);
            if (cnt >= _min) break;
        }
        if (cnt < _min) {
            result = cost;
            _min = cnt;
        }
    }

    return result;
}

double CostUpdater::detect_max_cost(cv::Mat& image, bool is_positive, bool draw_rect)
{
    // Return will be one of the possibles
    double result = 0.0;
    int _min = INT_MAX;
    auto possibles = battle_type_info_map.at(data->battle_type).possible_cost;
    int check_block_count = 0;
    if (is_positive == 1) check_block_count = negative_cost_check_block_count;

    for(const auto& cost : possibles) {
        _read_max_cost_mask_map(cost);
        const auto& mask_info = cost_mask_info_map[cost];
        if (is_positive == 0) check_block_count = mask_info.block_count;

        int cnt = 0;
        cv::Mat draw_img = cv::Mat(720, 1280, CV_8UC3, cv::Scalar(0, 0, 0));
        for(int i = 0; i < check_block_count; ++i) {
            cv::Mat img_roi = BAASImageUtil::crop(image, mask_info.rectangles[i]).clone();
            cnt += count_block_color(img_roi, true);
            cv::Rect roi = cv::Rect(cv::Point(mask_info.rectangles[i].ul.x, mask_info.rectangles[i].ul.y), cv::Point(mask_info.rectangles[i].lr.x, mask_info.rectangles[i].lr.y));
            img_roi.copyTo(draw_img(roi));
        }
//        cv::imwrite("cost_" + std::to_string(cost) + ".png", draw_img);
//        logger->BAASInfo("Cnt for cost " + std::to_string(cost) + " : " + std::to_string(cnt));
//        cv::imshow("img", image);
//        cv::waitKey(0);
        if (cnt >= _min) continue;

        result = cost;
        _min = cnt;
    }

    return result;
}

int CostUpdater::cost_is_positive()
{
    /*
     * Returns:
     *         0 positive
     *         1 negative
     *         2 unknown
     */
    if(baas->feature_appear("fight_" + data->battle_type + "_cost_positive")) return 0;
    if(baas->feature_appear("fight_" + data->battle_type + "_cost_negative")) return 1;

    // zero cost
    if(baas->feature_appear("fight_" + data->battle_type + "_cost_positive_0")) return 0;
    if(baas->feature_appear("fight_" + data->battle_type + "_cost_negative_0")) return 1;
    return 2;
}

void CostUpdater::_prepare_current_cost_recognize()
{
    detect_info = cost_mask_info_map[max_cost];
    roi_y_idx = roi_y - detect_info.start_y;
    double frac = max_cost - std::floor(max_cost);
    if (frac > 0.4 && frac < 0.6) last_block_is_half = true;
    else last_block_is_half = false;
    assert(roi_y_idx >= 0);
}

void CostUpdater::_detect_positive_cost()
{
    // cost color
    const cv::Vec3b c_c_min = {0, 0, 160};
    const cv::Vec3b c_c_max = {229, 255, 255};

    // cost end color
    const cv::Vec3b c_e_min = {230, 170, 230};
    const cv::Vec3b c_e_max = {255, 255, 255};

    int last_cost_pixel_block=-1, last_cost_pixel_x=-1;
    int last_end_pixel_block=-1, last_end_pixel_x=-1;

    for(int i = 0; i <= detect_info.block_count-1; ++i) {
        for (int j = detect_info.blocks[roi_y_idx][i].first; j <= detect_info.blocks[roi_y_idx][i].second; ++j) {
            // blue cost pixel
            if(BAASImageUtil::judge_rgb_range(img, {j, roi_y}, c_c_min, c_c_max)) {
                last_cost_pixel_block = i;
                last_cost_pixel_x = j;
                continue;
            }

            // bright cost end pixel in block (end of cost line)
            if (BAASImageUtil::judge_rgb_range(img,{j, roi_y},c_e_min,c_e_max)) {
                last_end_pixel_block = i;
                last_end_pixel_x = j;
                continue;
            }

            // end of cost detected
            // bright pixel in this block
            if (last_end_pixel_block != -1) {
                _set_cost(_calc_cost_from_block(last_end_pixel_x, last_end_pixel_block));
                return;
            }

            // 0 cost pixel
            if(last_cost_pixel_block == -1) {
                _set_cost(0.0);
                return;
            }

            // check if bright pixel is between blocks
            if (i > 0) {
                for (int k = detect_info.blocks[roi_y_idx][i - 1].second + 1;k < detect_info.blocks[roi_y_idx][i].first; ++k) {
                    if (BAASImageUtil::judge_rgb_range(img, {k, roi_y}, c_e_min, c_e_max)) {
                        _set_cost(double(i));
                        return;
                    }
                }
            }

            _set_cost(_calc_cost_from_block(last_cost_pixel_x, last_cost_pixel_block));
            return;
        }
    }

    _set_cost(max_cost);
}

void CostUpdater::_detect_negative_cost()
{
    const cv::Vec3b cost_pixel_min_rgb = {180, 100, 0};
    const cv::Vec3b cost_pixel_max_rgb = {255, 255, 120};

    for(int i = 0; i < negative_cost_check_block_count; ++i) {
        for (int j = detect_info.blocks[roi_y_idx][i].first; j <= detect_info.blocks[roi_y_idx][i].second; ++j) {
            if(!BAASImageUtil::judge_rgb_range(img,{j, roi_y},cost_pixel_min_rgb,cost_pixel_max_rgb)){
                _set_cost(-1 *_calc_cost_from_block(j, i));
                return;
            }
        }
    }

    _set_cost(-1 * double(negative_cost_check_block_count));

}

double CostUpdater::_calc_cost_from_block(int x, int block_idx)
{
    double decimal = double(x - detect_info.blocks[roi_y_idx][block_idx].first) * 1.0 / double(detect_info.blocks[roi_y_idx][block_idx].second - detect_info.blocks[roi_y_idx][block_idx].first + 1);
    // last block is half block
    if (block_idx == detect_info.block_count - 1 && last_block_is_half) decimal *= 0.5;
    return double(block_idx) + decimal;
}



BAAS_NAMESPACE_END