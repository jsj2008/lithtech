// ------------------------------------------------------------------------- //
//
// FILE      : L I B L T K E R N E L . H
//
// CREATED   : 12/14/99
//
// AUTHOR    : Matthew Scott
//
// COPYRIGHT : Monolith Productions Inc.
//
// ------------------------------------------------------------------------- //

#ifndef __LIBLTKERNEL_H__
#define __LIBLTKERNEL_H__

// ------------------------------------------------------------------------- //
// Includes that expose interfaces the LIBLTKERNEL provides
// ------------------------------------------------------------------------- //

// expose the threads interface
#include "systhread.h"
#include "systimer.h"

// expose the old memory interface !! TRY TO REMOVE THIS !!
#include "de_memory.h"

// ------------------------------------------------------------------------- //
// C Functions exposed in LIBLTKERNEL (externs)
// ------------------------------------------------------------------------- //

// FROM: sys/*/startup.c
// called only once ever, the first time the program is launched
extern LTRESULT SystemStart();
// called each time the program starts
extern LTRESULT SystemInit();

// FROM: sys/*/shutdown.c
// called only once ever, when the program exits (never returns)
extern void     SystemEnd();
// called each time the program starts
extern LTRESULT SystemTerm();

// ------------------------------------------------------------------------- //
// Typedefs & Enums
// ------------------------------------------------------------------------- //

#endif  // __LIBLTKERNEL_H__
