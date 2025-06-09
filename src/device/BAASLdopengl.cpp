//
// Created by pc on 2024/8/14.
//

#ifdef _WIN32

#include "device/BAASLdopengl.h"

#include "utils/BAASSystemUtil.h"

using namespace std;

BAAS_NAMESPACE_BEGIN

map<int, BAASLdopengl*> BAASLdopengl::connections;

BAASLdopengl* BAASLdopengl::get_instance(BAASConnection* connection)
{
    static BAASLdopengl* instance = nullptr;
    if (instance == nullptr) {
        instance = new BAASLdopengl(connection);
    }
    return instance;
}

BAASLdopengl::BAASLdopengl(std::string& installPath)
{
    logger = (BAASLogger *) BAASGlobalLogger;
    ldplayer_install_path = installPath;
    init_dll();
    serial = "127.0.0.1:5555";
}

void BAASLdopengl::detect_ldplayer_instance()
{
    pair<string, string> serial_emu_pair;
    BAASConnection::port_emu_pair_serial(serial);
    instance_id = BAASConnectionAttr::LDPlayer_serial2instance_id(serial);
    logger->BAASInfo("LDPlayer instance id : " + to_string(instance_id));

    if (instance_id == -1) {
        logger->BAASError("Invalid LDPlayer serial");
        throw LDOpenGLError("serial not valid");
    }

    string cmd = ldplayer_install_path + "\\ldconsole.exe";
    if (!filesystem::exists(cmd)) {
        logger->BAASError("ldconsole.exe not found in " + ldplayer_install_path);
        throw LDOpenGLError("ldconsole.exe not found");
    }
    cmd += " list2";
    string ret = BAASSystemUtil::executeCommandAndGetOutput(cmd);
    cout << ret << endl;

    istringstream iss(ret);
    string line;
    while (getline(iss, line)) {
        if (::sscanf_s(
                line.c_str(), "%u,%[^,],%u,%u,%d,%u,%u,%u,%u,%u",
                &emu_info.index,
                &emu_info.name,
                (unsigned int) sizeof(emu_info.name) - 1,
                reinterpret_cast<unsigned int *>(&emu_info.topWnd),
                reinterpret_cast<unsigned int *>(&emu_info.bndWnd),
                reinterpret_cast<unsigned int *>(&emu_info.sysBoot),
                &emu_info.playerPid,
                &emu_info.vboxPid,
                &emu_info.width,
                &emu_info.height,
                &emu_info.dpi
        ) == 10) {
            if (emu_info.index == instance_id) {
                logger->BAASInfo("LDPlayer instance found : " + to_string(emu_info.index));
                logger->BAASInfo("Resolution : " + to_string(emu_info.width) + "x" + to_string(emu_info.height));
                resolution = {int(emu_info.height), int(emu_info.width)};
                logger->BAASInfo("PID : " + to_string(emu_info.playerPid));
                return;
            }
        }
    }

    logger->BAASError("LDPlayer instance [" + to_string(instance_id) + "] not found");
    throw LDOpenGLError("LDPlayer instance not found");
}

void BAASLdopengl::init_dll()
{
    string dll_path = ldplayer_install_path + "\\ldopengl64.dll";
    if (!filesystem::exists(dll_path)) {
        logger->BAASError("ldopengl64.dll not found in " + ldplayer_install_path);
        logger->BAASInfo("ldopengl requires LDPlayer >= 9.0.75, please check your version");
        throw LDOpenGLError("ldopengl64.dll not found");
    }
    hDllInst = LoadLibrary(dll_path.c_str());
    if (hDllInst == NULL) {
        logger->BAASError("dll exist bu Load failed");
        throw LDOpenGLError("Load ldopengl64.dll failed");
    }
    createShotInstance = (CreateShotInstance) GetProcAddress(hDllInst, "CreateScreenShotInstance");
    if (createShotInstance == NULL) {
        logger->BAASError("GetProcAddress failed");
        throw LDOpenGLError("GetProcAddress failed");
    }
}

BAASLdopengl::BAASLdopengl(BAASConnection* connection)
{
    logger = connection->get_logger();
    ldplayer_install_path = connection->emulator_folder_path();
    serial = connection->get_serial();
    init_dll();
    detect_ldplayer_instance();
    shot_instance = (IScreenShotClass *) createShotInstance(instance_id, emu_info.playerPid);
}

void BAASLdopengl::release(int connectionId)
{
    auto it = connections.find(connectionId);
    if (it != connections.end()) {
        it->second->shot_instance->release();
        delete it->second;
        connections.erase(it);
    }
}

void BAASLdopengl::screenshot(cv::Mat& image)
{
    void* ptr = shot_instance->cap();
    if (ptr == nullptr) {
        logger->BAASError("LDOpenGL screenshot failed");
        throw LDOpenGLError("screenshot failed");
    }

    image = cv::Mat(resolution.first, resolution.second, CV_8UC3, (unsigned char *) ptr);
    cv::flip(image, image, 0);
}

BAAS_NAMESPACE_END

#endif // _WIN32
