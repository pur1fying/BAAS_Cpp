//
// Created by pc on 2024/5/31.
//
#include "module/auto_fight/BAASAutoFight.h"

using namespace std;
using namespace cv;

BAAS_NAMESPACE_BEGIN

BAASAutoFight::BAASAutoFight(BAAS *baas)
{
    this->baas = baas;
}
//
//BAASRectangle BAASAutoFight::costRegion = {820, 681, 1141, 696};
//
//double BAASAutoFight::costLineDeltaX = 31.7;


//void BAASAutoFight::setSkills(const std::vector<std::string>& skillNames) {
//    string temp;
//    for(auto &name: skillNames){
//        CharacterSkill skill;
//        skill.name = name;
//        temp = name + "_bright";
//        this->resource->get_image(temp, skill.iconBright);
//        temp = name + "_left";
//        this->resource->get_image(temp, skill.iconLeftBlack);
//        temp = name + "_right";
//        this->resource->get_image(temp, skill.iconRightGrey);
//        this->skills[name] = skill;
//    }
//    this->skillNames = skillNames;
//}



//void BAASAutoFight::searchAllSkillPosition() {
//    if(skillFull()) {
//        return;
//    }
//    BAASPoint p;
//    for(auto &key: skillNames){
//        if(skillFull())
//            break;
//        p = BAASImageUtil::imageSearch(lastScreenshot, skills[key].iconLeftBlack.image, threeStudentSkillRegion, 0.8, false);
//        if(p.x != -1){
//            currentSkill[pointToSkillIndex(p)] = key;
//            continue;
//        }
//        p = BAASImageUtil::imageSearch(lastScreenshot, skills[key].iconRightGrey.image, threeStudentSkillRegion, 0.8, false);
//        if(p.x != -1){
//            currentSkill[pointToSkillIndex(p)] = key;
//            continue;
//        }
//        Mat img = skills[key].iconBright.image;
//        Mat left =  BAASImageUtil::imageCrop(img, 0, 0, img.cols/2-3, img.rows);
//        Mat right = BAASImageUtil::imageCrop(img, img.cols/2+3, 0, img.cols, img.rows);
//        BAASImageUtil::imageSearch(lastScreenshot, left, threeStudentSkillRegion, 0.8, true);
//        if(p.x != -1){
//            currentSkill[pointToSkillIndex(p)] = key;
//            continue;
//        }
//        p = BAASImageUtil::imageSearch(lastScreenshot, right, threeStudentSkillRegion, 0.8, true);
//        if(p.x != -1){
//            currentSkill[pointToSkillIndex(p)] = key;
//            continue;
//        }
//    }
//}




//void BAASAutoFight::updateCost() {
//    currentCost = 0;
//    Vec3b brightPixelMin = {250, 250, 250}, brightPixelMax = {255, 255, 255};
//    for(int i = costRegion.lr.x; i >= costRegion.ul.x; i--){
//        for(int j = costRegion.ul.y; j <= costRegion.lr.y; j++){
//            if(BAASImageUtil::judgeRGBRange(lastScreenshot, {i, j}, brightPixelMin, brightPixelMax)){
//                int x = i - (costRegion.ul.x + (costRegion.lr.y - j) / 5);
//                int integer = int(double(x) *1.0 / costLineDeltaX);
//                double decimal = double((x - integer* 32 )) * 1.0 / 28;
//                if(decimal < 0)
//                    decimal = 0;
//                currentCost = integer + decimal;
//                currentCost = currentCost > 10.0 ? 10.0 : currentCost;
//                return;
//            }
//        }
//    }
//}



BAAS_NAMESPACE_END

