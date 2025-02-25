//
// Created by pc on 2025/2/25.
//

#ifndef BAAS_OCR_SERVER_SERVER_H_
#define BAAS_OCR_SERVER_SERVER_H_
#include <httplib.h>
#include <nlohmann/json.hpp>

#include <definitions.h>

OCR_NAMESPACE_BEGIN

class Server
{
public:
    Server();

    void start();

    static void handle_init_model(const httplib::Request &req, httplib::Response &res);

    static void handle_release_all(const httplib::Request &req, httplib::Response &res);

    static void handle_ocr(const httplib::Request &req, httplib::Response &res);

    static void handle_ocr_for_single_line(const httplib::Request &req, httplib::Response &res);

    static void out_req_params(const httplib::Request &req);

    static void out_req_params(const nlohmann::json &j);

private:
    httplib::Server svr;

    std::string host;

    int port;
};

OCR_NAMESPACE_END
#endif // BAAS_OCR_SERVER_SERVER_H_
