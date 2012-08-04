// ------------------------------------------------------------------------- //
//
// FILE      : S Y S O P T S . H
//
// CREATED   : 08 / 14 / 00
//
// AUTHOR    : 
//
// COPYRIGHT : Monolith Productions Inc.
//
// ------------------------------------------------------------------------- //

#ifndef __LTSYSOPTIM_H__
#define __LTSYSOPTIM_H__

// This is a redirector to get the system dependent include
#ifdef __LINUX
#include "sys/linux/linuxoptim.h"
#elif _WIN32
#include "sys/win/winoptim.h"
#endif


#endif // __LTSYSOPTIM_H__
