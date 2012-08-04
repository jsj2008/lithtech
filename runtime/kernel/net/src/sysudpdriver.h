#ifndef __SYSUDPDRIVER_H__
#define __SYSUDPDRIVER_H__

#ifdef __LINUX
#include "sys/win/udpdriver.h"
#elif __XBOX
#elif _WIN32
#include "sys/win/udpdriver.h"
#endif

#endif // __SYSUDPDRIVER_H__

