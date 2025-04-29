//
// Created by pc on 2024/8/6.


#include "feature/BAASFeature.h"
#include "feature/MatchTemplateFeature.h"
#include "feature/FilterRGBMatchTemplateFeature.h"
#include "feature/JudgePointRGBRangeFeature.h"
#include "BAASGlobals.h"
#include "BAAS.h"

using namespace std;
using namespace nlohmann;

BAAS_NAMESPACE_BEGIN

/*
 * Different from feature_state_map in class BAAS, feature is static, global.
 * feature_state_map should be a member (non-static). Because feature state is defined at run time, feature is defined
 * before program run.
 */
std::map<std::string, std::unique_ptr<BaseFeature>> BAASFeature::features;

BAASFeature *BAASFeature::instance = nullptr;

BAASFeature *baas_features = nullptr;

BAASFeature *BAASFeature::get_instance()
{
    if (instance == nullptr) {
        instance = new BAASFeature();
    }
    return instance;
}

void BAASFeature::load()
{
    if (!filesystem::exists(BAAS_FEATURE_DIR)) {
        BAASGlobalLogger->BAASError("Feature Dir :");
        BAASGlobalLogger->Path(BAAS_FEATURE_DIR);
        BAASGlobalLogger->BAASError("Not Exists");
        return;
    }
    // load from image_info folder
    // data stored in json and load
    string temp_path;
    int total_loaded = 0;
    for (const auto &entry: filesystem::recursive_directory_iterator(BAAS_FEATURE_DIR)) {
        temp_path = entry.path()
                         .string();
        if (filesystem::is_regular_file(entry) && temp_path.ends_with(".json"))
            total_loaded += load_from_json(temp_path);
    }
    BAASGlobalLogger->BAASInfo("Totally loaded [ " + to_string(total_loaded) + " ] features");
}

BAASFeature::BAASFeature()
{
    load();
    init_feature_ptr();
}

int BAASFeature::load_from_json(const string &path)
{
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
        BaseFeature* f = nullptr;
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
            features[i.key()] = std::unique_ptr<BaseFeature>(f);
            loaded++;
        }
    }
    return loaded;
}

void BAASFeature::show()
{
    BAASGlobalLogger->hr("Show All Feature Param.");
    for (auto &i: features) {
        BAASGlobalLogger->BAASInfo("[ " + i.first + " ]");
        i.second->show();
    }
}


BaseFeature *BAASFeature::get_feature(const string &name)
{
    auto it = features.find(name);
    if (it == features.end()) throw BAASFeatureError("Feature [ " + name + " ] not found");
    return it->second.get();
}

bool BAASFeature::reset_then_feature_appear(
        BAAS* baas,
        const string &feature_name
)
{
    baas->reset_feature(feature_name);
    BAASConfig temp;
    return feature_appear(baas, feature_name, temp, false);
}

bool BAASFeature::feature_appear(
        BAAS *baas,
        const std::string &feature_name,
        BAASConfig &output,
        bool show_log
)
{
    /*
     *
     * self true
     * self false
     * self true and
     * self false and
     * self true or
     * self false or
     * self true and or
     * self false and or
     *
    */
    auto feature = features.find(feature_name);

    auto feature_state = baas->feature_state_map.find(feature_name);

    if (feature == features.end()) {
        if (show_log) BAASGlobalLogger->BAASError("Feature [ " + feature_name + " ] not found");
        return false;
    }

    if (feature_state->second.round_feature_appear_state.has_value()) {
        if (show_log) BAASGlobalLogger->BAASInfo("Feature [ " + feature_name + " ] already checked, use cached result");
        return feature_state->second.round_feature_appear_state.value();
    }

    // check self
    int type = feature->second->get_config()->getInt("feature_type", -1);
    bool has_self = (type != -1);
    if (type != -1) {
        bool result;
        std::unique_ptr<BAASConfig> temp = std::make_unique<BAASConfig>(
                feature->second->get_config()->get_config(),
                (BAASLogger *) BAASGlobalLogger
        );
        try {
            result = feature->second->appear(baas, output);
            feature_state->second.round_feature_appear_state = result;
            if (show_log) {
                auto log = output.template get<vector<string>>("log", {});
                BAASGlobalLogger->BAASInfo(log);
            }
            if (feature->second->is_primitive()
            or ( result and feature->second->has_or_feature()  and !feature->second->has_and_feature())
            or (!result and feature->second->has_and_feature() and !feature->second->has_or_feature()))
                return result;

        } catch (json::exception &e) {
            BAASGlobalLogger->BAASError("Feature [ " + feature_name + " ] compare error : " + e.what());
            throw BAASFeatureError("Feature [ " + feature_name + " ] compare error : " + e.what());
        }
    }

    // check and/or features
    vector<pair<double, pair<BaseFeature*, bool>>> feature_queue;     // <cost, <name, is_or>>
    vector<BaseFeature*> bundle = feature->second->or_feature_ptr;

    if (!bundle.empty())
        for (const auto &i: bundle)
            feature_queue.emplace_back(i->all_average_cost(baas), make_pair(i, true));

    bundle = feature->second->and_feature_ptr;
    bool has_and = !bundle.empty();
    if (!bundle.empty())
        for (const auto &i: bundle)
            feature_queue.emplace_back(i->all_average_cost(baas), make_pair(i, false));

    sort(feature_queue.begin(), feature_queue.end());   // sort by cost

    for (const auto &i: feature_queue) {
        if (i.second.second) {   // or feature appear
            if (i.second.first->appear(baas,output)) {
                feature_state->second.round_feature_appear_state = true;
                return true;
            }
        } else {                // and feature didn't appear
            if (!i.second.first->appear(baas,output)) {
                feature_state->second.round_feature_appear_state = false;
                return false;
            }
        }
    }
    //  if has or feature, then or feature didn't appear
    //  if has and feature, then and feature all appear

    if (has_self || has_and) {
        feature_state->second.round_feature_appear_state = true;   // self and and feature all appear
        return true;
    }

    feature_state->second.round_feature_appear_state = false;       // or feature didn't appear
    return false;
}

std::vector<std::string> BAASFeature::get_feature_list()
{
    std::vector<std::string> feature_list;
    for (auto &i: features) {
        feature_list.push_back(i.first);
    }
    return feature_list;
}

void BAASFeature::init_feature_ptr()
{
    BAASGlobalLogger->sub_title("Init Feature Pointer.");
    std::vector<std::string> feature_list;
    for (auto &i: features) {
        feature_list = i.second->get_and_features();
        for (const auto &j: feature_list) {
            if (features.find(j) == features.end()) {
                BAASGlobalLogger->BAASError("[ " + i.first + " ] And Feature [ " + j + " ] not found");
                continue;
            }
            i.second->and_feature_ptr.push_back(features[j].get());
        }

        feature_list = i.second->get_or_features();
        for (const auto &j: feature_list) {
            if (features.find(j) == features.end()) {
                BAASGlobalLogger->BAASError("[ " + i.first + " ] Or Feature [ " + j + " ] not found");
                continue;
            }
            i.second->or_feature_ptr.push_back(features[j].get());
        }

        i.second->_is_primitive = i.second->and_feature_ptr.empty()
                                    && i.second->or_feature_ptr.empty();
    }
}

BAAS_NAMESPACE_END

