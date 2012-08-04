// ----------------------------------------------------------------------- //
//
// MODULE  : SystemDependant.h
//
// PURPOSE : Include operating system dependant files
//
// CREATED : 07.15.1999
//
// ----------------------------------------------------------------------- //

#ifndef _SYSTEM_DEPENDANT_
#define _SYSTEM_DEPENDANT_

#include "windowsx.h"

//windowsx.h includes a macro called DeleteFont() which interferes with use
//	of the client api call DeleteFont(), so the macro is undefined here
#undef DeleteFont


#endif // _SYSTEM_DEPENDANT_
