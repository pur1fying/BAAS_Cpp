//
// Created by pc on 2024/4/12.
//
#ifndef BAAS_CXX_REFACTOR_BAASEXCEPTIONS_H
#define BAAS_CXX_REFACTOR_BAASEXCEPTIONS_H
#include <iostream>
#include <stdexcept>
#include <string>
class ValueError: public std::exception {
public:
    explicit ValueError(const std::string msg): message(msg) {}
    const char* what() const noexcept override {
        return message.c_str();
    }
private:
    std::string message;
};
class RuntimeError: public std::exception {
public:
    explicit RuntimeError(const std::string msg): message(msg) {}
    const char* what() const noexcept override {
        return message.c_str();
    }
private:
    std::string message;
};
class ConnectionError: public std::exception {
public:
    explicit ConnectionError(const std::string msg): message(msg) {}
    const char* what() const noexcept override {
        return message.c_str();
    }
private:
    std::string message;
};
#endif //BAAS_CXX_REFACTOR_BAASEXCEPTIONS_H
