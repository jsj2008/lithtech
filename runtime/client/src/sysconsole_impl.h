// ------------------------------------------------------------------------- //
//
// FILE      : S Y S C O N S O L E _ I M P L . H
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

#ifndef __SYSCONSOLE_IMPL_H__
#define __SYSCONSOLE_IMPL_H__

// This is a redirector to get the system dependent include
#ifdef __LINUX
#include "sys/linux/linuxconsole_impl.h"
#elif _WIN32
#include "sys/win/winconsole_impl.h"
#endif

#endif

