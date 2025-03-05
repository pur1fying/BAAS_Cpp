//
// Created by pc on 2024/9/28.
//
#include "BAASExternalIPC.h"

#include <iostream>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#endif // _WIN32

BAAS_NAMESPACE_BEGIN

std::map<std::string, std::unique_ptr<Shared_Memory>> Shared_Memory::shm_map;

int Shared_Memory::put_data(
        const unsigned char *data,
        size_t sz
)
{
    if (sz > size) return -1;
    auto t1 = std::chrono::high_resolution_clock::now();
    memcpy(pBuf, data, size);
    std::cout << "put data time: " << std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now() - t1
    ).count() << "us" << std::endl;
    return 0;
}

Shared_Memory::~Shared_Memory()
{
    try {
        UnmapViewOfFile(pBuf);
        CloseHandle(hMapFile);
    }
    catch (const std::exception &e) {
        
    }
}

Shared_Memory::Shared_Memory(
        const std::string &name,
        size_t size,
        const unsigned char *data
)
{
    this->name = name;
    if (shared_memory_exists(name.c_str())) {
        this->hMapFile = OpenFileMapping(
                FILE_MAP_ALL_ACCESS,
                FALSE,
                name.c_str()
        );
        if (hMapFile == nullptr) {
            std::string msg = "OpenFileMapping failed " + std::to_string(GetLastError());
            std::cerr << msg << std::endl;
            throw Shared_Memory_Error(msg.c_str());
        }
        this->pBuf = MapViewOfFile(
                hMapFile,
                FILE_MAP_ALL_ACCESS,
                0,
                0,
                size
        );
        this->size = query_size(pBuf);  // adjust to actual size
    } else {
        this->hMapFile = CreateFileMapping(
                INVALID_HANDLE_VALUE,
                nullptr,
                PAGE_READWRITE,
                0,
                size,
                name.c_str()
        );
        if (hMapFile == nullptr) {
            std::string msg = "CreateFileMapping failed " + std::to_string(GetLastError());
            throw Shared_Memory_Error(msg.c_str());
        }
        this->size = size;  // create size
    }
    this->pBuf = MapViewOfFile(
            hMapFile,
            FILE_MAP_ALL_ACCESS,
            0,
            0,
            this->size
    );
    if (pBuf == nullptr) {
        std::string msg = "MapViewOfFile failed " + std::to_string(GetLastError());
        throw Shared_Memory_Error(msg.c_str());
    }

    if (data != nullptr) put_data(data, size);
}

unsigned char *Shared_Memory::get_data()
{
    return (unsigned char *) pBuf;
}

void Shared_Memory::release()
{
    UnmapViewOfFile(pBuf);
    CloseHandle(hMapFile);
}


size_t Shared_Memory::query_size(void *p_buf_ptr)
{
    MEMORY_BASIC_INFORMATION mbi;
    VirtualQuery(p_buf_ptr, &mbi, sizeof(mbi));
    return mbi.RegionSize;
}

void* Shared_Memory::get_shared_memory(
        const std::string &name,
        size_t size,
        const unsigned char *data
)
{
    try {
        auto it = shm_map.find(name);
        if (it != shm_map.end()) {
            if (size != 0 && data != nullptr) it->second->put_data(data, size);
            return it->second.get();
        }
        auto shared_memory = new Shared_Memory(name, size, data);
        shm_map[name] = std::unique_ptr<Shared_Memory>(shared_memory);
        return shared_memory;
    } catch (Shared_Memory_Error &e) {
        std::cerr << e.what() << std::endl;
        return nullptr;
    }
}

/*
 * Return:
 *         -1: shm size too small
 *          0: success
 *          1: shm not exists
 */
int Shared_Memory::set_shared_memory_data(
        const std::string &name,
        size_t size,
        const unsigned char *data
)
{
    auto it = shm_map.find(name);
    if (it == shm_map.end()) return 1;
    return it->second->put_data(data, size);

}

int Shared_Memory::get_shared_memory_data(
        const std::string &name,
        unsigned char* put_data_ptr,
        size_t size
)
{
    try {
        auto shared_memory = Shared_Memory(name, size);
        memcpy(put_data_ptr, shared_memory.get_data(), size);
        return 0;
    }
    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}

int Shared_Memory::release_shared_memory(const std::string &name)
{
    auto it = shm_map.find(name);
    if (it == shm_map.end()) return 1;
    it->second->release();
    it->second.reset();
    return 0;
}

size_t Shared_Memory::get_shared_memory_size(const std::string &name)
{
    auto it = shm_map.find(name);
    if (it == shm_map.end()) return 0;
    return it->second->get_size();
}

unsigned char* Shared_Memory::get_data_ptr(const std::string &name)
{
    auto it = shm_map.find(name);
    if (it == shm_map.end()) return nullptr;
    return it->second->get_data();
}

BAAS_NAMESPACE_END


void *get_shared_memory(
        const char *name,
        size_t size,
        const unsigned char *data
)
{
    return baas::Shared_Memory::get_shared_memory(name, size, data);
}



int set_shared_memory_data(
        const char *name,
        size_t size,
        const unsigned char *data
)
{
    return baas::Shared_Memory::set_shared_memory_data(name, size, data);
}

int release_shared_memory(const char *name)
{
    return baas::Shared_Memory::release_shared_memory(name);
}


bool shared_memory_exists(const char *name)
{
    HANDLE hFileMapping = OpenFileMapping(
            FILE_MAP_READ,
            FALSE,
            name
    );
    if (hFileMapping == nullptr) return false;
    CloseHandle(hFileMapping);
    return true;
}

size_t get_shared_memory_size(const char *name)
{
    HANDLE hFileMapping = OpenFileMapping(
            FILE_MAP_READ,
            FALSE,
            name
    );
    if (hFileMapping == nullptr) return 0;

    void *pBuf = MapViewOfFile(
            hFileMapping,
            FILE_MAP_READ,
            0,
            0,
            0
    );
    MEMORY_BASIC_INFORMATION mbi;
    VirtualQuery(pBuf, &mbi, sizeof(mbi));
    return mbi.RegionSize;
}

int get_shared_memory_data(
        const char *name,
        unsigned char *put_date_ptr,
        size_t size
)
{
    return baas::Shared_Memory::get_shared_memory_data(name, put_date_ptr, size);
}

