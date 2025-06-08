//
// Created by pc on 2024/8/7.
//

#include <stdexcept>

#include "feature/BaseFeature.h"
#include "feature/BAASFeature.h"

using namespace std;
using namespace nlohmann;

BAAS_NAMESPACE_BEGIN

BaseFeature::BaseFeature(BAASConfig* config)
{
    and_feature_ptr.clear();
    or_feature_ptr.clear();
    this->config = config;
}

bool BaseFeature::appear(
        const BAAS* baas,
        BAASConfig& output
)
{
    throw std::runtime_error("Base Feature class appear should not be called.");
}

double BaseFeature::self_average_cost(const BAAS* baas)
{
    throw std::runtime_error("Base Feature class self_average_cost should not be called.");
}

double BaseFeature::all_average_cost(const BAAS* baas)
{
    vector<double> all_costs;
    // self cost
    if (config->getInt("feature_type", -1) != -1) all_costs.push_back(self_average_cost(baas));

    // and_feature and or_feature cost
    for (const auto &i: and_feature_ptr)
        all_costs.push_back(i->all_average_cost(baas));
    for (const auto &i: or_feature_ptr)
        all_costs.push_back(i->all_average_cost(baas));

    double sum = 0;

    sort(all_costs.begin(), all_costs.end());
    int size = int(all_costs.size());
    // assume every feature has same appear possibility, calculate average cost
    for (int i = 0; i < all_costs.size(); i++) sum += all_costs[i] * (size--);
    return sum / int(all_costs.size());
}

std::vector<BaseFeature*> BaseFeature::get_and_feature_ptr()
{
    return and_feature_ptr;
}

std::vector<BaseFeature*> BaseFeature::get_or_feature_ptr()
{
    return or_feature_ptr;
}

void BaseFeature::show()
{
    BAASGlobalLogger->sub_title("BaseFeature");
    BAASGlobalLogger->BAASInfo("is_primitive        : [ " + std::to_string(_is_primitive) + " ]");
    BAASGlobalLogger->BAASInfo("and_feature_count   : [ " + std::to_string(and_feature_ptr.size()) + " ]");
    BAASGlobalLogger->BAASInfo("or_feature_count    : [ " + std::to_string(or_feature_ptr.size()) + " ]");
}

BaseFeature::~BaseFeature()
{
    delete config;
}

BAAS_NAMESPACE_END

