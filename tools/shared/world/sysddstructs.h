// ------------------------------------------------------------------------- //
//
// FILE      : S Y S D D S T R U C T S . H
//
// CREATED   : 08 / 14 / 00
//
// AUTHOR    : 
//
// COPYRIGHT : Monolith Productions Inc.
//
// ------------------------------------------------------------------------- //

#ifndef __SYSDDSTRUCTS_H__
#define __SYSDDSTRUCTS_H__

// This is a redirector to get the system dependent include
#ifdef __LINUX
#include "sys/linux/linuxddstructs.h"
#elif __PS2
#include "sys/ps2/ps2ddstructs.h"
#elif _WIN32
#include "sys/win/d3dddstructs.h"
#endif


#endif