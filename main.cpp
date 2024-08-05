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
        nemu.screenshot(img);
        BAASImageResource resource;
        resource.load(connection.get_server(), connection.get_language());

//        resource.get(connection.get_server(), connection.get_language(), "plot", "skip", img);
        BAASDevelopUtils::extract_image_rgb_range(img, "activity-fee", {43, 739, 168, 791}, {0, 0, 0}, {255, 255, 255});

        return 0;
        AdbScreenshot screenCap(&connection);
        screenCap.init();
        screenCap.screenshot(img);
        BAASImageUtil::filter_region_rgb(img, {924, 46, 1228, 116}, {180, 180, 180}, {255, 255, 255});
        string name = "skip.png";
        cv::imwrite(name, img);
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