//
// Created by pc on 2024/5/27.
//

#ifndef BAAS_BAASADBUTILS_H_
#define BAAS_BAASADBUTILS_H_
#include "WinSock2.h"
#include "WS2tcpip.h"

#include <vector>
#include <fstream>

#include "BAASEmulatorController.h"
#include "BAASExceptions.h"
#include "BAASGlobals.h"
#include "BAASUtil.h"
#include "Network.h"

#define _OKAY "OKAY"
#define _FAIL "FAIL"
#define _STAT "STAT"

// Connection
class BAASAdbConnection {
public:
    static bool checkServer(std::string host, std::string port);

    BAASAdbConnection();

    BAASAdbConnection(const std::string &host, const std::string &port, double socketTimeout=3000.0);

    BAASAdbConnection(const std::string &serial, double socketTimeout=3000.0);

    std::string readFully(int length);

    bool readUntilClose(std::string &res);

    bool sendMessage(const std::string &data);

    bool checkOKAY();

    bool checkSTAT();

    std::string readAdbReturnMessage();

    SOCKET createSocket();

    SOCKET safeCreateSocket();

    SOCKET getConnection();

    ~BAASAdbConnection();

    bool setCloseSocketWhenDestruct(bool state);
protected:
    std::string host;

    std::string port;

    std::string serial;

    SOCKET connection;

    sockaddr_in serverAddr;

    double socketTimeout;

    bool closeSocketWhenDestruct = true;
};

class ConnectionRefusedError : public std::exception {
public:
    const char* what() const noexcept override {
        return "Connection refused";
    }
};
class AdbError : public std::exception {
public:
    AdbError() = default;
    AdbError(const char* msg) {
        message = msg;
    }
    const char* what() const noexcept override {
        if(message.size() == 0) return "Adb Error";
        return message.c_str();
    }
private:
    std::string message;
};

// BaseClient

class BAASAdbBaseClient{
public:
    BAASAdbBaseClient();

    BAASAdbBaseClient(const std::string serial="127.0.0.1:5037", double socketTimeout=3000.0);

    BAASAdbBaseClient(const std::string host="127.0.0.1", const std::string port="5037", double socketTimeout=3000.0);

    BAASAdbConnection* makeConnection(double socketTimeout=0);
    /*
        40 will match 1.0.40
        Returns:
            int
     */
    int serverVersion();

    bool serverKill();

    std::string connect(const std::string address, double socketTimeout=3000.0);

    std::string disconnect(const std::string address, double socketTimeout=3000.0);

    std::string getHost() const;

    std::string getPort() const;

    std::string getSerial() const;

protected:
    std::string host;

    std::string port;

    std::string serial;

    double socketTimeout;
};

// BaseDevice

class BAASAdbBaseDevice{
public:

    BAASAdbBaseDevice(BAASAdbBaseClient* client, const std::string& serial,const int transportId = 0);

    std::string getSerial() const;

    BAASAdbConnection *openTransport(const std::string& command="", double socketTimeout=60000.0);

    std::string getCommandResult(std::string command, double socketTimeout=60000.0);

    std::string getState(double socketTimeout=60000.0);

    std::string getSerialNo(double socketTimeout=60000.0);

    std::string getDevPath(double socketTimeout=60000.0);

    std::string getFeatures(double socketTimeout=60000.0);

    BAASAdbConnection* createConnection(const std::string network,const std::string address);

    BAASAdbConnection* shellStream(const std::string command, double socketTimeout = 3000.0);

    BAASAdbConnection* shellStream(const std::vector<std::string> commandList, double socketTimeout = 3000.0);

    bool shellBytes(const std::string command, std::string &out,double socketTimeout = 3000.0);

    bool shellBytes(const std::vector<std::string> commandList, std::string &out,double socketTimeout = 3000.0);

    BAASAdbConnection* prepareSync(const std::string path,const std::string cmd);

    int stat(const std::string path);

    int push(const std::string &src, const std::string &dst, const int mode, bool check);


protected:
    BAASAdbBaseClient* client;

    std::string serial;

    int transportId;
};


// Device
class BAASAdbDevice : public BAASAdbBaseDevice {
public:
    BAASAdbDevice(BAASAdbBaseClient *client, const std::string &serial, const int transportId = 0);
};


// Client
class BAASAdbClient: public BAASAdbBaseClient{
public:
    BAASAdbClient();

    std::vector<std::pair<std::string, int>> listDevices();

    std::vector<BAASAdbDevice*> iterDevice();

    BAASAdbDevice* device(const std::string serial);
};

extern BAASAdbClient adb;

#endif //BAAS_BAASADBUTILS_H_
