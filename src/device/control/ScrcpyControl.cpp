//
// Created by pc on 2024/7/28.
//

#include "device/control/ScrcpyControl.h"

ScrcpyControl::ScrcpyControl(BAASConnection *connection) : BaseControl(connection) {
    client = BAASScrcpyClient::get_client(connection);
}

void ScrcpyControl::init() {
    logger->BAASInfo("ScrcpyControl init.");
    client->start();
}

void ScrcpyControl::click(int x, int y) {

}
