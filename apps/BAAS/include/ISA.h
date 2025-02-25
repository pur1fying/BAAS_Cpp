//
// Created by pc on 2025/2/24.
//

#define ISA_NAMESPACE_BEGIN namespace isa {
#define ISA_NAMESPACE_END }

#ifndef BAAS_APP_ISA_H_
#define BAAS_APP_ISA_H_

#include <BAAS.h>

ISA_NAMESPACE_BEGIN

class ISA {
public:
    ISA(baas::BAAS *baas);

    static void init_implement_funcs();

    static std::map<std::string, bool (*)(ISA *)> implement_funcs;

    bool solve(const std::string &task);

private:
    baas::BAASLogger *logger;
};

ISA_NAMESPACE_END

#endif //BAAS_APP_ISA_H_
