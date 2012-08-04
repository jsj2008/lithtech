// ----------------------------------------------------------------------------
//	SysInterfaceHelpers.h
//
//	redirects the system-specific includes for interface_helpers.cpp


#ifndef _SYSINTERFACE_HELPERS_H_
#define _SYSINTERFACE_HELPERS_H_


// system specific includes
#ifdef  __XBOX
#include "sys/xbox/interface_helpers.h"
#elif  _WIN32
#include "sys/win/interface_helpers.h"
#endif

#endif // _SYSINTERFACE_HELPERS_H_
