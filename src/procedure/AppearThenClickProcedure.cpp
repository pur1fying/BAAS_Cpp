//
// Created by pc on 2024/8/10.
//


#include "BAAS.h"
#include "procedure/AppearThenClickProcedure.h"

using namespace std;
using namespace nlohmann;

/*
  example:
  {
    "UI-GO-TO_main_page": {
        "procedure_type": 0,        // 0: AppearThenClickProcedure
        "ends": "UI-AT_main_page",  // end feature (string or array)
        "max_stuck_time": 30,       // 30s didn't find any feature throw error
        "max_click_times": 30,      // 30 times click between two features throw error
        "show_log": false,          // show feature compare log
        "possibles": [              // array  list all features to click
               ["UI-AT_restart_login-bonus", 360, 720, 1.0,      5,     1,    5,      0,              0.0,      0.0]
            // [feature_name ,               x,   y  , interval, count, type, offset, click_interval, pre_wait, post_wait]
        ],
        "tentative_click": [true, 640, 360, 10]  // click (640, 360) when 10s didn't find any feature
     }
  }
 *
 * }
 *
 */

BAAS_NAMESPACE_BEGIN

AppearThenClickProcedure::AppearThenClickProcedure(BAASConfig *possible_features) : BaseProcedure(possible_features)
{

}

void AppearThenClickProcedure::implement(
        BAAS *baas,
        BAASConfig &output
)
{
    this->baas = baas;
    this->logger = baas->get_logger();

    json end = possible_feature->get<json>("ends");
    vector<string> end_feature_names;

    bool show_log = baas->script_show_image_compare_log;

    if (end.is_array() or end.is_object()) { for (auto &i: end)if (i.is_string())end_feature_names.push_back(i); }
    else if (end.is_string()) { end_feature_names.push_back(end); }

    json possible = possible_feature->get<json>("possibles", json::array());
    if (possible.is_array())
        for (auto &i: possible)
            if (i.is_array() && i.size() >= 3) {
                possibles.push_back(new BAASConfig(i, logger));
                possibles_feature_names.push_back(possibles[possibles.size() - 1]->get<string>("/0"));
            }

    max_stuck_time = possible_feature->getInt("max_stuck_time", 20);
    max_click = possible_feature->getInt("max_click_times", 20);
    max_execute_time = possible_feature->getLLong("max_execute_time", LLONG_MAX);
    enable_tentative_click = possible_feature->getBool("/tentative_click/0", false);
    if (enable_tentative_click) {
        tentative_click_x = possible_feature->getInt("/tentative_click/1", 640);
        tentative_click_y = possible_feature->getInt("/tentative_click/2", 360);
        tentative_click_stuck_time = possible_feature->getLLong("/tentative_click/3", 10);
    }

    bool skip_first_screenshot = possible_feature->getBool("skip_first_screenshot", false);

    output.clear();

    start_time = BAASUtil::getCurrentTimeStamp();
    last_appeared_time = start_time;
    last_tentative_click_time = start_time;

    while (baas->is_running()) {
        this_round_start_time = BAASUtil::getCurrentTimeStamp();
        if (this_round_start_time - start_time >= max_execute_time) {
            logger->hr("Max execute time " + to_string(max_execute_time) + "s reached.");
            logger->BAASError("Looking for End features : ");
            logger->BAASError(end_feature_names);
            logger->BAASError("Looking for Possible features : ");
            logger->BAASError(possibles_feature_names);
            throw GameStuckError("Max execute time reached.");
        }

        if (this_round_start_time - last_appeared_time >= max_stuck_time) {
            logger->hr(to_string(max_stuck_time) + "s didn't find any feature, assume game stuck.");
            logger->BAASError("Looking for End features : ");
            logger->BAASError(end_feature_names);
            logger->BAASError("Looking for Possible features : ");
            logger->BAASError(possibles_feature_names);
            throw GameStuckError("Game stuck.");
        }

        if (enable_tentative_click && this_round_start_time - last_tentative_click_time >= tentative_click_stuck_time) {
            baas->click(tentative_click_x, tentative_click_y, 1, 1, 5, 0.0, 0.0, 0.0, "Tentative Click");
            last_tentative_click_time = BAASUtil::getCurrentTimeStamp();
        }

        if (!skip_first_screenshot)wait_loading();
        else skip_first_screenshot = false;

        for (auto &i: end_feature_names) baas->reset_feature(i);
        for (auto &i: possibles_feature_names) baas->reset_feature(i);


        for (int i = 0; i < end_feature_names.size(); ++i) {
            current_comparing_feature_name = end_feature_names[i];
            if (baas->feature_appear(current_comparing_feature_name, temp_output, show_log)) {
                logger->BAASInfo("End [ " + current_comparing_feature_name + " ]. ");
                output.insert("end", current_comparing_feature_name);
                return;
            }
        }

        for (int i = 0; i < possibles.size(); i++) {
            current_comparing_feature_name = possibles_feature_names[i];
            if (baas->feature_appear(current_comparing_feature_name, temp_output, show_log)) {
                logger->BAASInfo("Feature [ " + possibles_feature_names[i] + " ] appeared. ");
                last_appeared_feature_name = possibles_feature_names[i];
                last_appeared_time = BAASUtil::getCurrentTimeStamp();
                solve_feature_action_click(possibles[i]);
                last_tentative_click_time = last_appeared_time;
                break;
            }
        }
    }
}


void AppearThenClickProcedure::wait_loading()
{
    long long t_loading;
    baas->update_screenshot_array();
    long long start = BAASUtil::getCurrentTimeMS();
    string zero, ld;
    while (baas->is_running()) {
        if (BAASFeature::reset_then_feature_appear(baas, "common_loading_appear")) {
            t_loading = BAASUtil::getCurrentTimeMS() - start;
            ld = to_string(t_loading);
            zero = string(6 - ld.length(), ' ');
            if (ld.length() <= 6)
                zero += ld;
            logger->BAASInfo("Loading :" + zero + "ms");
            if (t_loading >= 20000 && baas->get_screenshot()
                                          ->get_interval() < 1) {
                logger->BAASInfo("Loading too long, add screenshot interval to 1s.");
                baas->get_screenshot()
                    ->set_interval(1);
            }
            baas->update_screenshot_array();
        } else break;
    }
}

void AppearThenClickProcedure::clear_possibles()
{
    for (auto &i: possibles) delete i;
    possibles.clear();
    possibles_feature_names.clear();
}

void AppearThenClickProcedure::solve_feature_appear(
        BAASConfig *feature,
        bool show_log
)
{

}

void AppearThenClickProcedure::solve_feature_action_click(BAASConfig *parameters)
{

    int interval = int(parameters->getDouble("/3", 1.0) * 1000);
    if (last_clicked_feature_name == last_appeared_feature_name &&
        (int(BAASUtil::getCurrentTimeMS() - last_clicked_time) < interval))
        return;

    int x = parameters->getInt("/1");
    int y = parameters->getInt("/2");
    int count = parameters->getInt("/4", 1);
    uint8_t type = parameters->getInt("/5", 1);
    int offset = parameters->getInt("/6", 5);
    double click_interval = parameters->getDouble("/7", 0.0);
    double pre_wait = parameters->getDouble("/8", 0.0);
    double post_wait = parameters->getDouble("/9", 0.0);

    baas->click(x, y, count, type, offset, click_interval, pre_wait, post_wait, current_comparing_feature_name);

    insert_last_clicked_queue(last_appeared_feature_name);  // this clicked
    pop_last_clicked_queue(max_click);                      // pop last clicked and check max click

    last_clicked_time = BAASUtil::getCurrentTimeMS();
    last_clicked_feature_name = last_appeared_feature_name;
}


void AppearThenClickProcedure::pop_last_clicked_queue(int size)
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

void AppearThenClickProcedure::insert_last_clicked_queue(string &feature_name)
{
    last_clicked.push(feature_name);
    last_clicked_counter[feature_name]++;
    if (last_clicked_pair_counter.first.first.empty() && last_clicked_pair_counter.second.first.empty()) { // both are empty
        last_clicked_pair_counter.first.first = feature_name;
        last_clicked_pair_counter.first.second = 1;
    } else if (!last_clicked_pair_counter.first.first.empty() && last_clicked_pair_counter.second.first.empty()) { // first is not empty, second is empty
        if (last_clicked_pair_counter.first.first != feature_name) {
            last_clicked_pair_counter.second.first = feature_name;
            last_clicked_pair_counter.second.second = 1;
        }
    } else if (last_clicked_pair_counter.first.first != feature_name
                && last_clicked_pair_counter.second.first !=feature_name) {  // 3rd feature appear
        last_clicked_pair_counter.first.first.clear();
        last_clicked_pair_counter.second.first.clear();
    } else if (last_clicked_pair_counter.first.first == feature_name
                || last_clicked_pair_counter.second.first ==feature_name) { // equal to first or second
        if (last_clicked_pair_counter.first.first == feature_name) {
            last_clicked_pair_counter.first.second++;
        } else if (last_clicked_pair_counter.second.first == feature_name) {
            last_clicked_pair_counter.second.second++;
        }
    }

    if (last_clicked_pair_counter.first.second + last_clicked_pair_counter.second.second >= max_click) {
        logger->BAASInfo(
                to_string(max_click) + " Clicks Between : " +
                last_clicked_pair_counter.first.first + " and " +
                last_clicked_pair_counter.second.first
        );
        throw TooManyClicksBetweenTwoClicksError("Too Many clicks between two features.");
    }
}

void AppearThenClickProcedure::clear_resource()
{
    clear_possibles();
    pop_last_clicked_queue(0);

    last_appeared_feature_name.clear();
    last_clicked_feature_name.clear();

    last_appeared_time = 0;
    last_clicked_time = 0;

    last_clicked_pair_counter.first.first.clear();
    last_clicked_pair_counter.second.first.clear();

    current_comparing_feature_name.clear();

    temp_output.clear();
}

BAAS_NAMESPACE_END

