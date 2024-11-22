//
// Created by pc on 2024/9/28.
//

#include <windows.h>
#include <iostream>
#include <chrono>
#include "BAASExternalIPC.h"

std::map<std::string, Shared_Memory*> Shared_Memory::baas_shared_memories;

Shared_Memory::Shared_Memory(const std::string &name, int size) {
    this->name = name;
    this->size = size;
    this->hMapFile = CreateFileMapping(
            INVALID_HANDLE_VALUE,   // 使用虚拟内存
            nullptr,                   // 默认安全属性
            PAGE_READWRITE,         // 可读可写
            0,                      // 高位文件大小
            size,                    // 共享内存大小 (256 bytes)
            name.c_str() // 共享内存名称
    );
    if (hMapFile == nullptr) {
        std::string msg = "CreateFileMapping failed" + std::to_string(GetLastError());
        throw Shared_Memory_Error(msg.c_str());
    }

    this->pBuf = MapViewOfFile(
            hMapFile,               // 共享内存句柄
            FILE_MAP_WRITE,         // 可写访问
            0,                      // 文件偏移的高位
            0,                      // 文件偏移的低位
            size                     // 映射的大小
    );
    if (pBuf == nullptr) {
        std::string msg = "MapViewOfFile failed" + std::to_string(GetLastError());
        throw Shared_Memory_Error(msg.c_str());
    }
}

int Shared_Memory::put_data(const unsigned char *data, int sz){
    if (sz > size) return -1;
    auto t1 = std::chrono::high_resolution_clock::now();
    memcpy(pBuf, data, size);
    std::cout << "time: " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - t1).count() << "us" << std::endl;
    return 0;
}

Shared_Memory *Shared_Memory::create_shared_memory(const std::string& name, int size,const unsigned char *data) {
    auto it = baas_shared_memories.find(name);
    if(it != baas_shared_memories.end()) {
        return nullptr;
    }
    return new Shared_Memory(name, size, data);
}

int Shared_Memory::release_shared_memory(const std::string& name) {
    auto it = baas_shared_memories.find(name);

    if(it == baas_shared_memories.end()) {
        return -1;
    }

    UnmapViewOfFile(it->second->pBuf);
    CloseHandle(it->second->hMapFile);
    baas_shared_memories.erase(it);
    return 0;
}

Shared_Memory* Shared_Memory::get_shared_memory(const std::string& name) {
    auto it = baas_shared_memories.find(name);
    if(it == baas_shared_memories.end()) {
        return nullptr;
    }
    return it->second;
}

Shared_Memory::~Shared_Memory() {

}

Shared_Memory::Shared_Memory(const std::string &name, int size,const unsigned char *data) {
    this->name = name;
    this->size = size;
    this->hMapFile = CreateFileMapping(
            INVALID_HANDLE_VALUE,   // 使用虚拟内存
            nullptr,                   // 默认安全属性
            PAGE_READWRITE,         // 可读可写
            0,                      // 高位文件大小
            size,                    // 共享内存大小 (256 bytes)
            name.c_str() // 共享内存名称
    );
    if (hMapFile == nullptr) {
        std::string msg = "CreateFileMapping failed" + std::to_string(GetLastError());
        throw Shared_Memory_Error(msg.c_str());
    }
    this->pBuf = MapViewOfFile(
            hMapFile,               // 共享内存句柄
            FILE_MAP_WRITE,         // 可写访问
            0,                      // 文件偏移的高位
            0,                      // 文件偏移的低位
            size                     // 映射的大小
    );
    if (pBuf == nullptr) {
        std::string msg = "MapViewOfFile failed" + std::to_string(GetLastError());
        throw Shared_Memory_Error(msg.c_str());
    }
    put_data(data, size);
}

unsigned char *Shared_Memory::get_data() {
    return (unsigned char*)pBuf;
}

