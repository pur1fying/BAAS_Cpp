#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>

#include <jni.h>
#include <android/log.h>

#include <server.h>
#include "utils.h"

#include "BAASGlobals.h"
#include "ocr/BAASOCR.h"

#define LOG_TAG "ONNX_JNI"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)
//
// Created by pc on 2025/2/22.
//

using namespace BAAS_OCR;

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_myapplication_MainActivity_testONNXInit(JNIEnv* env, jobject, jstring jPath) {
    try {
        std::string result = std::filesystem::current_path();
        const char* path = env->GetStringUTFChars(jPath, nullptr);
        std::filesystem::current_path(path);

        BAAS_OCR::_init();
        std::vector<std::string> lang;
        lang.push_back("zh-cn");
        baas::baas_ocr->init(lang);
        cv::Mat img = cv::imread(baas::BAAS_PROJECT_DIR.string() + "/resource/ocr_models/test_images/1.jpg");

        baas::OcrResult out;
        auto t1 = std::chrono::high_resolution_clock::now();
        baas::baas_ocr->ocr("zh-cn", img, out);
        auto t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        LOGD("OCR Time: %lld ms", duration);

        return env->NewStringUTF(out.strRes.c_str());

//        std::thread server_thread(BAAS_OCR::server_thread);
//        BAAS_OCR::handle_input();
//        BAAS_OCR::server.stop();

//        server_thread.join();
        BAAS_OCR::_cleanup();

        return env->NewStringUTF("Finish.");
    }
    catch (std::exception &e){

        return env->NewStringUTF(e.what());
    }
    BAAS_OCR::_cleanup();

}