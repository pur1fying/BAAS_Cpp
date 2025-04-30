//
// Created by pc on 2024/5/29.
//

#include <BAASImageResource.h>

using namespace std;
using namespace nlohmann;

BAAS_NAMESPACE_BEGIN

BAASImageResource *BAASImageResource::instance = nullptr;

BAASImageResource *resource = nullptr;

BAASImage::BAASImage(
        int ulx,
        int uly,
        int lrx,
        int lry,
        uint8_t dir
)
{
    region = {ulx, uly, lrx, lry};
    direction = dir;
}

BAASImage::BAASImage(
        vector<int> &rg,
        uint8_t dir
)
{
    region = {rg[0], rg[1], rg[2], rg[3]};
    direction = dir;
}

bool BAASImage::empty() const
{
    return image.empty();
}

std::string BAASImage::get_size() const
{
    return to_string(region.lr.x - region.ul.x) + "x" + to_string(region.lr.y - region.ul.y);
}

std::string BAASImage::gen_info() const
{
    string info = "Region : " + region.to_string();
    info += " Direction : " + to_string(direction);
    info += " Resolution : " + get_size();
    return info;
}


BAASImageResource *BAASImageResource::get_instance()
{
    if (instance == nullptr) {
        instance = new BAASImageResource();
    }
    return instance;
}

void BAASImageResource::get(
        const string &resource_pointer,
        cv::Mat &out
)
{
    auto it = images.find(resource_pointer);
    if (it == images.end()) throw GetResourceError("Image Resource [ " + resource_pointer + " ] not exist.");
    lock_guard<mutex> lock(resource_mutex);
    out = images[resource_pointer].image; // Don't need clone since template will not be revised.
}

void BAASImageResource::get(
        const string &resource_pointer,
        BAASImage &out
)
{
    auto it = images.find(resource_pointer);
    if (it == images.end()) throw GetResourceError("Image Resource [ " + resource_pointer + " ] not exist.");
    lock_guard<mutex> lock(resource_mutex);
    out = images[resource_pointer];
}

void BAASImageResource::set(
        const string &server,
        const string &language,
        const string &group,
        const string &name,
        const BAASImage &res
)
{
    lock_guard<mutex> lock(resource_mutex);
    images[get_resource_pointer(server, language, group, name)] = res;
}

bool BAASImageResource::remove(const string &key)
{
    if (images.find(key) == images.end()) {
        return false;
    }
    images.erase(key);
    return true;
}

void BAASImageResource::clear()
{
    images.clear();
}

void BAASImageResource::show()
{
    BAASGlobalLogger->hr("List Loaded Image Resource");
    for (auto &i: images)
        BAASGlobalLogger->BAASInfo("Image [ " +  i.first + " ]\n" +i.second.gen_info());
}

bool BAASImageResource::is_loaded(const string &key) const
{
    auto it = images.find(key);
    if (it == images.end()) return false;
    return true;
}

void BAASImageResource::keys(std::vector<std::string> &out)
{
    out.clear();
    for (auto &i: images) {
        out.push_back(i.first);
    }
}

void BAASImageResource::load(
        const string &server,
        const string &language
)
{
    BAASGlobalLogger->BAASInfo("Load Image Resource Server : [ " + server + " ] Language : [ " + language + " ]");
    std::string resource_load_checker = resource_pointer(server, language, "", "");
    if (is_loaded(resource_load_checker)) {
        BAASGlobalLogger->BAASInfo("Image Resource [ " + server + " ] [ " + language + " ] already loaded");
        return;
    }

    images[resource_load_checker] = BAASImage();

    std::filesystem::path info_path;
    resource_path(server, language, "image_info", info_path);
    if (!filesystem::exists(info_path)) {
        BAASGlobalLogger->BAASError("Image Resource [ " + server + " ] [ " + language + " ] not found");
        return;
    }
    // load from image_info folder
    // data stored in json and load
    string temp_path;
    int total_loaded = 0;
    for (const auto &entry: filesystem::recursive_directory_iterator(info_path)) {
        temp_path = entry.path().string();
        if (filesystem::is_regular_file(entry) && temp_path.ends_with(".json"))
            total_loaded += load_from_json(server, language, temp_path);
    }

    BAASGlobalLogger->BAASInfo(
            "Image Resource [ " + server + " ] [ " + language + " ] loaded [ " + to_string(total_loaded) + " ] images"
    );
}

bool BAASImageResource::is_loaded(
        const string &server,
        const string &language,
        const string &group,
        const string &name
) const
{
    return is_loaded(get_resource_pointer(server, language, group, name));
}

int BAASImageResource::load_from_json(
        const string &server,
        const string &language,
        const string &json_path
)
{
    int successfully_loaded_cnt = 0;
    BAASConfig info(json_path, (BAASLogger *) BAASGlobalLogger);
    string group = info.getString("group");
    string path = info.getString("path");
    std::string json_ptr = "/image";
    json j = info.get(json_ptr, json());
    std::filesystem::path group_path;
    std::filesystem::path image_path;
    resource_path(server, language, path, group_path);
    for (auto &it: j.items()) {
        const string &name = it.key();
        vector<int> region = info.get(json_ptr + "/" + name + "/region", vector<int>{-1, -1, -1, -1});
        uint8_t d = info.get(json_ptr + "/" + name + "/direction", uint8_t(0));
        BAASImage image(region, d);
        image_path = group_path / (name + ".png");
        if (!BAASImageUtil::load(image_path.string(), image.image)) {
            BAASGlobalLogger->BAASError(
                    "Image [ " + get_resource_pointer(server, language, group, name) +" ] load failed, "
                                                                                      "reason : not exist or broken"
            );
            continue;
        }
        if (!check_shape(image, server, language, group, name)) continue;
        image.mean_rgb = BAASImageUtil::get_mean_rgb(image.image);
        BAASGlobalLogger->BAASInfo(
                "Image [ " + group + "." + name + " ] loaded, " + "mean_rgb : [ " + std::to_string(image.mean_rgb[0]) + " , " +
                std::to_string(image.mean_rgb[1]) + " , " + std::to_string(image.mean_rgb[2]) + " ]"
        );
        successfully_loaded_cnt++;
        set(server, language, group, name, image);
    }
    return successfully_loaded_cnt;
}

void BAASImageResource::resource_path(
        const std::string &server,
        const std::string &language,
        const std::string &suffix,
        std::filesystem::path &out
)
{
    out = BAAS_IMAGE_RESOURCE_DIR / server / language / suffix;
}


inline bool BAASImageResource::check_shape(
        const BAASImage &image,
        const string &server,
        const string &language,
        const string &group,
        const string &name
)
{
    if (image.region == BAASRectangle(-1, -1, -1, -1)) return true;
    int h_record = image.region.lr.y - image.region.ul.y;
    int w_record = image.region.lr.x - image.region.ul.x;
    int h_true = image.image.rows;
    int w_true = image.image.cols;
    if (h_record != h_true || w_record != w_true) {
        BAASGlobalLogger->BAASError("Image [ " + resource_pointer(server, language, group, name) + " ] shape not match");
        BAASGlobalLogger->BAASError("Recorded Shape : [ " + to_string(h_record) + "x" + to_string(w_record) + " ]");
        BAASGlobalLogger->BAASError("True Shape : [ " + to_string(h_true) + "x" + to_string(w_true) + " ]");
        return false;
    }
    return true;
}

BAASImageResource::BAASImageResource()
{
    images.clear();
}

bool BAASImageResource::is_loaded(
        const BAAS *baas,
        const string &group,
        const string &name
) const
{
    return is_loaded(baas->get_image_resource_prefix() + group + "." + name);
}




BAAS_NAMESPACE_END
