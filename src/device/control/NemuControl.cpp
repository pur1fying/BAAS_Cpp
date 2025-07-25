//
// Created by pc on 2024/8/3.
//
#ifdef _WIN32

#include "device/control/NemuControl.h"

#include "device/utils.h"

BAAS_NAMESPACE_BEGIN

NemuControl::NemuControl(BAASConnection* connection) : BaseControl(connection)
{

}

void NemuControl::init()
{
    logger->hr("Control Method NemuControl init");
    nemu_connection = BAASNemu::get_instance(connection);
}

void NemuControl::click(
        int x,
        int y
)
{
    std::lock_guard<std::mutex> lock(control_mtx);
    nemu_connection->click(x, y);
}

void NemuControl::long_click(
        int x,
        int y,
        double duration
)
{
    nemu_connection->long_click(x, y, duration);
}

void NemuControl::swipe(
        int x1,
        int y1,
        int x2,
        int y2,
        double duration
)
{
    int step_len;
    double sleep_delay;
    calc_swipe_params(x1, y1, x2, y2, duration, step_len, sleep_delay);

    nemu_connection->swipe(x1, y1, x2, y2, step_len, sleep_delay);
}

void NemuControl::exit()
{
    // maybe same instance nemu screenshot still needs it, just try release, do not disconnect
    BAASNemu::try_release(nemu_connection, true);
}

BAAS_NAMESPACE_END

#endif // _WIN32