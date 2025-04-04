//
// Created by pc on 2024/4/12.
//

#ifndef BAAS_BAAS_H_
#define BAAS_BAAS_H_

#include "BAASUtil.h"
#include "BAASLogger.h"
#include "BAASExceptionHandler.h"

#include "device/screenshot/BAASScreenshot.h"
#include "device/control/BAASControl.h"

#include "BAASGlobals.h"

#include "BAASAutoFight.h"
#include "BAASImageUtil.h"
#include "BAASDevelopUtils.h"
#include "BAASImageResource.h"
#include "ocr/BAASOCR.h"

BAAS_NAMESPACE_BEGIN

class BAAS {
public:
    static void check_config(std::string &config_name);

    static void check_user_config(const std::string &config_name);

    static void write_all_default_config(const std::filesystem::path& dir);


    explicit BAAS(std::string &config_name);

    void update_screenshot_array();

    void get_latest_screenshot(cv::Mat &img);

    void get_latest_screenshot(
            cv::Mat &img,
            const BAASRectangle &region
    );

    void get_each_round_type(std::vector<int> &round_type);

    std::optional<int> point2type(const BAASPoint &point);

    bool judge_rgb_range(
            const BAASPoint &point,
            const cv::Vec3b &min,
            const cv::Vec3b &max
    );

    inline bool has_screenshot()
    {
        return !latest_screenshot.empty();
    }

    ~BAAS() = default;

    [[nodiscard]] inline bool is_running() const
    {
        return flag_run;
    }

    inline BAASConnection *get_connection()
    {
        return connection;
    }

    inline BAASUserConfig *get_config()
    {
        return config;
    }

    inline BAASLogger *get_logger()
    {
        return logger;
    }

    inline BAASScreenshot *get_screenshot()
    {
        return screenshot;
    }

    inline BAASControl *get_control()
    {
        return control;
    }

    [[nodiscard]] inline double get_screen_ratio() const
    {
        return screen_ratio;
    }

    inline void click(
            BAASPoint point,
            uint8_t type = 1,
            int offset = 5,
            const std::string &description = ""
    )
    {
        if (!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
        control->click(point, type, offset, description);
    }

    inline void click(
            BAASPoint point,
            int count,
            uint8_t type = 1,
            int offset = 5,
            double interval = 0.0,
            double pre_wait = 0.0,
            double post_wait = 0.0,
            const std::string &description = ""
    )
    {
        if (!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
        control->click(point, count, type, offset, interval, pre_wait, post_wait, description);
    }

    inline void click(
            int x,
            int y,
            int count,
            uint8_t type = 1,
            int offset = 5,
            double interval = 0.0,
            double pre_wait = 0.0,
            double post_wait = 0.0,
            const std::string &description = ""
    )
    {
        if (!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
        control->click(x, y, count, type, offset, interval, pre_wait, post_wait, description);
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
            const BAASRectangle &region,
            double frame_diff_ratio = 0.8,
            double min_static_time = 2,
            int min_frame_cnt = 5,
            double max_execute_time = 60
    );

    void screenshot_cut(
            const BAASRectangle &region,
            cv::Mat &output
    );

    void ocr_for_single_line(
            const std::string &language,
            TextLine &result,
            const BAASRectangle &region = {0, 0, 1280, 720},
            const std::string &log_content = std::string(),
            const std::string &candidates = std::string());

    void ocr(
            const std::string &language,
            OcrResult &result,
            const BAASRectangle &region = {0, 0, 1280, 720},
            const std::string &candidates = std::string());

private:
    bool script_show_image_compare_log;

    bool flag_run;

    cv::Mat latest_screenshot;

    double screen_ratio;

    BAASConnection *connection;

    BAASUserConfig *config;

    BAASLogger *logger;

    BAASScreenshot *screenshot;

    BAASControl *control;

    friend class BAASFeature;

    friend class AppearThenDoProcedure;

    friend class AppearThenClickProcedure;
};

BAAS_NAMESPACE_END

#endif //BAAS_BAAS_H_
