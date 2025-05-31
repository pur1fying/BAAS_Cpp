//
// Created by Administrator on 2025/5/16.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_SKILLNAMECONDITION_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_SKILLNAMECONDITION_H_

#include "BaseCondition.h"

BAAS_NAMESPACE_BEGIN

class SkillNameCondition : public BaseCondition {

public:

    SkillNameCondition(
            BAAS* baas,
            auto_fight_d* data,
            const BAASConfig& config
    );

    enum Op {
        APPEAR,
        AT
    };

    void reset_state() override;

    void display() const noexcept override;

    void set_d_update_flag() override;

    const static std::map<std::string, Op> op_map;

    const static std::vector<std::string> op_st_list;

    std::optional<bool> try_match() override;

private:

    void _parse_op();

    void _parse_skill_name();

    inline void _list_valid_name() const noexcept {
        logger->BAASInfo("Valid skill names are listed below :");
        int cnt = 0;
        for (const auto& skill : data->all_possible_skills)
            logger->BAASInfo(std::to_string(++cnt) + " : \"" + skill.name + "\"");
    }

    void _parse_p();

    Op _op;

    std::string _skill_name;

    uint64_t _skill_idx;

    uint64_t _p;

};



BAAS_NAMESPACE_END


#endif //BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_SKILLNAMECONDITION_H_
