//
// Created by pc on 2024/5/31.
//

#ifndef BAAS_BAASAUTOFIGHT_H_
#define BAAS_BAASAUTOFIGHT_H_

#include <map>

#include "BAASImageUtil.h"
#include "BAASImageResource.h"
#include "BAAS.h"

#define EVENT_RELEASE_SKILL 0
#define BOSS_POSITION BAASPoint(1106, 365)

class BAASAutoFight {
public:
    explicit BAASAutoFight(BAAS* baas);

    struct SimpleTrigger{
        std::string skillName;
        BAASPoint position;
        double cost;
    };

    std::vector<SimpleTrigger> procedure;

    bool fight_release_uiskill2mika();

    void flip_auto(int times, double interval);

    bool release_skill(const std::string& name, BAASPoint p,bool ensure_click=false, bool check_cost=false, int cost_decrease=3, bool check_skill_icon_change=false, int time_out=2000);

    void enter_fight();

    void reset_all();

    void restart_fight();

    void setSkills(const std::vector<std::string>& skillNames);

    void searchAllSkillPosition();

    void showSkillPosition();

    int get_skill_pos(const std::string& name);

    bool skillFull() const;

    void refreshSkillPosition();

    void updateCost();

    void showCost() const;

    void appendNextSkill(const std::string& skillName);

    bool startLoop();

    int has_skill(const std::string& name);

    bool inCurrentSkill(const std::string& skillName) const;

    void waitCost(double cost);

    void ocr_update_fight_left_time();

    void get_all_skill_name(int timeout=5000);

    void waitTime(double time);
private:
    class CharacterSkill {
        public:
            CharacterSkill();
            cv::Mat skill_templates[3];
            std::string name;
    };

    double currentCost = 0;

    size_t boss_health = 0;

    double fight_left_time = 0;

    size_t skillSlotLength = 3;

    std::string lastSkill= "None";

    std::vector<std::optional<std::string>> currentSkill;

    std::vector<std::string> skillNames;

    std::vector<std::string> formation;

    std::map<std::string, CharacterSkill> skills;

    static const BAASRectangle threeStudentSkillRegion;\

    static const cv::Vec3b costPixelMin;

    static const cv::Vec3b costPixelMax;

    static const BAASPoint threeStudentSkillPositionCenter[3];

    static const BAASRectangle skillSearchRegion[3];

    static const BAASRectangle costRegion;

    static const double costLineDeltaX;

    BAAS* baas = nullptr;

    BAASLogger* logger = nullptr;
};
#endif //BAAS_BAASAUTOFIGHT_H_
