//
// Created by pc on 2025/2/22.
//

#include <utils.h>
int main()
{
    BAAS_OCR::_init();
    std::thread server_thread(BAAS_OCR::server_thread);
    BAAS_OCR::handle_input();
    BAAS_OCR::server.stop();

    server_thread.join();
    BAAS_OCR::_cleanup();
    return 0;
}