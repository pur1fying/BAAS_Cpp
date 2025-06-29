#include "BAAS.h"
#include "utils.h"
#include "module/auto_fight/auto_fight.h"
#include "ocr/BAASOCR.h"
#include "feature/BAASFeature.h"
#include "queue"
#include "BAASDevelopUtils.h"
#include <iostream>

#include "BAASImageResource.h"
#include <yolo/BAAS_yolo.h>

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
        // feature information
        BAASFeature::show();
        baas::BAAS::check_config(config_name);
        BAAS baas(config_name);
        cv::Mat img;
        baas.update_screenshot_array();
        baas.get_latest_screenshot(img);
        cv::imwrite("screenshot.png", img);
        // image resource
        resource->show();

        AutoFight fight(&baas);
        fight.init_workflow();
        // cond
        fight.display_cond_idx_name_map();
        fight.display_all_cond_info();
        // state
        fight.display_all_state();
        fight.start_state_transition();


// How To Release Hoshino ( Swimsuit ) Skill when Cost Reach 5.0
//        baas.update_screenshot_array();
//        std::vector<std::string> names = {
//                "Aru",
//                "Shiroko",
//                "Cherino"
//        };
//        std::vector<BAASRectangle> regions = {
//                SKILL1_FULL,
//                SKILL2_FULL,
//                SKILL3_FULL
//        };
//        std::vector<int> type = {
//                SKILL_FULL,
//                SKILL_FULL,
//                SKILL_FULL
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
        return 0;

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