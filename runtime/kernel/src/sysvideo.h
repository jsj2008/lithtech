// ------------------------------------------------------------------------- //
//
// FILE      : S Y S T H R E A D . H
//
// CREATED   : 11/05/99
//
// AUTHOR    : 
//
// COPYRIGHT : Monolith Productions Inc.
//
// ORIGN     : lithtech 1.5/2.0
//
// ------------------------------------------------------------------------- //

#ifndef __SYSVIDEO_H__
#define __SYSVIDEO_H__


// This is a redirector to get the system dependent include
#ifdef __LINUX
#include "sys/linux/videomgr.h"
#elif __XBOX
#include "sys/xbox/videomgr.h"
#elif _WIN32
#include "sys/win/videomgr.h"
#endif


#endif