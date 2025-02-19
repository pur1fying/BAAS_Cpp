//
// Created by pc on 2024/7/27.
//

#include "device/control/AdbControl.h"

using namespace std;

BAAS_NAMESPACE_BEGIN

AdbControl::AdbControl(BAASConnection *connection) : BaseControl(connection)
{

}

void AdbControl::init()
{
    logger->hr("Control Method AdbControl");
    string res = connection->adb_shell_bytes("echo 000");
    assert(res == "000");
}

void AdbControl::click(
        int x,
        int y
)
{
    long long start = BAASUtil::getCurrentTimeMS();
    connection->adb_shell_bytes("input tap " + to_string(x) + " " + to_string(y));
    if (BAASUtil::getCurrentTimeMS() - start < 50)
        this_thread::sleep_for(chrono::milliseconds(50));
}

void AdbControl::swipe(
        int x1,
        int y1,
        int x2,
        int y2,
        double duration
)
{
    int duration_int = (int) (duration * 1000);
    connection->adb_shell_bytes(
            "input swipe " + to_string(x1) + " " + to_string(y1) + " " + to_string(x2) + " " + to_string(y2) + " " +
            to_string(duration_int));
}

void AdbControl::exit()
{

}

void AdbControl::long_click(
        int x,
        int y,
        double duration
)
{
    logger->BAASInfo("Adb Control Swipe to stimulate long_click");
    swipe(x, y, x, y, duration);
}


BAAS_NAMESPACE_END
