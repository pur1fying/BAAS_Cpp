//
// Created by pc on 2024/8/7.
//

#include <stdexcept>
#include "feature/BaseFeature.h"
#include "feature/BAASFeature.h"

using namespace std;
using namespace nlohmann;
BaseFeature::BaseFeature(BAASConfig* config) {
    and_features.clear();
    or_features.clear();
    this->config = config;
    this->and_features = this->config->get<vector<string>>("and_features", {});
    this->or_features = this->config->get<vector<string>>("or_features", {});
    is_enabled = this->config->getBool("enabled", true);
}

double BaseFeature::self_average_cost(const cv::Mat &image, const string& server, const string& language) {
    throw std::runtime_error("Base Feature class average_cost should not be called.");
}

std::vector<std::string> BaseFeature::get_and_features() {
    return and_features;
}

std::vector<std::string> BaseFeature::get_or_features() {
    return or_features;
}

bool BaseFeature::has_and_feature() {
    return and_features.empty();
}

bool BaseFeature::has_or_feature() {
    return or_features.empty();
}

double BaseFeature::all_average_cost(const cv::Mat &image, const std::string& server, const std::string& language) {
    vector<double> all_costs;
    // self cost
    if(config->getInt("feature_type", -1) != -1) all_costs.push_back(self_average_cost(image, server, language));

    for(const auto &i: and_features)    all_costs.push_back(BAASFeature::get_feature(i)->all_average_cost(image, server, language));
    for(const auto &i: or_features)     all_costs.push_back(BAASFeature::get_feature(i)->all_average_cost(image, server, language));

    double sum = 0;

    sort(all_costs.begin(), all_costs.end());
    int size = int(all_costs.size());
    for(int i = 0; i < all_costs.size(); i++) sum += all_costs[i] * (size--);
    return sum / int(all_costs.size());
}

void BaseFeature::get_image(BAASConfig* parameter, BAASImage &image) {
    string server = parameter->getString("server");
    assert(!server.empty());
    string language = parameter->getString("language");
    assert(!language.empty());
    string group = parameter->getString("group");
    assert(!group.empty());
    string name = parameter->getString("name");
    assert(!name.empty());
    if(!resource->is_loaded(server, language, group, name)) {
        image = BAASImage();
        return;
    }
    resource->get(server, language, group, name, image);
}






