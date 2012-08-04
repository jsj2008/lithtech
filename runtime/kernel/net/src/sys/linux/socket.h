#ifndef __SOCKET_H__
#define __SOCKET_H__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>

typedef int SOCKET;

inline int closesocket(SOCKET s) { return close(s); }

const int SOCKET_ERROR = -1;
const int INVALID_SOCKET = -1;

#include <errno.h>
inline int WSAGetLastError() { return errno; }

#endif // __SOCKET_H__

