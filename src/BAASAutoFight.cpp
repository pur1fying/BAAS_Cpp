////
//// Created by pc on 2024/5/31.
////
#include "BAASAutoFight.h"
#include "BAASImageResource.h"
using namespace std;
using namespace cv;
BAASRectangle const BAASAutoFight::threeStudentSkillRegion = {847, 594, 1130, 670};
BAASPoint const BAASAutoFight::threeStudentSkillPositionCenter[3] = {
        {883,  621},
        {985,  621},
        {1087, 621}
};
BAASRectangle const BAASAutoFight::skillSearchRegion[3] = {
        {827, 577, 931, 674},
        {931, 577, 1040, 674},
        {1040, 577, 1143, 674}
};

BAASRectangle const BAASAutoFight::costRegion = {820, 681, 1141, 696};

double const BAASAutoFight::costLineDeltaX = 31.7;

cv::Vec3b const BAASAutoFight::costPixelMin = {240, 240, 240};
cv::Vec3b const BAASAutoFight::costPixelMax = {255, 255, 255};

BAASAutoFight::BAASAutoFight(BAAS *baas)
{
    this->baas = baas;
    this->logger = baas->get_logger();
}

void BAASAutoFight::refreshSkillPosition() {
    currentSkill.clear();
    currentSkill.resize(skillSlotLength);
}

BAASAutoFight::CharacterSkill::CharacterSkill() = default;


void BAASAutoFight::searchAllSkillPosition() {
    for(int i = 0; i < currentSkill.size(); i++) {
        if(skillFull())
            break;
        if (currentSkill[i].has_value())
            continue;
        for(auto &key: skillNames){
            BAASPoint p;
            for(int j = 0; j <= 2; ++j) {
                p = BAASImageUtil::imageSearch(baas->get_latest_screenshot(), skills[key].skill_templates[j], skillSearchRegion[i]);
                if (p.x != -1) {
                    currentSkill[i] = key;
                    break;
                }
            }
            if(currentSkill[i].has_value())
                break;
        }
    }

}


void BAASAutoFight::showSkillPosition() {
    for(int i = 0; i < 3; i++) {
        if(!currentSkill[i].has_value()){
            BAASGlobalLogger->BAASInfo("Position : " + to_string(i) + " [ Empty ]");
            continue;
        }
        BAASGlobalLogger->BAASInfo("Position : " + to_string(i) + " [ " + currentSkill[i].value() + " ]");
    }
}

void BAASAutoFight::updateCost() {
    currentCost = 0;
    for(int i = costRegion.lr.x; i >= costRegion.ul.x; i--){
        for(int j = costRegion.ul.y; j <= costRegion.lr.y; j++){
            if(BAASImageUtil::judge_rgb_range(baas->get_latest_screenshot(), {i, j}, costPixelMin, costPixelMax)){
                int x = i - (costRegion.ul.x + (costRegion.lr.y - j) / 5);
                int integer = int(double(x) *1.0 / costLineDeltaX);
                double decimal = double((x - integer* 32 )) * 1.0 / 28;
                if(decimal < 0)
                    decimal = 0;
                currentCost = integer + decimal;
                currentCost = currentCost > 10.0 ? 10.0 : currentCost;
                return;
            }
        }
    }
}

void BAASAutoFight::showCost() const{
    BAASGlobalLogger->BAASInfo("Cost : " + to_string(currentCost));
}


bool BAASAutoFight::skillFull() const {
    for(auto &skill: currentSkill){
        if(!skill.has_value())
            return false;
    }
    return true;
}


//bool BAASAutoFight::releaseSkill(const string &skillName, const BAASPoint &position) {
//    for(int i = 0; i < 3; i++){
//        if(currentSkill[i] == skillName){
//            BAASGlobalLogger->BAASInfo("SKILL SLOT : " + to_string(i) );
//            click(threeStudentSkillPositionCenter[i]);
//            click(position);
//            currentSkill[i] = nextSkillQueue.front();
//            nextSkillQueue.pop();
//            nextSkillQueue.push(skillName);
//            return true;
//        }
//    }
//    return false;
//}
//
//
//
bool BAASAutoFight::startLoop() {
    logger->BAASInfo("Start loop");
    refreshSkillPosition();
    get_all_skill_name();
    waitCost(3.1);
    fight_release_uiskill2mika();
    get_all_skill_name();
    if (has_skill("Akane")) {
        logger->BAASWarn("Akane skill found after ui. Quit");
        return false;
    }
    std::string temp_name = "Ako";
    if (!has_skill(temp_name)) {
        temp_name = "Himari";
    }
    waitCost(9.6);
    release_skill(temp_name, {455, 361}, true, true, 3, true, 2000);
    get_all_skill_name();
    waitTime(150.0);
    if(has_skill("Akane")){
        release_skill("Mika", {1145, 282}, true, true, 3, true, 2000);
        flip_auto(2, 1.0);
    }
    else {
        flip_auto(4, 0.3);
    }
    refreshSkillPosition();
    get_all_skill_name();
    waitCost(3.1);
    optional<int> health;
    baas->ocr_get_boss_health(health, 7e6);
    if(!health.has_value() || health.value() > 5e6) {
        logger->BAASInfo("Boss health too high. Quit");
        return false;
    }
    release_skill("Ui", {455, 361}, true, true, 3, true, 2000);
    get_all_skill_name();
    waitCost(8.9);
    release_skill("Mika", {1145, 282}, true, true, 3, true, 2000);
    flip_auto(2, 0.5);
    waitCost(1.9);
    flip_auto(2, 1.0);
    waitCost(2.7);
    flip_auto(2, 1.0);
    health.reset();
    waitCost(3.1);
    baas->ocr_get_boss_health(health, 7e6);
    if(!health.has_value())
        logger->BAASInfo("Boss health too high. Quit");
    return true;
}

bool BAASAutoFight::fight_release_uiskill2mika()
{
    release_skill("Ui", {455, 361}, true, true, 3, true, 2000);
    return true;
}

//bool BAASAutoFight::inCurrentSkill(const string &skillName) const {
//    for(auto &skill: currentSkill){
//        if(skill == skillName)
//            return true;
//    }
//    return false;
//}
//
void BAASAutoFight::waitCost(double cost) {
    baas->update_screenshot_array();
    updateCost();
    while(currentCost < cost){
        baas->update_screenshot_array();
        updateCost();
        showCost();
    }
}

void BAASAutoFight::enter_fight()
{
    baas->solve_procedure("UI-GO-TO_total_assault_fight-page", true);
}

void BAASAutoFight::setSkills(const vector<std::string> &skillNames)
{
    this->skillNames = skillNames;
    std::string server = baas->get_connection()->get_server();
    std::string language = baas->get_connection()->get_language();
    for (auto &key: skillNames) {
        CharacterSkill skill;
        skill.name = key;
        resource->get(server, language, "skill_icon_bright", key, skill.skill_templates[0]);
        resource->get(server, language, "skill_icon_left_grey", key, skill.skill_templates[1]);
        resource->get(server, language, "skill_icon_right_grey", key, skill.skill_templates[2]);
        skills[key] = skill;
    }
}

void BAASAutoFight::restart_fight()
{
    baas->solve_procedure("UI-GO-TO_total_assault_fight-pause", true);
    baas->solve_procedure("restart_total_assault", true);
}

void BAASAutoFight::ocr_update_fight_left_time()
{
    TextLine ret;
    baas->ocr_for_single_line("zh-cn", ret, {1094, 23, 1199, 50}, "fight_left_time", "0123456789");
    if (ret.text.length() != 7) {
        return;
    }
    int minute = (ret.text[0] - '0') * 10 + (ret.text[1] - '0');
    int second = (ret.text[2] - '0') * 10 + (ret.text[3] - '0');
    int ms = (ret.text[4] - '0') * 100 + (ret.text[5] - '0') * 10 + (ret.text[6] - '0');
    fight_left_time = minute * 60 + second + ms * 0.001;
    logger->BAASInfo("Fight left time: " + to_string(fight_left_time));
}

bool BAASAutoFight::release_skill(
        const std::string& name,
        BAASPoint p,
        bool ensure_click,
        bool check_cost,
        int cost_decrease,
        bool check_skill_icon_change,
        int time_out
)
{
    int skill_pos = get_skill_pos(name);
    if (skill_pos == -1) {
        logger->BAASInfo("Skill " + name + " not in slots.");
        return false;
    }
    if (!ensure_click) {
        baas->click(threeStudentSkillPositionCenter[skill_pos]);
        baas->click(p);
    }
    else {
        std::string procedure_name = "UI-GO-TO_common_fight-skill-" + to_string(skill_pos + 1) + "-chosen";
        baas->solve_procedure(procedure_name, true);
        baas->click_until_disappear("common_fight-skill-" + to_string(skill_pos + 1) + "-chosen_appear", p, true, time_out);
    }
    currentSkill[skill_pos].reset();
    return true;
}

int BAASAutoFight::get_skill_pos(const string &name)
{
    for (int i = 0; i < 3; i++) {
        if (currentSkill[i] == name) {
            return i;
        }
    }
    return -1;
}

void BAASAutoFight::get_all_skill_name(int timeout)
{
    auto start = BAASUtil::getCurrentTimeMS();
    while (BAASUtil::getCurrentTimeMS() - start < timeout) {
        baas->update_screenshot_array();
        searchAllSkillPosition();
        showSkillPosition();
        if (skillFull()) {
            break;
        }
    }
}

int BAASAutoFight::has_skill(const string &name)
{
    for (int i = 0; i < 3; i++) {
        if (currentSkill[i] == name) {
            return i + 1;
        }
    }
    return 0;
}

void BAASAutoFight::waitTime(double time)
{
    ocr_update_fight_left_time();
    auto start = fight_left_time;
    while (fight_left_time > time) {
        baas->update_screenshot_array();
        ocr_update_fight_left_time();
    }
}

void BAASAutoFight::flip_auto(
        int times,
        double interval
)
{
    baas->click({1210, 675}, times, 1, 5, interval);
}

void BAASAutoFight::reset_all()
{
    currentCost = 0.0;
    currentSkill.clear();
    currentSkill.resize(skillSlotLength);
}








