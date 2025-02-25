//
// Created by pc on 2024/5/27.
//

#ifndef BAAS_DEVICE_BAASADBUTILS_H_
#define BAAS_DEVICE_BAASADBUTILS_H_

#include "WinSock2.h"
#include "WS2tcpip.h"

#include <vector>
#include <fstream>

#include "BAASExceptions.h"
#include "BAASUtil.h"

#define OKAY "OKAY"
#define FAIL "FAIL"
#define STAT "STAT"

BAAS_NAMESPACE_BEGIN

class Network {
public:
    static const std::string TCP;
    static const std::string UNIX;
    static const std::string DEV;
    static const std::string LOCAL;
    static const std::string LOCAL_RESERVED;
    static const std::string LOCAL_FILESYSTEM;
    static const std::string LOCAL_ABSTRACT;
};

class BAASAdbConnection {
public:
    static bool checkServer(
            std::string &host,
            std::string &port
    );

    BAASAdbConnection();

    BAASAdbConnection(
            const std::string &host,
            const std::string &port,
            double socketTimeout = 3000.0
    );

    explicit BAASAdbConnection(
            const std::string &serial,
            double socketTimeout = 3000.0
    );

    [[nodiscard]] std::string readFully(int length) const;

    bool readUntilClose(std::string &res) const;

    bool sendMessage(const std::string &data) const;

    bool checkOKAY() const;

    bool checkSTAT() const;

    // ret message has a len in the head of the message
    [[nodiscard]] std::string readAdbReturnMessage() const;

    SOCKET createSocket();

    SOCKET safeCreateSocket();

    [[nodiscard]] SOCKET getConnection() const;

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
    [[nodiscard]] const char *what() const noexcept override
    {
        return "Connection refused";
    }
};

class AdbError : public std::exception {
public:
    AdbError() = default;

    explicit AdbError(const char *msg)
    {
        message = msg;
    }

    [[nodiscard]] const char *what() const noexcept override
    {
        if (message.empty()) return "Adb Error";
        return message.c_str();
    }

private:
    std::string message;
};

class BAASAdbBaseClient {
public:
    BAASAdbBaseClient();

    explicit BAASAdbBaseClient(
            const std::string &serial = "127.0.0.1:5037",
            double socketTimeout = 3000.0
    );

    explicit BAASAdbBaseClient(
            const std::string &host = "127.0.0.1",
            const std::string &port = "5037",
            double socketTimeout = 3000.0
    );

    BAASAdbConnection *makeConnection(double socketTimeout = 0);

    /*
        40 will match 1.0.40
        Returns:
            int
     */
    int serverVersion();

    bool serverKill();

    std::string connect(
            const std::string &address,
            double timeout = 3000.0
    );

    std::string disconnect(
            const std::string &address,
            double timeout = 3000.0
    );

    [[nodiscard]] const std::string &getHost() const;

    [[nodiscard]] const std::string &getPort() const;

    [[nodiscard]] const std::string &getSerial() const;

protected:
    std::string host;

    std::string port;

    std::string serial;

    double socketTimeout;
};

class BAASAdbBaseDevice {
public:

    BAASAdbBaseDevice(
            BAASAdbBaseClient *client,
            const std::string &serial,
            const int transportId = 0
    );

    [[nodiscard]] std::string getSerial() const;

    BAASAdbConnection *openTransport(
            const std::string &command,
            double socketTimeout = 60000.0
    );

    std::string getCommandResult(
            const std::string &command,
            double socketTimeout = 60000.0
    );

    std::string getState(double socketTimeout = 60000.0);

    std::string getSerialNo(double socketTimeout = 60000.0);

    std::string getDevPath(double socketTimeout = 60000.0);

    std::string getFeatures(double socketTimeout = 60000.0);

    BAASAdbConnection *createConnection(
            const std::string &network,
            const std::string &address
    );

    BAASAdbConnection *shellStream(
            const std::string &command,
            double socketTimeout = 3000.0
    );

    BAASAdbConnection *shellStream(
            const std::vector<std::string> &commandList,
            double socketTimeout = 3000.0
    );

    bool shellBytes(
            const std::string &command,
            std::string &out,
            double socketTimeout = 3000.0
    );

    bool shellBytes(
            const std::vector<std::string> &commandList,
            std::string &out,
            double socketTimeout = 3000.0
    );

    BAASAdbConnection *prepareSync(
            const std::string &path,
            const std::string &cmd
    );

    int stat(const std::string &path);

    int push(
            const std::string &src,
            const std::string &dst,
            const int mode = 493,
            bool check = true
    );

    ~BAASAdbBaseDevice();

protected:
    BAASAdbBaseClient *client;

    std::string serial;

    int transportId;
};

// Device
class BAASAdbDevice : public BAASAdbBaseDevice {
public:
    BAASAdbDevice(
            BAASAdbBaseClient *client,
            const std::string &serial,
            const int transportId = 0
    );

    ~BAASAdbDevice();
};

// Client
class BAASAdbClient : public BAASAdbBaseClient {
public:
    BAASAdbClient();

    BAASAdbClient(
            const std::string &serial,
            double socketTimeout = 3000.0
    );

    BAASAdbClient(
            const std::string &host,
            const std::string &port,
            double socketTimeout = 3000.0
    );

    /*
     * static method
     * List all devices
     * Returns:
     *     vector<pair<string,      int>>
     *     list of     serial       status (0: offline, 1: online 2: unauthorized, 3: other)
     */
    static void list_device(std::vector<std::pair<std::string, int>> &devices);

    std::vector<BAASAdbDevice *> iter_device();

    BAASAdbDevice *device(const std::string &serial);
};

extern BAASAdbClient adb;

BAAS_NAMESPACE_END


#endif //BAAS_DEVICE_BAASADBUTILS_H_
