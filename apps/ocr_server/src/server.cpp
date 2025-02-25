//
// Created by pc on 2025/2/25.
//
#include <server.h>
#include <BAASLogger.h>
#include <config/BAASGlobalSetting.h>
#include <ocr/BAASOCR.h>
using namespace baas;

OCR_NAMESPACE_BEGIN
Server::Server(){

    BAASGlobalLogger->hr("BAAS Ocr Server Init.");

    host = global_setting->getString("/ocr/server/host", "localhost");
    port = global_setting->getInt("/ocr/server/port", 1145);

    BAASGlobalLogger->BAASInfo("Server serial : " + host + ":" + std::to_string(port));

}
void Server::handle_init_model(
        const httplib::Request &req,
        httplib::Response &res
)
{
    long long t_start = BAASUtil::getCurrentTimeMS();
    BAASGlobalLogger->sub_title("Init Model");

    nlohmann::json body = nlohmann::json::parse(req.body);
    out_req_params(body);

    BAASConfig temp = BAASConfig(body, (BAASLogger*)BAASGlobalLogger);
    int gpu_id, num_thread;
    std::vector<std::string> languages;
    try{
        languages = temp.get<std::vector<std::string>>("language");
        gpu_id = temp.getInt("gpu_id", -1);
        num_thread = temp.getInt("num_thread", 4);
        std::vector<int> ret = baas_ocr->init(languages, gpu_id, num_thread);
        long long t_end = BAASUtil::getCurrentTimeMS();
        ret.emplace_back(int(t_end - t_start));
        res.status = 200;
        res.set_content(nlohmann::json(ret).dump(), "application/json");
        return;
    }
    catch (std::exception &e){
        BAASGlobalLogger->BAASError(e.what());
        res.status = 400;
        std::string msg = "Bad Request : " + std::string(e.what());
        res.set_content(msg, "text/plain");
        return;
    }
}
void Server::handle_ocr(
        const httplib::Request &req,
        httplib::Response &res
)
{
    BAASGlobalLogger->BAASInfo("ocr");
}

void Server::handle_ocr_for_single_line(
        const httplib::Request &req,
        httplib::Response &res
)
{
    BAASGlobalLogger->BAASInfo("ocr for single line");
    //    baas::baas_ocr->ocr_for_single_line();

}

void Server::start()
{
    svr.Post("/init_model", [](const httplib::Request &req, httplib::Response &res) {
        BAAS_OCR::Server::handle_init_model(req, res);
    });

    svr.Get("/release_all", [](const httplib::Request &req, httplib::Response &res) {
        BAAS_OCR::Server::handle_release_all(req, res);
    });

    svr.Post("/ocr", [](const httplib::Request &req, httplib::Response &res) {
        BAAS_OCR::Server::handle_ocr(req, res);
    });
    svr.Post("/ocr_for_single_line", [](const httplib::Request &req, httplib::Response &res) {
        BAAS_OCR::Server::handle_ocr_for_single_line(req, res);
    });
    BAASGlobalLogger->hr("Server start.");
    svr.listen(host, port);
}

void Server::out_req_params(const httplib::Request &req)
{
    nlohmann::json body = nlohmann::json::parse(req.body);
    BAASGlobalLogger->sub_title("Request Body");
    BAASGlobalLogger->BAASInfo(body.dump(4));
}

void Server::handle_release_all(
        const httplib::Request &req,
        httplib::Response &res
)
{
    long long t_start = BAASUtil::getCurrentTimeMS();
    BAASGlobalLogger->BAASInfo("Release All model");
    baas::baas_ocr->release_all();
    res.status = 200;
    long long t_end = BAASUtil::getCurrentTimeMS();
    res.set_content(std::to_string(int(t_end - t_start)), "text/plain");
}

void Server::out_req_params(const nlohmann::json &j)
{
    BAASGlobalLogger->sub_title("Request Body");
    BAASGlobalLogger->BAASInfo(j.dump(4));
}


OCR_NAMESPACE_END