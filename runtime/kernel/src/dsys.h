// ------------------------------------------------------------------------- //
//
// FILE      : D S Y S _ I N T E R F A C E . H
//
// CREATED   : 
//
// AUTHOR    : 
//
// COPYRIGHT : Monolith Productions Inc.
//
//
// ------------------------------------------------------------------------- //

#ifndef __DSYS_H__
#define __DSYS_H__

// This is a redirector to get the system dependent include
#ifdef __LINUX
#include "sys/linux/linuxdsys.h"
#elif __XBOX
#include "sys/xbox/xbox_dsys.h"
#elif _WIN32
#include "sys/win/dsys_interface.h"
#endif



#endif

