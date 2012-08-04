// ------------------------------------------------------------------------- //
//
// FILE      : S Y S C L I E N T D E _ I M P L . H
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

#ifndef __SYSCLIENTDE_IMPL_H__
#define __SYSCLIENTDE_IMPL_H__

// This is a redirector to get the system dependent include
#ifdef __LINUX
#include "sys/linux/linuxclientde_impl.h"
#elif _WIN32
#include "sys/win/winclientde_impl.h"
#endif


#endif

