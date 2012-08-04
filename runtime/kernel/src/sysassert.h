// ------------------------------------------------------------------------- //
//
// FILE      : S Y S A S S E R T . H
//
// CREATED   : 02/03/00
//
// AUTHOR    : Matthew Scott
//
// COPYRIGHT : Monolith Productions Inc.
//
// ------------------------------------------------------------------------- //

// This is a redirector to get the system dependent include
#ifdef __LINUX
#include "sys/linux/linuxassert.h"
#elif  __XBOX
#include "sys/xbox/winassert.h"
#elif  _WIN32
#include "sys/win/winassert.h"
#endif
