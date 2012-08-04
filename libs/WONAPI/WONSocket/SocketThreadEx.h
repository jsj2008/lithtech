#ifndef __WON_SOCKETTHREADEX_H__
#define __WON_SOCKETTHREADEX_H__
#include "WONShared.h"

#if defined(WIN32)
#include "SocketThreadEx_Windows.h"
#elif defined(_LINUX)
#include "SocketThreadEx_Linux.h"
#else

#include "SocketThreadSimple.h"
namespace WONAPI
{

typedef SocketThreadSimple SocketThreadEx;

}; // namespace WONAPI


#endif
#endif
