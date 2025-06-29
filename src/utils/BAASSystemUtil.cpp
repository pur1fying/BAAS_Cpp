//
// Created by pc on 2025/6/8.
//

#include "utils/BAASSystemUtil.h"

#include "BAASLogger.h"
#include "BAASExceptions.h"
#include "utils/BAASStringUtil.h"

using namespace std;

BAAS_NAMESPACE_BEGIN

bool BAASSystemUtil::initWinsock()
{
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        BAASGlobalLogger->BAASError("WSAStartup failed");
        return false;
    }
    return true;
#endif // _WIN32
    return false;
}
#ifdef _WIN32

string BAASSystemUtil::executeCommandAndGetOutput(const string &command)
{
    FILE* stream = _popen(command.c_str(), "rb");
    if (stream == nullptr) {
        throw RuntimeError("Failed to execute command");
    }
    string output = getStreamOutput(stream);
    _pclose(stream);
    return output;
}

string BAASSystemUtil::executeCommandAndGetOutput(
        const vector<string> &commandList,
        int n
)
{
    string command;
    BAASStringUtil::stringJoin(commandList, " ", command);
    return executeCommandAndGetOutput(command);
}

FILE* BAASSystemUtil::executeCommand(const string &command)
{
    return _popen(command.c_str(), "rb");
}
#endif // _WIN32

string BAASSystemUtil::getStreamOutput(FILE *stream)
{
    char buffer[128];
    string output;
    while (fgets(buffer, 128, stream) != nullptr) {
        output += buffer;
    }
    return output;
}

void BAASSystemUtil::executeCommandWithoutOutPut(const string &command)
{
    if (system(command.c_str())) {
        throw RuntimeError("Failed to execute command : " + command);
    }
}

void BAASSystemUtil::executeCommandWithoutOutPut(
        const vector<string> &commandList,
        int n
)
{
    string command;
    BAASStringUtil::stringJoin(commandList, " ", command);
    if (system(command.c_str())) {
        throw RuntimeError("Failed to execute command : " + command);
    }
}




BAAS_NAMESPACE_END