#include <opencv2/opencv.hpp>
#include <iostream>
#include <winsock2.h>
#include "BAAS.h"

#include <cstdint>
#include <cstring>
#pragma comment(lib, "ws2_32.lib")
using namespace cv;
using namespace std;
using json = nlohmann::json;


optional<int> func(int c) {
    if(c == 0) return 0;
    return nullopt;
}
int main() {
    system("chcp 65001");
    try {
        initGlobals();
        string path = "1708148000\\config.json";
        BAASUserConfig config(path);
        config.update_name();
        config.config_update();
        config.save();

        BAASConnection connection(&config);


        ScrcpyControl control(&connection);
        control.init();


    }
    catch (const std::exception& e){
        BAASGlobalLogger->BAASInfo(e.what());
    }

    return 0;
}
/*
    y -->   601 ,  662
    x -->   847 , 932
            946, 1038
            1048, 1130

            883, 688
            985, 621
            1087, 622


*/