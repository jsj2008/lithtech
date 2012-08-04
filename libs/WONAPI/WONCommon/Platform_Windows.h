#ifndef __WON_PLATFORM_WINDOWS_H__
#define __WON_PLATFORM_WINDOWS_H__
#include "WONShared.h"

#define WIN32_LEAN_AND_MEAN
#ifndef _XBOX
#define WIN32_NOT_XBOX 
#endif


#ifdef _XBOX
#include <xtl.h>
#include <winsockX.h>
#else
#include <windows.h>
#include <winsock2.h>
#endif

#include <sys/utime.h>
#include <sys/stat.h>
#include <process.h>
typedef int socklen_t;

#define snprintf _snprintf

#endif


