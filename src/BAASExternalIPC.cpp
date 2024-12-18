//
// Created by pc on 2024/9/28.
//
#include "BAASExternalIPC.h"

#include <iostream>
#include <chrono>

#include <windows.h>

std::map<std::string, Shared_Memory*> shared_memory_map;

int Shared_Memory::put_data(const unsigned char *data, size_t sz){
    if (sz > size) return -1;
    auto t1 = std::chrono::high_resolution_clock::now();
    memcpy(pBuf, data, size);
    std::cout << "put data time: " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - t1).count() << "us" << std::endl;
    return 0;
}

Shared_Memory::~Shared_Memory() {
    if (shared_memory_map.find(name) != shared_memory_map.end())  {
        std::cout << "erase shared memory: " << name << std::endl;
        shared_memory_map.erase(name);
    }
    try{
        UnmapViewOfFile(pBuf);
        CloseHandle(hMapFile);
    }
    catch (const std::exception& e) {}
}

Shared_Memory::Shared_Memory(const std::string &name, size_t size,const unsigned char *data) {
    shared_memory_map[name] = this;
    this->name = name;
    bool exists = shared_memory_exists(name.c_str());
    if (exists == SHARED_MEMORY_EXISTS) {
        this->hMapFile = OpenFileMapping(
                FILE_MAP_ALL_ACCESS,
                FALSE,
                name.c_str()
        );
        if (hMapFile == nullptr) {
            std::string msg = "OpenFileMapping failed" + std::to_string(GetLastError());
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
            std::string msg = "CreateFileMapping failed" + std::to_string(GetLastError());
            std::cerr << msg << std::endl;

            throw Shared_Memory_Error(msg.c_str());
        }
        this->size = size;
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
        std::cerr << msg << std::endl;
        throw Shared_Memory_Error(msg.c_str());
    }

    if (data != nullptr) put_data(data, size);
}

unsigned char *Shared_Memory::get_data() {
    return (unsigned char*)pBuf;
}

void Shared_Memory::release() {
    UnmapViewOfFile(pBuf);
    CloseHandle(hMapFile);
}

size_t Shared_Memory::query_size(void *p_buf_ptr) {
    MEMORY_BASIC_INFORMATION mbi;
    VirtualQuery(p_buf_ptr, &mbi, sizeof(mbi));
    return mbi.RegionSize;
}


void *get_shared_memory(const char *name, size_t size, const unsigned char *data) {
    try {
        auto it = shared_memory_map.find(name);
        if (it != shared_memory_map.end()) {
            if (size != 0) it->second->put_data(data, size);
            return it->second;
        }
        auto shared_memory = new Shared_Memory(name, size, data);
        return shared_memory;
    } catch (Shared_Memory_Error& e) {
        std::cerr << e.what() << std::endl;
        return nullptr;
    }
}

int set_shared_memory_data(const char* name, size_t size, const unsigned char* data) {
    auto it = shared_memory_map.find(name);
    if (it == shared_memory_map.end()) return SHARED_MEMORY_NOT_EXISTS;
    return it->second->put_data(data, size);
}

int release_shared_memory(const char* name) {
    std::cout << "map size: " << shared_memory_map.size() << std::endl;
    auto it = shared_memory_map.find(name);
    if (it == shared_memory_map.end()) return 1;
    it->second->release();
    delete it->second;
    return 0;
}

int set_shared_memory_data(void *shared_memory, const unsigned char *data, size_t size) {
    return ((Shared_Memory*)shared_memory)->put_data(data, size);
}

int shared_memory_exists(const char* name) {
    HANDLE hFileMapping = OpenFileMapping(
            FILE_MAP_READ,
            FALSE,
            name
    );
    if (hFileMapping == nullptr) return SHARED_MEMORY_NOT_EXISTS;
    CloseHandle(hFileMapping);
    return SHARED_MEMORY_EXISTS;
}

size_t get_shared_memory_size(const char* name) {
    HANDLE hFileMapping = OpenFileMapping(
            FILE_MAP_READ,
            FALSE,
            name
    );
    if (hFileMapping == nullptr) return 0;

    void* pBuf = MapViewOfFile(
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

int get_shared_memory_data(const char* name,unsigned char * put_date_ptr,size_t size) {
    try{
        auto shared_memory = Shared_Memory(name, size);
        memcpy(put_date_ptr, shared_memory.get_data(), shared_memory.get_size());
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}

