#include "BAAS.h"
#include "utils.h"
#include "module/auto_fight/auto_fight.h"
#include "ocr/BAASOCR.h"
#include "feature/BAASFeature.h"
#include "queue"
#include "BAASDevelopUtils.h"
#include <windows.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")

using namespace cv;
using namespace std;
using namespace baas;
using json = nlohmann::json;

int main(int argc, char **argv) {
    system("chcp 65001");
    string config_name = "default_config";
    cv::Mat img;
    try{
//        auto hDll = LoadLibraryA("D:\\github\\BAAS_Cpp\\dll\\Windows\\onnxruntime_providers_cuda.dll");
//        if (hDll == NULL) {
//            std::cerr << "无法加载DLL: " << GetLastError() << std::endl;
//            return 1;
//        }
        init_globals();
        BAASFeature::show();
        baas::BAAS::check_config(config_name);
        BAAS baas(config_name);

//        std::vector<std::string> names = {
//                "Ui",
//                "Nagisa",
//                "Himari"
//        };
//        std::vector<BAASRectangle> regions = {
//                SKILL1_LEFT,
//                SKILL2_LEFT,
//                SKILL3_LEFT
//        };
//        std::vector<int> type = {
//                SKILL_LEFT,
//                SKILL_LEFT,
//                SKILL_LEFT
//        };
//
//        for (int i = 0; i < names.size(); ++i) {
//            BAASDevelopUtils::shotStudentSkill(&baas, names[i], regions[i], type[i]);
//        }
//        return 0;
//        std::vector<std::string> languages = {"en-us", "zh-cn"};
//        baas_ocr->init(languages);
        AutoFight fight(&baas);
        fight.init_workflow();
        fight.init_data_updaters();
        fight.set_data_updater_mask(0b111111);

//        fight.set_skill_slot_possible_templates(0, {5, 4, 3, 2, 1, 0});
//        fight.set_skill_slot_possible_templates(1, {5, 4, 3, 2, 0, 1});
//        fight.set_skill_slot_possible_templates(2, {5, 4, 3, 1, 0, 2});
        fight.set_skill_slot_possible_templates(0, {0, 1, 2});
        fight.set_skill_slot_possible_templates(1, {0, 1, 2});
        fight.set_skill_slot_possible_templates(2, {0, 1 ,2});
        auto start = BAASUtil::getCurrentTimeMS();
        int frame_count = 0;

        fight.set_boss_health_update_flag(0b100);
        fight.set_skill_cost_update_flag(0b111);
        cv::Mat img;

        while(1){
//            baas.update_screenshot_array();
            auto st = BAASUtil::getCurrentTimeMS();
            baas.reset_all_feature();
            fight.update_screenshot();
//            if (!baas.feature_appear("fight_pause-button_appear")) {
//                continue;
//            }

            fight.reset_data();
            fight.update_data();
            auto end = BAASUtil::getCurrentTimeMS();
            baas.get_logger()->BAASInfo("Shot And Update data time: " + std::to_string(end - st) + "ms");
            fight.display_data();

            baas.get_latest_screenshot(img);
            frame_count++;
            if (BAASUtil::getCurrentTimeMS() - start > 1000) {
                start = BAASUtil::getCurrentTimeMS();
                baas.get_logger()->BAASInfo("Process frame : " + std::to_string(frame_count) + " in 1s.");
                frame_count = 0;
            }
        }
        system("pause");


        return 0;
        baas.update_screenshot_array();
        baas.get_latest_screenshot(img);

//        cv::imshow("img", img);
//        cv::waitKey(0);

//        string name = "pause-button";
//        BAASRectangle region = {1215, 29, 1251, 65};
//        BAASDevelopUtils::extract_image_rgb_range(img, name, region, {0, 0, 0}, {255, 255, 255});
//        cv::imshow("img", img);
//        cv::waitKey(0);
//        return 0;

        register_baas_module(&baas);
        baas.solve("AutoFight");

        return 0;

//        baas_ocr->test_images();
        OcrResult result;
        TextLine result2;
        std::string a = "1234567890/";
        baas.update_screenshot_array();
        BAASRectangle region_ap = {345, 32, 452, 53};
        baas.ocr_for_single_line("zh-cn", result2, region_ap, "AP", a);
        BAASRectangle region_diamond = {549, 30, 663, 63};
        baas.ocr_for_single_line("zh-cn", result2, region_diamond, "Diamond", "1234567890,");
        json j;
//        BAASOCR::ocrResult2json(result, j);
//        cout << j.dump(4) << endl;
//            BAASUtil::stringReplace("/", "_", result2.text);
//            cv::imwrite(result2.text + ".png", img);
            }
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
                BAASGlobalLogger->BAASInfo(e.what());

                system("pause");
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