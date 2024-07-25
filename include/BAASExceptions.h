//
// Created by pc on 2024/4/12.
//
#ifndef BAAS_BAASEXCEPTIONS_H_
#define BAAS_BAASEXCEPTIONS_H_
#include <iostream>
#include <stdexcept>
#include <string>

// value that is filled by user
class ValueError: public std::exception {
public:
    explicit ValueError(const std::string &msg) {
        message = msg;
    }
    [[nodiscard]] const char* what() const noexcept override {
        return message.c_str();
    }
private:
    std::string message;
};


// type error / network type / config type
class TypeError: public std::exception {
public:
    explicit TypeError(const std::string &msg) {
        message = msg;
    }

    [[nodiscard]] const char *what() const noexcept override {
        return message.c_str();
    }
private:
    std::string message;
};

// key in config is incorrect, error of baas
class KeyError: public std::exception {
public:
    explicit KeyError(const std::string &msg) {
        message = msg;
    }

    [[nodiscard]] const char *what() const noexcept override {
        return message.c_str();
    }
private:
    std::string message;
};


class RuntimeError: public std::exception {
public:
    explicit RuntimeError(const std::string& msg) {
        message = msg;
    }
    [[nodiscard]] const char* what() const noexcept override {
        return message.c_str();
    }
private:
    std::string message;
};


class ConnectionError: public std::exception {
public:
    explicit ConnectionError(const std::string msg): message(msg) {}
    [[nodiscard]] const char* what() const noexcept override {
        return message.c_str();
    }
private:
    std::string message;
};

// possible an error of baas which is caused by error path of file
class PathError: public std::exception {
public:
    explicit PathError(const std::string& msg) {
        message = msg;
    }
    [[nodiscard]] const char* what() const noexcept override {
        return message.c_str();
    }
private:
    std::string message;
};

//  Request human takeover
//  BAAS is unable to handle such error, probably because of wrong settings.

class RequestHumanTakeOver: public std::exception {
public:
    explicit RequestHumanTakeOver(const std::string& msg){
        message = msg;
    }
    [[nodiscard]] const char* what() const noexcept override {
        return message.c_str();
    }
private:
    std::string message;
};

class EmulatorNotRunningError: public std::exception {
public:
    explicit EmulatorNotRunningError(const std::string& msg){
        message = msg;
    }
    [[nodiscard]] const char* what() const noexcept override {
        return message.c_str();
    }
private:
    std::string message;
};
#endif //BAAS_BAASEXCEPTIONS_H_
