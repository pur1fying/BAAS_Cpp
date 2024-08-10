//
// Created by pc on 2024/8/9.
//
#include "BAAS.h"
#include "procedure/BAASProcedure.h"

using namespace std;

using namespace nlohmann;

BAAS::BAAS(std::string &config_name) {
    std::string temp = config_name + "\\config.json";

    config = new BAASUserConfig(temp);
    config->update_name();
    config->config_update();
    config->save();

    flag_run = true;

    logger = config->get_logger();

    connection = new BAASConnection(config);

    connection->clear_cache(static_config->getString("google_store_package_name"));

    screenshot = new BAASScreenshot(config->screenshot_method(), connection, config->screenshot_interval());

    screen_ratio = screenshot->get_screen_ratio();

    control = new BAASControl(config->control_method(), screen_ratio, connection);
}

void BAAS::update_screenshot_array() {
    if(!flag_run) throw HumanTakeOverError("Flag Run turn to false manually");
    screenshot->screenshot(latest_screenshot);
}

void BAAS::get_latest_screenshot(cv::Mat &img) {
    if(!flag_run) throw HumanTakeOverError("Flag Run turn to false manually");
    img = latest_screenshot.clone();
}

void BAAS::solve_procedure(const string &name, BAASConfig &output) {
    if(!flag_run) throw HumanTakeOverError("Flag Run turn to false manually");
    BAASProcedure::implement(this, name, output);
}

void BAAS::feature_appear(const string &feature_name,BAASConfig &output, bool show_log) {
    if(!flag_run) throw HumanTakeOverError("Flag Run turn to false manually");
    BAASFeature::appear(connection, feature_name, latest_screenshot, output, show_log);
}








