//
// Created by pc on 2024/5/27.
//
#include "device/BAASAdbUtils.h"

using namespace std;


// Connection
bool BAASAdbConnection::checkServer(std::string& host, std::string& port) {
    SOCKET connection = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(port.c_str()));
    inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr);
    if (connect(connection, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) return false;
    closesocket(connection);
    return true;
}

BAASAdbConnection::BAASAdbConnection() {

}

BAASAdbConnection::BAASAdbConnection(const string& host, const string& port, double socketTimeout) {
    this->host = host;
    this->port = port;
    this->serial = host + ":" + port;
    this->socketTimeout = socketTimeout;
    this->connection = safeCreateSocket();
}

BAASAdbConnection::BAASAdbConnection(const string &serial, double socketTimeout) {
    pair<string, string> hostPort = BAASUtil::serialToHostPort(serial);
    if(hostPort.first.empty() || hostPort.second.empty()) throw ValueError("Invalid serial : " + serial);
    this->serial = serial;
    this->host = hostPort.first;
    this->port = hostPort.second;
    this->socketTimeout = socketTimeout;
    this->connection = safeCreateSocket();
}

string BAASAdbConnection::readFully(int length) const {
    int t = length;
    int oneTimeLen;
    string res;
    char buffer[1024];
    while (t > 0) {
        if(t < 1024) oneTimeLen=recv(connection, buffer, t, 0);
        else oneTimeLen=recv(connection, buffer, 1024, 0);
        if(oneTimeLen <= 0) break;
        string temp(buffer, oneTimeLen);
        t -= oneTimeLen;
        res += temp;
    }
    return res;
}

bool BAASAdbConnection::readUntilClose(string &res) {
    res = "";
    string temp;
    while (true) {
        temp = readFully(4096);
        if(temp.empty()) break;
        res += temp;
    }
    return true;
}

bool BAASAdbConnection::sendMessage(const string &data) const {
    string msgLengthHex = BAASUtil::int2hex(int(data.length()));
    msgLengthHex = msgLengthHex + data;
    cout<<"send : "<<msgLengthHex<<endl;
    send(connection, msgLengthHex.c_str(), int(msgLengthHex.length()), 0);
    return true;
}

bool BAASAdbConnection::checkOKAY() {
    string data = readFully(4);
    if(data == _OKAY) return true;
    else if(data == _FAIL) throw AdbError(readAdbReturnMessage().c_str());
    else throw AdbError("UNKNOWN ADB ERROR.");
}

bool BAASAdbConnection::checkSTAT() {
    string data = readFully(4);
    if(data == _STAT) return true;
    else if(data == _FAIL) throw AdbError(readAdbReturnMessage().c_str());
    else throw AdbError("UNKNOWN ADB ERROR.");
}

string BAASAdbConnection::readAdbReturnMessage() {
    int length = BAASUtil::hex2int(readFully(4), 4);
    string message = readFully(length);
    return message;
}

SOCKET BAASAdbConnection::createSocket() {
    cout<<"create socket to "<<host<<":"<<port<<endl;
    connection = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(port.c_str()));
    inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr);
    if (connect(connection, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) throw ConnectionRefusedError();
    setsockopt(connection, SOL_SOCKET, SO_RCVTIMEO, (char*)&socketTimeout, sizeof(socketTimeout));
    return connection;
}

SOCKET BAASAdbConnection::safeCreateSocket() {
    try {
        return createSocket();
    } catch (ConnectionRefusedError &e) {
        BAASGlobalLogger->BAASInfo("Start ADB server.");
        BAASEmulatorController::startAdbServer();
        return createSocket();
    }
}

SOCKET BAASAdbConnection::getConnection() {
    return connection;
}



BAASAdbConnection::~BAASAdbConnection() {
    if(closeSocketWhenDestruct){
        closesocket(connection);
    }
}

bool BAASAdbConnection::setCloseSocketWhenDestruct(bool state) {
    closeSocketWhenDestruct = state;
    return true;
}

// BaseClient

BAASAdbBaseClient::BAASAdbBaseClient() {
    this->host = "127.0.0.1";
    this->port = "5037";
    this->serial = "127.0.0.1:5037";
    this->socketTimeout = 3000.0;
}

BAASAdbBaseClient::BAASAdbBaseClient(const std::string& serial, double socketTimeout) {
    int pos = serial.find(":");
    if(pos == std::string::npos) throw ValueError("Invalid serial : " + serial);
    this->serial = serial;
    this->host = serial.substr(0, pos);
    this->port = serial.substr(pos + 1);
    this->socketTimeout = socketTimeout;
}

BAASAdbBaseClient::BAASAdbBaseClient(const std::string& host, const std::string& port, double socketTimeout) {
    this->host = host;
    this->port = port;
    this->serial = host + ":" + port;
    this->socketTimeout = socketTimeout;
}

BAASAdbConnection *BAASAdbBaseClient::makeConnection(double socketTimeout) {
    if (socketTimeout == 0) socketTimeout = this->socketTimeout;
    return new BAASAdbConnection(host, port, socketTimeout);
}

int BAASAdbBaseClient::serverVersion() {
    BAASAdbConnection conn = BAASAdbConnection(host, port, socketTimeout);
    conn.sendMessage("host:version");
    conn.checkOKAY();
    string versionHex = conn.readAdbReturnMessage();
    return BAASUtil::hex2int(versionHex, versionHex.length());
}

bool BAASAdbBaseClient::serverKill() {
    if(BAASAdbConnection::checkServer(host, port)) {
        BAASAdbConnection conn = BAASAdbConnection(host, port, socketTimeout);
        conn.sendMessage("host:kill");
        conn.checkOKAY();
    }
    return true;
}

std::string BAASAdbBaseClient::connect(const std::string& address, double timeout) {
    BAASAdbConnection conn = BAASAdbConnection(host, port, timeout);
    conn.sendMessage("host:connect:" + address);
    conn.checkOKAY();
    return conn.readAdbReturnMessage();
}

std::string BAASAdbBaseClient::disconnect(const std::string& address, double timeout) {
    try{
        BAASAdbConnection conn = BAASAdbConnection(host, port, timeout);
        conn.sendMessage("host:disconnect:" + address);
        conn.checkOKAY();
        return conn.readAdbReturnMessage();
    }
    catch (AdbError &e) {
        return e.what();
    }
}

const std::string& BAASAdbBaseClient::getHost() const {
    return host;
}

const std::string& BAASAdbBaseClient::getPort() const {
    return port;
}

const std::string& BAASAdbBaseClient::getSerial() const {
    return serial;
}

// BaseDevice
BAASAdbBaseDevice::BAASAdbBaseDevice(BAASAdbBaseClient *client, const string& serial,const int transportId) {
    this->client = client;
    this->serial = serial;
    this->transportId = transportId;
}

BAASAdbConnection *BAASAdbBaseDevice::openTransport(const string& command="", double socketTimeout) {
    BAASAdbConnection *conn = client->makeConnection(socketTimeout);
    if(command != ""){
        if(transportId != 0) conn->sendMessage("host-transport-id:" + to_string(transportId) + ":" + command);
        else conn->sendMessage("host-serial:" + serial + ":" + command);
        conn->checkOKAY();
    }
    else{
        if(transportId != 0) conn->sendMessage("host:transport-id:" + to_string(transportId));
        else conn->sendMessage("host:transport:" + serial);
        conn->checkOKAY();
    }
    return conn;
}

string BAASAdbBaseDevice::getCommandResult(const string& command, double socketTimeout) {
    BAASAdbConnection *conn = openTransport(command, socketTimeout);
    conn->sendMessage(command);
    string result = conn->readAdbReturnMessage();
    delete conn;
    return result;
}

string BAASAdbBaseDevice::getState(double socketTimeout) {
    return getCommandResult("get-state", socketTimeout);
}

string BAASAdbBaseDevice::getSerialNo(double socketTimeout) {
    return getCommandResult("get-serialno", socketTimeout);
}

string BAASAdbBaseDevice::getDevPath(double socketTimeout) {
    return getCommandResult("get-devpath", socketTimeout);
}

string BAASAdbBaseDevice::getFeatures(double socketTimeout) {
    return getCommandResult("features", socketTimeout);
}

BAASAdbConnection* BAASAdbBaseDevice::createConnection(const string& network, const string& address) {
    BAASAdbConnection *conn = openTransport();
    vector<string> allNetworks = {
            Network::TCP,
            Network::UNIX,
            Network::DEV,
            Network::LOCAL,
            Network::LOCAL_RESERVED,
            Network::LOCAL_FILESYSTEM,
            Network::LOCAL_ABSTRACT
    };
    if(find(allNetworks.begin(), allNetworks.end(), network) == allNetworks.end()) {
        throw TypeError("Invalid Network type : " + network);
    }
    conn->sendMessage(network + ":" + address);
    conn->checkOKAY();
    return conn;
}



BAASAdbConnection* BAASAdbBaseDevice::shellStream(const string& command, double socketTimeout) {  // run shell and get stream
    BAASAdbConnection *conn = openTransport();
    conn->sendMessage("shell:" + command);
    conn->checkOKAY();
    return conn;
}

BAASAdbConnection* BAASAdbBaseDevice::shellStream(const vector<string>& commandList, double socketTimeout) {
    string cmd;
    BAASUtil::stringJoin(commandList, " ", cmd);
    return shellStream(cmd, socketTimeout);
}

bool BAASAdbBaseDevice::shellBytes(const string& command, string &out, double socketTimeout) {
    BAASAdbConnection *conn = shellStream(command, socketTimeout);
    conn->readUntilClose(out);
    delete conn;
    return true;
}

bool BAASAdbBaseDevice::shellBytes(const vector<string>& commandList, string &out, double socketTimeout) {
    string cmd;
    BAASUtil::stringJoin(commandList, " ", cmd);
    return shellBytes(cmd, out, socketTimeout);
}

BAASAdbConnection* BAASAdbBaseDevice::prepareSync(const string& path,const string& cmd) {
    string msg = "host:transport:" + serial;
    auto conn = new BAASAdbConnection("127.0.0.1", "5037");
    conn->sendMessage(msg);
    conn->checkOKAY();
    conn->sendMessage("sync:");
    conn->checkOKAY();
    msg = cmd + BAASUtil::changeEndian(path.length()) + path;
    cout<<"send : "<<msg<<endl;
    send(conn->getConnection(), msg.c_str(), msg.length(), 0);
    return conn;
}

int BAASAdbBaseDevice::stat(const string& path) {
    BAASAdbConnection *conn = prepareSync(path, _STAT);
    conn->checkSTAT();
    string data = conn->readFully(12);
    cout<<data.size()<<endl;
    int temp = BAASUtil::binary2int(data.substr(4, 4), 4);
    int sizeInt = BAASUtil::binary2int(BAASUtil::changeEndian(temp), 4);
    return sizeInt;
}


int BAASAdbBaseDevice::push(const string &src, const string &dst, const int mode,bool check) {
    if (! (filesystem::exists(src) && filesystem::is_regular_file(src))) {
        throw(PathError("File " + src + " not exists or not a regular file."));
    }
    string dstPath = dst + "," + to_string(32768 | mode);
    BAASAdbConnection *conn = prepareSync(dstPath, "SEND");
    ifstream file(src, ios::binary);
    int fileSize = int(filesystem::file_size(src));
    char buffer[4096];
    string head;
    try{
        SOCKET connection = conn->getConnection();
        while(true) {
            file.read(buffer, 4096);
            int readSize = int(file.gcount());
            if(readSize == 0) {
                string time = BAASUtil::changeEndian(BAASUtil::getCurrentTimeStamp());
                head = "DONE" + time;
                send(connection, head.c_str(), int(head.length()), 0);
                break;
            }
            head = "DATA" + BAASUtil::changeEndian(readSize);
            send(connection, head.c_str(), int(head.length()), 0);
            send(connection, buffer, readSize, 0);
        }
        conn->checkOKAY();
        file.close();
    } catch (AdbError &e) {
        BAASGlobalLogger->BAASError(e.what());
        file.close();
        return -1;
    }
    if(check) {
        int remoteSize = stat(dst);
        if(remoteSize != fileSize) {
            string msg = fmt::format("Push FAILED. Remote size: {0}, local size: {1}", remoteSize, fileSize);
            BAASGlobalLogger->BAASError("Push file failed.");
            delete conn;
            throw AdbError(msg.c_str());
        }
    }
    delete conn;
    BAASGlobalLogger->BAASInfo("Push SUCCEEDED.");
    return fileSize;
}

std::string BAASAdbBaseDevice::getSerial() const {
    return serial;
}

// Device


BAASAdbDevice::BAASAdbDevice(BAASAdbBaseClient *client, const std::string &serial, const int transportId) : BAASAdbBaseDevice(client, serial, transportId) {

}

// Client

BAASAdbClient::BAASAdbClient() : BAASAdbBaseClient("127.0.0.1:5037", 3000.0) {

}

void BAASAdbClient::list_device(std::vector<std::pair<std::string, int>> &devices) {
    devices.clear();
    BAASAdbConnection conn = BAASAdbConnection("127.0.0.1", "5037", 3000.0);
    conn.sendMessage("host:devices");
    conn.checkOKAY();
    string data = conn.readAdbReturnMessage();
    int i = 0;
    string serial;
    string status;
    int length = int(data.length());
    while (i < length) {
        while (data[i] != '\t') serial += data[i++];
        i++;
        while (data[i] != '\n') status += data[i++];
        i++;
        if(status == "offline") devices.emplace_back(serial, 0);
        else if(status == "device") devices.emplace_back(serial, 1);
        else if(status == "unauthorized") devices.emplace_back(serial, 2);
        else devices.emplace_back(serial, 3);
        serial = "";
        status = "";
    }
    cout << "devices : " << devices.size() << endl;
    for (auto &device : devices) {
        cout << device.first << " " << device.second << endl;
    }
}

std::vector<BAASAdbDevice*> BAASAdbClient::iter_device() {
    vector<BAASAdbDevice*> devices;
    vector<pair<string, int>> dList;
    list_device(dList);
    for (auto &device : dList) {
        if(device.second == 1) {
            auto conn = new BAASAdbDevice(this, device.first);
            devices.push_back(conn);
        }
    }
    return devices;
}

BAASAdbDevice *BAASAdbClient::device(const std::string& serial) {
    return new BAASAdbDevice(this, serial);
}

BAASAdbClient::BAASAdbClient(const string &serial, double socketTimeout) : BAASAdbBaseClient(serial, socketTimeout) {

}

BAASAdbClient::BAASAdbClient(const string &host, const string &port, double socketTimeout) : BAASAdbBaseClient(host, port, socketTimeout) {

}

BAASAdbClient adb;


