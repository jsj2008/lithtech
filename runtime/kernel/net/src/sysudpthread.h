#ifndef __SYSUDPTHREAD_H__
#define __SYSUDPTHREAD_H__

#include "iltthread.h"
#include "iltthreadevent.h"


#ifdef __LINUX

#include "sys/linux/linux_ltthread.h"
#include "sys/linux/linux_ltthreadevent.h"
//#include "sys/linux/linux_ltcriticalsection.h"

typedef CLinux_LTThread			 		  CLTThread;
typedef CLinux_LTThreadEvent<AutoReset>	  CLTThreadEvent;
typedef CLinux_LTThreadEvent<ManualReset> CLTThreadEventManual;
//typedef CLinux_LTCriticalSection 		  CLTCriticalSection;

#elif _WIN32

#include "sys/win/win32_ltthread.h"
#include "sys/win/win32_ltthreadevent.h"
//#include "sys/win/win32_ltcriticalsection.h"

typedef CWin32_LTThread					  CLTThread;
typedef CWin32_LTThreadEvent<AutoReset>	  CLTThreadEvent;
typedef CWin32_LTThreadEvent<ManualReset> CLTThreadEventManual;
//typedef CWin32_LTCriticalSection		  CLTCriticalSection;

#endif

#endif // __SYSUDPTHREAD_H__
