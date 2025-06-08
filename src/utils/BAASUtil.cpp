//
// Created by pc on 2024/4/12.
//
#include "utils/BAASUtil.h"

using namespace std::chrono;
using namespace std;

BAAS_NAMESPACE_BEGIN

double BAASUtil::genRandDouble(
        const double &min,
        const double &max
)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(min, max);
    return dis(gen);
}

int BAASUtil::genRandInt(
        const int &min,
        const int &max
)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dis(min, max);
    return dis(gen);
}

bool BAASUtil::endsWith(
        const string &src,
        const string &suffix
)
{
    if (src.length() < suffix.length()) return false;
    return src.substr(src.length() - suffix.length()) == suffix;
}

std::istream &BAASUtil::safeGetLine(
        istream &is,
        string &t
)
{
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




BAAS_NAMESPACE_END
