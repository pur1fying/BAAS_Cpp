//
// Created by pc on 2024/8/6.
//
#include "feature/BAASFeature.h"
using namespace std;
using namespace nlohmann;

std::vector<std::function<bool(BAASConfig*, const cv::Mat&, BAASConfig&)>> BAASFeature::compare_functions;

std::map<std::string, BAASConfig*> BAASFeature::features;

BAASFeature *BAASFeature::instance = nullptr;

BAASFeature *features = nullptr;

BAASFeature *BAASFeature::get_instance() {
    if(instance == nullptr) {
        instance = new BAASFeature();
    }
    return instance;
}

void BAASFeature::init_funcs() {
    compare_functions.emplace_back(MatchTemplateFeature::compare);

}

void BAASFeature::load() {
    if(!filesystem::exists(BAAS_FEATURE_DIR)) {
        BAASGlobalLogger->BAASError("Feature Dir [ " + BAAS_FEATURE_DIR + " ] not exists");
        return;
    }
    // load from image_info folder
    // data stored in json and load
    string temp_path;
    int total_loaded = 0;
    for(const auto &entry : filesystem::recursive_directory_iterator(BAAS_FEATURE_DIR)) {
        temp_path = entry.path().string();
        if(filesystem::is_regular_file(entry) && temp_path.ends_with(".json"))
            total_loaded += load_from_json(temp_path);
    }
    BAASGlobalLogger->BAASInfo("Totally loaded [ " + to_string(total_loaded) + " ] features");
}

BAASFeature::BAASFeature() {
    init_funcs();
    load();
}

int BAASFeature::load_from_json(const string &path) {
    BAASConfig _feature(path, (BAASLogger*)BAASGlobalLogger);
    json j = _feature.get_config();
    assert(j.is_object());
    for(auto &i: j.items())
        features[i.key()] = new BAASConfig(i.value(), (BAASLogger*)BAASGlobalLogger);
    return j.size();
}

void BAASFeature::show() {
    for(auto &i: features)
        BAASGlobalLogger->BAASInfo("Feature [ " + i.first + " ]\n" + i.second->get_config().dump(4));
}

bool BAASFeature::appear(BAASConnection *connection,const string& name, const cv::Mat &image, BAASConfig &output, bool show_log) {
    return appear(connection->get_server(), connection->get_language(), name, image, output, show_log);
}

bool BAASFeature::appear(const string &server, const string &language, const string &name, const cv::Mat &image,BAASConfig &output, bool show_log) {
    auto it = features.find(name);
    if(it == features.end()) {
        BAASGlobalLogger->BAASError("Feature [ " + name + " ] not found");
        return false;
    }
    cout << it->second->get_config().dump(4) << endl;
    int type = it->second->getInt("feature_type", 0);

    BAASConfig temp = BAASConfig(it->second->get_config(), (BAASLogger*)BAASGlobalLogger);

    temp.insert("server", server);
    temp.insert("language", language);

    bool ret = compare_functions[type](&temp, image, output);

    if(show_log) {
        auto log = output.template get<vector<string>>("log");
        BAASGlobalLogger->BAASInfo(log);
    }

    return ret;
}





