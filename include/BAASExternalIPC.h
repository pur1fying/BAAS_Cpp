//
// Created by pc on 2024/9/28.
//

#ifndef BAAS_BAASEXTERNALIPC_H_
#define BAAS_BAASEXTERNALIPC_H_

#define SHARED_MEMORY_EXISTS 1
#define SHARED_MEMORY_NOT_EXISTS 0
#define SHARED_MEMORY_FAIL_OPEN_FILE_MAPPING 1
#define SHARED_MEMORY_FAIL_GET_MAP_VIEW 2

#include <map>
#include <string>

class Shared_Memory {
public:
    explicit Shared_Memory(const std::string &name, size_t size=0, const unsigned char *data=nullptr);

    inline int put_data(const unsigned char *data, size_t size);

    inline void release();

    inline unsigned char* get_data();

    inline size_t get_size() const {
        return size;
    }

    static size_t query_size(void* p_buf_ptr);

    ~Shared_Memory();

private:
    void* hMapFile;

    void* pBuf;     // virtual address of the shared memory, assigned by kernel

    std::string name;

    size_t size;
};

class Shared_Memory_Error : public std::exception {
public:
    explicit Shared_Memory_Error(const char* message) : message(message) {}

    const char* what() const noexcept override {
        return message;
    }

private:
    const char* message;
};

#ifdef BAAS_BUILD_DLL
#define BAAS_API __declspec(dllexport)
#else
#define BAAS_API __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C" {
#endif
    BAAS_API void* get_shared_memory(const char* name, size_t size=0, const unsigned char* data=nullptr);

    BAAS_API int set_shared_memory_data(const char* name, size_t size, const unsigned char* data);

    BAAS_API int release_shared_memory(const char* name);

    BAAS_API int shared_memory_exists(const char* name);

    BAAS_API size_t get_shared_memory_size(const char* name);

    BAAS_API int get_shared_memory_data(const char* name, unsigned char *data, size_t size=0);

    BAAS_API extern std::map<std::string, Shared_Memory*> shared_memory_map;

#ifdef __cplusplus
}
#endif

#endif //BAAS_BAASEXTERNALIPC_H_
