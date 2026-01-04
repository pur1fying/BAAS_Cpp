//
// Created by Administrator on 2025/12/21.
//

#include <utils.h>
#include <android/exports.h>

#ifdef __ANDROID__

OCR_NAMESPACE_BEGIN

std::thread Android_global_server_thread;

OCR_NAMESPACE_END

void start_server(const char* res_dir)
{
    std::filesystem::path resource_path(res_dir);
    BAAS_OCR::_init(resource_path);
    BAAS_OCR::Android_global_server_thread = std::thread(BAAS_OCR::server_thread);
}

void stop_server()
{
    BAAS_OCR::server.stop();
    if (BAAS_OCR::Android_global_server_thread.joinable())
        BAAS_OCR::Android_global_server_thread.join();
    BAAS_OCR::_cleanup();
}


#endif // __ANDROID__