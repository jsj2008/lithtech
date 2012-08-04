#ifndef __WON_PLATFORM_LINUX_H__
#define __WON_PLATFORM_LINUX_H__
#include "WONShared.h"

#define __cdecl

typedef bool BOOL;
typedef unsigned long DWORD;
typedef unsigned char BYTE;
typedef struct _GUID
{
    unsigned long Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char Data4[8];
} GUID;
typedef unsigned short wchar;

// _MT (and/or _MTAPI) must be defined at compile time

#define _MT					// pthreads
#define _MTAPI

#define INFINITE			0xFFFFFFFF
#define TRUE				(1)
#define FALSE				(0)

typedef char* LPCSTR;
typedef char* LPSTR;

#define __int8 char
#define __int16 short
#define __int32 long
#define __int64 long long

#include <pthread.h>

//#ifndef _WON_IN_DLL_H_C_

#include <memory>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <ostream.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/time.h>
#include <strstream.h>
#include <endian.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <linux/limits.h>
#include <unistd.h>
#include <netipx/ipx.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <asm/ioctls.h>
#include <assert.h>
#include <wctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <utime.h>
#include <termios.h> 
#include <signal.h>

#define _timezone timezone
#define _ASSERT assert
#define HOSTENT hostent
#define _MAX_PATH PATH_MAX
#define MAX_PATH PATH_MAX

#define NSPROTO_IPX		0		// Need to fix to support IPX under linux
#define NSPROTO_SPX		0		// Need to fix to support SPX under linux


#define SOCKET				int
#define INVALID_SOCKET		(-1)
#define SOCKET_ERROR		(-1)
#define closesocket			close
#define ioctlsocket			ioctl
#define SD_RECEIVE			(0)
#define SD_SEND				(1)

typedef sockaddr SOCKADDR;
typedef sockaddr_in SOCKADDR_IN;

#include <string>
namespace std 
{
	typedef basic_string<wchar_t> wstring;
};

inline unsigned long timeGetTime()
{
	timeb tp;
	ftime(&tp);
	return ((tp.time * 1000) + tp.millitm);
}

inline unsigned long GetTickCount()
{
	timeb tp;
	ftime(&tp);
	return ((tp.time * 1000) + tp.millitm);
}

inline void timeBeginPeriod(int)	{}
inline void timeEndPeriod(int)		{}

inline int WSAGetLastError()		{ return errno; }

typedef struct _FILETIME 
{ // ft 
    DWORD dwLowDateTime; 
    DWORD dwHighDateTime; 
} FILETIME; 
 
inline int strnicmp(const char *s1, const char *s2, int n)
{
    int i;
    char c1, c2;
    for (i=0; i<n; i++)
    {
        c1 = tolower(*s1++);
        c2 = tolower(*s2++);
        if (c1 < c2) return -1;
        if (c1 > c2) return 1;
        if (!c1) return 0;
    }
    return 0;
}

long InterlockedIncrement(long *addend);
long InterlockedDecrement(long *addend);
void Sleep(DWORD dwMilliseconds);

#endif

