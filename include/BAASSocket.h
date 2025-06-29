//
// Created by Administrator on 2025/6/10.
//

#ifndef BAAS_BAASSOCKET_H_
#define BAAS_BAASSOCKET_H_

#include "core_defines.h"

#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#elif UNIX_LIKE_PLATFORM
#include <unistd.h>
#include <arpa/inet.h>    
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h> 

#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif // INVALID_SOCKET

#ifndef SOCKET_ERROR
#define SOCKET_ERROR (-1)
#endif // SOCKET_ERROR

#endif // _WIN32

BAAS_NAMESPACE_BEGIN

#ifdef _WIN32
using BAASSocket_t = SOCKET;
#elif UNIX_LIKE_PLATFORM
using BAASSocket_t = int;
#endif // _WIN32

inline int close_socket(BAASSocket_t connection) {
#ifdef _WIN32
    return closesocket(connection);
#elif UNIX_LIKE_PLATFORM
    return close(connection);
#endif // _WIN32
}


inline bool set_nonblocking(BAASSocket_t sock, bool nonblocking) {
#ifdef _WIN32
    u_long mode = nonblocking ? 1 : 0;
    return ioctlsocket(sock, FIONBIO, &mode) == 0;
#elif UNIX_LIKE_PLATFORM
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1) return false;
    if (nonblocking)
        return fcntl(sock, F_SETFL, flags | O_NONBLOCK) == 0;
    else
        return fcntl(sock, F_SETFL, flags & ~O_NONBLOCK) == 0;
#endif // _WIN32
}

BAAS_NAMESPACE_END


#endif //BAAS_BAASSOCKET_H_
