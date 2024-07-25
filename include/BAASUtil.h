//
// Created by pc on 2024/4/12.
//
#ifndef BAAS_UTIL_H_
#define BAAS_UTIL_H_

#include <WinSock2.h>

#include <chrono>
#include <filesystem>
#include <locale>
#include <codecvt>
#include <random>
#include <regex>

#include <opencv2/opencv.hpp>

#include "BAASLogger.h"
#include "BAASImageUtil.h"
#include "BAASExceptions.h"

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
      static int hex2int(const std::string& input, int length);

      static int unsignedBinary2int(const std::string& input, int length);

      static int binary2int(const std::string& input, int length);

      static std::string current_time_string();

      static std::string executeCommandAndGetOutput(const std::string& command);

      static std::string executeCommandAndGetOutput(const std::vector<std::string> &commandList, int n);

      static FILE* executeCommand(const std::string& command);

      static std::string getStreamOutput(FILE* stream);

      static void executeCommandWithoutOutPut(const std::string &command);

      static void executeCommandWithoutOutPut(const std::vector<std::string> &commandList, int n);

      static std::string changeEndian(int a);

      static int getCurrentTimeStamp();
      /**
       * Convert int to string
       * @example 123 -> "123"
      */
      static std::string int2String(int a);

      static bool checkImageBroken(const std::string& path);

      static std::pair<int, int> deleteBrokenImage(const std::string &path);

      static std::wstring stringToWString(const std::string& str);

      static std::string wstringToString(const std::wstring& wstr);

      static bool serialHost(const std::string &serial, std::string &host);

      static bool serialPort(const std::string &serial, std::string &port);

      static int serial2port(const std::string &serial);

      static bool isMuMuFamily(const std::string &serial);

      static bool isMuMu12Family(const std::string &serial);

      static void stringReplace(const std::string& OLD,const std::string& NEW,std::string &src,std::string &dst);

      static void stringReplace(const std::string& OLD,const std::string& NEW,std::string &src);

      static void stringSplit(const std::string &src, const std::string &separator, std::vector<std::string> &dst);

      static void stringSplit(const std::string &src, const char separator, std::vector<std::string> &dst);

      static void stringJoin(const std::vector<std::string> &src, const std::string &joiner, std::string &dst);

      static int MuMuSerialToDisplayId(const std::string &serial);

      static std::pair<std::string, std::string> serialToHostPort(const std::string &serial);

      static bool sleepMS(int ms);

      static bool sleepS(int s);

      static long long getCurrentTimeMS();

      static double genRandDouble(const double& min,const double& max);

      static int genRandInt(const int &min, const int &max);

      static bool endsWith(const std::string &src, const std::string &suffix);

      static bool allNumberChar(const std::string &src);

      static void re_find_all(const std::string &src, const std::string &pattern, std::vector<std::string> &dst);

      static bool re_match(const std::string &src, const std::string &pattern);
    // from tinyObjLoader
    // See
// http://stackoverflow.com/questions/6089231/getting-std-ifstream-to-handle-lf-cr-and-crlf
    static std::istream &safeGetLine(std::istream &is, std::string &t) {
        t.clear();

        // The characters in the stream are read one-by-one using a std::streambuf.
        // That is faster than reading them one-by-one using the std::istream.
        // Code that uses streambuf this way must be guarded by a sentry object.
        // The sentry object performs various tasks,
        // such as thread synchronization and updating the stream state.

        std::istream::sentry se(is, true);
        std::streambuf *sb = is.rdbuf();

        if (se) {
            for (;;) {
                int c = sb->sbumpc();
                switch (c) {
                    case '\n':
                        return is;
                    case '\r':
                        if (sb->sgetc() == '\n') sb->sbumpc();
                        return is;
                    case EOF:
                        // Also handle the case when the last line has no line ending
                        if (t.empty()) is.setstate(std::ios::eofbit);
                        return is;
                    default:
                        t += static_cast<char>(c);
                }
            }
        }

        return is;
    }
};
#endif //BAAS_UTIL_H_
