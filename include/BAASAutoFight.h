//
// Created by pc on 2024/5/31.
//

#ifndef BAAS_BAASAUTOFIGHT_H_
#define BAAS_BAASAUTOFIGHT_H_

#include "BAASImageUtil.h"
#include "BAASImageResource.h"

#include <map>

#define EVENT_RELEASE_SKILL 0
#define BOSS_POSITION BAASPoint(1106, 365)
class BAASAutoFight {
//public:
//    struct SimpleTrigger{
//        std::string skillName;
//        BAASPoint position;
//        double cost;
//    };
//
//    bool restart = false;
//
//    std::vector<SimpleTrigger> procedure;
//
//    bool getAlive();
//
//    void restartLoop();
//
//    BAASAutoFight();
//
//    void setFormation(const std::vector<std::string>& studentNames);
//
//    void setSkills(const std::vector<std::string>& skillNames);
//
//    void setImageResource(BAASImageResource* imageResource);
//
//    void searchAllSkillPosition();
//
//    void showSkillPosition();
//
//    bool skillFull() const;
//
//    bool releaseSkill(const std::string& skillName, const BAASPoint& position);
//
//    void refreshSkillPosition();
//
//    void updateScreenshot();
//
//    void updateCost();
//
//    void showCost() const;
//
//    void showLastScreenshot() const;
//
//    void click(const BAASPoint& point, int duration = 0) const;
//
//    void appendNextSkill(const std::string& skillName);
//
//    bool startLoop();
//
//    bool keyboardInputThread();
//
//    bool inCurrentSkill(const std::string& skillName) const;
//
//    void waitCost(double cost);
//
//    bool enableStart;
//
//    bool alive = true;
//private:
//    class CharacterSkill {
//        public:
//            CharacterSkill();
//        private:
//
//            std::string name;
//            BAASImageResource::Image iconBright;
//            BAASImageResource::Image iconLeftBlack;
//            BAASImageResource::Image iconRightGrey;
//            int position;
//            friend class BAASAutoFight;
//    };
//
//    cv::Mat lastScreenshot;
//
//    double currentCost = 0;
//
//    int mode;
//
//    std::string lastSkill= "None";
//
//    BAASNemu* nemu;
//
//    std::vector<std::string> currentSkill;
//
//    std::queue<std::string> nextSkillQueue;
//
//    std::vector<std::string> skillNames;
//
//    std::vector<std::string> formation;
//
//    std::map<std::string, CharacterSkill> skills;
//
//    BAASImageResource* resource;
//
//    static const BAASRectangle threeStudentSkillRegion;
//
//    static const BAASPoint threeStudentSkillPositionCenter[3];
//
//    static int pointToSkillIndex(const BAASPoint& p);
//
//    static const BAASRectangle costRegion;
//
//    static const double costLineDeltaX;
};
#endif //BAAS_BAASAUTOFIGHT_H_
