/****************************************************************************
;
;	MODULE:		RezTypes (.H)
;
;	PURPOSE:
;
;	HISTORY:	04/08/95 [m]
;
;	NOTICE:		Copyright (c) 1995, MONOLITH, Inc.
;
****************************************************************************/

#ifndef __REZTYPES_H__
#define __REZTYPES_H__

// Types specific to the resource manager

typedef const char* REZCDIRNAME;   // The name of a resource directory (constant)
typedef const char* REZCNAME;      // The name of a resource (constant)

typedef char* REZDIRNAME;   // The name of a resource directory
typedef char* REZNAME;		// The name of a resource
typedef char* REZDESC;		// The description for a resource
typedef char* REZPATH;		// The path of a resource
typedef DWORD REZID;		// The ID of a resource
typedef DWORD REZTYPE;		// The type of a resource
typedef DWORD REZSIZE;		// The size type for a resource
typedef DWORD REZKEYINDEX;	// The index value for a resource key
typedef DWORD REZKEYVAL;	// The value in a resource key
typedef DWORD REZTIME;     // The time value for resources

#endif

