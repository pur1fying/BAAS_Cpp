//
// Created by pc on 2025/2/24.
//


#ifndef BAAS_APP_ISA_ISA_H_
#define BAAS_APP_ISA_ISA_H_

#include <BAAS.h>

BAAS_NAMESPACE_BEGIN

class ISA {
public:
    ISA(BAAS *baas);

    static void init_implement_funcs();

    static std::map<std::string, bool (*)(BAAS *)> implement_funcs;

    bool solve(const std::string &task);

    const static constexpr std::string script;
private:
    BAAS* baas;
    BAASLogger *logger;
};

BAAS_NAMESPACE_END

#endif //BAAS_APP_ISA_ISA_H_
