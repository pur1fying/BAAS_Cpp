//
// Created by Administrator on 2025/5/6.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_BASECONDITION_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_BASECONDITION_H_

#define BAAS_AUTO_FIGHT_CONDITION_DEFAULT_TIMEOUT 5000

#include <config/BAASConfig.h>
#include <BAAS.h>

#include "module/auto_fight/auto_fight_d.h"

BAAS_NAMESPACE_BEGIN

class BaseCondition {

public:

    enum ConditionType {
        A_COMBINED,
        O_COMBINED,
        COST,
        SKILL_NAME,
        SKILL_COST,
        ACC_PHASE,
        AUTO_STATE,
        BOSS_HEALTH
    };

    BaseCondition(BAAS* baas, auto_fight_d* data, const BAASConfig& config);

    virtual std::optional<bool> try_match();

    virtual void reset_state();

    virtual void set_d_update_flag();

    virtual ~BaseCondition();

    virtual void display() const noexcept;

    void set_and_cond(const std::vector<uint64_t>& and_cond) {
        and_conditions = and_cond;
    }

    void set_or_cond(const std::vector<uint64_t>& or_cond) {
        or_conditions = or_cond;
    }

    inline const std::vector<uint64_t>& get_and_cond() const noexcept {
        return and_conditions;
    }

    inline const std::vector<uint64_t>& get_or_cond() const noexcept {
        return or_conditions;
    }

    inline const std::vector<std::string> get_or_cond_st() const noexcept {
        return config.get<std::vector<std::string>>("or", {});
    }

    inline const std::vector<std::string> get_and_cond_st() const noexcept {
        return config.get<std::vector<std::string>>("and", {});
    }

    inline bool is_primitive() const noexcept {
        return _is_primitive;
    }

    inline bool has_or_cond() const noexcept {
        return !or_conditions.empty();
    }

    inline bool has_and_cond() const noexcept {
        return !and_conditions.empty();
    }

    inline long long get_timeout() {
        return timeout;
    }

    static const std::map<std::string, BaseCondition::ConditionType> cond_type_map;

    static const std::vector<std::string> cond_type_st_list;

protected:

    void _display_basic_info() const noexcept;

    BAAS* baas;
    BAASLogger* logger;
    auto_fight_d* data;
    BAASConfig config;

    bool _is_primitive;

    std::vector<uint64_t> or_conditions;
    std::vector<uint64_t> and_conditions;

    long long cond_j_start_t;

    long long timeout;

    ConditionType type;

    std::string name;

    std::string desc;

};

BAAS_NAMESPACE_END

#endif //BAAS_CPP_MODULE_AUTO_FIGHT_CONDITIONS_BASECONDITION_H_
