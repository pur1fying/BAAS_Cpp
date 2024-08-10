//
// Created by pc on 2024/8/10.
//

#ifndef CONFIG_JSON_BAASPROCEDURE_H
#define CONFIG_JSON_BAASPROCEDURE_H
#include "procedure/AppearThenDoProcedure.h"
#include "procedure/AppearThenClickProcedure.h"

class BAASProcedure {
public:
    static void implement(BAAS *baas,const std::string& procedure_name, BAASConfig& output);

    static BAASProcedure* get_instance();
private:

    static BAASProcedure* instance;

    static void load();

    static int load_from_json(const std::string& path);

    BAASProcedure();

    static std::map<std::string, BaseProcedure*> procedures;
};

extern BAASProcedure* baas_procedures;

#endif //CONFIG_JSON_BAASPROCEDURE_H
