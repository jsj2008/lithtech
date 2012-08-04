#ifndef __WON_PLATFORM_H__
#define __WON_PLATFORM_H__
#include "WONShared.h"


#if defined(WIN32)
#include "Platform_Windows.h"
#elif defined(WINCE)
#include "Platform_WinCE.h"
#elif defined(_LINUX)
#include "Platform_Linux.h"
#elif defined(macintosh) && (macintosh == 1)
#include "Platform_Mac.h"
#endif

#endif
