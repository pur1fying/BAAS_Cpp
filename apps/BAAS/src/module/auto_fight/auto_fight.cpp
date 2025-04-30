//
// Created by pc on 2025/4/23.
//

#include "module/auto_fight/auto_fight.h"

#include <BAASImageResource.h>

#include "module/auto_fight/screenshot_data/CostUpdater.h"
#include "module/auto_fight/screenshot_data/SkillCostUpdater.h"
#include "module/auto_fight/screenshot_data/SkillNameUpdater.h"
#include "module/auto_fight/screenshot_data/BossHealthUpdater.h"
#include "utils.h"

BAAS_NAMESPACE_BEGIN

// Default skill recognition template
const std::vector<std::string> AutoFight::default_active_skill_template = {
        "skill_active"
};
const std::vector<std::string> AutoFight::default_inactive_skill_template = {
        "skill_l_inactive", "skill_r_inactive",
};

const std::string AutoFight::template_j_ptr_prefix = "/BAAS/auto_fight/Skill/templates";

AutoFight::AutoFight(BAAS *baas)
{
    this->baas = baas;
    this->config = baas->get_config();
    logger = baas->get_logger();
    d_update_max_thread = config->getInt("/auto_fight/d_update_max_thread");
    d_update_thread_pool = std::make_unique<ThreadPool>(d_update_max_thread);
    d_update_thread_pool->init();
}

// Add data updaters here
void AutoFight::init_data_updaters()
{
    // cost
    d_updaters.push_back(std::make_unique<CostUpdater>(baas, &latest_screenshot_d));
    d_updater_map["cost"] = 1LL << 0;

    // boss health
    d_updaters.push_back(std::make_unique<BossHealthUpdater>(baas, &latest_screenshot_d));
    d_updater_map["boss_health"] = 1LL << 1;

    // skill name
    d_updaters.push_back(std::make_unique<SkillNameUpdater>(baas, &latest_screenshot_d));
    d_updater_map["skill_name"] = 1LL << 2;

//     skill cost
//    d_updaters.push_back(std::make_unique<SkillCostUpdater>(baas, &latest_screenshot_d));
//    d_updater_map["skill_cost"] = 1LL << 3;
}

void AutoFight::update_data()
{
    d_wait_to_update_idx.clear();

    for (int i = 0; i < d_updaters.size(); ++i) {
        if((1LL << i) & d_updater_mask) {
            d_wait_to_update_idx.push_back(i);
        }
    }

    if (d_wait_to_update_idx.empty()) {
        return;
    }

    // sort by cost
    std::sort(d_wait_to_update_idx.begin(),d_wait_to_update_idx.end(), [this](auto a, auto b) {
        return d_updaters[a]->estimated_time_cost() < d_updaters[b]->estimated_time_cost();
    });


    for (auto idx : d_wait_to_update_idx) {
        d_updater_queue.push(idx);
    }

    // put updater in queue into thread pool
    for (int i = 0; i < d_wait_to_update_idx.size(); ++i) {
        if (d_updater_running_thread_count < d_update_max_thread) ++d_updater_running_thread_count;
        else {
            std::unique_lock<std::mutex> d_update_thread_lock(d_update_thread_mutex);
            d_update_thread_finish_notifier.wait(d_update_thread_lock, [&]() {
                return d_updater_running_thread_count < d_update_max_thread;
            });
            // condition judgement
        }
        
        d_update_thread_pool->submit([this]() {
            submit_data_updater_task(this);
        });
    }

    // all updater submitted
    int start_running_t, initial_t_count = d_updater_running_thread_count;
    for (int i = 1; i <= initial_t_count; ++i) {
        std::unique_lock<std::mutex> d_update_thread_lock(d_update_thread_mutex);
        start_running_t = d_updater_running_thread_count;
        if (start_running_t == 0) break;
        d_update_thread_finish_notifier.wait(d_update_thread_lock, [&]() {
            return d_updater_running_thread_count < start_running_t;
        });
        // condition judgement
    }
    assert(d_updater_running_thread_count == 0);
}

void AutoFight::submit_data_updater_task(AutoFight *self)
{
    auto start_t = BAASUtil::getCurrentTimeMS();

    auto idx = self->d_updater_queue.front();
    self->d_updater_queue.pop();
    try {
        self->d_updaters[idx]->update();
    }
    catch (const std::exception &e) {
        self->logger->BAASError("In [ " + self->d_updaters[idx]->data_name() + " ] update | Error: " + e.what());
    }

    self->notify_d_update_thread_end();

    auto end_t = BAASUtil::getCurrentTimeMS();
    self->logger->BAASInfo("[ " + self->d_updaters[idx]->data_name() + " ] update | Time: " +
                                                                    std::to_string(end_t - start_t) + "ms");
}

void AutoFight::display_data()
{
    for (auto &updater : d_updaters) {
        updater->display_data();
    }
}


AutoFight::~AutoFight()
{
    for (auto &updater : d_updaters) {
        updater.reset();
    }
    d_updaters.clear();
    d_update_thread_pool->shutdown();
}

void AutoFight::init_workflow(const std::filesystem::path &name)
{

    // consider chinese name
    if(name.empty())  {
        workflow_name = config->getString("/auto_fight/workflow_name");
        workflow_path = config->getPath("/auto_fight/workflow_name").replace_extension(".json");
    }
    else {
        std::filesystem::path temp = name;
        workflow_name = name.string();
        workflow_path = temp.replace_extension(".json");
    }

    workflow_path = BAAS_AUTO_FIGHT_WORKFLOW_DIR / workflow_path;
    if(!std::filesystem::exists(workflow_path)) {
        logger->BAASCritical("Workflow [ " + workflow_name + " ] do not exist. Please check your config.");
        throw PathError("WorkFlow do not exist.");
    }

    logger->hr("Load WorkFlow");
    logger->BAASInfo("Name : " + workflow_name);
    logger->BAASInfo("Path : " + workflow_path.string());
    latest_screenshot_d.d_fight = BAASConfig(workflow_path, logger);

    // Skill
    auto skill_names = latest_screenshot_d.d_fight.get<std::vector<std::string>>("/formation/all_appeared_skills");
    for (int i = 0; i < skill_names.size(); ++i) {
        latest_screenshot_d.skill_name_to_index_map[skill_names[i]] = i;
        _init_single_skill_template(skill_names[i]);
    }
    latest_screenshot_d.slot_count = latest_screenshot_d.d_fight.getInt("/formation/slot_count", 3);
    logger->BAASInfo("Slot Count : [ " + std::to_string(latest_screenshot_d.slot_count) + " ]");

    latest_screenshot_d.skills.resize(latest_screenshot_d.slot_count);
    latest_screenshot_d.each_slot_possible_templates.resize(latest_screenshot_d.slot_count);
}

void AutoFight::_init_single_skill_template(std::string &skill_name)
{
    skill_template _template;
    _template.name = skill_name;

    std::vector<std::string> temp;
    // active templates
    std::string j_ptr = template_j_ptr_prefix + "/" + skill_name + "/active";

    if (static_config->contains(j_ptr)) temp = static_config->get<std::vector<std::string>>(j_ptr);
    else temp = default_active_skill_template;

    template_info _t_info;
    std::string res_ptr;
    BAASImage _img;
    for (const auto &group : temp) {
        res_ptr = BAASImageResource::resource_pointer(baas, group, skill_name);
        if(resource->is_loaded(res_ptr)) resource->get(res_ptr, _img);
        else {
            logger->BAASWarn("Image Resource [ " + res_ptr + " ] not find.");
            continue;
        }
        _t_info.template_image = _img.image;
        _t_info.mean_rgb = _img.mean_rgb;
        _template.skill_active_templates.push_back(_t_info);
    }

    // inactive templates
    j_ptr = template_j_ptr_prefix + "/" + skill_name + "/inactive";
    if (static_config->contains(j_ptr)) temp = static_config->get<std::vector<std::string>>(j_ptr);
    else temp = default_inactive_skill_template;

    for (const auto &group : temp) {
        res_ptr = BAASImageResource::resource_pointer(baas, group, skill_name);
        if(resource->is_loaded(res_ptr)) resource->get(res_ptr, _img);
        else {
            logger->BAASWarn("Image Resource [ " + res_ptr + " ] not find.");
            continue;
        }
        _t_info.template_image = _img.image;
        _t_info.mean_rgb = _img.mean_rgb;
        _template.skill_inactive_templates.push_back(_t_info);
    }

    logger->BAASInfo("Skill Name: [ " + skill_name + " ]");
    logger->BAASInfo("Index     : [ " + std::to_string( latest_screenshot_d.all_possible_skills.size()) + " ]");
    logger->BAASInfo("Active    : [ " + std::to_string(_template.skill_active_templates.size()) + " ]");
    logger->BAASInfo("Inactive  : [ " + std::to_string(_template.skill_inactive_templates.size()) + " ]");

    latest_screenshot_d.all_possible_skills.push_back(_template);
}

void AutoFight::set_skill_slot_possible_templates(
        int skill_idx,
        const std::vector<int> &possible_templates
)
{
    if (skill_idx < 0 || skill_idx >= latest_screenshot_d.skills.size()) {
        logger->BAASWarn("Invalid skill index: " + std::to_string(skill_idx));
        return;
    }
    latest_screenshot_d.each_slot_possible_templates[skill_idx] = possible_templates;
}

void AutoFight::set_slot_possible_skill_idx(
        int slot_idx,
        const std::vector<int> &possible_skill_idx
)
{
    if (slot_idx < 0 || slot_idx >= latest_screenshot_d.skills.size()) {
        logger->BAASError("Invalid slot index : [ " + std::to_string(slot_idx) + " ]");
        return;
    }
    for (auto &idx : possible_skill_idx) {
        if (idx < 0 || idx >= latest_screenshot_d.all_possible_skills.size()) {
            logger->BAASError("Invalid skill index : [ " + std::to_string(idx) + " ]");
            return;
        }
    }
    latest_screenshot_d.each_slot_possible_templates[slot_idx] = possible_skill_idx;
}


BAAS_NAMESPACE_END

