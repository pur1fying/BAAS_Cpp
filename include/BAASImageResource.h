//
// Created by pc on 2024/5/29.
//
#ifndef BAAS_CPP_BAASIMAGERESOURCE_H
#define BAAS_CPP_BAASIMAGERESOURCE_H
#include <nlohmann/json.hpp>

#include "BAASImageUtil.h"
#include "BAASGlobals.h"
class BAASImageResource{
public:
    struct Image{
        std::string path;
        BAASRectangle region;
        cv::Mat image;
    };
    BAASImageResource();

    void getResource(const std::string& key, Image &out);

    bool addResource(const std::string& key, const std::string& path);

    bool removeResource(const std::string& key);

    void setResource(const std::string& key, const Image &src);

    void clearResource();

    void showResource();

    void loadDirectoryImage(const std::string& path, const std::string& prefix="", const std::string& postfix="");

    bool isLoaded(const std::string& key);

    void keys(std::vector<std::string> &out);


private:
    std::map<std::string, Image> resource;
};


#endif //BAAS_CPP_BAASIMAGERESOURCE_H
