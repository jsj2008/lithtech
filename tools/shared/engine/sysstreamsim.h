// ------------------------------------------------------------------------- //
//
// FILE      : S Y S S T R E A M S I M . H
//
// CREATED   : 11/05/99
//
// AUTHOR    : 
//
// COPYRIGHT : Monolith Productions Inc.
//
// ORIGN     : lithtech 1.5/2.0 lthread.h
//
// ------------------------------------------------------------------------- //

#ifndef __SYSSTREAMSIM_H__
#define __SYSSTREAMSIM_H__

// This is a redirector to get the system dependent include
#ifdef __LINUX
#include "sys/linux/streamsim.h"
#elif __PS2
#include "sys/ps2/streamsim.h"
#elif _WIN32
#include "sys/win/streamsim.h"
#endif


#endif

