//
// Created by pc on 2024/5/29.
//

#include <BAASImageResource.h>

using namespace std;
using namespace nlohmann;

BAASImageResource *BAASImageResource::instance = nullptr;

BAASImageResource* resource = nullptr;

BAASImage::BAASImage(int ulx, int uly, int lrx, int lry, uint8_t dir) {
    region = {ulx, uly, lrx, lry};
    direction = dir;
}

BAASImage::BAASImage(vector<int> &rg, uint8_t dir) {
    region = {rg[0], rg[1], rg[2], rg[3]};
    direction = dir;
}

std::string BAASImage::get_size() const {
    return to_string(region.lr.x - region.ul.x) + "x" + to_string(region.lr.y - region.ul.y);
}

std::string BAASImage::gen_info() const {
    string info = "Region : [ " + to_string(region.ul.x) + ", " + to_string(region.ul.y) + " ] [ " + to_string(region.lr.x) + ", " + to_string(region.lr.y) + " ]";
    info += " Direction : " + to_string(direction);
    info += " Resolution : " + get_size();
    return info;
}

BAASImageResource *BAASImageResource::get_instance() {
    if(instance == nullptr) {
        instance = new BAASImageResource();
    }
    return instance;
}

void BAASImageResource::get(const string &server, const string &language, const string &task, const string &name, cv::Mat &out) {
    if(!is_loaded(server, language, task, name))throw GetResourceError("Image Resource [ " + resource_pointer(server, language, task, name) + " ] not loaded");
    lock_guard<mutex> lock(resource_mutex);
    out = images[server][language][task][name].image;
}

void BAASImageResource::get(const string &server, const string &language, const string &task, const string &name, BAASImage &out) {
    if(!is_loaded(server, language, task, name)) throw GetResourceError("Image Resource [ " + resource_pointer(server, language, task, name) + " ] not loaded");
    lock_guard<mutex> lock(resource_mutex);
    out = images[server][language][task][name];
}

void BAASImageResource::get(const string &resource_pointer, cv::Mat &out) {
    vector<string> tokens;
    BAASUtil::stringSplit(resource_pointer, ".", tokens);
    if(tokens.size() != 4)throw GetResourceError("Resource pointer [ " + resource_pointer + " ]  format error");
    get(tokens[0], tokens[1], tokens[2], tokens[3], out);

}

void BAASImageResource::set(const string &server, const string &language, const string &group, const string &name,const BAASImage &res) {
    lock_guard<mutex> lock(resource_mutex);
    images[server][language][group][name] = res;
}

bool BAASImageResource::remove(const string &key) {
    if (images.find(key) == images.end()) {
        return false;
    }
    images.erase(key);
    return true;
}

void BAASImageResource::clear() {
    images.clear();
}

void BAASImageResource::show() {
    BAASGlobalLogger->hr("Image Resource");
    for(auto &i: images)
        for(auto &j: i.second)
            for(auto &k: j.second)
                for(auto &l: k.second)
                    BAASGlobalLogger->BAASInfo("Image [ " + resource_pointer(i.first, j.first, k.first, l.first) + " ]\n" + l.second.gen_info());
}

bool BAASImageResource::is_loaded(const string &key) {
    auto it = images.find(key);
    if(it == images.end())return false;
    return true;
}

void BAASImageResource::keys(std::vector<std::string> &out) {
    out.clear();
    for(auto &i: images){
        out.push_back(i.first);
    }
}

void BAASImageResource::load(const BAASConnection *conn) {
    load(conn->get_server(), conn->get_language());
}

void BAASImageResource::load(const string &server, const string &language) {
    BAASGlobalLogger->BAASInfo("Load Image Resource Server : [ " + server + " ] Language : [ " + language + " ]");
    if(is_loaded(server, language)) {
        BAASGlobalLogger->BAASInfo("Image Resource [ " + server + " ] [ " + language + " ] already loaded");
        return;
    }
    string info_path;
    resource_path(server, language, "image_info", info_path);
    if(!filesystem::exists(info_path)){
        BAASGlobalLogger->BAASError("Image Resource [ " + server + " ] [ " + language + " ] not found");
        return;
    }
    // load from image_info folder
    // data stored in json and load
    string temp_path;
    int total_loaded = 0;
    for(const auto &entry : filesystem::recursive_directory_iterator(info_path)) {
        temp_path = entry.path().string();
        if(filesystem::is_regular_file(entry) && temp_path.ends_with(".json"))
            total_loaded += load_from_json(server, language, temp_path);
    }

    BAASGlobalLogger->BAASInfo("Image Resource [ " + server + " ] [ " + language + " ] loaded [ " + to_string(total_loaded) + " ] images");
}

bool BAASImageResource::is_loaded(const string &server, const string &language, const string &group, const string &name) {
    auto it1 = images.find(server);
    if(it1 == images.end()) return false;
    if(language.empty()) return true;

    auto it2 = it1->second.find(language);
    if(it2 == it1->second.end()) return false;
    if(group.empty()) return true;

    auto it3 = it2->second.find(group);
    if(it3 == it2->second.end()) return false;
    if(name.empty()) return true;

    auto it4 = it3->second.find(name);
    if(it4 == it3->second.end()) return false;
    return true;
}

int BAASImageResource::load_from_json(const string &server, const string &language, const string &json_path) {
    int successfully_loaded_cnt = 0;
    BAASConfig info(json_path, (BAASLogger*)BAASGlobalLogger);
    string group = info.getString("group");
    string path = info.getString("path");
    json j = info.get<json>("image");
    string group_path;
    string image_path;
    resource_path(server, language, group, group_path);
    for(auto &it: j.items()){
        const string& name = it.key();
        json image_info = it.value();
        vector<int> region = image_info["region"];
        if (region.size()!= 4) {
            BAASGlobalLogger->BAASError("Image [ " + resource_pointer(server, language, group, name) + " ] region cnt not equal to 4");
            continue;
        }
        uint8_t d = 0;
        if(image_info.contains("direction"))
            d = image_info["direction"];
        BAASImage image(region, d);
        image_path = group_path;
        image_path += "\\";
        image_path += name+".png";
        if(!BAASImageUtil::load(image_path, image.image)){
            BAASGlobalLogger->BAASError("Image [ " + resource_pointer(server, language, group, name) + " ] load failed, reason : not exist or broken");
            continue;
        }
        if(!check_shape(image, server, language, group, name)) continue;
        successfully_loaded_cnt++;
        set(server, language, group, name, image);
    }
    return successfully_loaded_cnt;
}

inline void BAASImageResource::resource_path(const string &server, const string &language, const std::string& suffix, string &out) {
    out = BAAS_IMAGE_RESOURCE_DIR + "\\" + server + "\\" + language + "\\" + suffix;
}

std::string BAASImageResource::resource_pointer(const string &server, const string &language, const string &group,const string &name) {
    return server + "." + language + "." + group + "." + name;
}

inline bool BAASImageResource::check_shape(const BAASImage &image, const string& server, const string& language, const string& group, const string& name) {
    int h_record = image.region.lr.y - image.region.ul.y;
    int w_record = image.region.lr.x - image.region.ul.x;
    int h_true = image.image.rows;
    int w_true = image.image.cols;
    if(h_record != h_true || w_record != w_true){
        BAASGlobalLogger->BAASError("Image [ " + resource_pointer(server, language, group, name) + " ] shape not match");
        BAASGlobalLogger->BAASError("Recorded Shape : [ " + to_string(h_record) + "x" + to_string(w_record) + " ]");
        BAASGlobalLogger->BAASError("True Shape : [ " + to_string(h_true) + "x" + to_string(w_true) + " ]");
        return false;
    }
    return true;
}

BAASImageResource::BAASImageResource() {
    images.clear();
}











