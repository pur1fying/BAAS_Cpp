//
// Created by pc on 2024/8/10.
//

#include "procedure/BAASProcedure.h"

using namespace std;
using namespace nlohmann;

std::map<std::string, BaseProcedure*> BAASProcedure::procedures;

BAASProcedure *BAASProcedure::instance = nullptr;

BAASProcedure *baas_procedures = nullptr;

void BAASProcedure::implement(BAAS *baas,const std::string& procedure_name, BAASConfig& output) {
    assert(baas != nullptr);
    auto it = procedures.find(procedure_name);
    if(it == procedures.end()) {
        BAASGlobalLogger->BAASError("Procedure [ " + procedure_name + " ] not found");
        return;
    }
    it->second->implement(baas, output);
    it->second->clear_resource();
}

BAASProcedure *BAASProcedure::get_instance() {
    if(instance == nullptr) instance = new BAASProcedure();
    return instance;
}

void BAASProcedure::load() {
    if(!filesystem::exists(BAAS_PROCEDURE_DIR)) {
        BAASGlobalLogger->BAASError("Procedure Dir [ " + BAAS_PROCEDURE_DIR + " ] not exists");
        return;
    }

    string temp_path;
    int total_loaded = 0;
    for(const auto &entry : filesystem::recursive_directory_iterator(BAAS_PROCEDURE_DIR)) {
        temp_path = entry.path().string();
        if(filesystem::is_regular_file(entry) && temp_path.ends_with(".json"))
            total_loaded += load_from_json(temp_path);
    }
    BAASGlobalLogger->BAASInfo("Totally loaded [ " + to_string(total_loaded) + " ] procedures");
}

int BAASProcedure::load_from_json(const std::string &path) {
    BAASConfig _feature(path, (BAASLogger *) BAASGlobalLogger);
    json j = _feature.get_config();
    assert(j.is_object());
    BAASConfig *temp;
    int loaded = 0;
    for (auto &i: j.items()) {
        temp = new BAASConfig(i.value(), (BAASLogger *) BAASGlobalLogger);
        auto it = procedures.find(i.key());
        if (it != procedures.end()) {
            BAASGlobalLogger->BAASError("Procedure [ " + i.key() + " ] already exists");
            delete temp;
            continue;
        }
        int tp = temp->getInt("procedure_type", -1);
        BaseProcedure *p = nullptr;
        switch (tp) {
            case -1:
                p = new BaseProcedure(temp);
                break;
            case BAAS_PROCEDURE_TYPE_APPEAR_THEN_CLICK:
                p = new AppearThenClickProcedure(temp);
                break;
            case BAAS_PROCEDURE_TYPE_APPEAR_THEN_DO:
                p = new AppearThenDoProcedure(temp);
                break;
            default:
                BAASGlobalLogger->BAASError("Procedure Type [ " + to_string(tp) + " ] not found");
                break;
        }
        if (p != nullptr) {
            procedures[i.key()] = p;
            loaded++;
        }
    }
    return loaded;
}

BAASProcedure::BAASProcedure() {
    load();
}









