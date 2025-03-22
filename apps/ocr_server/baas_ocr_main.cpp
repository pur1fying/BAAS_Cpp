//
// Created by pc on 2025/2/22.
//

#include <utils.h>
#include <server.h>

using namespace baas;
int main()
{
    BAAS_OCR::_init();
    BAAS_OCR::Server server;
    server.start();

    BAAS_OCR::_cleanup();
    return 0;
}