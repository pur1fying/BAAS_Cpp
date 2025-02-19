//
// Created by pc on 2024/5/29.
//
#ifndef BAAS_BAASIMAGERESOURCE_H_
#define BAAS_BAASIMAGERESOURCE_H_

#include <nlohmann/json.hpp>

#include "BAASImageUtil.h"
#include "BAASGlobals.h"
#include "config/BAASConfig.h"
#include "device/BAASConnection.h"

BAAS_NAMESPACE_BEGIN

struct BAASImage {
    BAASRectangle region;
    uint8_t direction = {};  // some app rotates the screen
    cv::Mat image;

    explicit BAASImage(
            int ulx,
            int uly,
            int lrx,
            int lry,
            uint8_t dir
    );

    explicit BAASImage(
            std::vector<int> &rg,
            uint8_t dir
    );

    BAASImage() = default;

    [[nodiscard]] bool empty() const;

    [[nodiscard]] std::string get_size() const;

    [[nodiscard]] std::string gen_info() const;
};

typedef std::map<std::string, BAASImage> BAASNameImageMap;
typedef std::map<std::string, BAASNameImageMap> BAASGroupImageMap;
typedef std::map<std::string, BAASGroupImageMap> BAASLanguageImageMap;
typedef std::map<std::string, BAASLanguageImageMap> BAASServerImageMap;

// <server>.<language>.<group>.<name>

class BAASImageResource {
public:
    static BAASImageResource *get_instance();

    void get(
            const std::string &server,
            const std::string &language,
            const std::string &task,
            const std::string &name,
            cv::Mat &out
    );

    void get(
            const std::string &server,
            const std::string &language,
            const std::string &task,
            const std::string &name,
            BAASImage &out
    );

    void get(
            const std::string &resource_pointer,
            cv::Mat &out
    );

    void set(
            const std::string &server,
            const std::string &language,
            const std::string &group,
            const std::string &name,
            const BAASImage &res
    );

    bool remove(const std::string &key);

    void clear();

    void show();

    bool is_loaded(const std::string &key);

    void keys(std::vector<std::string> &out);

    void load(const BAASConnection *conn);

    void load(
            const std::string &server,
            const std::string &language
    );

    bool is_loaded(
            const std::string &server,
            const std::string &language = "",
            const std::string &group = "",
            const std::string &name = ""
    );

    int load_from_json(
            const std::string &server,
            const std::string &language,
            const std::string &json_path
    );

    static std::string resource_pointer(
            const std::string &server,
            const std::string &language,
            const std::string &group,
            const std::string &name
    );

    static void resource_path(
            const std::string &server,
            const std::string &language,
            const std::string &suffix,
            std::string &out
    );

    static bool check_shape(
            const BAASImage &image,
            const std::string &server,
            const std::string &language,
            const std::string &group,
            const std::string &name
    );

private:
    static BAASImageResource *instance;

    BAASImageResource();

    BAASServerImageMap images;

    std::mutex resource_mutex;
};


class GetResourceError : public std::exception {
public:
    explicit GetResourceError(const std::string &msg) : msg(msg) {}

    const char *what() const noexcept override
    {
        return msg.c_str();
    }

private:
    std::string msg;
};

extern BAASImageResource *resource;

BAAS_NAMESPACE_END

#endif //BAAS_BAASIMAGERESOURCE_H_
