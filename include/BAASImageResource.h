//
// Created by pc on 2024/5/29.
//
#ifndef BAAS_BAASIMAGERESOURCE_H_
#define BAAS_BAASIMAGERESOURCE_H_
#include <nlohmann/json.hpp>

#include "BAASImageUtil.h"
#include "BAASGlobals.h"
class BAASImageResource{

public:
    // <server>.<language>.<group>.<name>
    struct BAASImage{
        BAASRectangle region;
        cv::Mat image;
        uint8_t direction;  // some app rotates the screen
    };

    BAASImageResource();

    void get_image(const std::string& server, const std::string& language, const std::string& task, const std::string& name, BAASImage &out);

    void get_image(const std::string& key, BAASImage &out);

    bool add(const std::string& key, const std::string& path);

    bool remove(const std::string& key);

    void setResource(const std::string& key, const BAASImage &src);

    void clearResource();

    void showResource();

    void loadDirectoryImage(const std::string& path, const std::string& prefix="", const std::string& postfix="");

    bool isLoaded(const std::string& key);

    void keys(std::vector<std::string> &out);


private:
    std::map<std::string, BAASImage> resource;
};


#endif //BAAS_BAASIMAGERESOURCE_H_
