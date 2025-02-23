#include "opencv2/opencv.hpp"
#include <iostream>
#include "BAAS.h"
#include "httplib.h"
#include "BAASGlobals.h"

#pragma comment(lib, "ws2_32.lib")
#include "BAASExternalIPC.h"
using namespace cv;
using namespace std;
using json = nlohmann::json;
#include "device/BAASLdopengl.h"
#include "benchmark/benchmark.h"

int main(int argc, char **argv) {
    system("chcp 65001");
    string config_name = "default_config";
    cv::Mat img;
    try{
        baas::init_globals();
        filesystem::path curr = filesystem::current_path();
        cout <<shared_memory_exists("test") << endl;
//        img = cv::imread("game_update.png");
        auto sm = (baas::Shared_Memory*)get_shared_memory("test", 1280*720*3, img.data);
        auto t1 = std::chrono::high_resolution_clock::now();
        img = cv::Mat(720, 1280, CV_8UC3, sm->get_data());
//        cv::imshow("img", img);
//        cv::waitKey(0);
////        Shared_Memory* sm = new Shared_Memory("test", 1280*720*3);
//        auto t1 = std::chrono::high_resolution_clock::now();
//        img = cv::Mat(720, 1280, CV_8UC3, sm->get_data());
//
//        cout << "time: " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - t1).count() << "us" << endl;
//        cv::imshow("img", img);
//        cv::waitKey(0);
//        system("pause");
//        img = cv::Mat(720, 1280, CV_8UC3, get_shared_memory_data(sm));
//        cout << "time: " << BAASUtil::getCurrentTimeMS() - t1 << endl;
//        cv::imshow("img", img);
//        cv::waitKey(0);
//        return 0;
//        system("pause");
        baas::BAAS::check_config(config_name);
        baas::BAAS baas(config_name);
//        return 0;
        baas::BAASConfig config;
//        img = cv::imread("1.png");
        baas::global_setting->show();
//        BAASConnection* conn = baas.get_connection();
//        baas.update_screenshot_array();
//        baas.get_latest_screenshot(img);

//        cv::imshow("img", img);
//        cv::waitKey(0);

//        cv::imwrite("1.png", img);
//        cv::imshow("img", img);
//          img = cv::imread("NUM.png");
//        cv::waitKey(0);
//        string name = "rank-down";
//        BAASRectangle region = {100, 291, 650, 363};
//        BAASDevelopUtils::extract_image_rgb_range(img, name, region, {200, 200, 200}, {255, 255, 255});
        baas.solve("restart");
        baas.solve("collect_activity_fee");
        baas.solve("mail");
        baas.solve("work");
        baas.solve("competition");
        baas.solve("collect_reward");
//        cv::imshow("img", img);
//        cv::waitKey(0
        baas::baas_ocr->init("zh-cn");
        baas::baas_ocr->init("zh-tw");
        baas::baas_ocr->init("en-us");
        baas::baas_ocr->init("ja-jp");
        baas::baas_ocr->init("ko-kr");
        baas::baas_ocr->init("ru-ru");
//        baas_ocr->test_ocr();
        baas::OcrResult result;
        baas::TextLine result2;
        std::string a = "1234567890/";
        baas.update_screenshot_array();
        baas::BAASRectangle region_ap = {345, 32, 452, 53};
        baas.ocr_for_single_line("zh-cn", result2, region_ap, "AP", a);
        baas::BAASRectangle region_diamond = {549, 30, 663, 63};
        baas.ocr_for_single_line("zh-cn", result2, region_diamond, "Diamond", "1234567890,");
        json j;
//        BAASOCR::ocrResult2json(result, j);
//        cout << j.dump(4) << endl;
//        cv::imshow("img", result.boxImg);
        cv::waitKey(0);
//            BAASUtil::stringReplace("/", "_", result2.text);
//            cv::imwrite(result2.text + ".png", img);
            }
//        for(int i = 1; i <= 10; ++i) {
//        }

//        for (int i = 1; i<=10; ++i) {
//            baas_ocr->ocr("en-us", img, result);
//        }
//        baas_ocr->ocr("en-us", img, result);
//        for (int i = 1; i<=10; ++i) {
//            baas.update_screenshot_array();
//            baas.get_latest_screenshot(img);
//            cv::imwrite(to_string(i) + ".png", img);
//            imgName = to_string(i) + ".png";
//
//
//        }
            catch (const std::exception& e){
                baas::BAASGlobalLogger->BAASInfo(e.what());
            }

            return 0;
        }


//
//
//
//int main() {
//    system("chcp 65001");
//    using namespace httplib;
//    httplib::Client cli("http://localhost:1234");
//
//    httplib::Params para;
////    para.emplace("name", "value");
////    auto res = cli.Get("/path", para);
//    auto res = cli.Post("/path", "text", "text/plain");
//
//
//    return 0;
//    try {
//        init_globals();
//        string config_name = "default_config";
//        cv::Mat img;
//
//
//        BAAS baas(config_name);
//        BAASConfig config;
//        BAASConnection* conn = baas.get_connection();
//
////        BAASLdopengl* ldopengl = BAASLdopengl::get_instance(conn);
////        auto t1 = std::chrono::high_resolution_clock::now();
////        ldopengl->screenshot(img);
////        cout << "time: " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - t1).count() << "us" << endl;
////
////        cv::imshow("img", img);
////        cv::waitKey(0);
//
//        baas.update_screenshot_array();
//        baas.get_latest_screenshot(img);
//        Shared_Memory* img_mem_buf = Shared_Memory::create_shared_memory("img", img.cols*img.rows*3, img.data);
////        auto t1 = std::chrono::high_resolution_clock::now();
////        for(int i = 1; i <= 10; ++i) {
////            img_mem_buf->put_data(img.data, img.cols*img.rows*3);
////        }
////        cout << "time: " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - t1).count() << "us" << endl;
////        system("pause");
////        return 0;
////        cout<<&img.data<<endl;
////        auto ptr = img.data;
////        cv::Mat img2 = cv::Mat(img.rows, img.cols, CV_8UC3, ptr);
////        cv::imshow("img", img2);
////        cv::waitKey(0);
////        system("pause");
////        vector<int> round_type;
////        baas.get_each_round_type(round_type);
////        cout << round_type.size() << endl;
////        std::map<int, string> round_type_map = {
////                {1, "red"},
////                {2, "blue"},
////                {3, "yellow"}
////        };
////        for(int i = 0; i < round_type.size(); i++){
////            cout << round_type_map[round_type[i]] << endl;
////        }
////        return 0;
////        while(true){
////            try{
////                baas.solve_procedure("COMPETITION_SOLVE", true);
////            }
////            catch (const std::exception& e){
////                BAASGlobalLogger->BAASInfo(e.what());
////            }
////        }
//
////
////        BAASConnection* connection = baas.get_connection();
////        connection->start_self();
////        resource->show();
//
//
////        config_name = "resource\\module_usage\\main_page.json";
////        BAASConfig procedure(config_name, baas.get_logger());
////        BAASConfig c = BAASConfig(procedure.get<json>("UI-GO-TO_main_page_home"), baas.get_logger());
//
//        baas.get_logger()->hr("ISA Exited");
//        }
////        resource->get(connection.get_server(), connection.get_language(), "common", "back", img);
////        cv::Mat mast;
////        BAASRectangle region = {0, 0, img.cols, img.rows};
////        BAASImageUtil::gen_not_black_region_mask(img, mast, region);
//
//
////        return 0;
//
////    }
//    catch (const std::exception& e){
//        BAASGlobalLogger->BAASInfo(e.what());
//    }
//
//    return 0;
//}
/*
//    y -->   601 ,  662
//    x -->   847 , 932
//            946, 1038
//            1048, 1130
//
//            883, 688
//            985, 621
//            1087, 622
//
//
//*/