//
// Created by pc on 2024/8/10.
//
#if defined(BAAS_APP_BUILD_FEATURE) && defined(BAAS_APP_BUILD_PROCEDURE)

#include "procedure/AppearThenDoProcedure.h"
#include "BAAS.h"

using namespace std;
using namespace nlohmann;

BAAS_NAMESPACE_BEGIN

AppearThenDoProcedure::AppearThenDoProcedure(BAASConfig *possible_feature) : BaseProcedure(possible_feature)
{

}

void AppearThenDoProcedure::implement(
        BAAS *baas,
        BAASConfig &output
)
{
    this->baas = baas;
    logger = baas->logger;

    json end = possible_feature->get<json>("end");
    vector<string> end_feature_names;

    bool show_log = possible_feature->getBool("show_log", false);

    if (end.is_array() or end.is_object()) { for (auto &i: end)if (i.is_string())end_feature_names.push_back(i); }
    else if (end.is_string()) { end_feature_names.push_back(end); }

    clear_possibles();
    json possible = possible_feature->get<json>("possible_features");
    if (possible.is_array())
        for (auto &i: possible) {
            possibles.push_back(new BAASConfig(i, logger));
            possibles_feature_names.push_back(possibles[possibles.size() - 1]->getString("name"));
        }

    max_stuck_time = possible_feature->getInt("max_stuck_time", 20);
    max_click = possible_feature->getInt("max_click_times", 20);
    output.clear();
    pop_last_clicked_queue(0);

    last_appeared_feature_name.clear();
    last_clicked_feature_name.clear();

    last_appeared_time = 0;
    last_clicked_time = 0;

    last_clicked_pair_counter.first
                             .first
                             .clear();
    last_clicked_pair_counter.second
                             .first
                             .clear();

    current_comparing_feature_name.clear();

    temp_output.clear();

    start_time = BAASUtil::getCurrentTimeStamp();
    while (baas->is_running()) {
        if (last_appeared_time - BAASUtil::getCurrentTimeStamp() >= max_stuck_time) {
            logger->hr(to_string(max_stuck_time) + "s didn't find any feature, assume game stuck.");
            logger->BAASError("Looking for End features : ");
            logger->BAASError(end_feature_names);
            logger->BAASError("Possible features : ");
            logger->BAASError(possibles_feature_names);
            throw GameStuckError("Game stuck.");
        }
        for (auto &i: end_feature_names) BAASFeature::reset_feature(i);
        for (auto &i: possibles_feature_names) BAASFeature::reset_feature(i);

        wait_loading();

        for (int i = 0; i < end_feature_names.size(); ++i) {
            current_comparing_feature_name = end_feature_names[i];
            if (BAASFeature::appear(
                    baas->connection, current_comparing_feature_name, baas->latest_screenshot, temp_output, show_log
            )) {
                logger->BAASInfo("End [ " + current_comparing_feature_name + " ]. ");
                output.insert("end", current_comparing_feature_name);
                return;
            }
        }

        for (int i = 0; i < possibles.size(); i++) {
            current_comparing_feature_name = possibles_feature_names[i];
            if (BAASFeature::appear(
                    baas->connection, possibles_feature_names[i], baas->latest_screenshot, temp_output, show_log
            )) {
                logger->BAASInfo("Feature [ " + possibles_feature_names[i] + " ] appeared. ");
                last_appeared_feature_name = possibles_feature_names[i];
                last_appeared_time = BAASUtil::getCurrentTimeStamp();
                solve_feature_appear(possibles[i], show_log);
            }
        }
    }

}

void AppearThenDoProcedure::wait_loading()
{
    baas->update_screenshot_array();
}

void AppearThenDoProcedure::clear_possibles()
{
    for (auto &i: possibles) delete i;
    possibles.clear();
    possibles_feature_names.clear();
}

void AppearThenDoProcedure::solve_feature_appear(
        BAASConfig *feature,
        bool show_log
)
{
    int tp = feature->getInt("/action/type", 0);

    switch (tp) {
        case BAAS_ACTION_TYPE_DO_NOTHING:
            return;
        case BAAS_ACTION_TYPE_CLICK:
            solve_feature_action_click(feature);
            return;
        case BAAS_ACTION_TYPE_SWIPE:
            solve_feature_action_swipe(feature);
            return;
        case BAAS_ACTION_TYPE_LONG_CLICK:
            solve_feature_action_long_click(feature);
            return;
        default:
            return;
    }
}

void AppearThenDoProcedure::solve_feature_action_click(BAASConfig *feature)
{
    int interval = int(feature->getDouble("/action/click_interval", 1.0) * 1000);
    if (last_clicked_feature_name == last_appeared_feature_name &&
        (int(BAASUtil::getCurrentTimeMS() - last_clicked_time) < interval))
        return;

    auto point = feature->get<pair<int, int>>("/action/position");
    int count = feature->getInt("/action/count", 1);
    uint8_t type = feature->getInt("/action/type", 1);
    int size = feature->getInt("/action/size", 5);
    double pre_wait = feature->getDouble("/action/pre_wait", 0.0);
    double post_wait = feature->getDouble("/action/post_wait", 0.0);
    baas->click(
            point.first, point.second, count, type, size, interval, pre_wait, post_wait, feature->getString("name"));
    insert_last_clicked_queue(last_appeared_feature_name);

    last_clicked_time = BAASUtil::getCurrentTimeMS();
    last_clicked_feature_name = last_appeared_feature_name;
}

void AppearThenDoProcedure::solve_feature_action_long_click(BAASConfig *feature)
{

}

void AppearThenDoProcedure::solve_feature_action_swipe(BAASConfig *feature)
{

}

void AppearThenDoProcedure::pop_last_clicked_queue(int size)
{
    assert(size >= 0);
    int q_size = int(last_clicked.size());
    while (q_size > size) {
        last_clicked_counter[last_clicked.front()]--;
        if (last_clicked_counter[last_clicked.front()] == 0) last_clicked_counter.erase(last_clicked.front());
        last_clicked.pop();
        q_size = int(last_clicked.size());
    }
}

void AppearThenDoProcedure::insert_last_clicked_queue(string &feature_name)
{
    last_clicked.push(feature_name);
    last_clicked_counter[feature_name]++;
    if (last_clicked_pair_counter.first
                                 .first
                                 .empty() && last_clicked_pair_counter.second
                                                                      .first
                                                                      .empty()) { // both are empty
        last_clicked_pair_counter.first
                                 .first = feature_name;
        last_clicked_pair_counter.first
                                 .second = 1;
    } else if (!last_clicked_pair_counter.first
                                         .first
                                         .empty() && last_clicked_pair_counter.second
                                                                              .first
                                                                              .empty()) { // first is not empty, second is empty
        if (last_clicked_pair_counter.first
                                     .first != feature_name) {
            last_clicked_pair_counter.second
                                     .first = feature_name;
            last_clicked_pair_counter.second
                                     .second = 1;
        }
    } else if (last_clicked_pair_counter.first
                                        .first != feature_name && last_clicked_pair_counter.second
                                                                                           .first !=
                                                                  feature_name) {  // 3rd feature appear
        last_clicked_pair_counter.first
                                 .first
                                 .clear();
        last_clicked_pair_counter.second
                                 .first
                                 .clear();
    } else if (last_clicked_pair_counter.first
                                        .first == feature_name || last_clicked_pair_counter.second
                                                                                           .first ==
                                                                  feature_name) { // equal to first or second
        if (last_clicked_pair_counter.first
                                     .first == feature_name) {
            last_clicked_pair_counter.first
                                     .second++;
        } else if (last_clicked_pair_counter.second
                                            .first == feature_name) {
            last_clicked_pair_counter.second
                                     .second++;
        }
    }

    if (last_clicked_pair_counter.first
                                 .second + last_clicked_pair_counter.second
                                                                    .second >= max_click) {
        logger->BAASInfo(
                to_string(max_click) + " Clicks Between : " + last_clicked_pair_counter.first
                                                                                       .first + " and " +
                last_clicked_pair_counter.second
                                         .first
        );
        throw TooManyClicksBetweenTwoClicksError("Too Many clicks between two features.");
    }
}

void AppearThenDoProcedure::clear_resource()
{
    clear_possibles();
}

BAAS_NAMESPACE_END

#endif