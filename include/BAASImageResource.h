//
// Created by pc on 2024/5/29.
//
#ifndef BAAS_BAASIMAGERESOURCE_H_
#define BAAS_BAASIMAGERESOURCE_H_

#include <nlohmann/json.hpp>

#include "BAAS.h"
#include "BAASImageUtil.h"
#include "BAASGlobals.h"
#include "config/BAASConfig.h"
#include "device/BAASConnection.h"

BAAS_NAMESPACE_BEGIN

struct BAASImage {
    BAASRectangle region;
    uint8_t direction = 0;  // some app rotates the screen
    cv::Mat image;
    cv::Vec3b mean_rgb;

    explicit BAASImage(
            int ulx,
            int uly,
            int lrx,
            int lry,
            uint8_t dir
    );

    explicit BAASImage(
            const std::vector<int>& rg,
            uint8_t dir
    );

    BAASImage() = default;

    [[nodiscard]] bool empty() const;

    [[nodiscard]] std::string get_size() const;

    [[nodiscard]] std::string get_mean_rgb() const;

    static constexpr auto _vec3_format = "({:>3},{:>3},{:>3})";
};

// {server}.{language}.{group}.{name}

class BAASImageResource {
public:
    static BAASImageResource *get_instance();

    void get(
            const std::string &resource_pointer,
            cv::Mat &out
    );

    void get(
            const std::string &resource_pointer,
            BAASImage &out
    );

    void set(
            const std::string& server,
            const std::string& language,
            const std::string& group,
            const std::string& name,
            const BAASImage& res
    );

    static inline std::string get_resource_pointer(
            const std::string& server,
            const std::string& language,
            const std::string& group,
            const std::string& name
    )
    {
        return server + "." + language + "." + group + "." + name;
    }

    bool remove(const std::string &key);

    void clear();

    void show();

    bool is_loaded(
            const BAAS* baas,
            const std::string& group,
            const std::string& name
    ) const;

    bool is_loaded(
            const std::string& server,
            const std::string& language,
            const std::string& group,
            const std::string& name
    ) const;

    bool is_loaded(const std::string &key) const;

    void keys(std::vector<std::string>& out);

    void load(
            const std::string& server,
            const std::string& language
    );

    int load_from_json(
            const std::string& server,
            const std::string& language,
            const std::filesystem::path& json_path
    );

    static std::string resource_pointer(
            const std::string& server,
            const std::string& language,
            const std::string& group,
            const std::string& name
    )
    {
        return server + "." + language + "." + group + "." + name;
    }

    inline static std::string resource_pointer(
            const std::string& prefix,
            const std::string& group,
            const std::string& name
    )
    {
        return prefix + group + "." + name;
    }

    static void resource_path(
            const std::string& server,
            const std::string& language,
            const std::string& suffix,
            std::filesystem::path& out
    );

    static bool check_shape(
            const BAASImage& image,
            const std::string& server,
            const std::string& language,
            const std::string& group,
            const std::string& name
    );

private:
    static BAASImageResource* instance;

    BAASImageResource();

    std::map<std::string, BAASImage> images;

    std::mutex resource_mutex;
};


class GetResourceError : public std::exception {
public:
    explicit GetResourceError(const std::string& msg) : msg(msg) {}

    const char *what() const noexcept override
    {
        return msg.c_str();
    }

private:
    std::string msg;
};

extern BAASImageResource* resource;

BAAS_NAMESPACE_END

#endif //BAAS_BAASIMAGERESOURCE_H_
