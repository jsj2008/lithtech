// ------------------------------------------------------------------------- //
//
// FILE      : S T D L T E R R O R . C P P
//
// CREATED   : 10/22/99
//
// AUTHOR    : Matthew Scott
//
// COPYRIGHT : LithTech Inc.  All Rights Reserved.
//
// ORIGN     : 
//
// ------------------------------------------------------------------------- //

#include "bdefs.h"	// for VC++ precompiled headers

// ------------------------------------------------------------------------- //
// Includes this module depends on
// ------------------------------------------------------------------------- //
#include "stdlterror.h"


// ------------------------------------------------------------------------- //
// Globals
// ------------------------------------------------------------------------- //

// choose the correct type for the error class
#ifdef __WIN32
#include "sys/win/winstdlterror.h"
CWINError STDLTError;
#elif __LINUX
#include "sys/linux/linuxstdlterror.h"
CLINUXError STDLTError;
#else
CNULLError  STDLTError;
#endif

IBaseError *g_pSTDLTError = &STDLTError;
