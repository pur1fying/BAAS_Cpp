//
// Created by pc on 2024/4/12.
//
#pragma once
#ifndef BAAS_CXX_REFACTOR_UTIL_H
#define BAAS_CXX_REFACTOR_UTIL_H
#include <chrono>
#include "opencv2/opencv.hpp"
#include <WinSock2.h>
#include "BAASLogger.h"
#include "BAASExceptions.h"
#include <filesystem>
#include <locale>
#include <codecvt>
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
      static int hex2int(const std::string input, int length);

      static int unsignedBinary2int(const std::string input, int length);

      static int binary2int(const std::string input, int length);

      static std::string getCurrentTimeString();

      static std::string executeCommandAndGetOutput(const std::string command);

      static std::string executeCommandAndGetOutput(const std::vector<std::string> commandList, int n);

      static FILE* executeCommand(std::string command);

      static std::string getStreamOutput(FILE* stream);

      static void executeCommandWithoutOutPut(const std::string command);

      static void executeCommandWithoutOutPut(const std::vector<std::string> commandList, int n);

      static std::string changeEndian(int a);

      static int getCurrentTimeStamp();
      /**
   * Convert int to string
   * @example 123 -> "123"
   */
      static std::string int2String(int a);

      static bool compareImage(const cv::Mat &image1);

      static bool checkImageBroken(const std::string path);

      static std::pair<int, int> deleteBrokenImage(const std::string path);

      static std::wstring stringToWString(const std::string& str);

      static std::string wstringToString(const std::wstring& wstr);

      static bool reviseEmulatorSerial(std::string &serial);

      static bool serialHost(const std::string serial, std::string &host);

      static bool serialPort(const std::string serial, std::string &port);

      static bool serialPort(const std::string serial, int &port);

      static bool isMuMuFamily(const std::string serial);

      static bool isMuMu12Family(const std::string serial);

      static void stringReplace(const std::string OLD,const std::string NEW,std::string &src,std::string &dst);

      static void stringSplit(const std::string &src, const std::string &separator, std::vector<std::string> &dst);

      static void stringJoin(const std::vector<std::string> &src, const std::string &joiner, std::string &dst);

      static int MuMuSerialToDisplayId(const std::string serial);

      static std::pair<std::string, std::string> serialToHostPort(const std::string serial);

      static bool sleepMS(int ms);

      static bool sleepS(int s);

      static long long getCurrentTimeMS();

};
#endif //BAAS_CXX_REFACTOR_UTIL_H
