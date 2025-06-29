//
// Created by pc on 2024/8/9.
//

#ifndef BAAS_DEVICE_CONTROL_BAASCONTROL_H_
#define BAAS_DEVICE_CONTROL_BAASCONTROL_H_

#include "BaseControl.h"

#define OFFSET_TYPE_NOCHANGE 0
#define OFFSET_TYPE_RECTANGLE 1
#define OFFSET_TYPE_CIRCLE 2

BAAS_NAMESPACE_BEGIN

class BAASControl {

public:

    explicit BAASControl(
            const std::string& method,
            double screen_ratio,
            BAASConnection* connection
    );

    void init();

    void click(
            BAASPoint point,
            const std::string& description = ""
    );

    void click(
            int x,
            int y,
            uint8_t type = 1,
            int offset = 5,
            const std::string& description = ""
    );

    void click(
            BAASPoint point,
            int count,
            uint8_t type = 1,
            int offset = 5,
            double interval = 0.0,
            double pre_wait = 0.0,
            double post_wait = 0.0,
            const std::string& description = ""
    );

    void click(
            int x,
            int y,
            int count,
            uint8_t type = 1,
            int offset = 5,
            double click_interval = 0.0,
            double pre_wait = 0.0,
            double post_wait = 0.0,
            const std::string& description = ""
    );

    void long_click(
            BAASPoint point,
            double duration,
            uint8_t type = 1,
            int offset = 5
    );

    void long_click(
            int x,
            int y,
            double duration,
            uint8_t type = 1,
            int offset = 5
    );

    void swipe(
            BAASPoint start,
            BAASPoint end,
            double duration
    );

    void swipe(
            int x1,
            int y1,
            int x2,
            int y2,
            double duration
    );

    void exit();

    void set_control_method(
            const std::string& method,
            bool exit = true
    );

    void set_x_y_offset(
            int& x,
            int& y,
            uint8_t type,
            int size
    );

    void gen_click_log(
            int x,
            int y,
            int count = 1,
            const std::string& description = ""
    );

    void gen_swipe_log(
            int x1,
            int y1,
            int x2,
            int y2,
            double duration
    );

private:

    BAASLogger* logger;

    double ratio;

    BAASConnection* connection;

    std::string control_method;

    static const std::set<std::string> available_methods;

    BaseControl* control;
};

BAAS_NAMESPACE_END

#endif //BAAS_DEVICE_CONTROL_BAASCONTROL_H_
