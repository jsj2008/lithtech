#ifndef __SYSDRAWPRIM_H__
#define __SYSDRAWPRIM_H__


#ifdef __NULLREND
    #include "sys/null/nulldrawprim.h"
#else

	#ifdef __D3DREND
    #include "sys/d3d/d3ddrawprim.h"
    #endif

	#ifdef __XBOXREND
	#include "sys/xbox/xbox_drawprim.h"
	#endif

#endif

#endif