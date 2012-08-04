#ifndef __STDAFX_H_
	#define __STDAFX_H_

// This removes warnings about truncating symbol names when using stl maps.
//
#pragma warning( disable : 4786 )  

// increase the stack buffer size for MPW2A and MPA2W
// if the string to be converted is less than 
// STRCONV_BUFFER_LENGTH characters then no allocations will be performed
#ifndef STRCONV_BUFFER_LENGTH
#	define STRCONV_BUFFER_LENGTH 256
#endif

#include "engine.h"

#include <stdio.h>

#if defined( PLATFORM_WIN32 )
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <limits.h>

#include "DebugNew.h"

#include "BuildDefines.h"
#include "iltclient.h"
#include "iltserver.h"
#include "iltmessage.h"
#include "globals.h"

#include "iltmodel.h"
#include "iltphysics.h"
#include "iltsoundmgr.h"
#include "ltobjref.h"
#include "ltobjectcreate.h"
#include "iltcommon.h"

#include "TemplateList.h"

extern ILTClient *g_pLTClient;

#endif
