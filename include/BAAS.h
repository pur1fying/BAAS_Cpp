//
// Created by pc on 2024/4/12.
//

#ifndef BAAS_BAAS_H_
#define BAAS_BAAS_H_

#include "ocr/OcrStruct.h"
#include "procedure/BaseProcedure.h"
#include "device/control/BAASControl.h"
#include "device/screenshot/BAASScreenshot.h"

#define PROCEDURE_BEGIN
#define PROCEDURE_END

BAAS_NAMESPACE_BEGIN

class BAAS {

public:

    void register_module_implement_func(
            const std::string& module_name,
            std::function<bool(BAAS*)> module
    );

    bool solve(const std::string& module_name);

    static void check_config(std::string& config_name);

    static void check_user_config(const std::string& config_name);

    static void write_all_default_config(const std::filesystem::path& dir);

    explicit BAAS(std::string& config_name);

    void update_screenshot_array();

    void i_update_screenshot_array();

    void get_latest_screenshot_clone(cv::Mat& img);

    void get_latest_screenshot(cv::Mat& img);

    void get_latest_screenshot(
            cv::Mat& img,
            const BAASRectangle& region
    );

    void get_each_round_type(std::vector<int>& round_type);

    std::optional<int> point2type(const BAASPoint& point);

    bool judge_rgb_range(
            const BAASPoint& point,
            const cv::Vec3b& min,
            const cv::Vec3b& max
    );

    inline bool has_screenshot()
    {
        return !latest_screenshot.empty();
    }

    ~BAAS();

    [[nodiscard]] inline bool is_running() const
    {
        return flag_run;
    }

    inline BAASConnection* get_connection() const
    {
        return connection;
    }

    inline BAASUserConfig* get_config() const
    {
        return config;
    }

    inline BAASLogger* get_logger() const
    {
        return logger;
    }

    inline BAASScreenshot* get_screenshot() const
    {
        return screenshot;
    }

    inline BAASControl* get_control() const
    {
        return control;
    }

    [[nodiscard]] inline double get_screen_ratio() const
    {
        return screen_ratio;
    }

    inline void click(
            BAASPoint point,
            const std::string& description = ""
    )
    {
        if (!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
        control->click(point, description);
    }

    inline void click(
            BAASPoint point,
            int count=1,
            uint8_t type = 1,
            int offset = 5,
            double click_interval = 0.0,
            double pre_wait = 0.0,
            double post_wait = 0.0,
            const std::string& description = ""
    )
    {
        if (!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
        control->click(point, count, type, offset, click_interval, pre_wait, post_wait, description);
    }

    inline void click(
            int x,
            int y,
            int count=1,
            uint8_t type = 1,
            int offset = 5,
            double click_interval = 0.0,
            double pre_wait = 0.0,
            double post_wait = 0.0,
            const std::string& description = ""
    )
    {
        if (!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
        control->click(x, y, count, type, offset, click_interval, pre_wait, post_wait, description);
    }

    void long_click(
            BAASPoint point,
            double duration,
            uint8_t type = 1,
            int offset = 5
    )
    {
        if (!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
        control->long_click(point, duration, type, offset);
    }

    void long_click(
            int x,
            int y,
            double duration,
            uint8_t type = 1,
            int offset = 5
    )
    {
        if (!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
        control->long_click(x, y, duration, type, offset);
    }

    void swipe(
            BAASPoint start,
            BAASPoint end,
            double duration
    )
    {
        if (!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
        control->swipe(start, end, duration);
    }

    void swipe(
            int x1,
            int y1,
            int x2,
            int y2,
            double duration
    )
    {
        if (!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
        control->swipe(x1, y1, x2, y2, duration);
    }

    void wait_region_static(
            const BAASRectangle& region,
            double frame_diff_ratio = 0.8,
            double min_static_time = 2,
            int min_frame_cnt = 5,
            double max_execute_time = 60
    );

    void screenshot_cut(
            const BAASRectangle& region,
            cv::Mat& output
    );

    void ocr_for_single_line(
            const std::string& language,
            TextLine& result,
            const BAASRectangle& region = {0, 0, 1280, 720},
            const std::string& log_content = std::string(),
            const std::string& candidates = std::string());

    void ocr(
            const std::string& language,
            OcrResult& result,
            const BAASRectangle& region = {0, 0, 1280, 720},
            const std::string& candidates = std::string());

    bool feature_appear(
            const std::string& feature_name,
            BAASConfig& output,
            bool show_log = false
    );

    bool feature_appear(const std::string& feature_name);

    inline void reset_feature(const std::string& name) {
        auto it = feature_state_map.find(name);
        if(it != feature_state_map.end()) it->second.round_feature_appear_state.reset();
    }

    inline void reset_all_feature() {
        for (auto &i: feature_state_map)
            i.second.round_feature_appear_state.reset();
    }

    [[nodiscard]] inline const std::string get_image_resource_prefix() const
    {
        return image_resource_prefix;
    }

    [[nodiscard]] inline const std::string get_rgb_feature_key() const
    {
        return rgb_feature_key;
    }

PROCEDURE_BEGIN

public:

    void solve_procedure(
            const std::string& procedure_name,
            bool skip_first_screenshot = false
    );

    void solve_procedure(
            const std::string& procedure_name,
            const BAASConfig& patch,
            bool skip_first_screenshot = false
    );

    void solve_procedure(
            const std::string& procedure_name,
            BAASConfig& output,
            bool skip_first_screenshot = false
    );

    void solve_procedure(
            const std::string& procedure_name,
            BAASConfig& output,
            const BAASConfig& patch,
            bool skip_first_screenshot = false
    );

private:

    void _init_procedures();

    int _load_procedure_from_json(const std::filesystem::path& j_path);

    BaseProcedure* _create_procedure(const std::string& procedure_name, const BAASConfig& cfg, bool insert = true);

    std::map<std::string, std::unique_ptr<BaseProcedure>> procedures;

PROCEDURE_END

    static std::map<std::string, std::function<bool(BAAS*)>> module_implement_funcs;

    std::string image_resource_prefix;

    std::string rgb_feature_key;

    std::string server;

    std::string language;

    bool script_show_image_compare_log;

    bool flag_run;

    cv::Mat latest_screenshot;

    double screen_ratio;

    BAASConnection* connection;

    BAASUserConfig* config;

    BAASLogger* logger;

    BAASScreenshot* screenshot;

    BAASControl* control;

    struct feature_state {
        // record if a feature is check in screenshot, when screenshot update ,state should be reset.
        std::optional<bool> round_feature_appear_state;

        // time cost for a image feature to detect
        double feature_average_cost;

        void reset() {
            round_feature_appear_state.reset();
        }
    };

    std::map<std::string, feature_state> feature_state_map;

    void _init_feature_state_map();

    friend class BAASFeature;

    friend class FilterRGBMatchTemplateFeature;

    friend class MatchTemplateFeature;

    friend class JudgePointRGBRangeFeature;

    friend class AppearThenClickProcedure;

    friend class BaseProcedure;
};

BAAS_NAMESPACE_END

#endif //BAAS_BAAS_H_
