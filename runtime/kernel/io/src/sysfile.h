// ------------------------------------------------------------------------- //
//
// FILE      : S Y S F I L E . H
//
// CREATED   : 12/06/99
//
// AUTHOR    : 
//
// COPYRIGHT : Monolith Productions Inc.
//
// ORIGN     : 
//
// ------------------------------------------------------------------------- //

#ifndef __SYSFILE_H__
#define __SYSFILE_H__

// This is a redirector to get the system dependent include
#ifdef __LINUX
#define MAX_FILETREES_TO_SEARCH		(32)
#include "sys/linux/linuxfile.h"
#elif _WIN32
#define MAX_FILETREES_TO_SEARCH		(32)
#include "sys/win/de_file.h"
#endif

#endif

