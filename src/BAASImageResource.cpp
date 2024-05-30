//
// Created by pc on 2024/5/29.
//

#include <BAASImageResource.h>


BAASImageResource::BAASImageResource() {
    resource.clear();
}

bool BAASImageResource::addResource(const std::string &key, const std::string &path) {
    cv::Mat image;
    if (!BAASImageUtil::loadImage(path, image)) {
        return false;
    }
    resource[key] = {path, BAASRectangle(0, 0, image.cols, image.rows), image};
    return true;
}

bool BAASImageResource::removeResource(const std::string &key) {
    if (resource.find(key) == resource.end()) {
        return false;
    }
    resource.erase(key);
    return true;
}

void BAASImageResource::setResource(const std::string &key, const Image &src) {
    resource[key] = src;
}

void BAASImageResource::clearResource() {
    resource.clear();
}

void BAASImageResource::showResource() {
    for (auto &i : resource) {
        if(i.second.image.empty())continue;
        BAASLoggerInstance->BAASDebug(i.first + ":" +i.second.path);
    }
}

bool BAASImageResource::isLoaded(const std::string &key) {
    auto it = resource.find(key);
    if(it == resource.end())return false;
    if(it->second.image.empty())return false;
    return true;
}

void BAASImageResource::loadSkillIconResource(const std::string &path) {


}



