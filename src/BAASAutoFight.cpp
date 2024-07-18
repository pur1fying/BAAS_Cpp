//
// Created by pc on 2024/5/31.
//
#include "BAASAutoFight.h"

using namespace std;
using namespace cv;
BAASRectangle const BAASAutoFight::threeStudentSkillRegion = {847, 594, 1130, 670};
BAASPoint const BAASAutoFight::threeStudentSkillPositionCenter[3] = {
        {883,  621},
        {985,  621},
        {1087, 621}
};

BAASRectangle const BAASAutoFight::costRegion = {820, 681, 1141, 696};

double const BAASAutoFight::costLineDeltaX = 31.7;
BAASAutoFight::BAASAutoFight() {
    string serial = "127.0.0.1:16384";
    nemu = new BAASNemu(MuMuInstallPath, serial);         // temporarily use menu method
    currentSkill.resize(3);
}

void BAASAutoFight::refreshSkillPosition() {
    currentSkill.clear();
    currentSkill.resize(3);
}

BAASAutoFight::CharacterSkill::CharacterSkill() = default;

void BAASAutoFight::setFormation(const std::vector<std::string>& studentNames) {
    this->formation = studentNames;
}

void BAASAutoFight::setSkills(const std::vector<std::string>& skillNames) {
    string temp;
    for(auto &name: skillNames){
        CharacterSkill skill;
        skill.name = name;
        temp = name + "_bright";
        this->resource->getResource(temp, skill.iconBright);
        temp = name + "_left";
        this->resource->getResource(temp, skill.iconLeftBlack);
        temp = name + "_right";
        this->resource->getResource(temp, skill.iconRightGrey);
        this->skills[name] = skill;
    }
    this->skillNames = skillNames;
}

void BAASAutoFight::setImageResource(BAASImageResource *imageResource) {
    this->resource = imageResource;
}

void BAASAutoFight::searchAllSkillPosition() {
    if(skillFull()) {
        return;
    }
    BAASPoint p;
    for(auto &key: skillNames){
        if(skillFull())
            break;
        p = BAASImageUtil::imageSearch(lastScreenshot, skills[key].iconLeftBlack.image, threeStudentSkillRegion, 0.8, false);
        if(p.x != -1){
            currentSkill[pointToSkillIndex(p)] = key;
            continue;
        }
        p = BAASImageUtil::imageSearch(lastScreenshot, skills[key].iconRightGrey.image, threeStudentSkillRegion, 0.8, false);
        if(p.x != -1){
            currentSkill[pointToSkillIndex(p)] = key;
            continue;
        }
        Mat img = skills[key].iconBright.image;
        Mat left =  BAASImageUtil::imageCrop(img, 0, 0, img.cols/2-3, img.rows);
        Mat right = BAASImageUtil::imageCrop(img, img.cols/2+3, 0, img.cols, img.rows);
        BAASImageUtil::imageSearch(lastScreenshot, left, threeStudentSkillRegion, 0.8, true);
        if(p.x != -1){
            currentSkill[pointToSkillIndex(p)] = key;
            continue;
        }
        p = BAASImageUtil::imageSearch(lastScreenshot, right, threeStudentSkillRegion, 0.8, true);
        if(p.x != -1){
            currentSkill[pointToSkillIndex(p)] = key;
            continue;
        }
    }
}

int BAASAutoFight::pointToSkillIndex(const BAASPoint& p){
    int minLoc = INT_MAX;
    int minIndex = -1;
    int temp;
    for(int i = 0; i < 3; i++){
        temp = BAASImageUtil::pointDistance(p, threeStudentSkillPositionCenter[i]);
        if(temp < minLoc){
            minLoc = temp;
            minIndex = i;
        }
    }
    return minIndex;
}

void BAASAutoFight::showSkillPosition() {
//    imshow("Skill Position", lastScreenshot);
    for(int i = 0; i < 3; i++) {
        if(currentSkill[i].empty()){
            BAASGlobalLogger->BAASInfo("Position : " + to_string(i) + " [ Empty ]");
            continue;
        }
        BAASGlobalLogger->BAASInfo("Position : " + to_string(i) + " [ " + currentSkill[i] + " ]");
    }
//    waitKey(0);
}

void BAASAutoFight::updateCost() {
    currentCost = 0;
    Vec3b brightPixelMin = {250, 250, 250}, brightPixelMax = {255, 255, 255};
    for(int i = costRegion.lr.x; i >= costRegion.ul.x; i--){
        for(int j = costRegion.ul.y; j <= costRegion.lr.y; j++){
            if(BAASImageUtil::judgeRGBRange(lastScreenshot, {i, j}, brightPixelMin, brightPixelMax)){
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

void BAASAutoFight::updateScreenshot() {
    nemu->screenshot(lastScreenshot);
}

void BAASAutoFight::showCost() const{
    BAASGlobalLogger->BAASInfo("Cost : " + to_string(currentCost));
}

void BAASAutoFight::showLastScreenshot() const {
    imshow("Last Screenshot", lastScreenshot);
    waitKey(0);

}

bool BAASAutoFight::skillFull() const {
    for(auto &skill: currentSkill){
        if(skill.empty())
            return false;
    }
    return true;
}

void BAASAutoFight::click(const BAASPoint &point, int duration) const {
    BAASGlobalLogger->BAASInfo("Click : (" + to_string(point.x) + ", " + to_string(point.y) + ")");
    nemu->click(point);
    BAASUtil::sleepMS(duration);
}

void BAASAutoFight::appendNextSkill(const string &skillName) {
    nextSkillQueue.push(skillName);
}

bool BAASAutoFight::releaseSkill(const string &skillName, const BAASPoint &position) {
    for(int i = 0; i < 3; i++){
        if(currentSkill[i] == skillName){
            BAASGlobalLogger->BAASInfo("SKILL SLOT : " + to_string(i) );
            click(threeStudentSkillPositionCenter[i]);
            click(position);
            currentSkill[i] = nextSkillQueue.front();
            nextSkillQueue.pop();
            nextSkillQueue.push(skillName);
            return true;
        }
    }
    return false;
}



bool BAASAutoFight::startLoop() {
    while(!nextSkillQueue.empty()) nextSkillQueue.pop();
    nextSkillQueue.emplace("");
    nextSkillQueue.emplace("");
    nextSkillQueue.emplace("");
    int needCheckLast;
    for(int i = 0; i < procedure.size(); i++){
        if(restart)
            return false;
        auto &step = procedure[i];
        BAASGlobalLogger->BAASInfo("Step : {");
        BAASGlobalLogger->BAASInfo("       Skill : " + step.skillName);
        BAASGlobalLogger->BAASInfo("       Position : (" + to_string(step.position.x) + ", " + to_string(step.position.y) + ")");
        BAASGlobalLogger->BAASInfo("       Cost : " + to_string(step.cost));
        BAASGlobalLogger->BAASInfo("       }");
        showSkillPosition();
        while(!restart) {
            updateScreenshot();
            updateCost();
            if(step.cost <= currentCost){
                BAASGlobalLogger->BAASInfo("Cost : " + to_string(currentCost) );
                BAASGlobalLogger->BAASInfo("Release skill : [ " + step.skillName + " ] at position : " + to_string(step.position.x) + " " + to_string(step.position.y));
                if(!releaseSkill(step.skillName, step.position)){
                    updateScreenshot();
                    refreshSkillPosition();
                    searchAllSkillPosition();
                    if(!releaseSkill(step.skillName, step.position)) {
                        cout << "Release skill failed" << endl;
                        return false;
                    }
                }
                BAASUtil::sleepMS(300);
                double lastCost = currentCost;
                updateScreenshot();
                updateCost();
                cout<<"Updated Cost : "<<currentCost<<endl;
                if(lastCost > currentCost){
                    needCheckLast = false;
                    break;
                }
                else {
                    needCheckLast = true;
                }
                break;
            }
            else {
                refreshSkillPosition();
                searchAllSkillPosition();
                if(i > 0 && inCurrentSkill(procedure[i-1].skillName) && needCheckLast) {
                    BAASGlobalLogger->BAASInfo("Again Release : [ " + procedure[i - 1].skillName + " ] ");
                    waitCost(procedure[i-1].cost);
                    releaseSkill(procedure[i-1].skillName, procedure[i-1].position);
                    BAASUtil::sleepMS(300);
                }
            }
        }
    }
    return true;
}

bool BAASAutoFight::keyboardInputThread() {
    while(true){
        char c = getchar();
        if(c == 'q'){
            alive = false;
            return false;
        }
        else if(c == 'r'){
            restart = true;
        }
        else if(!enableStart){
            enableStart = true;
        }
    }
}


bool BAASAutoFight::inCurrentSkill(const string &skillName) const {
    for(auto &skill: currentSkill){
        if(skill == skillName)
            return true;
    }
    return false;
}

void BAASAutoFight::waitCost(double cost) {
    while(currentCost < cost){
        updateScreenshot();
        updateCost();
        refreshSkillPosition();
        searchAllSkillPosition();
        BAASUtil::sleepMS(1);
    }
}

void BAASAutoFight::restartLoop() {
    click({1228, 50}, 500);
    click({724, 509}, 500);
    click({724, 509}, 5000);
    click({724, 509}, 500);
    click({724, 509}, 500);

}

bool BAASAutoFight::getAlive() {
    return alive;
}
