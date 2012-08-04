/******
nonport.c
GameSpy Developer SDK 
  
Copyright 1999-2001 GameSpy Industries, Inc

18002 Skypark Circle
Irvine, California 92614
949.798.4200 (Tel)
949.798.4299 (Fax)
devsupport@gamespy.com

******/

#ifdef __cplusplus
extern "C" {
#endif

#include "nonport.h"


#if defined(_WIN32) && !defined(UNDER_CE)
#pragma comment(lib, "wsock32")
#pragma comment(lib, "advapi32")
#endif

#ifdef UNDER_CE
#pragma comment(lib, "platutil")
#pragma comment(lib, "winsock")
#endif

#ifdef UNDER_CE
time_t time(time_t *timer)
{
	static time_t ret;
	SYSTEMTIME systime;
	FILETIME ftime;
	LONGLONG holder;

	GetLocalTime(&systime);
	SystemTimeToFileTime(&systime, &ftime);
	holder = (ftime.dwHighDateTime << 32) + ftime.dwLowDateTime;
	if (timer == NULL)
		timer = &ret;
	*timer = (time_t)((holder - 116444736000000000) / 10000000);
	return *timer; 
}

int isdigit( int c )
{
	return (c >= '0' && c <= '9');
}

int isxdigit( int c )
{
	return ((c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f') || (c >= '0' && c <= '9'));
}

int isalnum( int c )
{
	return ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'));
}

int isspace(int c)
{
	return ((c >= 0x9 && c <= 0xD) || (c == 0x20)); 
}

#endif

#ifdef __KATANA__
/*
631152000 -- diff b/t 1950 & 1970 GMT
631126800 -- diff b/t 1950 GMT & 1970 PDT
*/
time_t time(time_t *timer)
{
	static time_t ret;
	SYS_RTC_DATE date;
	Uint32 count;
	syRtcGetDate(&date);
	syRtcDateToCount(&date, &count);
	if (timer == NULL)
		timer = & ret;
	*timer = (time_t)(count - 631152000);	
}

void *fixed_realloc(void *ptr, int newsize)
{
	if (ptr && newsize == 0)
	{
		gsifree(ptr);
		return NULL;
	}
	else if (ptr)
		return realloc(ptr, newsize);
	else //ptr is null
		return gsimalloc(newsize);
					
	
}

#endif

#ifdef __mips64
unsigned long goa_frame_ct = 0;

#endif

unsigned long current_time()  //returns current time in msec
{ 
#ifdef _WIN32
	return (GetTickCount()); 
#endif
#ifdef _MACOS
	return (TickCount() * 50) / 3;
#endif
#ifdef __KATANA__
	return (syTmrCountToMicro(syTmrGetCount()))/1000;
#endif
#ifdef __mips64
	/*struct SysClock mc;
	int sec, int usec;
	GetSystemTime(&mc);
	SysClock2USec(&mc, sec, usec);
	return (sec * 1000 + usec / 1000);*/
//	return 0;//MUSTFIX
//	return clock();
//	return time(NULL)*1000;
	return goa_frame_ct * 1000 / 60;
	//return goa_frame_ct += 100;
#endif
#ifdef UNDER_UNIX
	struct timeval time;
	
	gettimeofday(&time, NULL);
	return (time.tv_sec * 1000 + time.tv_usec / 1000);
#endif
}

#ifdef __KATANA__
extern void DrawScreen(void);
#endif

void msleep(unsigned long msec)
{
#ifdef _WIN32
	Sleep(msec);
#endif
#ifdef __KATANA__
	//wait for the approx msec based on vertical refreshes
	unsigned long stoptime = current_time() + msec;
	do
	{
		//vsWaitVSync(1);
		njWaitVSync();
		DrawScreen();
	} while (current_time() < stoptime);
#endif
#ifdef _MACOS
//	EventRecord rec;
	WaitNextEvent(everyEvent,/*&rec*/NULL, (msec*1000)/60, NULL);
#endif
#ifdef __mips64
	sn_delay(msec);
#endif
#ifdef UNDER_UNIX
	usleep(msec * 1000);
#endif
}

void SocketStartUp()
{
#if defined(_WIN32) || defined(_MACOS)
	WSADATA data;
	WSAStartup(0x0101, &data);
#endif


}

void SocketShutDown()
{
#if defined(_WIN32) || defined(_MACOS)
	WSACleanup();
#endif

}

#if defined(_WIN32) && !defined(UNDER_CE)
//do nothign
#else
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
char *_strdup(const char *src)
{
	char *res;
	if(src == NULL)      //PANTS|02.11.00|check for NULL before strlen
		return NULL;
	res = (char *)malloc(strlen(src) + 1);
	if(res != NULL)      //PANTS|02.02.00|check for NULL before strcpy
		strcpy(res, src);
	return res;
}

char *_strlwr(char *string)
{
	char *hold = string;
	while (*string)
	{
		*string = tolower(*string);
		string++;
	}

	return hold;
}
char *_strupr(char *string)
{
	char *hold = string;
	while (*string)
	{
		*string = toupper(*string);
		string++;
	}

	return hold;
}
#endif

int SetSockBlocking(SOCKET sock, int isblocking)
{
#ifdef __KATANA__
	#ifdef KGTRN_ACCESS
		short   argp;
	#endif
	#ifdef KGTRN_PLANET_WEB
		long argp;
	#endif
	#ifdef KGTRN_NEXGEN
		int argp;
	#endif
#else
	unsigned long argp;
#endif
	int rcode; 
	
	if (isblocking)
		argp = 0;
	else
		argp = 1;
#ifdef __KATANA__

	#ifdef KGTRN_ACCESS
		rcode = setsockopt(sock, SOL_SOCKET, SO_NOBLK,(char *)&argp, sizeof(argp));
	#endif
	#ifdef KGTRN_PLANET_WEB
		if ( (argp = net_fcntl( sock, F_GETFL, 0)) < 0 )
			return -1;
		if (isblocking)
			argp &= ~O_NONBLOCK;	
		else
			argp |= O_NONBLOCK;	
		rcode = net_fcntl( sock, F_SETFL, argp );
	#endif
	#ifdef KGTRN_NEXGEN
		if ( (argp = fcntlsocket( sock, F_GETFL, 0)) < 0 )
			return -1;
		if (isblocking)
			argp &= ~O_NONBLOCK;	
		else
			argp |= O_NONBLOCK;	
		rcode = fcntlsocket( sock, F_SETFL, argp );		
	#endif
#else
	#ifdef __mips64
		rcode = setsockopt(sock, SOL_SOCKET, (isblocking) ? SO_BIO : SO_NBIO, &argp, sizeof(argp));
	#else
		rcode = ioctlsocket(sock, FIONBIO, &argp);
	#endif
#endif
	if (rcode == 0)
		return 1;
	else
		return 0;
}

int DisableNagle(SOCKET sock)
{
#ifdef KGTRN_ACCESS
	return 0; //access doesn't support this
#endif
#if defined(_WIN32) || defined(KGTRN_PLANET_WEB) || defined(KGTRN_NEXGEN) || defined(UNDER_UNIX)
	int rcode;
	int noDelay = 1;

	rcode = setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (char *)&noDelay, sizeof(int));

	return (rcode != SOCKET_ERROR);	

#endif
	return 0;
}

int SetReceiveBufferSize(SOCKET sock, int size)
{
	int rcode;

	rcode = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, (const char *)&size, sizeof(int));

	return (rcode != SOCKET_ERROR);
}


#if defined(_MACOS) || defined(UNDER_CE) || defined(__KATANA__)
int strcasecmp(const char *string1, const char *string2)
{
	while (tolower(*string1) == tolower(*string2) && *string1 != 0 && *string2 != 0)
	{
		*string1++; *string2++;
	}
	return tolower(*string1) - tolower(*string2);
}

int strncasecmp(const char *string1, const char *string2, size_t count)
{
	while (--count > 0 && tolower(*string1) == tolower(*string2) && *string1 != 0 && *string2 != 0)
	{
		*string1++; *string2++;
	}
	return tolower(*string1) - tolower(*string2);
}
#endif

#ifdef __KATANA__

#ifndef __GNUC__
void abort(void)
{
	//called by assert on dreamcast but not present
	//could do a hardware break in here if needed	
	
}
#endif

unsigned long fixed_inet_addr (const char * cp)
{
	int idata[4];
	unsigned char data[4];
	int pc = 0;
	char *pos;
	
	//validate the IP
	for (pos = (char *)cp ; *pos != 0; pos++)
	{
		if( (*pos < '0' || *pos > '9') && *pos != '.')
			return INADDR_NONE;
		if (*pos == '.')
			pc++; //increment the period counter
	}
	if (pc != 3)
		return INADDR_NONE; //not enough periods
		
	sscanf(cp, "%d.%d.%d.%d", idata, idata+1,idata+2,idata+3);
	data[0] = (unsigned char)idata[0]; data[1] = (unsigned char)idata[1]; data[2] = (unsigned char)idata[2]; data[3] = (unsigned char)idata[3];
	return *(unsigned long *)data;
}


void *grealloc(void *ptr, int newlen)
{
	return realloc(ptr,newlen);
	/*void *outptr;
	if (ptr == NULL &&  newlen == 0)
		return NULL;
	if (ptr == NULL)
		return malloc(newlen);
	if (newlen == 0)
	{
		free(ptr);
		return NULL;		
	}	
	outptr = malloc(newlen);
	if (!outptr)
		return NULL;
	//note: this is NOT optimal, since we may copy more than we need to
	memcpy(outptr, ptr, newlen);
	return outptr;	*/
}

#ifdef KGTRN_PLANET_WEB

#define MAX_HOST_ADDRS 16
struct hostent *pwgethostbyname(const char *name)
{
	static char *aliases = NULL;
	static struct in_addr *paddrs[MAX_HOST_ADDRS];
	static struct hostent hent;
	int i;
	int err;
	static struct addrinfo *ai = NULL;
	struct addrinfo *curai = NULL;
	
	if (ai != NULL)
		freeaddrinfo(ai);
	ai = NULL;
	err = getaddrinfo(name, NULL, NULL, &ai);
	if (err != 0 || ai == NULL)
	{
		return NULL;
	}
	hent.h_name = ai->ai_canonname;
	hent.h_aliases = &aliases;
	hent.h_addrtype = 2;
	hent.h_length = 4;
	
	for (i = 0, curai = ai ; i < MAX_HOST_ADDRS - 1, curai != NULL ; i++, curai = curai->ai_next)
		paddrs[i] = &((struct sockaddr_in *)curai->ai_addr)->sin_addr;
	paddrs[i] = NULL;
	hent.h_addr_list = (char **)paddrs;
	 
	return &hent;
}

#endif //PWEB

 
#endif //KATANA

#ifdef __mips64
int GOAClearSocketError(SOCKET s)
{
	int val;
	int soval = sizeof(val);
	getsockopt(s,SOL_SOCKET,SO_ERROR,&val,&soval);
	return val;
}
#endif


#define RANa 16807         /* multiplier */
#define LONGRAND_MAX 2147483647L   /* 2**31 - 1 */

static long randomnum = 1;


static long nextlongrand(long seed)
{
	unsigned

	long lo, hi;
	lo = RANa *(long)(seed & 0xFFFF);
	hi = RANa *(long)((unsigned long)seed >> 16);
	lo += (hi & 0x7FFF) << 16;

	if (lo > LONGRAND_MAX)
	{
		lo &= LONGRAND_MAX;
		++lo;
	}
	lo += hi >> 15;

	if (lo > LONGRAND_MAX)
	{
		lo &= LONGRAND_MAX;
		++lo;
	}
	return(long)lo;
}


static long longrand(void)/* return next random long */
{
	randomnum = nextlongrand(randomnum);
	return randomnum;
}


static void Util_RandSeed(unsigned long seed)/* to seed it */
{
	randomnum = seed ? (seed & LONGRAND_MAX) : 1;
	/* nonzero seed */
}


static int Util_RandInt(int low, int high)
{
	int range = high-low;
	int num = longrand() % range;

	return(num + low);
}


#define REG_KEY					"Software\\GameSpy\\GameSpy 3D\\Registration"

#ifdef UNDER_CE
const char * GOAGetUniqueID(void)
{
	static char keyval[17];
	unsigned char buff[8];
	DWORD size;

	size = 0;
	for (size = 0 ; size < sizeof(buff) ; size++)
		buff[size] = 0;
	size = sizeof(buff);
	FirmwareGetSerialNumber(buff, &size);
	for (size = 0 ; size < sizeof(buff) ; size++)
	{
		sprintf(keyval + (size * 2),"%02x",buff[size]);
	}
	return keyval;
}


#endif //UNDER_CE

#if (defined(_WIN32) || defined(UNDER_UNIX)) && !defined(UNDER_CE)

static void GenerateID(char *keyval)
{
	int i;
	const char crypttab[63] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
#ifdef _WIN32
	LARGE_INTEGER l1;
	UINT seed;
	if (QueryPerformanceCounter(&l1))
		seed = (l1.LowPart ^ l1.HighPart);
	else
		seed = 0;
	Util_RandSeed(seed ^ GetTickCount() ^ time(NULL) ^ clock());
#else
	Util_RandSeed(time(NULL) ^ clock());
#endif
	for (i = 0; i < 19; i++)
		if (i == 4 || i == 9 || i == 14)
			keyval[i] = '-';
	else
		keyval[i] = crypttab[Util_RandInt(0, 62)];
	keyval[19] = 0;
}

#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif


const char * GOAGetUniqueID(void)
{
	static char keyval[PATH_MAX] = "";
	unsigned int ret;

	int docreate;



#ifdef _WIN32
	HKEY thekey;
	DWORD thetype = REG_SZ;
	DWORD len = MAX_PATH;
	DWORD disp;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, REG_KEY, 0, KEY_ALL_ACCESS, &thekey) != ERROR_SUCCESS)
		docreate = 1;
	else
		docreate = 0;
	ret = RegQueryValueEx(thekey, "Crypt", 0, &thetype, keyval, &len);
#else
	FILE *f;
	f = fopen("id.bin","r");
	if (!f)
		ret = 0;
	else
	{
		ret = fread(keyval,1,19,f);
		keyval[ret] = 0;
		fclose(f);
	}
#endif

	if (ret != 0 || strlen(keyval) != 19)//need to generate a new key
	{
		GenerateID(keyval);
#ifdef _WIN32
		if (docreate)
		{
			ret = RegCreateKeyEx(HKEY_CURRENT_USER, REG_KEY, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &thekey, &disp);
		}
		RegSetValueEx(thekey, "Crypt", 0, REG_SZ, keyval, strlen(keyval)+1);
#else
		f = fopen("id.bin","w");
		if (f)
		{
			fwrite(keyval,1,19,f);
			fclose(f);
		} else
			keyval[0] = 0; //don't generate one each time!!
#endif
	}
#ifdef _WIN32
	RegCloseKey(thekey);
#endif

	// Strip out the -'s.
	/////////////////////
	memmove(keyval + 4, keyval + 5, 4);
	memmove(keyval + 8, keyval + 10, 4);
	memmove(keyval + 12, keyval + 15, 4);
	keyval[16] = '\0';
	
	return keyval;
}

#endif

#ifdef __cplusplus
}
#endif
