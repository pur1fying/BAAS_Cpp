//
// Created by pc on 2024/9/28.
//

#ifndef BAAS_BAASEXTERNALIPC_H_
#define BAAS_BAASEXTERNALIPC_H_

#define SHARED_MEMORY_EXISTS 1
#define SHARED_MEMORY_NOT_EXISTS 2
#define SHARED_MEMORY_FAIL_OPEN_FILE_MAPPING 1
#define SHARED_MEMORY_FAIL_GET_MAP_VIEW 2

#include <map>
#include <string>
#include <memory>
#include "core_defines.h"

BAAS_NAMESPACE_BEGIN
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
            const unsigned char *data
    );

    static int release_shared_memory(const std::string& name);

    static size_t get_shared_memory_size(const std::string& name);

    static int get_shared_memory_data(
            const std::string& name,
            unsigned char* put_data_ptr,
            size_t size = 0
    );

    static unsigned char *get_data_ptr(const std::string &name);

private:
    explicit Shared_Memory(
            const std::string &name,
            size_t size = 0,
            const unsigned char *data = nullptr
    );

    inline int put_data(
            const unsigned char *data,
            size_t size
    );

    inline void release();

    inline unsigned char *get_data();

    inline size_t get_size() const
    {
        return size;
    }

    static size_t query_size(void *p_buf_ptr);

#ifdef _WIN32
    void *hMapFile;

    void *pBuf;     // virtual address of the shared memory, assigned by kernel
#endif // _WIN32
    std::string name;

    size_t size;

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

#ifdef BAAS_BUILD_DLL
#define BAAS_API __declspec(dllexport)
#else
#define BAAS_API __declspec(dllimport)
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
