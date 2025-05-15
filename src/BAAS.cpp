//
// Created by pc on 2024/8/9.
//

#include "BAAS.h"

#include "ocr/BAASOCR.h"
#include "procedure/AppearThenClickProcedure.h"

using namespace std;
using namespace cv;
using namespace nlohmann;

BAAS_NAMESPACE_BEGIN

std::map<std::string, std::function<bool(BAAS *)>> BAAS::module_implement_funcs;

bool BAAS::feature_appear(
        const string& feature_name,
        BAASConfig& output,
        bool show_log
)
{
    if (!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");

    return BAASFeature::feature_appear(this, feature_name, output, show_log);
}

bool BAAS::feature_appear(const string& feature_name)
{
    BAASConfig output;
    return feature_appear(feature_name, output, script_show_image_compare_log);
}

BAAS::BAAS(std::string& config_name)
{
    logger = BAASLogger::get(config_name);
    logger->hr("BAAS Instance [ " + config_name + " ]");

    std::filesystem::path temp = config_name;
    temp =  temp / "config.json";

    config = new BAASUserConfig(temp);
    config->update_name();
    config->config_update();
    config->save();

    flag_run = true;

    script_show_image_compare_log = config->script_show_image_compare_log();

    logger->BAASInfo("Show compare image log: " + to_string(script_show_image_compare_log));

    connection = new BAASConnection(config);

    screenshot = new BAASScreenshot(config->screenshot_method(), connection, config->screenshot_interval());

    screen_ratio = screenshot->get_screen_ratio();

    control = new BAASControl(config->control_method(), screen_ratio, connection);

    server = connection->get_server();

    language = connection->get_language();

    image_resource_prefix = server + "." + language + ".";

    rgb_feature_key = server + "_" + language;

    _init_procedures();

    _init_feature_state_map();
}

void BAAS::update_screenshot_array()
{
    if (!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
    screenshot->screenshot(latest_screenshot);
}

void BAAS::i_update_screenshot_array()
{
    if (!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
    screenshot->immediate_screenshot(latest_screenshot);
}

void BAAS::get_latest_screenshot_clone(cv::Mat &img)
{
    if (!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
    img = latest_screenshot.clone();
}

void BAAS::get_latest_screenshot(Mat &img)
{
    if (!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
    img = latest_screenshot;
}

void BAAS::get_latest_screenshot(
        Mat &img,
        const BAASRectangle &region
)
{
    if (!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
    img = BAASImageUtil::crop(latest_screenshot, region);
}
void BAAS::wait_region_static(
        const BAASRectangle &region,
        double frame_diff_ratio,
        double min_static_time,
        int min_frame_cnt,
        double max_execute_time
)
{
    if (!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");

    Mat last_frame, this_frame, last_frame_gray, this_frame_gray, diff;
    int static_frame_cnt = 0;
    long long needed_static_time = int(min_static_time * 1000);
    long long start_time = BAASUtil::getCurrentTimeMS();
    long long static_start_time = BAASUtil::getCurrentTimeMS();
    long long exe_time = int(max_execute_time * 1000);
    get_latest_screenshot(last_frame, region);

    long long this_round_time;

    int diff_pixel_cnt = int(double(region.width() * region.height()) * 1.0 * (1.0 - frame_diff_ratio));
    while (flag_run) {
        this_round_time = BAASUtil::getCurrentTimeMS();
        if (this_round_time - start_time >= exe_time) {
            throw RequestHumanTakeOver("Wait region static timeout");
        }
        if (this_round_time - static_start_time >= needed_static_time && static_frame_cnt >= min_frame_cnt) {
            break;
        }
        update_screenshot_array();
        get_latest_screenshot(this_frame, region);
        cvtColor(last_frame, last_frame_gray, COLOR_BGR2GRAY);
        cvtColor(this_frame, this_frame_gray, COLOR_BGR2GRAY);
        absdiff(last_frame_gray, this_frame_gray, diff);
        int diff_cnt = countNonZero(diff);
        if (diff_cnt <= diff_pixel_cnt) {
            static_frame_cnt++;
        } else {
            static_frame_cnt = 0;
            static_start_time = BAASUtil::getCurrentTimeMS();
        }
        last_frame = this_frame.clone();
    }
}

void BAAS::get_each_round_type(vector<int> &round_type)
{
    round_type.clear();
    BAASPoint center(62, 89);
    int r = 40;
    optional<int> type;
    int last_type = -1, pixel_cnt = 0;
    for (int i = 0; i <= 359; i++) {
        BAASPoint p = center.rotate(r, i);
        type = point2type(p);
        if (type.has_value()) {
            if (type == 0) {     // meet background
                last_type = 0;
                continue;
            } else {
                if (last_type == type) {
                    pixel_cnt++;
                    if (pixel_cnt == 4) round_type.push_back(type.value());
                } else {
                    last_type = type.value();
                    pixel_cnt = 1;
                }
            }
        }
        latest_screenshot.at<cv::Vec3b>(p.y, p.x) = {0, 0, 255};
    }
}

std::optional<int> BAAS::point2type(const BAASPoint &point)
{
    if (judge_rgb_range(point, {26, 30, 59}, {66, 70, 99})) {        // background
        return 0;
    }
    if (judge_rgb_range(point, {222, 33, 112}, {255, 73, 152})) {    // red
        return 1;
    }
    if (judge_rgb_range(point, {0, 113, 200}, {50, 153, 255})) {     // blue
        return 2;
    }
    if (judge_rgb_range(point, {235, 147, 26}, {255, 187, 66})) {    // yellow
        return 3;
    }

    return std::nullopt;
}

bool BAAS::judge_rgb_range(
        const BAASPoint &point,
        const Vec3b &min,
        const Vec3b &max
)
{
    return BAASImageUtil::judge_rgb_range(latest_screenshot, point, min, max, screen_ratio);
}

void BAAS::ocr_for_single_line(
        const string &language,
        TextLine &result,
        const BAASRectangle &region,
        const std::string &log_content,
        const string &candidates
)
{
    if (!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
    cv::Mat roi_img;
    screenshot_cut(region, roi_img);

    baas_ocr->ocr_for_single_line(language, roi_img, result, log_content, logger, candidates);
}

void BAAS::screenshot_cut(
        const BAASRectangle &region,
        Mat &output
)
{
    // crop an screenshot, region resized by screen_ratio
    int x1 = int(region.ul.x * screen_ratio);
    int y1 = int(region.ul.y * screen_ratio);
    int x2 = int(region.lr.x * screen_ratio);
    int y2 = int(region.lr.y * screen_ratio);
    BAASImageUtil::crop(latest_screenshot, x1, y1, x2, y2).copyTo(output);
}

void BAAS::ocr(
        const string &language,
        OcrResult &result,
        const BAASRectangle &region,
        const string &candidates
)
{
    if (!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
    cv::Mat roi_img;
    screenshot_cut(region, roi_img);
    baas_ocr->ocr(language, roi_img, result, logger, candidates);
}

void BAAS::check_config(string &config_name)
{
    BAASGlobalLogger->sub_title("Config Check");
    BAASGlobalLogger->BAASInfo("Name : " + config_name);
    std::filesystem::path config_dir = BAAS_CONFIG_DIR / config_name;
    if (!std::filesystem::exists(config_dir)) {
        BAASGlobalLogger->BAASInfo("Config Dir Not Exist");
        std::filesystem::create_directory(config_dir);
        write_all_default_config(config_dir);
    }
    else {
        check_user_config(config_name);
    }
}

void BAAS::write_all_default_config(const std::filesystem::path& dir)
{
    BAASGlobalLogger->sub_title("Write All Default Config");
    // user config
    std::ofstream ofs(dir / "config.json");
    ofs << config_template->get_config().dump(4);
    ofs.close();
}

void BAAS::check_user_config(const string &config_name)
{
    std::filesystem::path user_config_path = BAAS_CONFIG_DIR / config_name / "config.json";
    bool write_default = false;
    if (!std::filesystem::exists(user_config_path)) {
        BAASGlobalLogger->BAASInfo("User Config Not Exist");
        write_default = true;
    }
    else {
        std::ifstream file(user_config_path);
        // check json
        if (nlohmann::json::accept(file)) {
        } else {
            BAASGlobalLogger->BAASWarn("User Config Invalid");
            write_default = true;
        }
    }
    if (write_default) {
        BAASGlobalLogger->sub_title("Write Default User Config");
        std::ofstream ofs(user_config_path);
        ofs << config_template->get_config().dump(4);
        ofs.close();
    }
}

void BAAS::_init_feature_state_map()
{
    std::vector<std::string> feature_list = BAASFeature::get_feature_list();
    for (const auto &feature: feature_list)
        feature_state_map[feature] = {
            nullopt,
            BAASFeature::get_feature_ptr(feature)->all_average_cost(this)
        };
}

void BAAS::solve_procedure(const string &procedure_name, bool skip_first_screenshot)
{
    BAASConfig _t;
    solve_procedure(procedure_name, _t, skip_first_screenshot);
}

void BAAS::solve_procedure(const string &procedure_name, const BAASConfig &patch, bool skip_first_screenshot)
{
    BAASConfig _t;
    solve_procedure(procedure_name, _t, patch, skip_first_screenshot);
}

void BAAS::solve_procedure(
        const string &procedure_name,
        BAASConfig &output,
        bool skip_first_screenshot
)
{
    auto it = procedures.find(procedure_name);
    if (it == procedures.end()) {
        BAASGlobalLogger->BAASError("Procedure [ " + procedure_name + " ] not found");
        return;
    }

    it->second->implement(output, skip_first_screenshot);
}

void BAAS::solve_procedure(
        const string& procedure_name,
        BAASConfig& output,
        const BAASConfig& patch,
        bool skip_first_screenshot

)
{
    auto it = procedures.find(procedure_name);
    if (it == procedures.end()) {
        BAASGlobalLogger->BAASError("Procedure [ " + procedure_name + " ] not found");
        return;
    }
    auto baas_config = BAASConfig(it->second->get_config().get_config(), logger);
    baas_config.update(&patch);

    BaseProcedure* p = _create_procedure("", baas_config, false);
    if (p == nullptr) {
        BAASGlobalLogger->BAASError("Failed to create procedure.");
        return;
    }
    try {
        p->implement(output, skip_first_screenshot);
    }
    catch (exception &e) {
        p->clear_resource();
        delete p;
        throw e;
    }

    p->clear_resource();
    delete p;
}
bool BAAS::solve(const string &module_name)
{
    auto it = module_implement_funcs.find(module_name);
    if (it == module_implement_funcs.end()) {
        throw runtime_error("Module [ " + module_name + " ] not found");
    }
    auto func = it->second;
    try {
        return func(this);
    } catch (const std::exception &e) {
        BAASGlobalLogger->BAASError("Module [ " + module_name + " ] error: " + string(e.what()));
        return false;
    }
}

void BAAS::register_module_implement_func(
        const string &module_name,
        std::function<bool(BAAS *)> func
)
{
    if (module_implement_funcs.find(module_name) != module_implement_funcs.end()) {
        throw runtime_error("Module [ " + module_name + " ] already registered");
    }
    module_implement_funcs[module_name] = func;
}

void BAAS::_init_procedures()
{
    if (!filesystem::exists(BAAS_PROCEDURE_DIR)) {
        BAASGlobalLogger->BAASError("Procedure Dir :");
        BAASGlobalLogger->Path(BAAS_PROCEDURE_DIR, 3);
        BAASGlobalLogger->BAASError("not exists.");
        return;
    }

    string temp_path;
    int total_loaded = 0;
    for (const auto &entry: filesystem::recursive_directory_iterator(BAAS_PROCEDURE_DIR)) {
        temp_path = entry.path()
                .string();
        if (filesystem::is_regular_file(entry) && temp_path.ends_with(".json"))
            total_loaded += _load_procedure_from_json(temp_path);
    }
    BAASGlobalLogger->BAASInfo("Totally loaded [ " + to_string(total_loaded) + " ] procedures");
}

int BAAS::_load_procedure_from_json(const filesystem::path &j_path)
{
    BAASConfig _feature(j_path, (BAASLogger *) BAASGlobalLogger);
    json j = _feature.get_config();
    assert(j.is_object());
    BAASConfig temp;
    int loaded = 0;
    for (auto &i: j.items()) {
        temp = BAASConfig(i.value(), (BAASLogger*) BAASGlobalLogger);
        auto it = procedures.find(i.key());
        if (it != procedures.end()) {
            BAASGlobalLogger->BAASError("Procedure [ " + i.key() + " ] already exists");
            continue;
        }
        if (_create_procedure(i.key(), temp) != nullptr) loaded++;
    }
    return loaded;
}

BaseProcedure* BAAS::_create_procedure(const string& procedure_name, const BAASConfig& cfg, bool insert)
{
    if (!procedure_name.empty()) {
        auto it = procedures.find(procedure_name);
        if (it != procedures.end()) {
            BAASGlobalLogger->BAASError("Procedure [ " + procedure_name + " ] already exists");
            return nullptr;
        }
    }

    int tp = config->getInt("procedure_type", 0);
    BaseProcedure* p = nullptr;
    switch (tp) {
        case -1:
            p = new BaseProcedure(this, cfg);
            break;
        case 0:
            p = new AppearThenClickProcedure(this, cfg);
            break;
        default:
            BAASGlobalLogger->BAASError("Procedure Type [ " + to_string(tp) + " ] not found");
            break;
    }

    if (p != nullptr) {
        if (insert)
        procedures[procedure_name] = std::unique_ptr<BaseProcedure> (p);
    }

    return p;
}

BAAS::~BAAS() {
    delete screenshot;
    delete control;
    delete config;
    delete connection;
}

BAAS_NAMESPACE_END
