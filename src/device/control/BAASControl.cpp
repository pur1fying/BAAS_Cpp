// Created by pc on 2024/8/9.
//
#include "device/control/BAASControl.h"

#include "config/BAASStaticConfig.h"

using namespace std;

BAAS_NAMESPACE_BEGIN
vector <string> BAASControl::available_methods;

BAASControl::BAASControl(
        const std::string &method,
        double screen_ratio,
        BAASConnection *connection
)
{
    assert(connection != nullptr);
    this->connection = connection;
    logger = connection->get_logger();

    available_methods = static_config->get<std::vector<std::string>>("available_control_methods");
    logger->BAASInfo("Available control methods : ");
    logger->BAASInfo(available_methods);


    this->ratio = screen_ratio;

    control = nullptr;
    set_control_method(method);
}

void BAASControl::init()
{
    control->init();
}

void BAASControl::click(
        BAASPoint point,
        const string &description
)
{
    click(point.x, point.y, 1, 5, description);
}

void BAASControl::click(
        int x,
        int y,
        uint8_t type,
        int offset,
        const std::string &description
)
{
    set_x_y_offset(x, y, type, offset);
    x = int(double(x) * 1.0 * ratio);
    y = int(double(y) * 1.0 * ratio);
    gen_click_log(x, y, 1, description);
    control->click(x, y);
}

void BAASControl::click(
        BAASPoint point,
        int count,
        uint8_t type,
        int offset,
        double click_interval,
        double pre_wait,
        double post_wait,
        const string &description
)
{
    click(point.x, point.y, count, type, offset, click_interval, pre_wait, post_wait, description);
}

void BAASControl::click(
        int x,
        int y,
        int count,
        uint8_t type,
        int offset,
        double click_interval,
        double pre_wait,
        double post_wait,
        const string &description
)
{
    gen_click_log(x, y, count, description);

    if (offset > 0)
        set_x_y_offset(x, y, type, offset);

    x = int(double(x) * 1.0 * ratio);
    y = int(double(y) * 1.0 * ratio);

    if (pre_wait > 0)
        BAASChronoUtil::sleepMS(int(pre_wait * 1000));

    int itv = int(click_interval * 1000);
    for (int i = 0; i < count; i++) {
        control->click(x, y);
        if (i < count - 1)
            BAASChronoUtil::sleepMS(itv);
    }

    if (post_wait > 0)
        BAASChronoUtil::sleepMS(int(post_wait * 1000));
}

void BAASControl::long_click(
        BAASPoint point,
        double duration,
        uint8_t type,
        int offset
)
{
    long_click(point.x, point.y, duration, type, offset);
}

void BAASControl::long_click(
        int x,
        int y,
        double duration,
        uint8_t type,
        int offset
)
{
    set_x_y_offset(x, y, type, offset);
    x = int(double(x) * 1.0 * ratio);
    y = int(double(y) * 1.0 * ratio);
    string msg = "Long Click (\t" + to_string(x) + ",\t" + to_string(y) + "\t) ";
    logger->BAASInfo(msg);
    control->long_click(x, y, duration);
}

void BAASControl::swipe(
        BAASPoint start,
        BAASPoint end,
        double duration
)
{
    swipe(start.x, start.y, end.x, end.y, duration);
}

void BAASControl::swipe(
        int x1,
        int y1,
        int x2,
        int y2,
        double duration
)
{
    gen_swipe_log(x1, y1, x2, y2, duration);
    x1 = int(double(x1) * 1.0 * ratio);
    y1 = int(double(y1) * 1.0 * ratio);
    x2 = int(double(x2) * 1.0 * ratio);
    y2 = int(double(y2) * 1.0 * ratio);
    control->swipe(x1, y1, x2, y2, duration);
}

void BAASControl::set_control_method(
        const std::string &method,
        bool exit
)
{
    if (std::find(available_methods.begin(), available_methods.end(), method) == available_methods.end()) {
        logger->BAASCritical("Unsupported control method : [ " + method + " ]");
        throw RequestHumanTakeOver("Unsupported control method: " + method);
    }

    if (method == control_method) {
        logger->BAASInfo("Control method is already set to " + method);
        return;
    }

    if (control != nullptr) {
        if (exit) {
            logger->BAASInfo("Exiting current control method : [ " + control_method + " ]");
            control->exit();
        }
        delete control;
    }

    control_method = method;
    if (control_method == "adb") {
        control = new AdbControl(connection);
    } else if (control_method == "scrcpy") {
        control = new ScrcpyControl(connection);
    } else if (control_method == "nemu") {
        control = new NemuControl(connection);
    }
    init();
}

void BAASControl::set_x_y_offset(
        int &x,
        int &y,
        uint8_t type,
        int size
)
{
    switch (type) {
        case OFFSET_TYPE_NOCHANGE:
            break;
        case OFFSET_TYPE_RECTANGLE: {
            x += BAASUtil::genRandInt(-size, size);
            y += BAASUtil::genRandInt(-size, size);
            break;
        }
        case OFFSET_TYPE_CIRCLE: {
            double angle = BAASUtil::genRandDouble(0, 2 * M_PI);
            x += int(size * cos(angle));
            y += int(size * sin(angle));
            break;
        }
        default:
            logger->BAASError("Invalid offset type : " + to_string(type));
            break;
    }
}

void BAASControl::exit()
{
    control->exit();
}

void BAASControl::gen_click_log(
        int x,
        int y,
        int count,
        const string &description
)
{
    string t = to_string(x);
    string msg = "Click ( ";
    for (int i = 0; i < 4 - int(t.size()); i++)msg += " ";
    msg += t + ", ";
    t = to_string(y);
    for (int i = 0; i < 4 - int(t.size()); i++)msg += " ";
    msg += t + ")";
    if (!description.empty())msg += " @ " + description;
    if (count > 1)msg += " " + to_string(count) + " times ";
    logger->BAASInfo(msg);
}

void BAASControl::gen_swipe_log(
        int x1,
        int y1,
        int x2,
        int y2,
        double duration
)
{
    string msg = "Swipe From ( ";
    string t = to_string(x1);
    for (int i = 0; i < 4 - int(t.size()); i++)msg += " ";
    msg += t + ", ";
    t = to_string(y1);
    for (int i = 0; i < 4 - int(t.size()); i++)msg += " ";
    msg += t + " ) --> ( ";
    t = to_string(x2);
    for (int i = 0; i < 4 - int(t.size()); i++)msg += " ";
    msg += t + ", ";
    t = to_string(y2);
    for (int i = 0; i < 4 - int(t.size()); i++)msg += " ";
    msg += t + " ) ";
    msg += " Duration : " + to_string(duration) + " s";
    logger->BAASInfo(msg);
}

BAAS_NAMESPACE_END
