//
// Created by pc on 2024/8/9.
//

#include "BAAS.h"
#include "procedure/BAASProcedure.h"
#include "module/competition/Competition.h"
#include "module/work/Work.h"
#include "module/collect_activity_fee/CollectActivityFee.h"
#include "module/restart/Restart.h"

using namespace std;

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

void BAAS::solve_procedure(const string &name, BAASConfig &output) {
    if(!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
    BAASProcedure::implement(this, name, output);
}

void BAAS::solve_procedure(const string &name, BAASConfig &output, const bool skip_first_screenshot) {
    if(!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
    json j;
    j["skip_first_screenshot"] = skip_first_screenshot;
    j["show_log"] = true;
    BAASConfig patch(j, logger);
    BAASProcedure::implement(this, name, patch, output);
}

void BAAS::feature_appear(const string &feature_name,BAASConfig &output, bool show_log) {
    if(!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
    BAASFeature::appear(connection, feature_name, latest_screenshot, output, show_log);
}

void BAAS::init_implement_funcs() {
    implement_funcs["competition"] = ISA::Competition::implement;
    implement_funcs["work"] = ISA::Work::implement;
    implement_funcs["collect_activity_fee"] = ISA::CollectActivityFee::implement;
    implement_funcs["restart"] = ISA::Restart::implement;
}











