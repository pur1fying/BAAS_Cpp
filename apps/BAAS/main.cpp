#include "BAAS.h"
#include "utils.h"
#include "module/auto_fight/auto_fight.h"
#include "ocr/BAASOCR.h"
#include "feature/BAASFeature.h"
#include "queue"
#include "BAASDevelopUtils.h"
#include <windows.h>
#include <iostream>

#include <yolo/BAAS_yolo.h>

#pragma comment(lib, "ws2_32.lib")

using namespace cv;
using namespace std;
using namespace baas;
using json = nlohmann::json;

int main(int argc, char **argv)
{
    system("chcp 65001");
    string config_name = "default_config";
    cv::Mat img;
    try {

        init_globals();
        BAASFeature::show();
        baas::BAAS::check_config(config_name);
        BAAS baas(config_name);
//        int count = 1000;
//        long long t_total = 0;
//        for (int i = 1 ;i <= count; ++i) {
//            auto t1 = BAASUtil::getCurrentTimeMS();
//
//            baas.click(640, 360);
//            auto t2 = BAASUtil::getCurrentTimeMS();
//            t_total+= (t2 - t1);
//        }
//        baas.get_logger()->BAASInfo("Click ave time: " + std::to_string(t_total / count) + "ms");
        long long benchmark_run_t = 10010; // run for 10 seconds
        int d_update_total_t = 0, screenshot_total_t = 0;
        int total_frame_count = 0;
        long long benchmark_start_time = baas::BAASUtil::getCurrentTimeMS();
        AutoFight fight(&baas);
        fight.init_workflow();
        fight.set_data_updater_mask(0b1111111);
        fight.set_boss_health_update_flag(0b100);
        fight.set_skill_cost_update_flag(0b100);
        while (1) {
            long long loop_start_t = baas::BAASUtil::getCurrentTimeMS();
            if(loop_start_t - benchmark_start_time > benchmark_run_t) {
                baas::BAASGlobalLogger->BAASInfo("Benchmark Run Time : " + std::to_string(benchmark_run_t / 1000) + "s, Exit.");
                break;
            }
            fight.update_screenshot();
            fight.update_data();
            total_frame_count++;
        }

        baas::BAASGlobalLogger->BAASInfo("Fps : " + std::to_string(total_frame_count / (benchmark_run_t / 1000.0)));
//        fight.init_workflow();
//        fight.display_cond_idx_name_map();
//        fight.display_all_state();
//        fight.display_all_cond_info();
//        fight.start_state_transition();


//        baas.update_screenshot_array();
//
//        std::vector<std::string> names = {
//                "Koharu",
//                "Eimi",
//                "Fuuka (New Year)"
//        };
//        std::vector<BAASRectangle> regions = {
//                SKILL1_RIGHT,
////                SKILL2_RIGHT,
////                SKILL3_RIGHT
//        };
//        std::vector<int> type = {
//                SKILL_RIGHT,
////                SKILL_RIGHT,
////                SKILL_RIGHT
//        };
//        for (int i = 0; i < names.size(); ++i) {
//            BAASDevelopUtils::shotStudentSkill(&baas, names[i], regions[i], type[i]);
//        }
//        return 0;
//        std::vector<std::string> languages = {"en-us", "zh-cn"};
//        baas_ocr->init(languages);
//        BAASDevelopUtils::fight_screenshot_extract(
//                &baas,
//                screenshot_extract_params(
//                        R"(D:\github\datasets\baas\raw)",
//                        0.0,
//                        0.5,
//                        60,
//                        false,
//                        60
//                )
//        );
//        return 0;



//        fight.set_data_updater_mask(0b1111111);

//        fight.set_skill_slot_possible_templates(0, {5, 4, 3, 2, 1, 0});
//        fight.set_skill_slot_possible_templates(1, {5, 4, 3, 2, 0, 1});
//        fight.set_skill_slot_possible_templates(2, {5, 4, 3, 1, 0, 2});
//        fight.set_skill_slot_possible_templates(0, {0, 1, 2});
//        fight.set_skill_slot_possible_templates(1, {0, 1, 2});
//        fight.set_skill_slot_possible_templates(2, {0, 1, 2});
//        auto start = BAASUtil::getCurrentTimeMS();
//        int frame_count = 0;
//
//        fight.set_boss_health_update_flag(0b100);
//        fight.set_skill_cost_update_flag(0b111);
//        cv::Mat img;
////
//        while(1){
//            baas.update_screenshot_array();
//            auto st = BAASUtil::getCurrentTimeMS();
//            baas.reset_all_feature();
////            fight.update_screenshot();
////            if (!baas.feature_appear("fight_pause-button_appear")) {
////                continue;
////            }
//
//            fight.reset_data();
//            fight.update_data();
//            auto end = BAASUtil::getCurrentTimeMS();
//            baas.get_logger()->BAASInfo("Update data time: " + std::to_string(end - st) + "ms");
//            fight.display_screenshot_extracted_data();
//
//            baas.get_latest_screenshot(img);
//            frame_count++;
//            if (BAASUtil::getCurrentTimeMS() - start > 1000) {
//                start = BAASUtil::getCurrentTimeMS();
//                baas.get_logger()->BAASInfo("Process frame : " + std::to_string(frame_count) + " in 1s.");
//                frame_count = 0;
//            }
//        }
    }
    catch (const std::exception& e) {
        BAASGlobalLogger->BAASInfo(e.what());

    }

    system("pause");
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