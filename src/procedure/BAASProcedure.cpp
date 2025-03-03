//
// Created by pc on 2024/8/10.
//

#if defined(BAAS_APP_BUILD_FEATURE) && defined(BAAS_APP_BUILD_PROCEDURE)

#include "procedure/BAASProcedure.h"
#include "BAASGlobals.h"
#include "BAAS.h"

using namespace std;
using namespace nlohmann;

BAAS_NAMESPACE_BEGIN

std::map<std::string, BaseProcedure *> BAASProcedure::procedures;

BAASProcedure *BAASProcedure::instance = nullptr;

BAASProcedure *baas_procedures = nullptr;

void BAASProcedure::implement(
        BAAS *baas,
        const std::string &procedure_name,
        BAASConfig &output
)
{
    assert(baas != nullptr);
    auto it = procedures.find(procedure_name);
    if (it == procedures.end()) {
        BAASGlobalLogger->BAASError("Procedure [ " + procedure_name + " ] not found");
        return;
    }
    it->second
      ->implement(baas, output);
    it->second
      ->clear_resource();
}

BAASProcedure *BAASProcedure::get_instance()
{
    if (instance == nullptr) instance = new BAASProcedure();
    return instance;
}

void BAASProcedure::load()
{
    if (!filesystem::exists(BAAS_PROCEDURE_DIR)) {
        BAASGlobalLogger->BAASError("Procedure Dir :");
        BAASGlobalLogger->Path(BAAS_PROCEDURE_DIR, 3);
        BAASGlobalLogger->BAASError("not exists.");
        return;
    }

    string temp_path;
    int total_loaded = 0;
    for (const auto &entry: filesystem::recursive_directory_iterator(BAAS_PROCEDURE_DIR)) {
        temp_path = entry.path()
                         .string();
        if (filesystem::is_regular_file(entry) && temp_path.ends_with(".json"))
            total_loaded += load_from_json(temp_path);
    }
    BAASGlobalLogger->BAASInfo("Totally loaded [ " + to_string(total_loaded) + " ] procedures");
}

int BAASProcedure::load_from_json(const std::string &path)
{
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
        BaseProcedure *p = create_procedure(temp);
        if (p != nullptr) {
            procedures[i.key()] = p;
            loaded++;
        }
    }
    return loaded;
}

BAASProcedure::BAASProcedure()
{
    load();
}

void BAASProcedure::implement(
        BAAS *baas,
        const string &procedure_name,
        const BAASConfig &patch,
        BAASConfig &output
)
{
    assert(baas != nullptr);
    auto it = procedures.find(procedure_name);
    if (it == procedures.end()) {
        BAASGlobalLogger->BAASError("Procedure [ " + procedure_name + " ] not found");
        return;
    }
    auto *baas_config = new BAASConfig(it->second->get_config()->get_config(), baas->get_logger());
    baas_config->update(&patch);

    BaseProcedure *p = create_procedure(baas_config);
    try {
        p->implement(baas, output);
    }
    catch (exception &e) {
        p->clear_resource();
        delete baas_config;
        delete p;
        throw e;
    }

    p->clear_resource();
    delete baas_config;
    delete p;
}

BaseProcedure *BAASProcedure::create_procedure(BAASConfig *config)
{
    assert(config != nullptr);
    BaseProcedure *p = nullptr;
    int tp = config->getInt("procedure_type", -1);
    switch (tp) {
        case -1:
            p = new BaseProcedure(config);
            break;
        case BAAS_PROCEDURE_TYPE_APPEAR_THEN_CLICK:
            p = new AppearThenClickProcedure(config);
            break;
        case BAAS_PROCEDURE_TYPE_APPEAR_THEN_DO:
            p = new AppearThenDoProcedure(config);
            break;
        default:
            BAASGlobalLogger->BAASError("Procedure Type [ " + to_string(tp) + " ] not found");
            break;
    }
    return p;
}


void BAASProcedure::solve_procedure(const string &name)
{
    if (!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
    BAASConfig output;
    solve_procedure(name, output);
}

void BAASProcedure::solve_procedure(
        const string &name,
        BAASConfig &output
)
{
    if (!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
    BAASProcedure::implement(this, name, output);
}

void BAASProcedure::solve_procedure(
        const string &name,
        const bool skip_first_screenshot
)
{
    if (!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
    BAASConfig output;
    solve_procedure(name, output, skip_first_screenshot);
}

void BAASProcedure::solve_procedure(
        const string &name,
        BAASConfig &output,
        const bool skip_first_screenshot
)
{
    if (!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
    json j;
    j["skip_first_screenshot"] = skip_first_screenshot;
    BAASConfig patch(j, logger);
    BAASProcedure::implement(this, name, patch, output);
}

void BAASProcedure::solve_procedure(
        const string &name,
        json &patch
)
{
    if (!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
    BAASConfig output;
    BAASConfig p(patch, logger);
    solve_procedure(name, output, p);
}

void BAASProcedure::solve_procedure(
        const string &name,
        BAASConfig &output,
        json &patch
)
{
    if (!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
    BAASConfig p(patch, logger);
    solve_procedure(name, output, p);
}

void BAASProcedure::solve_procedure(
        const string &name,
        json &patch,
        const bool skip_first_screenshot
)
{
    if (!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
    BAASConfig output;
    BAASConfig p(patch, logger);
    solve_procedure(name, output, p, skip_first_screenshot);
}

void BAASProcedure::solve_procedure(
        const string &name,
        BAASConfig &output,
        json &patch,
        bool skip_first_screenshot
)
{
    if (!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
    BAASConfig p(patch, logger);
    p.update("skip_first_screenshot", skip_first_screenshot);
    solve_procedure(name, output, p);
}

void BAASProcedure::solve_procedure(
        const string &name,
        BAASConfig &output,
        BAASConfig &patch
)
{
    if (!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
    BAASProcedure::implement(this, name, patch, output);
}

void BAASProcedure::solve_procedure(
        const string &name,
        BAASConfig &output,
        BAASConfig &patch,
        const bool skip_first_screenshot
)
{
    if (!flag_run) throw HumanTakeOverError("Flag Run turned to false manually");
    patch.update("skip_first_screenshot", skip_first_screenshot);
    BAASProcedure::implement(this, name, patch, output);
}
BAAS_NAMESPACE_END

#endif //BAAS_APP_BUILD_FEATURE && BAAS_APP_BUILD_PROCEDURE