//
// Created by pc on 2024/7/28.
//

#include "device/control/ScrcpyControl.h"

using namespace std;

BAAS_NAMESPACE_BEGIN

ScrcpyControl::ScrcpyControl(BAASConnection *connection) : BaseControl(connection)
{
    client = BAASScrcpyClient::get_client(connection);
}

void ScrcpyControl::init()
{
    logger->hr("Control Method ScrcpyControl Method init.");
    if (client->get_alive()) logger->BAASInfo("Scrcpy loop already started.");
    else {
        logger->BAASInfo("Start Scrcpy loop.");
        client->start();
    }
}

void ScrcpyControl::click(

        int x,
        int y
)
{
    client->touch(x, y);
    client->touch(x, y, ScrcpyConst::ACTION_UP);
    this_thread::sleep_for(chrono::milliseconds(5));
}

void ScrcpyControl::swipe(
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

    client->swipe(x1, y1, x2, y2, step_len, sleep_delay);
}

void ScrcpyControl::exit()
{
    BAASScrcpyClient::try_release_client(client);
}

void ScrcpyControl::long_click(
        int x,
        int y,
        double duration
)
{
    client->touch(x, y);
    this_thread::sleep_for(chrono::milliseconds(int(duration * 1000)));
    client->touch(x, y, ScrcpyConst::ACTION_UP);
    this_thread::sleep_for(chrono::milliseconds(5));
}


BAAS_NAMESPACE_END

