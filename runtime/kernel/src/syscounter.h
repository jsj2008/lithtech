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

#ifndef __SYSCOUNTER_H__
#define __SYSCOUNTER_H__

// This is a redirector to get the system dependent include
#ifdef __LINUX
#include "sys/linux/counter.h"
#elif  __XBOX
#include "sys/xbox/counter.h"
#elif  _WIN32
#include "sys/win/counter.h"
#endif


#endif

