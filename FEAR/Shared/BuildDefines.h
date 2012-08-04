// ----------------------------------------------------------------------- //
//
// MODULE  : BuildDefines.h
//
// PURPOSE : Definition of build variables, like _DEMO.
//
// CREATED : 08/26/03
//
// (c) 2000-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __BUILDDEFINES_H__
#define __BUILDDEFINES_H__

// Project defines.  Project should define its unique project define here.  Project
// defines take the form of PROJECT_<projectname>.  Only one PROJECT_<projectname>
// can be defined at one time.
#define PROJECT_FEAR
//#define PROJECT_DARK

// Flags code that should be part of any demo
//#define _DEMO

// Flags code that should be part of the MP Demo.
//#define _MPDEMO

// Flags code that should be part of the SP Demo.
//#define _SPDEMO

// Defines to tell this product to check for cdkey 1, the main cdkey.
#define ENABLE_CDKEY_1_CHECK
// Defines to tell this product to check for cdkey 2, the additional content cdkey.
//#define ENABLE_CDKEY_2_CHECK

// Make sure the demo flag is set if any of the demo sku's are defined.
#if defined( _MPDEMO ) || defined( _SPDEMO )
#define _DEMO
#endif // defined( _MPDEMO ) || defined( _SPDEMO )

// This is used as the registry entry.
// This is used for folder paths and registry entries.  Don't put invalid characters into the name.
// This is not to be localized.
#define GAME_NAME "FEAR"

#if defined( PUBLIC_TOOLS )
// Executable names.
#define LAUNCHER_EXE		"FEARDevSP.exe"		
#define LAUNCHER_EXE_SP		LAUNCHER_EXE		
#define LAUNCHER_EXE_MP		"FEARDevMP.exe"
#else // defined( PUBLIC_TOOLS )
#define LAUNCHER_EXE		"FEAR.exe"		
#define LAUNCHER_EXE_SP		LAUNCHER_EXE		
#define LAUNCHER_EXE_MP		"FEARMP.exe"
#endif // defined( PUBLIC_TOOLS )

#endif // __BUILDDEFINES_H__
