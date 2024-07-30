//
// Created by pc on 2024/5/29.
//

#include <BAASImageResource.h>
using namespace std;
BAASImageResource::BAASImageResource() {
    resource.clear();
}

bool BAASImageResource::add(const string &key, const string &path) {
    cv::Mat image;
    if (!BAASImageUtil::loadImage(path, image)) {
        return false;
    }
    resource[key] = {BAASRectangle(-1, -1, -1, -1), image};
    return true;
}

bool BAASImageResource::remove(const string &key) {
    if (resource.find(key) == resource.end()) {
        return false;
    }
    resource.erase(key);
    return true;
}

void BAASImageResource::setResource(const string &key, const BAASImage &src) {
    resource[key] = src;
}

void BAASImageResource::clearResource() {
    resource.clear();
}

void BAASImageResource::showResource() {
    for (auto &i : resource) {
        if(i.second.image.empty())continue;
    }
}

bool BAASImageResource::isLoaded(const string &key) {
    auto it = resource.find(key);
    if(it == resource.end())return false;
    if(it->second.image.empty())return false;
    return true;
}

void BAASImageResource::loadDirectoryImage(const string &dirPath, const string &prefix, const string &postfix) {
    if(!filesystem::exists(dirPath)){
        BAASGlobalLogger->BAASError("Directory not exist : " + dirPath);
        return;
    }
    string temp;
    for(auto &p: filesystem::directory_iterator(dirPath)){
        string key = p.path().stem().string();
        string extension = p.path().extension().string();
        if(extension != ".png" && extension != ".jpg" && extension != ".jpeg")continue;
        temp = prefix + key + postfix;
        add(temp, p.path().string());
    }
}

void BAASImageResource::keys(std::vector<std::string> &out) {
    out.clear();
    for(auto &i: resource){
        out.push_back(i.first);
    }
}

void BAASImageResource::get_image(const string &key, BAASImageResource::BAASImage &out) {
    auto it = resource.find(key);
    if(it == resource.end()){
        return;
    }
    out = it->second;
}





