// ------------------------------------------------------------------------- //
//
// FILE      : S Y S C L I E N T . H
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

#ifndef __SYSCLIENT_H__
#define __SYSCLIENT_H__

// This is a redirector to get the system dependent include
#ifdef __LINUX
#include "sys/linux/linuxclient.h"
#elif _WIN32
#include "sys/win/winclient.h"
#endif


#endif