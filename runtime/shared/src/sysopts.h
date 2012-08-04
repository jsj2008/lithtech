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

#ifndef __SYSOPTS_H__
#define __SYSOPTS_H__

// This is a redirector to get the system dependent include
#ifdef __LINUX
#include "sys/linux/linuxopt.h"
#elif _WIN32
#include "sys/win/winopts.h"
#endif


#endif
