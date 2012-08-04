#ifndef __SYSSYSTEMINFO_H__
#define __SYSSYSTEMINFO_H__

#ifdef __LINUX
#include "sys/linux/systeminfo.h"
#elif __XBOX
#elif _WIN32
#include "sys/win/systeminfo.h"
#endif

#endif // __SYSSYSTEMINFO_H__
