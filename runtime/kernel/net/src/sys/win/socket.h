#ifndef __SOCKET_H__
#define __SOCKET_H__

#ifndef  _WINSOCKAPI_
#include <winsock.h>
#endif //_WINSOCKAPI_

const int EWOULDBLOCK = WSAEWOULDBLOCK;
const int ECONNRESET = WSAECONNRESET;

typedef int socklen_t;

#endif __SOCKET_H__