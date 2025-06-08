//
// Created by pc on 2024/11/5.
//

#ifndef BAAS_CONFIG_BAASGLOBALSETTING_H_
#define BAAS_CONFIG_BAASGLOBALSETTING_H_

#include "config/BAASConfig.h"

BAAS_NAMESPACE_BEGIN

class BAASGlobalSetting : public BAASConfig {

public:

    static BAASGlobalSetting* getGlobalSetting();

    static void check_global_setting_exist();

    inline int ocr_gpu_id() const noexcept
    {
        return getInt("/ocr/gpu_id", -1);
    }

    inline int ocr_num_thread() const noexcept
    {
        return getInt("/ocr/num_thread", 4);
    }

    inline bool ocr_enable_cpu_memory_arena() const noexcept
    {
        return getBool("/ocr/enable_cpu_memory_arena", false);
    }

private:

    BAASGlobalSetting();

    void update_global_setting();

    void create_modify_history_file();

    static BAASGlobalSetting *globalSetting;
};

extern BAASGlobalSetting *global_setting;

BAAS_NAMESPACE_END


#endif //BAAS_CONFIG_BAASGLOBALSETTING_H_
