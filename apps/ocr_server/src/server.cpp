//
// Created by pc on 2025/2/25.
//
#include "server.h"
#include "BAASLogger.h"
#include "config/BAASGlobalSetting.h"
#include "ocr/BAASOCR.h"

#include "BAASExternalIPC.h"

using namespace baas;

OCR_NAMESPACE_BEGIN

std::vector<std::string> Server::image_pass_method_names = {
    "shared_memory",
    "post file",
    "local file"
};

Server::Server(){
    BAASGlobalLogger->hr("BAAS Ocr Server Init.");

    host = global_setting->getString("/ocr/server/host", "localhost");
    port = global_setting->getInt("/ocr/server/port", 1145);

    BAASGlobalLogger->BAASInfo("Server serial : " + host + ":" + std::to_string(port));
}

void Server::start()
{
    svr.Post("/create_shared_memory", [](const httplib::Request &req, httplib::Response &res) {
        BAAS_OCR::Server::handle_create_shared_memory(req, res);
    });
    svr.Post("/release_shared_memory", [](const httplib::Request &req, httplib::Response &res) {
        BAAS_OCR::Server::handle_release_shared_memory(req, res);
    });
    svr.Post("/init_model", [](const httplib::Request &req, httplib::Response &res) {
        BAAS_OCR::Server::handle_init_model(req, res);
    });
    svr.Post("/release_model", [](const httplib::Request &req, httplib::Response &res) {
        BAAS_OCR::Server::handle_release_model(req, res);
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
    svr.Post("/enable_thread_pool", [](const httplib::Request &req, httplib::Response &res) {
        BAAS_OCR::Server::handle_enable_thread_pool(req, res);
    });
    svr.Post("/disable_thread_pool", [](const httplib::Request &req, httplib::Response &res) {
        BAAS_OCR::Server::handle_disable_thread_pool(req, res);
    });
    svr.Post("/test", [](const httplib::Request &req, httplib::Response &res) {
        BAAS_OCR::Server::handler_test(req, res);
    });
    BAASGlobalLogger->hr("Server start.");
    svr.listen(host, port);
}

void Server::handle_init_model(
        const httplib::Request &req,
        httplib::Response &res
)
{
    long long t_start = BAASUtil::getCurrentTimeMS();
    BAASGlobalLogger->sub_title("Init Model");

    BAASConfig temp = BAASConfig(nlohmann::json::parse(req.body), (BAASLogger*)BAASGlobalLogger);
    out_req_params(temp.get_config());

    int gpu_id, num_thread;
    bool EnableCpuMemoryArena;
    std::vector<std::string> languages;
    try{
        nlohmann::json j_ret;
        languages = temp.get<std::vector<std::string>>("language");
        gpu_id = temp.getInt("gpu_id", -1);
        num_thread = temp.getInt("num_thread", 4);
        EnableCpuMemoryArena = temp.getBool("EnableCpuMemoryArena", false);
        std::vector<int> ret = baas_ocr->init(languages, gpu_id, num_thread, EnableCpuMemoryArena);
        long long t_end = BAASUtil::getCurrentTimeMS();
        j_ret["ret"] = ret;
        j_ret["time"] = int(t_end - t_start);
        res.status = 200;
        res.set_content(j_ret.dump(), "application/json");
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

void Server::handle_release_model(
        const httplib::Request &req,
        httplib::Response &res
)
{
    long long t_start = BAASUtil::getCurrentTimeMS();
    BAASGlobalLogger->sub_title("Release Model");

    nlohmann::json body = nlohmann::json::parse(req.body);
    out_req_params(body);

    BAASConfig temp = BAASConfig(body, (BAASLogger*)BAASGlobalLogger);
    std::vector<std::string> languages;
    std::vector<bool> ret;
    try{
        nlohmann::json j_ret;
        languages = temp.get<std::vector<std::string>>("language");
        ret = baas_ocr->release(languages);
        long long t_end = BAASUtil::getCurrentTimeMS();
        res.status = 200;
        j_ret["ret"] = ret;
        j_ret["time"] = int(t_end - t_start);
        res.set_content(j_ret.dump(), "application/json");
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
    try{
        long long t_start = BAASUtil::getCurrentTimeMS();
        BAASGlobalLogger->sub_title("OCR");
        BAASConfig temp;
        if (req.has_file("data")){
            temp =  BAASConfig(
                    nlohmann::json::parse(req.get_file_value("data").content),
                    (BAASLogger*)BAASGlobalLogger
            );
        }
        else{
            temp = BAASConfig(
                    nlohmann::json::parse(req.body),
                    (BAASLogger*)BAASGlobalLogger
            );
        }
        out_req_params(temp.get_config());

        if (!temp.contains("language")){
            set_error_response(res, "Request must contains 'language' key.");
            return;
        }
        if(temp.value_type("language") != nlohmann::detail::value_t::string){
            set_error_response(res, "Value type of 'language' must be string.");
            return;
        }
        std::string language = temp.getString("language");
        if (!baas_ocr->language_inited(language)){
            set_error_response(res, "Language [ " + language + " ] not inited.");
            return;
        }

        // get img
        cv::Mat image;
        BAASConfig image_info;
        temp.getBAASConfig("image", image_info);

        if (req_get_image(req, image_info, image)) {
            set_error_response(res, "Failed to get image.");
            return;
        }

        std::string candidates = temp.getString("candidates", "");

        // ocr
        OcrResult result;
        auto t_mid = BAASUtil::getCurrentTimeMS();
        baas_ocr->ocr(language, image, result, (BAASLogger*)BAASGlobalLogger, candidates);
        BAASGlobalLogger->BAASInfo("OCR time : " + std::to_string(BAASUtil::getCurrentTimeMS() - t_mid) + "ms");

        // return
        nlohmann::json j_ret;
        auto ret_options = temp.getUInt8("ret_options", 0b111);
        BAASGlobalLogger->BAASInfo("ret_options : " + std::to_string(ret_options));
        baas::BAASOCR::ocrResult2json(result, j_ret, ret_options);
        auto t_end = BAASUtil::getCurrentTimeMS();
        j_ret["time"] = int(t_end - t_start);
        res.status = 200;
        res.set_content(j_ret.dump(), "application/json");
        return;
    } catch (std::exception &e){
        set_error_response(res, std::string(e.what()));
        return;
    }
}

void Server::handle_ocr_for_single_line(
        const httplib::Request &req,
        httplib::Response &res
)
{
    try{
        long long t_start = BAASUtil::getCurrentTimeMS();
        BAASGlobalLogger->sub_title("OCR for single line");
        BAASConfig temp;
        if (req.has_file("data")){
            temp =  BAASConfig(
                    nlohmann::json::parse(req.get_file_value("data").content),
                    (BAASLogger*)BAASGlobalLogger
            );
        }
        else{
            temp = BAASConfig(
                    nlohmann::json::parse(req.body),
                    (BAASLogger*)BAASGlobalLogger
            );
        }

        out_req_params(temp.get_config());
        if (!temp.contains("language")){
            set_error_response(res, "Request must contains 'language' key.");
            return;
        }
        if(temp.value_type("language") != nlohmann::detail::value_t::string){
            set_error_response(res, "Value type of 'language' must be string.");
            return;
        }
        std::string language = temp.getString("language");
        if (!baas_ocr->language_inited(language)){
            set_error_response(res, "Language [ " + language + " ] not inited.");
            return;
        }

        // get img
        cv::Mat image;
        BAASConfig image_info;
        temp.getBAASConfig("image", image_info);

        if (req_get_image(req, image_info, image)) {
            set_error_response(res, "Failed to get image.");
            return;
        }
        std::string candidates = temp.getString("candidates", "");

        // ocr for single line
        TextLine result;
        baas_ocr->ocr_for_single_line(language, image, result, "", (BAASLogger*)BAASGlobalLogger, candidates);
        int ocr_time = int(result.time);
        BAASGlobalLogger->BAASInfo("OCR time : " + std::to_string(ocr_time) + "ms");

        // return
        nlohmann::json j_ret;
        auto t_end = BAASUtil::getCurrentTimeMS();
        j_ret["ocr_time"] = ocr_time;
        j_ret["text"] = result.text;
        j_ret["char_scores"] = result.charScores;
        j_ret["time"] = t_end - t_start;
        res.status = 200;
        res.set_content(j_ret.dump(), "application/json");
        return;
    } catch (std::exception &e){
        set_error_response(res, std::string(e.what()));
        return;
    }
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
    BAASGlobalLogger->sub_title("Release All model");
    baas::baas_ocr->release_all();
    res.status = 200;

    long long t_end = BAASUtil::getCurrentTimeMS();
    res.set_content(std::to_string(int(t_end - t_start)), "text/plain");
}

void Server::out_req_params(const nlohmann::json &j)
{
    BAASGlobalLogger->sub_title("Request Body");
    BAASGlobalLogger->BAASInfo("\n" + j.dump(4));
}

void Server::stop()
{
    svr.stop();
    BAASGlobalLogger->hr("Server stop.");
}

int Server::req_get_image(
        const httplib::Request &req,
        const BAASConfig &image_info,
        cv::Mat &ret
)
{
    long long t_start = BAASUtil::getCurrentTimeMS();
    if (!image_info.contains("pass_method")){
        BAASGlobalLogger->BAASError("Body must contains 'pass_method' key.");
        return 1;
    }
    if (image_info.value_type("pass_method") != nlohmann::detail::value_t::number_unsigned){
        BAASGlobalLogger->BAASError("Value type of 'pass_method' must be unsigned");
        return 1;
    }
    unsigned int pass_method = image_info.getUInt("pass_method");
    bool is_remote = req_is_remote(req);
    std::string pass_method_name = image_pass_method_names[pass_method];
    if(pass_method >= 3){
        BAASGlobalLogger->BAASError("Invalid pass method : " + std::to_string(pass_method));
        return 1;
    }
    if (is_remote and (pass_method == 0 or pass_method == 2) ){
        BAASGlobalLogger->BAASError("Remote request invalid pass_method : " + pass_method_name);
        return 1;
    }

    BAASGlobalLogger->BAASInfo("Pass method : " + pass_method_name);
    // shared memory
    if(pass_method == 0) {
        if (!image_info.contains("shared_memory_name")) {
            BAASGlobalLogger->BAASError("image_info must contains 'shared_memory_name'.");
            return 1;
        }
        if (image_info.value_type("shared_memory_name") != nlohmann::detail::value_t::string) {
            BAASGlobalLogger->BAASError("'/image_info/shared_memory_name' must be string.");
            return 1;
        }
        int x = image_info.getInt("/resolution/0", 0);
        int y = image_info.getInt("/resolution/1", 0);
        if (x == 0 or y == 0) {
            BAASGlobalLogger->BAASError("Resolution must be set.");
            return 1;
        }
        std::string shared_memory_name = image_info.getString("shared_memory_name");
        unsigned char *data = Shared_Memory::get_data_ptr(shared_memory_name);
        if (data == nullptr) {
            BAASGlobalLogger->BAASError("Failed to get shared memory data.");
            return 1;
        }
        size_t size = x * y * 3;
        size_t shm_size = Shared_Memory::get_shared_memory_size(shared_memory_name);
        if (size > shm_size) {
            BAASGlobalLogger->BAASError
                (
                "Required size [" + std::to_string(size) + "] "
                "is larger than shared memory size [" + std::to_string(shm_size) + "]."
                );
            return 1;
        }
        ret = cv::Mat(y, x, CV_8UC3, data);
    }
    // post file
    else if (pass_method == 1) {
        const auto& file = req.get_file_value("image");
        if (file.content.empty()){
            BAASGlobalLogger->BAASError("Request doesn't contain 'image' file.");
            return 1;
        }
        const auto* image_data = (const uchar*)file.content.data();
        ret = cv::imdecode(cv::Mat(1, int(file.content.size()), CV_8UC1, (void*)image_data), cv::IMREAD_COLOR);
    }
    // local file
    else {
        if (!image_info.contains("local_path")) {
            BAASGlobalLogger->BAASError("image_info must contains 'local_path'.");
            return 1;
        }
        if (image_info.value_type("local_path") != nlohmann::detail::value_t::string) {
            BAASGlobalLogger->BAASError("'/image_info/local_path' must be string.");
            return 1;
        }
        std::filesystem::path image_path = image_info.getPath("local_path");
        if (!std::filesystem::exists(image_path)) {
            BAASGlobalLogger->BAASError("Image file not found : " + image_path.string());
            return 1;
        }
        ret = cv::imread(image_path.string());
    }
    if (ret.empty())  {
        BAASGlobalLogger->BAASError("Failed to decode image.");
        return 1;
    }
    BAASGlobalLogger->BAASInfo("Decode image time : " + std::to_string(BAASUtil::getCurrentTimeMS() - t_start) + "ms");
    return 0;
}

bool Server::req_is_remote(const httplib::Request &req)
{
    std::string remote_ip = req.remote_addr;
    if (remote_ip == "127.0.0.1" || remote_ip == "::1") return false;
    else return true;

}

void Server::handler_test(
        const httplib::Request &req,
        httplib::Response &res
)
{
    BAASGlobalLogger->sub_title("Test");
    // 输出请求的基本信息
    std::cout << "Request Method: " << req.method << std::endl;
    std::cout << "Request Path: " << req.path << std::endl;
    // 输出请求头
    std::cout << "Request Headers:" << std::endl;
    for (const auto& header : req.headers) {
        std::cout << header.first << ": " << header.second << std::endl;
    }
    // 输出查询参数
    if (!req.params.empty()) {
        std::cout << "Request Params:" << std::endl;
        for (const auto& param : req.params) {
            std::cout << param.first << ": " << param.second << std::endl;
        }
    }
    // 输出请求体（如果有的话）
    if (!req.body.empty()) {
        std::cout << "Request Body: " << req.body << std::endl;
    }
    // 返回响应
    res.set_content("Request received", "text/plain");
}

void Server::set_error_response(
        httplib::Response &res,
        const std::string &msg
)
{
    BAASGlobalLogger->BAASError(msg);
    res.status = 400;
    res.set_content(msg, "text/plain");
}

void Server::handle_enable_thread_pool(
        const httplib::Request &req,
        httplib::Response &res
)
{
    BAASGlobalLogger->BAASInfo("Enable thread pool");

    BAASConfig temp = BAASConfig(nlohmann::json::parse(req.body), (BAASLogger*)BAASGlobalLogger);
    out_req_params(temp.get_config());

    if (!temp.contains("thread_count")){
        set_error_response(res, "Request must contains 'thread_count' key.");
        return;
    }
    if (temp.value_type("thread_count") != nlohmann::detail::value_t::number_unsigned){
        set_error_response(res, "Value type of 'thread_count' must be unsigned.");
        return;
    }
    auto num_thread = temp.getUInt("thread_count");
    BAASGlobalLogger->sub_title("thread_count : " + std::to_string(num_thread) + "");
    baas::BAASOCR::enable_thread_pool(num_thread);

    res.status = 200;
    res.set_content("Success.", "text/plain");
}

void Server::handle_disable_thread_pool(
        const httplib::Request &req,
        httplib::Response &res
)
{
    BAASGlobalLogger->sub_title("Disable thread pool");
    baas::BAASOCR::disable_thread_pool();
    res.status = 200;
    res.set_content("Success.", "text/plain");
}

void Server::handle_create_shared_memory(
        const httplib::Request &req,
        httplib::Response &res
)
{
    BAASGlobalLogger->sub_title("Ocr Create Shared Memory");
    BAASConfig temp = BAASConfig(nlohmann::json::parse(req.body), (BAASLogger*)BAASGlobalLogger);
    out_req_params(temp.get_config());

    if (!temp.contains("shared_memory_name")){
        set_error_response(res, "Request must contains 'shared_memory_name' key.");
        return;
    }
    if (temp.value_type("shared_memory_name") != nlohmann::detail::value_t::string) {
        set_error_response(res, "Value type of 'shared_memory_name' must be string.");
        return;
    }
    if (!temp.contains("size")){
        set_error_response(res, "Request must contains 'size' key.");
        return;
    }
    if (temp.value_type("size") != nlohmann::detail::value_t::number_unsigned){
        set_error_response(res, "Value type of 'size' must be unsigned.");
        return;
    }
    auto name = temp.getString("shared_memory_name");
    auto size = temp.getUInt("size");

    void* p = baas::Shared_Memory::get_shared_memory(name, size, nullptr);
    if (p == nullptr) {
        res.status = 400;
        res.set_content("Failed to create shared memory.", "text/plain");
        return;
    }
    res.status = 200;
    res.set_content("Success.", "text/plain");
}

void Server::handle_release_shared_memory(
        const httplib::Request &req,
        httplib::Response &res
)
{
    BAASGlobalLogger->sub_title("Ocr Release Shared Memory");
    BAASConfig temp = BAASConfig(nlohmann::json::parse(req.body), (BAASLogger*)BAASGlobalLogger);
    out_req_params(temp.get_config());

    if (!temp.contains("shared_memory_name")){
        set_error_response(res, "Request must contains 'shared_memory_name' key.");
        return;
    }
    if (temp.value_type("shared_memory_name") != nlohmann::detail::value_t::string) {
        set_error_response(res, "Value type of 'shared_memory_name' must be string.");
        return;
    }
    auto name = temp.getString("shared_memory_name");

    int ret = baas::Shared_Memory::release_shared_memory(name);
    if (ret == 0){
        res.status = 200;
        res.set_content("Success.", "text/plain");
    }
    else{
        res.status = 400;
        res.set_content("Shm not exists.", "text/plain");
    }
}


OCR_NAMESPACE_END