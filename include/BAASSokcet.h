//
// Created by Administrator on 2025/6/10.
//

#ifndef BAAS_BAASSOCKET_H_
#define BAAS_BAASSOCKET_H_

#include "core_defines.h"

#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>

#elif UNIX_LIKE_PLATFORM
#include <sys/socket.h>
#include <sys/types.h>
#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif
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
#else
    return close(connection);
#endif
}

BAAS_NAMESPACE_END


#endif //BAAS_BAASSOCKET_H_
