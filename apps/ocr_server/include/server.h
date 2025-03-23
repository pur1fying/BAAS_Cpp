//
// Created by pc on 2025/2/25.
//

#ifndef BAAS_OCR_SERVER_SERVER_H_
#define BAAS_OCR_SERVER_SERVER_H_

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <opencv2/opencv.hpp>

#include "definitions.h"
#include "config/BAASConfig.h"

OCR_NAMESPACE_BEGIN

class Server
{
public:
    Server();

    void init();

    void start();

    void stop();

    static void stop_server_thread(httplib::Server &svr);

    static void handle_create_shared_memory(const httplib::Request &req, httplib::Response &res);

    static void handle_release_shared_memory(const httplib::Request &req, httplib::Response &res);

    static void handle_enable_thread_pool(const httplib::Request &req, httplib::Response &res);

    static void handle_disable_thread_pool(const httplib::Request &req, httplib::Response &res);

    static void handle_init_model(const httplib::Request &req, httplib::Response &res);

    static void handle_release_model(const httplib::Request &req, httplib::Response &res);

    static void handle_release_all(const httplib::Request &req, httplib::Response &res);

    static void handle_ocr(const httplib::Request &req, httplib::Response &res);

    static void handle_ocr_for_single_line(const httplib::Request &req, httplib::Response &res);

    static void out_req_params(const httplib::Request &req);

    static void out_req_params(const nlohmann::json &j);

    static void set_error_response(httplib::Response &res, const std::string &msg);

    static void handler_test(const httplib::Request& req, httplib::Response& res);

    static int req_get_image(
            const httplib::Request &req,
            const baas::BAASConfig &image_info,
            cv::Mat &img
            );

    static bool req_is_remote(const httplib::Request &req);

private:
    httplib::Server svr;

    std::string host;

    int port;

    static std::vector<std::string> image_pass_method_names;
};

OCR_NAMESPACE_END
#endif // BAAS_OCR_SERVER_SERVER_H_
