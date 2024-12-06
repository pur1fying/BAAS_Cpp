//
// Created by pc on 2024/4/12.
//

#ifndef BAAS_BAAS_H_
#define BAAS_BAAS_H_
#include "BAASUtil.h"
#include "BAASLogger.h"
#include "BAASExceptionHandler.h"
#include "BAASEmulatorController.h"
#include "config.h"

#include "device/screenshot/BAASScreenshot.h"
#include "device/control/BAASControl.h"

#include "feature/BAASFeature.h"
#include "BAASGlobals.h"

#include "BAASAutoFight.h"
#include "BAASImageUtil.h"
#include "BAASDevelopUtils.h"
#include "BAASImageResource.h"
#include "ocr/BAASOCR.h"

class BAAS{
public:
    bool solve(const std::string& task);

    explicit BAAS(std::string& config_name);

    void update_screenshot_array();

    void get_latest_screenshot(cv::Mat& img);

    void get_latest_screenshot(cv::Mat& img, const BAASRectangle &region);

    bool reset_then_feature_appear(const std::string& feature_name);

    bool feature_appear(const std::string& feature_name, BAASConfig &output, bool show_log = false);

    bool feature_appear(const std::string& feature_name);

    void solve_procedure(const std::string &name);

    void solve_procedure(const std::string &name, BAASConfig& output);

    void solve_procedure(const std::string &name, bool skip_first_screenshot);

    void solve_procedure(const std::string &name, BAASConfig& output, bool skip_first_screenshot);

    void solve_procedure(const std::string &name, nlohmann::json& patch);

    void solve_procedure(const std::string &name, BAASConfig& output, nlohmann::json& patch);

    void solve_procedure(const std::string &name, nlohmann::json& patch, bool skip_first_screenshot);

    void solve_procedure(const std::string &name, BAASConfig& output, nlohmann::json& patch, bool skip_first_screenshot);

    void solve_procedure(const std::string &name, BAASConfig& output, BAASConfig& patch);

    void solve_procedure(const std::string &name, BAASConfig& output, BAASConfig& patch, bool skip_first_screenshot);

    void get_each_round_type(std::vector<int>& round_type);

    std::optional<int> point2type(const BAASPoint& point);

    bool judge_rgb_range(const BAASPoint& point, const cv::Vec3b &min, const cv::Vec3b &max);

    inline bool has_screenshot() {
        return !latest_screenshot.empty();
    }

    ~BAAS() = default;

    [[nodiscard]] inline bool is_running() const {
        return flag_run;
    }

    inline BAASConnection* get_connection() {
        return connection;
    }

    inline BAASUserConfig* get_config() {
        return config;
    }

    inline BAASLogger* get_logger() {
        return logger;
    }

    inline BAASScreenshot* get_screenshot() {
        return screenshot;
    }

    inline BAASControl* get_control() {
        return control;
    }

    [[nodiscard]] inline double get_screen_ratio() const {
        return screen_ratio;
    }

    inline void click(BAASPoint point, uint8_t type = 1, int offset = 5, const std::string &description = "") {
        if(!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
        control->click(point, type, offset, description);
    }

    inline void click(BAASPoint point, int count, uint8_t type = 1, int offset = 5, double interval = 0.0, double pre_wait = 0.0, double post_wait = 0.0, const std::string &description = "") {
        if(!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
        control->click(point, count, type, offset, interval, pre_wait, post_wait, description);
    }

    inline void click(int x, int y, int count, uint8_t type = 1, int offset = 5, double interval = 0.0, double pre_wait = 0.0 , double post_wait = 0.0, const std::string &description = "") {
        if(!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
        control->click(x, y, count, type, offset, interval, pre_wait, post_wait, description);
    }

    void long_click(BAASPoint point, double duration, uint8_t type = 1, int offset = 5){
        if(!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
        control->long_click(point, duration, type, offset);
    }

    void long_click(int x, int y, double duration, uint8_t type = 1, int offset = 5) {
        if(!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
        control->long_click(x, y, duration, type, offset);
    }
    void swipe(BAASPoint start, BAASPoint end, double duration) {
        if(!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
        control->swipe(start, end, duration);
    }

    void swipe(int x1, int y1, int x2, int y2, double duration) {
        if(!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
        control->swipe(x1, y1, x2, y2, duration);
    }

    void wait_region_static(const BAASRectangle& region,double frame_diff_ratio = 0.8 ,double min_static_time = 2, int min_frame_cnt = 5, double max_execute_time = 60);

    static void init_implement_funcs();

    void screenshot_cut(const BAASRectangle &region,cv::Mat &output);

    void ocr_for_single_line(const std::string &language, TextLine &result,const BAASRectangle &region={0, 0, 1280, 720},
                             const std::string &log_content=std::string(),const std::string& candidates=std::string());

    void ocr(const std::string &language, OcrResult &result, const BAASRectangle &region={0, 0, 1280, 720},
             const std::string& candidates=std::string());

private:

    bool flag_run;

    cv::Mat latest_screenshot;

    double screen_ratio;

    BAASConnection* connection;

    BAASUserConfig* config;

    BAASLogger* logger;

    BAASScreenshot* screenshot;

    BAASControl* control;

    static std::map<std::string, std::function<bool (BAAS*)>> implement_funcs;

    friend class AppearThenDoProcedure;
    friend class AppearThenClickProcedure;
};



#endif //BAAS_BAAS_H_
