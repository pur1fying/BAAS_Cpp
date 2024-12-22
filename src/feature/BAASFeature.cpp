//
// Created by pc on 2024/8/6.
//
#include "feature/BAASFeature.h"

#include "feature/MatchTemplateFeature.h"
#include "feature/FilterRGBMatchTemplateFeature.h"
#include "feature/JudgePointRGBRangeFeature.h"

using namespace std;
using namespace nlohmann;

std::vector<std::function<bool(BAASConfig*, const cv::Mat&, BAASConfig&)>> BAASFeature::compare_functions;

std::map<std::string, BaseFeature*> BAASFeature::features;

BAASFeature *BAASFeature::instance = nullptr;

BAASFeature *baas_features = nullptr;

BAASFeature *BAASFeature::get_instance() {
    if(instance == nullptr) {
        instance = new BAASFeature();
    }
    return instance;
}

void BAASFeature::init_funcs() {
    compare_functions.emplace_back(MatchTemplateFeature::compare);
    compare_functions.emplace_back(FilterRGBMatchTemplateFeature::compare);
    compare_functions.emplace_back(JudgePointRGBRangeFeature::compare);
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
    BAASConfig _feature(path, (BAASLogger *) BAASGlobalLogger);
    json j = _feature.get_config();
    assert(j.is_object());
    BAASConfig *temp;
    int loaded = 0;
    for (auto &i: j.items()) {
        temp = new BAASConfig(i.value(), (BAASLogger *) BAASGlobalLogger);
        auto it = features.find(i.key());
        if (it != features.end()) {
            BAASGlobalLogger->BAASError("Feature [ " + i.key() + " ] already exists");
            delete temp;
            continue;
        }
        int tp = temp->getInt("feature_type", -1);
        BaseFeature *f = nullptr;
        switch (tp) {
            case -1:
                f = new BaseFeature(temp);
                break;
            case BAAS_MATCH_TEMPLATE_FEATURE:
                f = new MatchTemplateFeature(temp);
                break;
            case BAAS_FILTER_RGB_MATCH_TEMPLATE_FEATURE:
                f = new FilterRGBMatchTemplateFeature(temp);
                break;
            case BAAS_JUDGE_POINT_RGB_RANGE_FEATURE:
                f = new JudgePointRGBRangeFeature(temp);
                break;
            default:
                BAASGlobalLogger->BAASError("Feature Type [ " + to_string(tp) + " ] not found");
                break;
        }
        if (f != nullptr) {
            f->set_path(path);
            features[i.key()] = f;
            loaded++;
        }
    }
    return loaded;
}

void BAASFeature::show() {
    for(auto &i: features)
        BAASGlobalLogger->BAASInfo("Feature [ " + i.first + " ]\n" + i.second->get_config()->get_config().dump(4));

}

bool BAASFeature::appear(BAASConnection *connection,const string& name, const cv::Mat &image, BAASConfig &output, bool show_log) {

    return appear(connection->get_server(), connection->get_language(), name, image, output, show_log);
}

bool BAASFeature::appear(const string &server, const string &language, const string &name, const cv::Mat &image,BAASConfig &output, bool show_log) {
    auto it = features.find(name);
    if(it == features.end()) {
        if(show_log) BAASGlobalLogger->BAASError("Feature [ " + name + " ] not found");
        return false;
    }

    if(!it->second->get_enabled()) {
        if(show_log) BAASGlobalLogger->BAASInfo("Feature [ " + name + " ] is disabled");
        return false;
    }

    if(it->second->is_checked_this_round()) {
        if(show_log) BAASGlobalLogger->BAASInfo("Feature [ " + name + " ] already checked, use cached result");
        return it->second->get_this_round_result();
    }

    int type = it->second->get_config()->getInt("feature_type", -1);
    assert(type <= (int(compare_functions.size())-1));

    bool has_self = (type != -1);
    if(type != -1) {
        bool result;
        BAASConfig* temp;
        temp = new BAASConfig(it->second->get_config()->get_config(), (BAASLogger*)BAASGlobalLogger);
        temp->insert("server", server);
        temp->insert("language", language);
        try {
            result = compare_functions[type](temp, image, output);
            delete temp;
            if(show_log) {
                auto log = output.template get<vector<string>>("log", {});
                BAASGlobalLogger->BAASInfo(log);
            }
            if(!result) return it->second->set_checked_this_round(false);
        } catch (json::exception &e) {
            delete temp;
            BAASGlobalLogger->BAASError("Feature [ " + name + " ] compare error : " + e.what());
            throw BAASFeatureError("Feature [ " + name + " ] compare error : " + e.what());
        }
    }

    vector<pair<double, pair<string, bool>>> feature_queue;     // <cost, <name, is_or>>
    vector<string> bundle = it->second->get_or_features();
    if(!bundle.empty())
        for(const auto &i: bundle)
            feature_queue.emplace_back(BAASFeature::get_feature(i)->all_average_cost(image, server, language), make_pair(i, true));

    bundle = it->second->get_and_features();
    bool has_and = !bundle.empty();
    if(!bundle.empty())
        for(const auto &i: bundle)
            feature_queue.emplace_back(BAASFeature::get_feature(i)->all_average_cost(image, server, language), make_pair(i, false));

    sort(feature_queue.begin(), feature_queue.end());   // sort by cost

    for(const auto &i: feature_queue) {
        if(i.second.second) {   // or feature appear
            if(appear(server, language, i.second.first, image, output, show_log)) return it->second->set_checked_this_round(true);
        } else {                // and feature didn't appear
            if(!appear(server, language, i.second.first, image, output, show_log)) return it->second->set_checked_this_round(false);
        }
    }
    //  if has or feature, then or feature didn't appear
    //  if has and feature, then and feature all appear

    if(has_self || has_and) return it->second->set_checked_this_round(true);   // self and and feature all appear

    return it->second->set_checked_this_round(false);           // or feature didn't appear
}

BaseFeature* BAASFeature::get_feature(const string &name){
    auto it = features.find(name);
    if(it == features.end()) throw BAASFeatureError("Feature [ " + name + " ] not found");
    return it->second;
}

void BAASFeature::reset_feature(const std::string& name) {
    auto it = features.find(name);
    if(it == features.end()) return;

    it->second->reset_checked();

    vector<string> bundle = it->second->get_and_features();
    for(const auto &i: bundle)
        reset_feature(i);

    bundle = it->second->get_or_features();
    for(const auto &i: bundle)
        reset_feature(i);

}





