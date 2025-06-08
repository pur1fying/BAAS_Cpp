//
// Created by pc on 2024/4/12.
//
#ifndef BAAS_UTIL_H_
#define BAAS_UTIL_H_

#ifdef _WIN32
#include <WinSock2.h>
#endif

#include <chrono>
#include <filesystem>
#include <locale>
#include <codecvt>
#include <random>
#include <regex>
#include <thread>
#include <simdutf.h>
#include <opencv2/opencv.hpp>

#include "BAASExceptions.h"

BAAS_NAMESPACE_BEGIN

class BAASUtil {
public:
    /**
     * Initialize winsock to use socket functions
    */
    static bool initWinsock();

    /**
     * Convert integer to hex char. The length of the char is 4 and can be output by "cout"
     * @example 5 -> "0005"
     */
    static std::string int2hex(int a);

    /**
     * Divide a integer (4 bytes) to 4 binary chars
     * @example 255 -> "0b11111111"
     */
    static std::string int2binary(int a);

    /**
     * Convert hex char to integer
     * @example "0x001B" -> 27 (1 * 16^1 + 11 * 16^0)
     */
    static int hex2int(
            const std::string &input,
            int length
    );

    static int unsignedBinary2int(
            const std::string &input,
            int length
    );

    static int binary2int(
            const std::string &input,
            int length
    );

    static std::string getStreamOutput(FILE *stream);

    static void executeCommandWithoutOutPut(const std::string &command);

    static void executeCommandWithoutOutPut(
            const std::vector<std::string> &commandList,
            int n
    );

    static std::string executeCommandAndGetOutput(const std::string &command);

    static std::string executeCommandAndGetOutput(
            const std::vector<std::string> &commandList,
            int n
    );

    static FILE *executeCommand(const std::string &command);

    static std::string changeEndian(int a);

    static int getCurrentTimeStamp();

    /**
     * Convert int to string
     * @example 123 -> "123"
    */
    static std::string int2String(int a);

    static bool checkImageBroken(const std::string &path);

    static std::pair<int, int> deleteBrokenImage(const std::string &path);

    static std::wstring stringToWString(const std::string &str);

    static std::string wstringToString(const std::wstring &wstr);

    static bool serialHost(
            const std::string &serial,
            std::string &host
    );

    static bool serialPort(
            const std::string &serial,
            std::string &port
    );

    static int serial2port(const std::string &serial);

    static bool isMuMuFamily(const std::string &serial);

    static bool isMuMu12Family(const std::string &serial);

    static void stringReplace(
            const std::string &OLD,
            const std::string &NEW,
            std::string &src,
            std::string &dst
    );

    static void stringReplace(
            const std::string &OLD,
            const std::string &NEW,
            std::string &src
    );

    static void stringSplit(
            const std::string &src,
            const std::string &separator,
            std::vector<std::string> &dst
    );

    static void stringSplit(
            const std::string &src,
            const char separator,
            std::vector<std::string> &dst
    );

    static void stringJoin(
            const std::vector<std::string> &src,
            const std::string &joiner,
            std::string &dst
    );

    static int MuMu_serial2instance_id(const std::string &serial);


    static std::pair<std::string, std::string> serialToHostPort(const std::string &serial);

    static inline void sleepMS(int ms)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

    static inline void sleepS(int s)
    {
        std::this_thread::sleep_for(std::chrono::seconds(s));
    }

    static long long getCurrentTimeMS();

    static double genRandDouble(
            const double &min,
            const double &max
    );

    static int genRandInt(
            const int &min,
            const int &max
    );

    static bool endsWith(
            const std::string &src,
            const std::string &suffix
    );

    static bool allNumberChar(const std::string &src);

    static void re_find_all(
            const std::string &src,
            const std::string &pattern,
            std::vector<std::smatch> &dst
    );

    static void re_find_all(
            const std::string &src,
            const std::string &pattern,
            std::vector<std::string> &dst
    );

    static void re_find(
            const std::string &src,
            const std::string &pattern,
            std::string &dst
    );

    static bool re_match(
            const std::string &src,
            const std::string &pattern
    );

    static uint32_t st2u32(const std::string &src);

    static void insert_swipe(
            std::vector<std::pair<int, int>> &output,
            int start_x,
            int start_y,
            int end_x,
            int end_y,
            int step_len = 5
    );

    //  not use real distance for accuracy and int will not overflow in this case (x and y are both less than 10000)
    static inline int squared_distance(
            int x1,
            int y1,
            int x2,
            int y2
    )
    {
        return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2);
    }

    template<typename T>
    static inline void append_big_endian(
            std::string &dst,
            T src
    )
    {
        for (int i = sizeof(T) - 1; i >= 0; i--) {
            dst.push_back((src >> (i * 8)) & 0xFF);
        }
    }

    template<typename T>
    static inline void append_little_endian(
            std::string &dst,
            T src
    )
    {
        for (int i = 0; i < sizeof(T); i++) {
            dst.push_back((src >> (i * 8)) & 0xFF);
        }
    }

    static bool count_bit(int num);

    // from tinyObjLoader
    // See
    // http://stackoverflow.com/questions/6089231/getting-std-ifstream-to-handle-lf-cr-and-crlf
    static std::istream &safeGetLine(
            std::istream &is,
            std::string &t
    );

    static void calc_swipe_params(
            int x1,
            int y1,
            int x2,
            int y2,
            double duration,
            int &step_len,
            double &sleep_delay
    );

    inline static void str2wstr(const std::string& in, std::wstring & out)
    {
#ifdef _WIN32
        const size_t size = in.size();
        if (size == 0) {
            out.clear();
            return;
        }
        const size_t expected_utf16_words = simdutf::utf16_length_from_utf8(in.c_str(), size);
        out.resize(expected_utf16_words);
        const size_t real = simdutf::convert_utf8_to_utf16le(
                in.c_str(),
                size,
                reinterpret_cast<char16_t*>(out.data())
        );
#else
#endif
    }

    inline static void wstr2str(const std::wstring& in, std::string & out)
    {
#ifdef _WIN32
        const size_t size = in.size();
        if (size == 0) {
            out.clear();
            return;
        }
        const size_t expected_utf8_bytes = simdutf::utf8_length_from_utf16(
                                                    reinterpret_cast<const char16_t*>(in.c_str()),
                                                    size
                                           );
        out.resize(expected_utf8_bytes);
        simdutf::convert_utf16le_to_utf8(
                reinterpret_cast<const char16_t*>(in.c_str()),
                size,
                out.data()
        );
#else
#endif
    }

};

BAAS_NAMESPACE_END

#endif //BAAS_UTIL_H_
