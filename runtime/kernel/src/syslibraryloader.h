// *********************************************************************** //
//
// MODULE  : ltlibraryloader.h
//
// PURPOSE : Helpers for loading libraries with the dynamic linker.
//
// CREATED : 05/21/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// *********************************************************************** //

#ifndef __LTLIBRARYLOADER_H__
#define __LTLIBRARYLOADER_H__

//#include "platform.h"

// types
typedef void*	HLTMODULE;
typedef void*	HLTPROC;

class LTLibraryLoader
{
public:

	// Load the specified module - returns NULL if not found, otherwise returns
	// a handle.  Note that calls to LoadLibrary are reference counted.
	static HLTMODULE OpenLibrary(const char* pszFileName);

	// Unload the specified module.  Note that the library is actually freed only
	// when the reference count reaches zero.
	static void      CloseLibrary(const HLTMODULE hModule);

	// Checks to see if the specified library is already loaded.  Does not affect
	// reference count.
	static bool      IsLibraryLoaded(const char* pszFileName);

	// Gets the handle of a loaded library (returns NULL if the library has not been loaded).
	static HLTMODULE GetLibraryHandle(const char* pszFileName);

	// Gets the main module's handle. This is the module that was used to create 
	// the process.
	static HLTMODULE GetMainHandle();

	// Get the address of the function with the specified name - returns NULL if
	// not found.
	static HLTPROC   GetProcAddress(HLTMODULE hModule, const char* pszProcName);

};


#endif // __LTLIBRARYLOADER_H__
