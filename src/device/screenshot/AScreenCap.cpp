//
// Created by pc on 2024/7/26.
//
#include <lz4.h>

#include "device/screenshot/AscreenCap.h"
#include "BAASGlobals.h"

using namespace std;

BAAS_NAMESPACE_BEGIN
string AScreenCap::shot_cmd = " --pack 2 --stdout";

AScreenCap::AScreenCap(BAASConnection *connection) : BaseScreenshot(connection)
{
    this->connection = connection;
    byte_pointer = 0;
    available = true;
    shot_cmd = ASCREENCAP_REMOTE_DIR.string() + shot_cmd;
}

void AScreenCap::init()
{
    logger->hr("ScreenShot Method AScreenCap Init.");
    string arch = connection->cpu_abi();
    int sdk = connection->sdk_ver();
    std::string ver = "0";
    if (sdk >= 21 && sdk < 26) ver = "Android_5.x-7.x";
    else if (sdk >= 26 && sdk <= 27) ver = "Android_8.x";
    else if (sdk == 28) ver = "Android_9.x";
    if (ver == "0") {
        logger->BAASError(
                "Only support emulator with skd version range from [ 21 ] to [ 28 ], current sdk version : " +
                to_string(sdk));
        throw RequestHumanTakeOver("AScreenCap Version Not Supported");
    }
    filesystem::path p = ASCREENCAP_BIN_DIR / ver / arch / "ascreencap";
    assert(filesystem::exists(p));
    logger->BAASInfo("Pushing");
    logger->Path(p);
    logger->BAASInfo(" to ");
    logger->Path(ASCREENCAP_REMOTE_DIR);
    connection->adb_push(p.string(), ASCREENCAP_REMOTE_DIR.string());

    connection->adb_shell_bytes("chmod 0777 " + ASCREENCAP_REMOTE_DIR.string());

    ret_buffer = connection->adb_shell_bytes(shot_cmd);

    reposition_byte_pointer();
    uint32_t id = BAASStringUtil::st2u32(ret_buffer.substr(byte_pointer, sizeof(uint32_t)));
    if (id != 0x315A4D42) {
        logger->BAASError("AScreenCap Verify Code Error : expected 0x315A4D42, got " + to_string(id));
        available = false;
        throw AScreenCapError("Verify Error");
    }

    int s = int(BAASStringUtil::st2u32(ret_buffer.substr(byte_pointer + 1 * sizeof(uint32_t), sizeof(uint32_t)))); // size
    int w = int(BAASStringUtil::st2u32(ret_buffer.substr(byte_pointer + 3 * sizeof(uint32_t), sizeof(uint32_t)))); // width
    int h = int(BAASStringUtil::st2u32(ret_buffer.substr(byte_pointer + 4 * sizeof(uint32_t), sizeof(uint32_t)))); // height
    logger->BAASInfo("Window Size : " + to_string(w) + " x " + to_string(h));

    decompress_buffer_size = s;
    logger->BAASInfo("Malloc decompress buffer with size : " + to_string(decompress_buffer_size) + " bytes");
    decompress_buffer = new char[decompress_buffer_size];

    logger->BAASInfo("AScreenCap init success");
}

void AScreenCap::screenshot(cv::Mat &img)
{
    // 100ms +
    ret_buffer = connection->adb_shell_bytes(shot_cmd);

    // 4ms
    decompress();
    img = image.clone();
}

void AScreenCap::uninstall()
{
    logger->BAASInfo("Remove AScreenCap from device [ " + connection->get_serial() + " ]");
    connection->adb_shell_bytes("rm " + ASCREENCAP_REMOTE_DIR.string());
}

void AScreenCap::decompress()
{
    reposition_byte_pointer();
    uint32_t id = BAASStringUtil::st2u32(ret_buffer.substr(byte_pointer, sizeof(uint32_t)));
    if (id != 0x315A4D42) {
        logger->BAASError("AScreenCap Verify Code Error : expected 0x315A4D42, got " + to_string(id));
        available = false;
        throw AScreenCapError("Verify Error");
    }

    int s = int(BAASStringUtil::st2u32(ret_buffer.substr(byte_pointer + 1 * sizeof(uint32_t), sizeof(uint32_t)))); // size
    int w = int(BAASStringUtil::st2u32(ret_buffer.substr(byte_pointer + 3 * sizeof(uint32_t), sizeof(uint32_t)))); // width
    int h = int(BAASStringUtil::st2u32(ret_buffer.substr(byte_pointer + 4 * sizeof(uint32_t), sizeof(uint32_t)))); // height

    LZ4_decompress_safe(
            ret_buffer.c_str() + byte_pointer + 5 * sizeof(uint32_t), decompress_buffer, s, decompress_buffer_size
    );

    image = cv::Mat(h, w, CV_8UC3, decompress_buffer + decompress_buffer_size - h * w * 3);
    cv::flip(image, image, 0);
}

void AScreenCap::reposition_byte_pointer()
{
    while (ret_buffer.substr(byte_pointer, 4) != "BMZ1") {
        byte_pointer++;
        if (byte_pointer >= ret_buffer.size()) {
            logger->BAASError("AScreenCap repositioning byte pointer failed, corrupted aScreenCap data received");
            available = false;
            throw AScreenCapError("Repositioning byte pointer failed.");
        }
    }
    ret_buffer = ret_buffer.substr(byte_pointer);
}

void AScreenCap::exit()
{
    uninstall();
}

bool AScreenCap::is_lossy()
{
    return false;
}

BAAS_NAMESPACE_END
