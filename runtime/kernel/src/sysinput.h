// ------------------------------------------------------------------------- //
//
// FILE      : S Y S I N P U T . H
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

#ifndef __SYSINPUT_H__
#define __SYSINPUT_H__


// This is a redirector to get the system dependent include
#ifdef __LINUX
#include "sys/linux/input.h"
#elif  __XBOX
#include "sys/xbox/input.h"
#elif  _WIN32
#include "sys/win/input.h"
#endif


#endif