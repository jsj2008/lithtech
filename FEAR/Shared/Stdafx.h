/* 
	Shared game code base header
	This header has the includes for both the client and server, to allow for proper compilation
	under both environments.  Note that the client- and server- specific headers also have the
	subdirectories explicitly defined to avoid issues with overlapping filenames that would be
	caused by including these subdirectories in the project settings.
*/

#ifndef __STDAFX_H__
#define __STDAFX_H__

// This removes warnings about truncating symbol names when using stl maps.
//
#pragma warning( disable : 4786 )  
#pragma warning( disable : 4503 )  
#pragma warning( disable : 4530 )  

// Note : This block of includes must come first to avoid issues with windows.h renaming symbols like GetObject->GetObjectA
#include "platform.h"
#if defined( PLATFORM_WIN32 )
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "engine.h"

#include <stdio.h>
#include <limits.h>
#include <malloc.h>

#include "DebugNew.h"

#include "BuildDefines.h"
#ifdef _CLIENTBUILD
#include "clientheaders.h" // engine\sdk\inc
#endif
#include "iltclient.h"
#include "iltserver.h"
#include "iltmessage.h"
#include "iltsoundmgr.h"
#include "Globals.h"
#include "iltmodel.h"
#include "iltphysics.h"
#include "ltobjectcreate.h"
#include "ltbasetypes.h"
#include "ltobjref.h"
#include "Factory.h"
#ifdef _SERVERBUILD
#include "../ObjectDLL/ServerUtilities.h"
#include "../ObjectDLL/GameServerShell.h"
#endif
#ifdef _CLIENTBUILD
#include "..\ClientShellDLL\ClientUtilities.h"
#include "..\ClientShellDLL\GameClientShell.h"
#endif
#include "CommonUtilities.h"
#include "AutoMessage.h"

// In SEM configurations, define the gamespy interface as an export instead of an import
#if defined(PLATFORM_SEM)
	#define GAMESPY_EXPORTS
#endif

// Linux-specific includes
#if defined(PLATFORM_LINUX)
#include <sys/linux/linux_stlcompat.h>
#endif 

#endif // __STDAFX_H__
