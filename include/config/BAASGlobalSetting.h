//
// Created by pc on 2024/11/5.
//

#ifndef BAAS_CONFIG_BAASGLOBALSETTING_H_
#define BAAS_CONFIG_BAASGLOBALSETTING_H_

#include "config/BAASConfig.h"

class BAASGlobalSetting : public BAASConfig {
public:
    static BAASGlobalSetting* getGlobalSetting();

    static void check_global_setting_exist();

    inline int ocr_flagGpu() noexcept{
        return get("/ocr/flagGpu", -1);
    }

    inline int ocr_numThread() noexcept {
        return get("/ocr/numThread", 4);
    }
private:
    BAASGlobalSetting();

    void update_global_setting();

    void create_modify_history_file();

    static BAASGlobalSetting* globalSetting;
};

extern BAASGlobalSetting* global_setting;


#endif //BAAS_CONFIG_BAASGLOBALSETTING_H_
