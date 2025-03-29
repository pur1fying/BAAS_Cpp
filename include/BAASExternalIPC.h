//
// Created by pc on 2024/9/28.
//

#ifndef BAAS_BAASEXTERNALIPC_H_
#define BAAS_BAASEXTERNALIPC_H_

#include <map>
#include <string>
#include <memory>
#include "core_defines.h"

BAAS_NAMESPACE_BEGIN

size_t query_shm_size(void *p_buf_ptr);

struct shm_core{    // metadata of shm

public:
    shm_core() = default;

    explicit shm_core(const std::string& name, size_t size, const unsigned char *data = nullptr);

    int put_data(
            const unsigned char *data,
            size_t sz,
            size_t offset = 0
    ) const noexcept;

    void safe_release() noexcept;

    inline unsigned char *get_data() const noexcept{
#ifdef _WIN32
        return (unsigned char *)pBuf;
#elif UNIX_LIKE_PLATFORM
        return (unsigned char *)shm_address;
#endif // _WIN32
    }
#ifdef _WIN32
    void *hMapFile;

    void *pBuf;
#elif UNIX_LIKE_PLATFORM
    int shm_fd = 0;

    void * shm_address = nullptr;
#endif // _WIN32
    std::string shm_name;

    size_t size = 0;
};

/*
 * for BAAS to exchange image data between different processes
 * 1. Every shm is used by one BAAS instance
 * 2. every instance will write into shm in only one thread
 * 3. Max Image Size is fixed, so shm size is fixed
 */

class Shared_Memory {
public:
    static void * get_shared_memory(
            const std::string& name,
            size_t size = 0,
            const unsigned char *data = nullptr
    );

    static int set_shared_memory_data(
            const std::string& name,
            size_t size,
            const unsigned char *data,
            size_t offset = 0
    ) noexcept;

    static int release_shared_memory(const std::string& name) noexcept;

    static size_t get_shared_memory_size(const std::string& name) noexcept;

    static int get_shared_memory_data(
            const std::string& name,
            unsigned char* put_data_ptr,
            size_t size = 0,
            size_t offset = 0
    ) noexcept;

    static unsigned char *get_data_ptr(const std::string &name) noexcept;

    static void release_all() noexcept;

private:
    explicit Shared_Memory(
            const std::string &name,
            size_t size = 0,
            const unsigned char *data = nullptr
    );

    inline int put_data(
            const unsigned char *data,
            size_t size,
            size_t offset = 0
    ) const noexcept;

    inline void release() noexcept;

    inline unsigned char *get_data() const noexcept;

    shm_core shm;

    static std::map<std::string, std::unique_ptr<Shared_Memory>> shm_map;
public:
    ~Shared_Memory();

};

class Shared_Memory_Error : public std::exception {
public:
    explicit Shared_Memory_Error(const char *message) : message(message) {}

    const char *what() const noexcept override
    {
        return message;
    }

private:
    const char *message;
};

BAAS_NAMESPACE_END
#ifdef _WIN32
#ifdef BAAS_BUILD_DLL
    #define BAAS_API __declspec(dllexport)
#else
    #define BAAS_API __declspec(dllimport)
#endif
#endif

#if UNIX_LIKE_PLATFORM
#define BAAS_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif
BAAS_API void * get_shared_memory(
        const char *name,
        size_t size = 0,
        const unsigned char *data = nullptr
);

BAAS_API int set_shared_memory_data(
        const char *name,
        size_t size,
        const unsigned char *data
);

BAAS_API int release_shared_memory(const char *name);

BAAS_API bool shared_memory_exists(const char *name);

BAAS_API size_t get_shared_memory_size(const char *name);

BAAS_API int get_shared_memory_data(
        const char *name,
        unsigned char *data,
        size_t size = 0
);

#ifdef __cplusplus
}
#endif

#endif //BAAS_BAASEXTERNALIPC_H_
