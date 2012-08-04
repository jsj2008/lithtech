// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#ifndef __STDAFX_H__
#define __STDAFX_H__

// This removes warnings about truncating symbol names when using stl maps.
//
#pragma warning( disable : 4786 )  
#pragma warning( disable : 4503 )  
#pragma warning( disable : 4530 )  

// increase the stack buffer size for MPW2A and MPA2W
// if the string to be converted is less than 
// STRCONV_BUFFER_LENGTH characters then no allocations will be performed
#ifndef STRCONV_BUFFER_LENGTH
#	define STRCONV_BUFFER_LENGTH 256
#endif

#include "engine.h"

#if defined( PLATFORM_WIN32 )
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <stdio.h>
#include <limits.h>

#include "DebugNew.h"

#include "BuildDefines.h"
#include "clientheaders.h"
#include "iltserver.h"
#include "iltmessage.h"
#include "iltsoundmgr.h"
#include "globals.h"
#include "iltmodel.h"
#include "iltphysics.h"
#include "iltsoundmgr.h"
#include "ltobjectcreate.h"
#include "ltbasetypes.h"
#include "ltobjref.h"
#include "Factory.h"
#include "ClientUtilities.h"
#include "CommonUtilities.h"
#include "AutoMessage.h"
#include "GameClientShell.h"

// In SEM configurations, define the gamespy interface as an export instead of an import
#if defined(PLATFORM_SEM)
	#define GAMESPY_EXPORTS
#endif

#endif // __STDAFX_H__