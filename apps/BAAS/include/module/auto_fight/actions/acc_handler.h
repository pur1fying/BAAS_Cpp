//
// Created by Administrator on 2025/5/17.
//

#ifndef BAAS_CPP_MODULE_AUTO_FIGHT_ACTIONS_ACC_HANDLER_H_
#define BAAS_CPP_MODULE_AUTO_FIGHT_ACTIONS_ACC_HANDLER_H_

#include "base_handler.h"

BAAS_NAMESPACE_BEGIN

class acc_handler : public base_handler {

public:

    enum Op {
        PHASE_1,
        PHASE_2,
        PHASE_3,
    };


    acc_handler(
            BAAS* baas,
            auto_fight_d* data,
            const BAASConfig& config
    );

    bool execute() override;

    void display() override;

private:

    const static std::map<std::string, Op> op_map;

    void _parse_op();

    inline void _display_valid_acc_op() const noexcept {
        logger->BAASInfo("Valid acc op are as displayed as follow.");
        int cnt = 0;
        for (const auto& [key, value] : op_map)
            logger->BAASInfo(std::to_string(++cnt) + " : \"" + key + "\"");
    }

    Op _op;

};



BAAS_NAMESPACE_END

#endif //BAAS_CPP_MODULE_AUTO_FIGHT_ACTIONS_ACC_HANDLER_H_
