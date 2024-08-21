//
// Created by pc on 2024/8/9.
//

#include "BAAS.h"
#include "procedure/BAASProcedure.h"
#include "module/competition/Competition.h"
#include "module/work/Work.h"
#include "module/collect_activity_fee/CollectActivityFee.h"
#include "module/restart/Restart.h"
#include "module/collect_reward/CollectReward.h"
#include "module/mail/Mail.h"

using namespace std;
using namespace cv;
using namespace nlohmann;

map<string, function<bool (BAAS*)>> BAAS::implement_funcs = {};

bool BAAS::solve(const std::string& task) {
    auto it = BAAS::implement_funcs.find(task);
    if(it == BAAS::implement_funcs.end()) {
        logger->BAASError("Task implement not found : [ " + task + " ]");
        return false;
    }
    try {
        logger->hr(task);
        return it->second(this);
    } catch (exception& e) {
        logger->BAASError("Error in solve task: [ " + task + " ] " + e.what());
        return false;
    }
}

BAAS::BAAS(std::string &config_name) {
    std::string temp = config_name + "\\config.json";

    config = new BAASUserConfig(temp);
    config->update_name();
    config->config_update();
    config->save();

    flag_run = true;

    logger = config->get_logger();

    connection = new BAASConnection(config);

    screenshot = new BAASScreenshot(config->screenshot_method(), connection, config->screenshot_interval());

    screen_ratio = screenshot->get_screen_ratio();

    control = new BAASControl(config->control_method(), screen_ratio, connection);
}

void BAAS::update_screenshot_array() {
    if(!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
    screenshot->screenshot(latest_screenshot);
}

void BAAS::get_latest_screenshot(cv::Mat &img) {
    if(!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
    img = latest_screenshot.clone();
}

void BAAS::get_latest_screenshot(Mat &img, const BAASRectangle &region) {
    if(!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
    img = BAASImageUtil::crop(latest_screenshot, region);
}

bool BAAS::feature_appear(const string &feature_name,BAASConfig &output, bool show_log) {
    if(!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
    return BAASFeature::appear(connection, feature_name, latest_screenshot, output, show_log);
}

bool BAAS::feature_appear(const string &feature_name) {
    if(!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
    BAASConfig output;
    return BAASFeature::appear(connection, feature_name, latest_screenshot, output);
}
void BAAS::solve_procedure(const string &name) {
    if(!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
    BAASConfig output;
    solve_procedure(name, output);
}

void BAAS::solve_procedure(const string &name, BAASConfig &output) {
    if(!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
    BAASProcedure::implement(this, name, output);
}

void BAAS::solve_procedure(const string &name, const bool skip_first_screenshot) {
    if(!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
    BAASConfig output;
    solve_procedure(name, output, skip_first_screenshot);
}

void BAAS::solve_procedure(const string &name, BAASConfig &output, const bool skip_first_screenshot) {
    if(!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
    json j;
    j["skip_first_screenshot"] = skip_first_screenshot;
    BAASConfig patch(j, logger);
    BAASProcedure::implement(this, name, patch, output);
}

void BAAS::solve_procedure(const string &name, json &patch) {
    if(!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
    BAASConfig output;
    BAASConfig p(patch, logger);
    solve_procedure(name, output, p);
}

void BAAS::solve_procedure(const string &name, BAASConfig &output, json &patch) {
    if(!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
    BAASConfig p(patch, logger);
    solve_procedure(name, output, p);
}

void BAAS::solve_procedure(const string &name, json &patch, const bool skip_first_screenshot) {
    if(!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
    BAASConfig output;
    BAASConfig p(patch, logger);
    solve_procedure(name, output, p, skip_first_screenshot);
}

void BAAS::solve_procedure(const string &name, BAASConfig &output, json &patch, bool skip_first_screenshot) {
    if(!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
    BAASConfig p(patch, logger);
    p.update("skip_first_screenshot", skip_first_screenshot);
    solve_procedure(name, output, p);
}

void BAAS::solve_procedure(const string &name, BAASConfig &output, BAASConfig &patch) {
    if(!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
    BAASProcedure::implement(this, name, patch, output);
}

void BAAS::solve_procedure(const string &name, BAASConfig &output, BAASConfig &patch, const bool skip_first_screenshot) {
    if(!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
    patch.update("skip_first_screenshot", skip_first_screenshot);
    BAASProcedure::implement(this, name, patch, output);
}


void BAAS::init_implement_funcs() {
    implement_funcs["competition"] = ISA::Competition::implement;
    implement_funcs["work"] = ISA::Work::implement;
    implement_funcs["collect_activity_fee"] = ISA::CollectActivityFee::implement;
    implement_funcs["restart"] = ISA::Restart::implement;
    implement_funcs["collect_reward"] = ISA::CollectReward::implement;
    implement_funcs["mail"] = ISA::Mail::implement;
}

void BAAS::wait_region_static(const BAASRectangle &region, double frame_diff_ratio, double min_static_time, int min_frame_cnt, double max_execute_time) {
    if(!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");

    Mat last_frame, this_frame, last_frame_gray, this_frame_gray, diff;
    int static_frame_cnt = 0;
    long long needed_static_time = int(min_static_time * 1000);
    long long start_time = BAASUtil::getCurrentTimeMS();
    long long static_start_time = BAASUtil::getCurrentTimeMS();
    long long exe_time = int(max_execute_time * 1000);
    get_latest_screenshot(last_frame, region);

    long long this_round_time;

    int diff_pixel_cnt = int(double(region.width() * region.height()) * 1.0 * (1.0-frame_diff_ratio));
    while (flag_run) {
        this_round_time = BAASUtil::getCurrentTimeMS();
        if(this_round_time - start_time >= exe_time) {
            throw RequestHumanTakeOver("Wait region static timeout");
        }
        if(this_round_time - static_start_time >= needed_static_time && static_frame_cnt >= min_frame_cnt) {
            break;
        }
        update_screenshot_array();
        get_latest_screenshot(this_frame, region);
        cvtColor(last_frame, last_frame_gray, COLOR_BGR2GRAY);
        cvtColor(this_frame, this_frame_gray, COLOR_BGR2GRAY);
        absdiff(last_frame_gray, this_frame_gray, diff);
        int diff_cnt = countNonZero(diff);
        if(diff_cnt <= diff_pixel_cnt) {
            static_frame_cnt++;
        } else {
            static_frame_cnt = 0;
            static_start_time = BAASUtil::getCurrentTimeMS();
        }
        last_frame = this_frame.clone();
    }
}

bool BAAS::judge_rgb_range(const BAASPoint &point, const Vec3b &min, const Vec3b &max) {
    return BAASImageUtil::judge_rgb_range(latest_screenshot, point, min, max, screen_ratio);
}


























