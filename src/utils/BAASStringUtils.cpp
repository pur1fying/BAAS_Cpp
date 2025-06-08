//
// Created by pc on 2025/6/8.
//

#include "utils/BAASStringUtil.h"

using namespace std;

BAAS_NAMESPACE_BEGIN

string BAASStringUtil::int2hex(int a)
{
    string st;
    int length = 4;
    for (int i = 0; i < length; ++i) {
        st.push_back(static_cast<char>((a >> (length - i - 1) * 4) & 0xF));
        if (st[i] < 10) st[i] += '0';
        else st[i] += 'a' - 10;
    }
    return st;
}

string BAASStringUtil::int2binary(int a)
{
    string st;
    for (int i = 0; i < 4; ++i)st += static_cast<char>((a >> (24 - i * 8) & 0xFF));
    return st;
}

int BAASStringUtil::hex2int(
        const string &input,
        int length
)
{
    int output = 0;
    for (int i = 0; i < length; ++i) {
        if (input[i] >= '0' && input[i] <= '9') output = output * 16 + input[i] - '0';
        else if ((input[i] >= 'A' && input[i] <= 'F') || (input[i] >= 'a' && input[i] <= 'f'))
            output = output * 16 + (input[i] & 0xF) + 9;
    }
    return output;
}

int BAASStringUtil::unsignedBinary2int(
        const string &input,
        int length
)
{
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

int BAASStringUtil::binary2int(
        const string &input,
        int length
)
{
    int res = 0;
    int mask = 0xFF;
    for (int i = 0; i < length; ++i) {
        res |= (mask << (24 - i * 8) & (input[i] << (24 - i * 8)));
    }
    return res;
}


string BAASStringUtil::changeEndian(int a)
{
    string st;
    for (int i = 0; i < 4; ++i)st.push_back(a >> (i * 8) & 0xFF);
    return st;
}

string BAASStringUtil::int2String(int a)
{
    string st = "";
    while (a) {
        st += a % 10 + '0';
        a /= 10;
    }
    return st;
}


void BAASStringUtil::stringReplace(
        const std::string &OLD,
        const std::string &NEW,
        string &src,
        string &dst
)
{
    if (&src != &dst) {
        dst = src;
    }
    int start = 0;
    while ((start = int(dst.find(OLD, start))) != string::npos) {
        dst.replace(start, OLD.length(), NEW);
        start += int(NEW.length());
    }
}

void BAASStringUtil::stringReplace(
        const string &OLD,
        const string &NEW,
        string &tar
)
{
    int start = 0;
    while ((start = int(tar.find(OLD, start))) != string::npos) {
        tar.replace(start, OLD.length(), NEW);
        start += int(NEW.length());
    }
}

void BAASStringUtil::stringSplit(
        const string &src,
        const string &separator,
        vector<std::string> &dst
)
{
    dst.clear();
    int currentStart, currentEnd;
    currentStart = 0;
    currentEnd = int(src.find(separator, currentStart));
    while (currentEnd != string::npos) {
        dst.push_back(src.substr(currentStart, currentEnd - currentStart));
        currentStart = currentEnd + int(separator.length());
        currentEnd = int(src.find(separator, currentStart));
    }
    if (currentStart != currentEnd) {
        dst.push_back(src.substr(currentStart, currentEnd - currentStart));
    }
}

void BAASStringUtil::stringJoin(
        const vector<std::string> &src,
        const string &joiner,
        string &dst
)
{
    dst.clear();
    for (int i = 0; i < src.size(); i++) {
        dst += src[i];
        if (i != src.size() - 1) {
            dst += joiner;
        }
    }
}

bool BAASStringUtil::count_bit(int num)
{
    int count = 0;
    while (num) {
        num &= (num - 1);
        count++;
    }
    return count;
}

void BAASStringUtil::stringSplit(
        const string &src,
        const char separator,
        vector<std::string> &dst
)
{
    dst.clear();
    int currentStart, currentEnd;
    currentStart = 0;
    currentEnd = int(src.find(separator, currentStart));
    while (currentEnd != string::npos) {
        if (currentStart != currentEnd)dst.push_back(src.substr(currentStart, currentEnd - currentStart));
        currentStart = currentEnd + 1;
        currentEnd = int(src.find(separator, currentStart));
    }
    if (currentStart != currentEnd) {
        dst.push_back(src.substr(currentStart, currentEnd - currentStart));
    }
}


bool BAASStringUtil::allNumberChar(const string &src)
{
    auto it = src.begin();
    if (*it == '-') it++;
    for (; it != src.end(); it++) {
        if ((*it < '0' or *it > '9') && *it != '.') return false;
    }
    return true;
}

void BAASStringUtil::re_find_all(
        const string &src,
        const string &pattern,
        vector<std::smatch> &dst
)
{
    std::regex regex_pattern(pattern);
    std::string::const_iterator search_start(src.cbegin());

    std::smatch match;
    while (std::regex_search(search_start, src.cend(), match, regex_pattern)) {
        dst.push_back(match);
        search_start = match.suffix().first;
    }
}

void BAASStringUtil::re_find_all(
        const string &src,
        const string &pattern,
        vector<std::string> &dst
)
{
    dst.clear();

    std::regex regex_pattern(pattern);
    std::smatch match;
    std::string::const_iterator search_start(src.cbegin());

    while (std::regex_search(search_start, src.cend(), match, regex_pattern)) {
        dst.push_back(match[1].str());
        search_start = match.suffix()
                            .first;
    }
}

void BAASStringUtil::re_find(
        const string &src,
        const string &pattern,
        string &dst
)
{
    std::regex regex_pattern(pattern);
    std::smatch match;
    std::regex_search(src, match, regex_pattern);
    if (match.size() > 1) dst = match[1].str();
    else dst = "";
}

bool BAASStringUtil::re_match(
        const string &src,
        const string &pattern
)
{
    std::regex regex_pattern(pattern);
    return std::regex_match(src, regex_pattern);
}

uint32_t BAASStringUtil::st2u32(const string &src)
{
    uint32_t ret;
    memcpy(&ret, src.data(), sizeof(uint32_t));
    return ret;
}


BAAS_NAMESPACE_END

