//
// Created by pc on 2024/9/28.
//
#include "BAASExternalIPC.h"

#include <iostream>
#include <chrono>

#ifdef _WIN32
    #include <windows.h>
#elif UNIX_LIKE_PLATFORM
    #include <fcntl.h>
    #include <sys/mman.h>
    #include <sys/stat.h>
    #include <unistd.h>
#endif // _WIN32



BAAS_NAMESPACE_BEGIN

#ifdef _WIN32
size_t query_shm_size(void *p_buf_ptr)
{
    MEMORY_BASIC_INFORMATION mbi;
    VirtualQuery(p_buf_ptr, &mbi, sizeof(mbi));
    return mbi.RegionSize;
}
#elif UNIX_LIKE_PLATFORM
size_t query_shm_size(int shm_fd)
{
    struct stat sb{};
    fstat(shm_fd, &sb);    
    return sb.st_size;
}

#endif // _WIN32


std::map<std::string, std::unique_ptr<Shared_Memory>> Shared_Memory::shm_map;

shm_core::shm_core(
        const std::string &name,
        size_t size,
        const unsigned char *data
)
{
    this->shm_name = name;
    if (shared_memory_exists(name.c_str())) {
        // deal with existing shm
#ifdef _WIN32
        this->hMapFile = OpenFileMapping(
                FILE_MAP_ALL_ACCESS,
                FALSE,
                name.c_str()
        );
        if (hMapFile == nullptr) {
            std::string msg = "OpenFileMapping failed " + std::to_string(GetLastError());
            throw Shared_Memory_Error(msg.c_str());
        }
        this->size = query_shm_size(pBuf);  // adjust to actual size
        this->pBuf = MapViewOfFile(
                hMapFile,
                FILE_MAP_ALL_ACCESS,
                0,
                0,
                0   // map whole file
        );
        if (pBuf == nullptr) {
            std::string msg = "MapViewOfFile failed " + std::to_string(GetLastError());
            throw Shared_Memory_Error(msg.c_str());
        }
#elif UNIX_LIKE_PLATFORM
        this->shm_fd = shm_open(name.c_str(), O_RDWR, 0);
        if (shm_fd == -1) {
            std::string msg = "shm_open failed " + std::to_string(errno);
            throw Shared_Memory_Error(msg.c_str());
        }
        this->size = query_shm_size(shm_fd);  
        this->shm_address = mmap(
                nullptr,
                this->size,
                PROT_READ | PROT_WRITE,
                MAP_SHARED,
                shm_fd,
                0
        );
        if (shm_address == MAP_FAILED) {
            std::string msg = "mmap failed " + std::to_string(errno);
            throw Shared_Memory_Error(msg.c_str());
        }
#endif // _WIN32
    }
    else {
        // create new shm
#ifdef _WIN32
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
        this->size = query_shm_size(hMapFile);  
    }
    this->pBuf = MapViewOfFile(
            hMapFile,
            FILE_MAP_ALL_ACCESS,
            0,
            0,
            0
    );
    if (pBuf == nullptr) {
        std::string msg = "MapViewOfFile failed " + std::to_string(GetLastError());
        throw Shared_Memory_Error(msg.c_str());
    }
#elif UNIX_LIKE_PLATFORM
        // create new shm
        this->shm_fd = shm_open(name.c_str(), O_CREAT | O_RDWR, 0666);
        if (shm_fd == -1) {
            std::string msg = "shm_open failed " + std::to_string(errno);
            throw Shared_Memory_Error(msg.c_str());
        }
        if (ftruncate(shm_fd, long(size)) == -1) {
            std::string msg = "ftruncate failed " + std::to_string(errno);
            throw Shared_Memory_Error(msg.c_str());
        }
        this->size = query_shm_size(shm_fd);  // adjust to actual size ( create size may be larger than required size )
        this->shm_address = mmap(
                nullptr,
                this->size,
                PROT_READ | PROT_WRITE,
                MAP_SHARED,
                shm_fd,
                0
        );
        if (shm_address == MAP_FAILED) {
            std::string msg = "mmap failed " + std::to_string(errno);
            throw Shared_Memory_Error(msg.c_str());
        }
    }

#endif // _WIN32
    if (data != nullptr) put_data(data, size);
}

int shm_core::put_data(
        const unsigned char *data,
        size_t sz
) const noexcept
{
    if (sz > size) return -1;
    auto t1 = std::chrono::high_resolution_clock::now();
#ifdef _WIN32
    memcpy(pBuf, data, size);
#elif UNIX_LIKE_PLATFORM
    memcpy(shm_address, data, size);
#endif // _WIN32
    std::cout << "put data time: " <<
    std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - t1).count()
    << "us" << std::endl;
    return 0;
}

void shm_core::safe_release() noexcept
{
#ifdef _WIN32
    UnmapViewOfFile(pBuf);
    CloseHandle(hMapFile);
#elif UNIX_LIKE_PLATFORM
    munmap(shm_address, size);
    close(shm_fd);
    shm_unlink(shm_name.c_str());
#endif // _WIN32
}


int Shared_Memory::put_data(
        const unsigned char *data,
        size_t sz
)
{
    return shm.put_data(data, sz);
}

Shared_Memory::~Shared_Memory()
{

}

Shared_Memory::Shared_Memory(
        const std::string &name,
        size_t size,
        const unsigned char *data
)
{
    shm = shm_core(name, size, data);
}

unsigned char *Shared_Memory::get_data()
{
    return (unsigned char *)(shm.get_data());
}

void Shared_Memory::release()
{
    shm.safe_release();
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
        Shared_Memory *shared_memory;
        try{
            shared_memory = new Shared_Memory(name, size, data);
        }catch (Shared_Memory_Error &e){
            return nullptr;
        }
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
) noexcept
{
    auto it = shm_map.find(name);
    if (it == shm_map.end()) return 1;
    return it->second->put_data(data, size);

}

int Shared_Memory::get_shared_memory_data(
        const std::string &name,
        unsigned char* put_data_ptr,
        size_t size
) noexcept
{
    try {
        Shared_Memory *shm = static_cast<Shared_Memory *>(get_shared_memory(name));
        memcpy(put_data_ptr, shm->get_data(), size);
        return 0;
    }
    catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}

int Shared_Memory::release_shared_memory(const std::string &name) noexcept
{
    auto it = shm_map.find(name);
    if (it == shm_map.end()) return 1;
    it->second->release();
    it->second.reset();
    shm_map.erase(it);
    return 0;
}

size_t Shared_Memory::get_shared_memory_size(const std::string &name) noexcept
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

void Shared_Memory::release_all()
{
    for (auto &it : shm_map) {
        it.second->release();
        it.second.reset();
    }
    shm_map.clear();
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


bool shared_memory_exists(const char *name) // check shm "name" exist
{
#ifdef _WIN32
    HANDLE hFileMapping = OpenFileMapping(
            FILE_MAP_READ,
            FALSE,
            name
    );
    if (hFileMapping == nullptr) return false;
    CloseHandle(hFileMapping);
#elif UNIX_LIKE_PLATFORM
    int shm_fd = shm_open(name, O_RDONLY, 0);
    if (shm_fd == -1) return false;
    else close(shm_fd);
#endif // _WIN32
    return true;
}

size_t get_shared_memory_size(const char *name)
{
    size_t sz=0;
#ifdef _WIN32
    HANDLE hFileMapping = OpenFileMapping(
            FILE_MAP_READ,
            FALSE,
            name
    );
    if (hFileMapping == nullptr) {
        // shm not exists
    }
    else {
        void *pBuf = MapViewOfFile(
            hFileMapping,
            FILE_MAP_READ,
            0,
            0,
            0
    );
        MEMORY_BASIC_INFORMATION mbi;
        VirtualQuery(pBuf, &mbi, sizeof(mbi));
        CloseHandle(hFileMapping);
        sz = mbi.RegionSize;
    }
#elif UNIX_LIKE_PLATFORM
    int shm_fd = shm_open(name, O_RDONLY, 0);
    if (shm_fd == -1){
        // shm not exists
    }
    else {
        struct stat sb{};
        fstat(shm_fd, &sb);
        close(shm_fd);
        sz = sb.st_size;
    }
#endif // _WIN32
    return sz;
}

int get_shared_memory_data(
        const char *name,
        unsigned char *put_date_ptr,
        size_t size
)
{
    return baas::Shared_Memory::get_shared_memory_data(name, put_date_ptr, size);
}


