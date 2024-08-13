#include <opencv2/opencv.hpp>
#include <iostream>
#include <winsock2.h>
#include "BAAS.h"

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
        init_globals();
        string config_name = "default_config";
        cv::Mat img;
        BAAS baas(config_name);
        BAASConfig config;
        BAASConnection* conn = baas.get_connection();
        baas.update_screenshot_array();
        baas.get_latest_screenshot(img);
        baas.solve("restart");
        baas.solve("collect_activity_fee");
        baas.solve("work");
//        baas.get_latest_screenshot(img);
//        cv::imshow("img", img);
//        cv::waitKey(0);
//        return 0;
        baas.solve("competition");

//        BAASConnection* connection = baas.get_connection();
//        connection->start_self();
//        resource->show();


//        config_name = "resource\\module_usage\\main_page.json";
//        BAASConfig procedure(config_name, baas.get_logger());
//        BAASConfig c = BAASConfig(procedure.get<json>("UI-GO-TO_main_page_home"), baas.get_logger());
//        string name = "login-BANDAI-NAMCO-icon";
//        BAASRectangle region = {583, 36, 681, 79};
//        BAASDevelopUtils::extract_image_rgb_range(img, name, region, {0, 0, 0}, {255, 255, 255});
        baas.get_logger()->BAASInfo("ISA Exit.");
        }
//        resource->get(connection.get_server(), connection.get_language(), "common", "back", img);
//        cv::Mat mast;
//        BAASRectangle region = {0, 0, img.cols, img.rows};
//        BAASImageUtil::gen_not_black_region_mask(img, mast, region);


//        return 0;

//    }
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