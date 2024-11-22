//
// Created by pc on 2024/9/28.
//

#ifndef BAAS_BAASEXTERNALIPC_H_
#define BAAS_BAASEXTERNALIPC_H_

#include <map>
#include <string>


class Shared_Memory {
public:
    static Shared_Memory* create_shared_memory(const std::string& name, int size,const unsigned char *data);

    static int release_shared_memory(const std::string& name);

    static Shared_Memory* get_shared_memory(const std::string& name);

    inline int put_data(const unsigned char *data, int size);

    unsigned char* get_data();

    ~Shared_Memory();

private:
    Shared_Memory(const std::string& name, int size);

    Shared_Memory(const std::string& name, int size,const unsigned char *data);

    void* hMapFile;

    void* pBuf;

    std::string name;

    int size;

    static std::map<std::string, Shared_Memory*> baas_shared_memories;

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

#endif //BAAS_BAASEXTERNALIPC_H_
