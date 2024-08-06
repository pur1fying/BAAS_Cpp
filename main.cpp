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
        string path = "default_config\\config.json";
        BAASUserConfig config(path);
        config.update_name();
        config.config_update();
        config.save();
        BAASConnection connection(&config);
        connection.clear_cache("com.android.vending");
        connection.start_self();
        cv::Mat img;
        NemuScreenshot nemu = NemuScreenshot(&connection);
        NemuControl nemuControl = NemuControl(&connection);
        nemu.init();
        resource->load(connection.get_server(), connection.get_language());
        BAASFeature::show();
        while(true) {
            nemu.screenshot(img);
            BAASConfig output;
            if(BAASFeature::appear(&connection, "competition_start-battle_appear", img, output, true)) {
                BAASGlobalLogger->BAASInfo("Found");
            }
            this_thread::sleep_for(chrono::seconds(1));
        }
//        resource->get(connection.get_server(), connection.get_language(), "plot", "skip", img);
//        string name = "competition-checked";
//        BAASRectangle region = {446, 1112, 544, 1196};
//        BAASDevelopUtils::extract_image_rgb_range(img, name, region, {0, 0, 0}, {120, 127, 120});

        return 0;

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