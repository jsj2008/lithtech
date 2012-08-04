//
//   (c) 1998-1999 Monolith Productions, Inc.  All Rights Reserved
//
// ---------------------------------------------------------------
//
#ifndef __OLDTYPES_H__
#define __OLDTYPES_H__

#include "ltbasetypes.h"

// These have been removed from the engine and replaced with size specific types.
// However, they have not yet been removed from the tools sl we need to setup
// a compatability with the two usages
#undef DWORD
#define DWORD uint32
#undef SDWORD
#define SDWORD int32
#undef WORD
#define WORD uint16
#undef SWORD
#define SWORD int16
#undef BYTE
#define BYTE uint8

#endif