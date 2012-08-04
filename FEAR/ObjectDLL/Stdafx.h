// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#ifndef __STDAFX_H__
#define __STDAFX_H__

// This removes warnings about truncating symbol names when using stl maps.
//
#pragma warning( disable : 4786 )  

// increase the stack buffer size for MPW2A and MPA2W
// if the string to be converted is less than 
// STRCONV_BUFFER_LENGTH characters then no allocations will be performed
#ifndef STRCONV_BUFFER_LENGTH
#	define STRCONV_BUFFER_LENGTH 256
#endif

// Note : This block of includes must come first to avoid issues with windows.h renaming symbols like GetObject->GetObjectA
#include "platform.h"
#if defined( PLATFORM_WIN32 ) && !defined( _WINDOWS_ )
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <crtdbg.h>
#endif

#include "engine.h"

#include <stdio.h>
#include <limits.h>
#include <float.h>
#include <vector>
#include <map>
#include <algorithm>

#include "stdlith.h"
#include "ltassert.h"

#include "DebugNew.h"

#include "BuildDefines.h"
#include "iltclient.h"
#include "iltserver.h"
#include "iltmessage.h"
#include "Globals.h"

#include "iltmodel.h"
#include "iltphysics.h"
#include "iltsoundmgr.h"
#include "ltobjectcreate.h"
#include "ltbasetypes.h"

#include "ltobjref.h"

#include "Factory.h"

#include "ServerUtilities.h"
#include "GameServerShell.h"
#include "CommonUtilities.h"
#include "AutoMessage.h"

// This is cuts the compile time for this project approximately in half.
#include "AI.h"
#include "AIBlackBoard.h"
#include "AIUtils.h"

// In SEM configurations, define the gamespy interface as an export instead of an import
#if defined(PLATFORM_SEM)
	#define GAMESPY_EXPORTS
#endif

#endif // __STDAFX_H__
