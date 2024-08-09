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
#include "procedure/BAASProcedure.h"
#include "BAASGlobals.h"

#include "BAASAutoFight.h"
#include "BAASImageUtil.h"
#include "BAASDevelopUtils.h"
#include "BAASImageResource.h"

#define BAAS_ACTION_TYPE_DO_NOTHING 0
#define BAAS_ACTION_TYPE_CLICK 1
#define BAAS_ACTION_TYPE_LONG_CLICK 2
#define BAAS_ACTION_TYPE_SWIPE 3



class BAAS{
public:
    explicit BAAS(std::string& config_name);

    void update_screenshot_array();

    void get_latest_screenshot(cv::Mat& img);

    ~BAAS() = default;

    inline bool is_run() const {
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

    inline double get_screen_ratio() const {
        return screen_ratio;
    }

    inline void click(BAASPoint point, uint8_t type = 1, int offset = 5, const std::string &description = "") {
        if(!flag_run) throw HumanTakeOverError("Flag Run turn to false manually");
        control->click(point, type, offset, description);
    }

    inline void click(BAASPoint point, int count, uint8_t type = 1, int offset = 5, double interval = 0.0, double pre_wait = 0.0, double post_wait = 0.0, const std::string &description = "") {
        if(!flag_run) throw HumanTakeOverError("Flag Run turn to false manually");
        control->click(point, count, type, offset, interval, pre_wait, post_wait, description);
    }

    inline void click(int x, int y, int count, uint8_t type = 1, int offset = 5, double interval = 0.0, double pre_wait = 0.0 , double post_wait = 0.0, const std::string &description = "") {
        if(!flag_run) throw HumanTakeOverError("Flag Run turn to false manually");
        control->click(x, y, count, type, offset, interval, pre_wait, post_wait, description);
    }

    void long_click(BAASPoint point, double duration, uint8_t type = 1, int offset = 5){
        if(!flag_run) throw HumanTakeOverError("Flag Run turn to false manually");
        control->long_click(point, duration, type, offset);
    }

    void long_click(int x, int y, double duration, uint8_t type = 1, int offset = 5) {
        if(!flag_run) throw HumanTakeOverError("Flag Run turn to false manually");
        control->long_click(x, y, duration, type, offset);
    }
    void swipe(BAASPoint start, BAASPoint end, double duration) {
        if(!flag_run) throw HumanTakeOverError("Flag Run turn to false manually");
        control->swipe(start, end, duration);
    }

    void swipe(int x1, int y1, int x2, int y2, double duration) {
        if(!flag_run) throw HumanTakeOverError("Flag Run turn to false manually");
        control->swipe(x1, y1, x2, y2, duration);
    }




private:
    void init_procedures();

    std::function<void(BAASConfig*)> procedures;

    bool flag_run;

    cv::Mat latest_screenshot;

    double screen_ratio;

    BAASConnection* connection;

    BAASUserConfig* config;

    BAASLogger* logger;

    BAASScreenshot* screenshot;

    BAASControl* control;

    friend class AppearThenDoProcedure
};



#endif //BAAS_BAAS_H_
