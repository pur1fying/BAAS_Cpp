//
// Created by pc on 2025/6/8.
//

#ifndef BAAS_UTILS_BAASSTRINGUTIL_H_
#define BAAS_UTILS_BAASSTRINGUTIL_H_

#include <regex>

#include <simdutf.h>

#include "core_defines.h"

BAAS_NAMESPACE_BEGIN

class BAASStringUtil {

public:

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
            const std::string& input,
            int length
    );

    static int unsignedBinary2int(
            const std::string& input,
            int length
    );

    static int binary2int(
            const std::string& input,
            int length
    );

    static std::string changeEndian(int a);

    static std::string int2String(int a);

    static void stringReplace(
            const std::string& OLD,
            const std::string& NEW,
            std::string& src,
            std::string& dst
    );

    static void stringReplace(
            const std::string& OLD,
            const std::string& NEW,
            std::string& src
    );

    static void stringSplit(
            const std::string& src,
            const std::string& separator,
            std::vector<std::string>& dst
    );

    static void stringSplit(
            const std::string& src,
            const char separator,
            std::vector<std::string>& dst
    );

    static void stringJoin(
            const std::vector<std::string>& src,
            const std::string& joiner,
            std::string& dst
    );

    static bool allNumberChar(const std::string& src);

    static void re_find_all(
            const std::string& src,
            const std::string& pattern,
            std::vector<std::smatch>& dst
    );

    static void re_find_all(
            const std::string& src,
            const std::string& pattern,
            std::vector<std::string>& dst
    );

    static void re_find(
            const std::string& src,
            const std::string& pattern,
            std::string& dst
    );

    static bool re_match(
            const std::string& src,
            const std::string& pattern
    );

    static uint32_t st2u32(const std::string& src);

    template<typename T>
    static inline void append_big_endian(
            std::string& dst,
            T src
    )
    {
        for (int i = sizeof(T) - 1; i >= 0; i--) {
            dst.push_back((src >> (i * 8)) & 0xFF);
        }
    }

    static bool count_bit(int num);

    template<typename T>
    static inline void append_little_endian(
            std::string& dst,
            T src
    )
    {
        for (int i = 0; i < sizeof(T); i++) {
            dst.push_back((src >> (i * 8)) & 0xFF);
        }
    }

    inline static void str2wstr(const std::string& in, std::wstring& out)
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

    inline static void wstr2str(const std::wstring& in, std::string& out)
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

#endif //BAAS_UTILS_BAASSTRINGUTIL_H_
