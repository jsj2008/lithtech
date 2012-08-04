/******
nonport.h
GameSpy Developer SDK 
  
Copyright 1999-2001 GameSpy Industries, Inc

18002 Skypark Circle
Irvine, California 92614
949.798.4200 (Tel)
949.798.4299 (Fax)
devsupport@gamespy.com

******/

#ifndef _NONPORT_H_
#define _NONPORT_H_

#if defined(applec) || defined(THINK_C) || defined(__MWERKS__) && !defined(__KATANA__) && !defined(__mips64)
	#define _MACOS
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <winsock.h>
#else
#ifdef _MACOS
	#include <events.h>
	#include "mwinsock.h"
#else
#ifdef __KATANA__
	#ifdef KGTRN_PLANET_WEB
		#ifdef __GNUC__
			#include <errno.h>
		#endif
		#include <pwebinc.h>
		#include <dbdebug.h>
		#include <netinet/tcp.h>
	#endif
	#ifdef KGTRN_ACCESS
		#include <types.h>
		#include <socket.h>
		#include <sock_errno.h>
		#include <aveppp.h>
		#include <vsyncpro.h>
		#include <dns.h>
	#endif
	#ifdef KGTRN_NEXGEN
		#include <ngos.h>
		#include <ngdsock.h>
		#include <ngappp.h>
		#include <ngadns.h>
		#include <ngnet.h>
		#include <ngsocket.h>
		#include <ngtcp.h>

	#endif

	
	#include <stdio.h>
	#include <stdlib.h>
	#include <stddef.h>
	#include <shinobi.h>
	#include <sg_sytmr.h> //for time functions
#else
#ifdef __mips64 //PS2
	#include "sntypes.h"       /* SN Systems types                     */
	#include "snsocket.h"      /* SN Systems socket API header file    */
	#include "sneeutil.h"      /* SN Systems PS2 EE Utilites (General) */
	#include "sntcutil.h"      /* SN Systems PS2 EE Utilites (TCP/IP)  */
#else //UNIX
	#define UNDER_UNIX
	#include <unistd.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <sys/ioctl.h>
	#include <netinet/in.h>
	#include <netdb.h>
	#include <arpa/inet.h>
	#include <ctype.h>
	#include <errno.h>
	#include <sys/time.h>
	#include <netinet/tcp.h>
#endif
#endif
#endif
#endif 

#ifdef UNDER_CE
#include <platutil.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define gsimalloc malloc
#define gsifree free

#ifdef __KATANA__
void *fixed_realloc(void *ptr, int newsize);
#define gsirealloc fixed_realloc
#else
#define gsirealloc realloc
#endif



typedef int goa_int32;
typedef unsigned int goa_uint32;
	
unsigned long current_time();
void msleep(unsigned long msec);

void SocketStartUp();
void SocketShutDown();

#ifndef SOCKET_ERROR 
	#define SOCKET_ERROR (-1)
#endif

#ifndef INADDR_NONE
   #define INADDR_NONE 0xffffffff
#endif

#ifndef INVALID_SOCKET 
	#define INVALID_SOCKET (-1)
#endif

#ifdef KGTRN_NEXGEN
	#define FD_SETSIZE NG_FD_MAX
	#define select selectsocket
	//#define shutdown(a,b) //ACK! shutdown isn't linking! FIX THIS
#endif

#if defined(_WIN32) && !defined(UNDER_CE)
	#define strcasecmp _stricmp
	#define strncasecmp _strnicmp
#else
	#include <string.h>
	#include <stdlib.h>
	#include <ctype.h>
	
	#ifndef strdup
		#define strdup _strdup
	#endif
	char *_strdup(const char *src);
	char *_strlwr(char *string);
	char *_strupr(char *string);
#endif


#if defined(_MACOS) || defined(UNDER_CE)
	int strcasecmp(const char *string1, const char *string2);
	int strncasecmp(const char *string1, const char *string2, size_t count);
#endif

#ifdef __mips64
	#define IPPROTO_TCP PF_INET
	#define IPPROTO_UDP PF_INET
	#define FD_SETSIZE SN_MAX_SOCKETS
#endif

#if !defined(_MACOS) && !defined(_WIN32)

	#ifdef KGTRN_NEXGEN
		#define GOAGetLastError(s) ngGlobalErrno
	#endif
	
	#ifdef KGTRN_ACCESS
		#define GOAGetLastError(s) sock_errno
	#endif
	
	#if defined (KGTRN_PLANET_WEB)
		#if defined(__GNUC__) && defined(__KATANA__)
			#define GOAGetLastError(s) (*__errno())	
			extern int *__errno _PARAMS ((void));
		#else
			#define GOAGetLastError(s) errno
		#endif
	#endif
	
	#if defined(__mips64)
		#define GOAGetLastError(s) sn_errno(s)
	#endif
	
	#ifdef UNDER_UNIX
		#define GOAGetLastError(s) errno
	#endif

	#ifdef KGTRN_ACCESS
		#define closesocket sock_close
	#endif
	
	#ifdef KGTRN_PLANET_WEB
		#define closesocket net_close
	#endif
	
	#ifdef UNDER_UNIX
		#define closesocket close //on unix
	#endif
	
	#define SOCKET int	
	#define ioctlsocket ioctl
	#ifdef KGTRN_NEXGEN
		#define WSAEWOULDBLOCK NG_EWOULDBLOCK
		#define WSAEINPROGRESS NG_EINPROGRESS
		#define WSAEALREADY NG_EALREADY
		#define WSAENOTSOCK NG_ENOTSOCK
		#define WSAEDESTADDRREQ NG_EDESTADDRREQ
		#define WSAEMSGSIZE NG_EMSGSIZE
		#define WSAEPROTOTYPE NG_EPROTOTYPE
		#define WSAENOPROTOOPT NG_ENOPROTOOPT
		#define WSAEPROTONOSUPPORT NG_EPROTONOSUPPORT
		#define WSAESOCKTNOSUPPORT NG_ESOCKTNOSUPPORT
		#define WSAEOPNOTSUPP NG_EOPNOTSUPP
		#define WSAEPFNOSUPPORT NG_EPFNOSUPPORT
		#define WSAEAFNOSUPPORT NG_EAFNOSUPPORT
		#define WSAEADDRINUSE NG_EADDRINUSE
		#define WSAEADDRNOTAVAIL NG_EADDRNOTAVAIL
		#define WSAENETDOWN NG_ENETDOWN
		#define WSAENETUNREACH NG_ENETUNREACH
		#define WSAENETRESET NG_ENETRESET
		#define WSAECONNABORTED NG_ECONNABORTED
		#define WSAECONNRESET NG_ECONNRESET
		#define WSAENOBUFS NG_ENOBUFS
		#define WSAEISCONN NG_EISCONN
		#define WSAENOTCONN NG_ENOTCONN
		#define WSAESHUTDOWN NG_ESHUTDOWN
		#define WSAETOOMANYREFS NG_ETOOMANYREFS
		#define WSAETIMEDOUT NG_ETIMEDOUT
		#define WSAECONNREFUSED NG_ECONNREFUSED
		#define WSAELOOP NG_ELOOP
		#define WSAENAMETOOLONG NG_ENAMETOOLONG
		#define WSAEHOSTDOWN NG_EHOSTDOWN
		#define WSAEHOSTUNREACH NG_EHOSTUNREACH
		#define WSAENOTEMPTY NG_ENOTEMPTY
		#define WSAEPROCLIM NG_EPROCLIM
		#define WSAEUSERS NG_EUSERS
		#define WSAEDQUOT NG_EDQUOT
		#define WSAESTALE NG_ESTALE
		#define WSAEREMOTE NG_EREMOTE
	#else
		#define WSAEWOULDBLOCK EWOULDBLOCK             
		#define WSAEINPROGRESS EINPROGRESS             
		#define WSAEALREADY EALREADY                
		#define WSAENOTSOCK ENOTSOCK                
		#define WSAEDESTADDRREQ EDESTADDRREQ            
		#define WSAEMSGSIZE EMSGSIZE                
		#define WSAEPROTOTYPE EPROTOTYPE              
		#define WSAENOPROTOOPT ENOPROTOOPT             
		#define WSAEPROTONOSUPPORT EPROTONOSUPPORT         
		#define WSAESOCKTNOSUPPORT ESOCKTNOSUPPORT         
		#define WSAEOPNOTSUPP EOPNOTSUPP              
		#define WSAEPFNOSUPPORT EPFNOSUPPORT            
		#define WSAEAFNOSUPPORT EAFNOSUPPORT            
		#define WSAEADDRINUSE EADDRINUSE              
		#define WSAEADDRNOTAVAIL EADDRNOTAVAIL           
		#define WSAENETDOWN ENETDOWN                
		#define WSAENETUNREACH ENETUNREACH             
		#define WSAENETRESET ENETRESET               
		#define WSAECONNABORTED ECONNABORTED            
		#define WSAECONNRESET ECONNRESET              
		#define WSAENOBUFS ENOBUFS                 
		#define WSAEISCONN EISCONN                 
		#define WSAENOTCONN ENOTCONN                
		#define WSAESHUTDOWN ESHUTDOWN               
		#define WSAETOOMANYREFS ETOOMANYREFS            
		#define WSAETIMEDOUT ETIMEDOUT               
		#define WSAECONNREFUSED ECONNREFUSED            
		#define WSAELOOP ELOOP                   
		#define WSAENAMETOOLONG ENAMETOOLONG            
		#define WSAEHOSTDOWN EHOSTDOWN               
		#define WSAEHOSTUNREACH EHOSTUNREACH            
		#define WSAENOTEMPTY ENOTEMPTY               
		#define WSAEPROCLIM EPROCLIM                
		#define WSAEUSERS EUSERS                  
		#define WSAEDQUOT EDQUOT                  
		#define WSAESTALE ESTALE                  
		#define WSAEREMOTE EREMOTE 
	#endif               
#else
	#define GOAGetLastError(s) WSAGetLastError()
#endif

#ifndef _WIN32
	typedef struct sockaddr SOCKADDR;
	typedef struct sockaddr_in SOCKADDR_IN;
	typedef struct in_addr IN_ADDR;
	typedef struct hostent HOSTENT;
	typedef struct timeval TIMEVAL;
#endif

#ifndef max
#define max(a,b)    (((a) > (b)) ? (a) : (b))
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

#ifdef _WIN32
	#define PATHCHAR '\\'
#else
#ifdef MACOS
	#define PATHCHAR ':'
#else
	#define PATHCHAR '/'

#endif
#endif

int SetSockBlocking(SOCKET sock, int isblocking);
int DisableNagle(SOCKET sock);
int SetReceiveBufferSize(SOCKET sock, int size);

#if defined(UNDER_CE) || defined(KGTRN_PLANET_WEB) || defined(KGTRN_NEXGEN)
//CE does not have the stdlib time() call
	#if defined(KGTRN_PLANET_WEB) || defined(KGTRN_NEXGEN)
		typedef long	time_t;
	#endif
	time_t time(time_t *timer);
#else
	#include <time.h>
#endif



#if defined(__KATANA__) && defined(KGTRN_ACCESS)
	unsigned long fixed_inet_addr (const char * cp);
	#define inet_addr fixed_inet_addr
#endif


#if defined(__KATANA__) && defined(KGTRN_PLANET_WEB)
	#define gethostbyname pwgethostbyname
	struct hostent *pwgethostbyname(const char *name);
	#undef shutdown
	#define shutdown(s,h)  //shutdown locks up!
	
#endif

#ifdef UNDER_CE
int isdigit( int c );
int isxdigit( int c );
int isalnum( int c );
int isspace(int c);
#endif


#ifdef __mips64
int GOAClearSocketError(SOCKET s);
#else
#define GOAClearSocketError(s) (0)
#endif

#if defined(UNDER_CE) || defined(__KATANA__)
#define NOFILE
#endif

const char * GOAGetUniqueID(void);

#ifdef __cplusplus
}
#endif


#endif 
