//
// Created by pc on 2024/4/12.
//
#include "BAASUtil.h"
using namespace std::chrono;
using namespace std;

bool BAASUtil::initWinsock() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        BAASLoggerInstance->BAASError("WSAStartup failed");
        return false;
    }
    return true;
}

string BAASUtil::int2hex(int a) {
    string st = "";
    int length = 4;
    for (int i = 0; i < length; ++i) {
        st.push_back((a >> (length - i - 1) * 4) & 0xF);
        if (st[i] < 10) st[i] += '0';
        else st[i] += 'a' - 10;
    }
    return st;
}

string BAASUtil::int2binary(int a) {
    string st = "";
    for (int i = 0; i < 4; ++i)st += (a >> (24 - i * 8) & 0xFF);
    return st;
}

int BAASUtil::hex2int(const string input, int length) {
    int output = 0;
    for (int i = 0; i < length; ++i) {
        if (input[i] >= '0' && input[i] <= '9') output = output * 16 + input[i] - '0';
        else if ((input[i] >= 'A' && input[i] <= 'F') || (input[i] >= 'a' && input[i] <= 'f'))
            output = output * 16 + (input[i] & 0xF) + 9;
    }
    return output;
}

int BAASUtil::unsignedBinary2int(const string input, int length) {
    int output = 0;
    int temp = 1;
    unsigned int t;
    for (int i = length - 1; i >= 0; --i) {
        t = input[i] & 0xFF;
        output += t * temp;
        temp *= 256;
    }
    return output;
}

int BAASUtil::binary2int(const string input, int length) {
    int res = 0;
    int mask = 0xFF;
    for (int i = 0; i < length; ++i) {
        res |= (mask << (24 - i * 8) & (input[i] << (24 - i * 8)));
    }
    return res;
}

string BAASUtil::getCurrentTimeString() {
    time_t currentTime = time(nullptr);
    tm* localTime = localtime(&currentTime);
    ostringstream oss;
    oss << std::put_time(localTime, "|%Y-%m-%d|%H:%M:%S|");
    return oss.str();
}

string BAASUtil::executeCommandAndGetOutput(const string command) {
    FILE* stream = _popen(command.c_str(), "rb");
    if(stream == nullptr) {
        throw RuntimeError("Failed to execute command");
    }
    string output = getStreamOutput(stream);
    _pclose(stream);
    return output;
}

string BAASUtil::executeCommandAndGetOutput(const vector<string> commandList, int n) {
    string command;
    BAASUtil::stringJoin(commandList, " ", command);
    return executeCommandAndGetOutput(command);
}

FILE* BAASUtil::executeCommand(string command) {
    return _popen(command.c_str(), "rb");
}

string BAASUtil::getStreamOutput(FILE* stream) {
    char buffer[128];
    string output = "";
    while(fgets(buffer, 128, stream) != nullptr) {
        output += buffer;
    }
    return output;
}

void BAASUtil::executeCommandWithoutOutPut(const string command) {
    if(system(command.c_str())) {
        throw RuntimeError("Failed to execute command : " + command);
    }
}

void BAASUtil::executeCommandWithoutOutPut(const vector<string> commandList, int n) {
    string command;
    BAASUtil::stringJoin(commandList, " ", command);
    if(system(command.c_str())) {
        throw RuntimeError("Failed to execute command : " + command);
    }
}



string BAASUtil::changeEndian(int a) {
    string st = "";
    for (int i = 0; i < 4; ++i)st.push_back(a >> (i * 8) & 0xFF);
    return st;
}

int BAASUtil::getCurrentTimeStamp() {
    return chrono::system_clock::to_time_t(chrono::system_clock::now());
}

string BAASUtil::int2String(int a) {
    string st = "";
    while (a) {
        st += a % 10 + '0';
        a /= 10;
    }
    return st;
}

bool BAASUtil::checkImageBroken(const std::string path) {
    if(!filesystem::exists(path)){
        throw ValueError("File : [ " + path + " ] not exists");
    }
    cv::Mat image = cv::imread(path);
    if(image.empty()) {
        BAASLoggerInstance->BAASError("Broken Image Path : " + path);
        return false;
    }
    return true;
}

pair<int, int> BAASUtil::deleteBrokenImage(const std::string path) {
    int totalFiles = 0, brokenFiles = 0;
    if(filesystem::is_directory(path)) {
         for(auto &p: filesystem::directory_iterator(path)) {
             totalFiles++;
             if(!checkImageBroken(p.path().string())) {
                 brokenFiles++;
                 filesystem::remove(p.path());
             }
         }
    }
    else {
        totalFiles = 1;
        if(!filesystem::exists(path))throw ValueError("File not exists");
        if(!checkImageBroken(path)) {
            brokenFiles = 1;
            filesystem::remove(path);
        }
    }
    return make_pair(brokenFiles, totalFiles);
}

wstring BAASUtil::stringToWString(const std::string& str) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(str);
}

string BAASUtil::wstringToString(const std::wstring& wstr) {
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

bool BAASUtil::reviseEmulatorSerial(std::string &serial) {
    stringReplace(" ", "", serial, serial);
    stringReplace("；", ":", serial, serial);
    stringReplace("。", ".", serial, serial);
    stringReplace(",", ".", serial, serial);
    stringReplace("，", ".", serial, serial);
    stringReplace("127.0.0.1.", "127.0.0.1:", serial, serial);
    try{
        stoi(serial);
        serial = "127.0.0.1:" + serial;
    }
    catch (invalid_argument &e) {

    }
    return true;
}

bool BAASUtil::serialHost(const std::string serial, string &host) {
    int pos = serial.find(':');
    if(pos == string::npos) {
        host = "";
        return false;
    }
    host = serial.substr(0, pos);
    return true;
}

bool BAASUtil::serialPort(const std::string serial, string &port) {
    int pos = serial.find(':');
    if(pos == string::npos) {
        port = "";
        return false;
    }
    port = serial.substr(pos + 1);
    return true;
}

bool BAASUtil::serialPort(const std::string serial, int &port) {
    int pos = serial.find(':');
    if(pos == string::npos) {
        port = 0;
        return false;
    }
    try{
        port = stoi(serial.substr(pos + 1));
    }
    catch (invalid_argument &e) {
        port = 0;
        return false;
    }
    return true;
}

bool BAASUtil::isMuMuFamily(const std::string serial) {
    return serial == "127.0.0.1:7555" or isMuMuFamily(serial);
}

bool BAASUtil::isMuMu12Family(const std::string serial) {
    int port;
    serialPort(serial, port);
    return port >= 16384 and port <= 17408;
}

void BAASUtil::stringReplace(const std::string OLD, const std::string NEW, string &src, string &dst) {
    cout << &src << " " << &dst << endl;
    if(&src != &dst) {
        dst = src;
    }
    int start = 0;
    while((start = dst.find(OLD, start)) != string::npos) {
        dst.replace(start, OLD.length(), NEW);
        start += NEW.length();
    }
}

void BAASUtil::stringSplit(const string &src, const string &separator, vector<std::string> &dst) {
    string str = src;
    int currentStart,currentEnd;
    currentStart = currentEnd = 0;
    while((currentEnd = str.find(separator, currentStart)) != string::npos) {
        dst.push_back(str.substr(currentStart, currentEnd - currentStart));
        currentStart = currentEnd + separator.length();
    }
}

int BAASUtil::MuMuSerialToDisplayId(const std::string serial) {
    int port;
    serialPort(serial, port);
    port -= 16384;
    int index = port / 32, offset = port % 32;
    if ((offset == 0 or offset == 1 or offset == 2) && index >= 0 && index <= 3) {
        return index;
    }
    return -1;
}

void BAASUtil::stringJoin(const vector<std::string> &src, const string &joiner, string &dst) {
    for(int i = 0; i < src.size(); i++) {
        dst += src[i];
        if(i != src.size() - 1) {
            dst += joiner;
        }
    }
}

std::pair<std::string, std::string> BAASUtil::serialToHostPort(const std::string serial) {
    int pos = serial.find(':');
    if(pos == string::npos) {
        return make_pair("", "");
    }
    return make_pair(serial.substr(0, pos), serial.substr(pos + 1));
}

bool BAASUtil::sleepMS(int ms) {
    this_thread::sleep_for(chrono::milliseconds(ms));
    return true;
}

bool BAASUtil::sleepS(int s) {
    this_thread::sleep_for(chrono::seconds(s));
    return true;
}

long long BAASUtil::getCurrentTimeMS() {
    std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    std::chrono::milliseconds duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return duration.count();
}
