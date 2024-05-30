//
// Created by pc on 2024/5/29.
//
#include "BAASImageUtil.h"
#include <nlohmann/json.hpp>
#include "BAASGlobals.h"
#ifndef BAAS_CPP_BAASIMAGERESOURCE_H
#define BAAS_CPP_BAASIMAGERESOURCE_H

class BAASImageResource{
    struct Image{
        std::string path;
        BAASRectangle region;
        cv::Mat image;
    };
public:
    BAASImageResource();

    bool addResource(const std::string& key, const std::string& path);

    bool removeResource(const std::string& key);

    void setResource(const std::string& key, const Image &src);

    void clearResource();

    void showResource();

    bool isLoaded(const std::string& key);

private:
    std::map<std::string, Image> resource;
};


#endif //BAAS_CPP_BAASIMAGERESOURCE_H
