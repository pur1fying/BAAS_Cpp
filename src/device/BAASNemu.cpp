//
// Created by pc on 2024/5/22.
//

#include "device/BAASNemu.h"

using namespace std;

BAAS_NAMESPACE_BEGIN
std::map<int, BAASNemu *> BAASNemu::connections;

BAASNemu *BAASNemu::get_instance(BAASConnection *connection)
{
    string mm_path = connection->emulator_folder_path();
    int id = BAASUtil::MuMu_serial2instance_id(connection->get_serial());
    for (auto &conn: connections)
        if (conn.second
                ->mumu_install_path == mm_path && conn.second
                                                      ->instance_id == id) {
            connection->get_logger()
                      ->BAASInfo(
                              "Nemu path [ " + conn.second
                                                   ->mumu_install_path + " ], instance : " + to_string(id) +
                              " already exist, use it."
                      );
            return conn.second;
        }
    return new BAASNemu(connection);
}

BAASNemu::BAASNemu(BAASConnection *connection)
{
    logger = connection->get_logger();
    mumu_install_path = connection->emulator_folder_path();
    instance_id = BAASUtil::MuMu_serial2instance_id(connection->get_serial());
    display_id = 0;
    init_dll();
    connect();
}

BAASNemu::BAASNemu(string &installPath)
{
    logger = (BAASLogger *) BAASGlobalLogger;
    mumu_install_path = installPath;
    instance_id = 0;
    display_id = 0;
    init_dll();
    connect();
}


void BAASNemu::connect()
{
    for (auto &connection: connections) {
        if (connection.second
                      ->mumu_install_path == mumu_install_path && connection.second
                                                                            ->instance_id == instance_id) {
            logger->BAASWarn("Already connected");
            return;
        }
    }
    connection_id = nemu_connect(BAASUtil::stringToWString(mumu_install_path).c_str(), instance_id);
    if (connection_id == 0) {
        logger->BAASError("Nemu connect failed");
        throw NemuIpcError("Nemu connect failed");
    }
    connections[connection_id] = this;
    if (get_resolution(connection_id, display_id, resolution) > 0) {
        logger->BAASError("Nemu get display resolution failed.");
        throw NemuIpcError("Nemu get display resolution failed");
    }
    pixels.resize(resolution.first * resolution.second * 4);
    vector<string> msg = {
            "Nemu connected :   {",
            "                   MuMuPath:\t" + mumu_install_path,
            "                   DisplayID:\t" + to_string(display_id),
            "                   ConnID:   \t" + to_string(connection_id),
            "                   Resolution:\t" + to_string(resolution.first) + "x" + to_string(resolution.second),
            "                   }"
    };
    logger->BAASInfo(msg);
}

void BAASNemu::disconnect()
{
    if (connections.find(connection_id) == connections.end()) {
        logger->BAASError("Invalid connection_id : " + to_string(connection_id));
    }
    nemu_disconnect(connection_id);
    connections.erase(connection_id);
}


void BAASNemu::screenshot(cv::Mat &image)
{
    if (nemu_capture_display(
            connection_id,
            0,
            int(pixels.size()),
            &resolution.first,
            &resolution.second,
            pixels.data()
       ) != 0
    ) {
        BAASGlobalLogger->BAASError("Nemu capture display failed");
        throw NemuIpcError("Nemu capture display failed");
    }

    cv::Mat temp = cv::Mat(resolution.second, resolution.first, CV_8UC4, pixels.data());
    cv::cvtColor(temp, image, cv::COLOR_BGRA2RGB);
    cv::flip(image, image, 0);
}

int BAASNemu::get_resolution(
        int connectionId,
        int displayId,
        std::pair<int, int> &resolution
)
{
    return nemu_capture_display(connectionId, displayId, 0, &resolution.first, &resolution.second, nullptr);
}

void BAASNemu::click(
        int x,
        int y
)
{
    update_resolution();
    int t = BAASUtil::genRandInt(10, 20);
    convertXY(x, y);
    down(x, y);
    BAASUtil::sleepMS(t);
    up();
    BAASUtil::sleepMS(50 - t);
}

void BAASNemu::click(BAASPoint point)
{
    click(point.x, point.y);
}

void BAASNemu::long_click(
        int x,
        int y,
        double duration
)
{
    update_resolution();
    convertXY(x, y);
    down(x, y);
    BAASUtil::sleepMS(int(duration * 1000));
    up();
}

void BAASNemu::long_click(
        BAASPoint point,
        double duration
)
{
    long_click(point.x, point.y, duration);
}

void BAASNemu::swipe(
        int start_x,
        int start_y,
        int end_x,
        int end_y,
        int step_len,
        double step_duration
)
{
    update_resolution();
    convertXY(start_x, start_y);
    convertXY(end_x, end_y);

    vector<pair<int, int>> points;
    BAASUtil::insert_swipe(points, start_x, start_y, end_x, end_y, step_len);

    int sleep_time = int(step_duration * 1000);

    down(start_x, start_y);

    for (int i = 1; i < points.size(); i++) {
        BAASUtil::sleepMS(sleep_time);
        down(points[i].first, points[i].second);
    }

    up();
}


void BAASNemu::down(
        int x,
        int y
) const
{
    int ret = nemu_input_event_touch_down(connection_id, display_id, x, y);
    if (ret > 0)
        throw NemuIpcError("nemu_input_event_touch_down failed");
}

void BAASNemu::down(BAASPoint point) const
{
    down(point.x, point.y);
}

void BAASNemu::convertXY(
        int &x,
        int &y
) const
{
    if (resolution.first < resolution.second) return;

    int temp = x;
    x = resolution.second - y;
    y = temp;
    printf("x: %d, y: %d\n", x, y);
}

void BAASNemu::convertXY(BAASPoint &point) const
{
    int temp = point.x;
    point.x = resolution.second - point.y;
    point.y = temp;
}

void BAASNemu::init_dll()
{
    string dll_path = mumu_install_path + "./shell/sdk/external_renderer_ipc.dll";
    logger->BAASInfo("dll_path : " + dll_path);
    if (!filesystem::exists(dll_path)) {
        logger->BAASError("Nemu dll not found : [ " + dll_path + " ]");
        throw PathError("Nemu dll not found");
    }
    hDllInst = LoadLibrary(dll_path.c_str());
    if (hDllInst == nullptr) {
        logger->BAASError("LoadLibrary : [ " + dll_path + " ] failed");
        throw NemuIpcError("Load Dynamic Library failed");
    }

    nemu_connect = (nemuConnect) GetProcAddress(hDllInst, "nemu_connect");
    nemu_disconnect = (nemuDisconnect) GetProcAddress(hDllInst, "nemu_disconnect");
    nemu_capture_display = (nemuCaptureDisplay) GetProcAddress(hDllInst, "nemu_capture_display");
    nemu_input_text = (nemuInputText) GetProcAddress(hDllInst, "nemu_input_text");
    nemu_input_event_touch_down = (nemuInputEventTouchDown) GetProcAddress(hDllInst, "nemu_input_event_touch_down");
    nemu_input_event_touch_up = (nemuInputEventTouchUp) GetProcAddress(hDllInst, "nemu_input_event_touch_up");
    nemu_input_event_key_down = (nemuInputEventKeyDown) GetProcAddress(hDllInst, "nemu_input_event_key_down");
    nemu_input_event_key_up = (nemuInputEventKeyUp) GetProcAddress(hDllInst, "nemu_input_event_key_up");
}

void BAASNemu::up() const
{
    int ret = nemu_input_event_touch_up(connection_id, display_id);
    if (ret > 0)
        throw NemuIpcError("nemu_input_event_touch_up failed");
}

void BAASNemu::reconnect()
{
    disconnect();
    connect();
}

void BAASNemu::update_resolution()
{
    pair<int, int> new_resolution;
    if (get_resolution(connection_id, display_id, new_resolution) > 0) {
        logger->BAASError("Nemu get display resolution failed.");
        throw NemuIpcError("Nemu get display resolution failed");
    }
    if (new_resolution != resolution) {
        logger->BAASInfo(
                "Resolution changed : " + to_string(resolution.first) + "x" + to_string(resolution.second) +
                " -> " + to_string(new_resolution.first) + "x" + to_string(new_resolution.second));
        resolution = new_resolution;
    }
}

void BAASNemu::release(int connectionId)
{
    auto it = connections.find(connectionId);
    if (it == connections.end()) {
        BAASGlobalLogger->BAASError("Invalid connection_id : " + to_string(connectionId));
    }
    it->second
      ->disconnect();
    delete it->second;
    connections.erase(connectionId);
}

BAAS_NAMESPACE_END
