
// This file defines the resource ID used for the version number in
// Lithtech executables.

#ifndef __VERSION_RESOURCE_H__
#define __VERSION_RESOURCE_H__


    #ifndef __BDEFS_H__
	#include "bdefs.h"
    #endif

    #ifndef __VERSION_INFO_H__
	#include "version_info.h"
    #endif


	// Resource ID
	#define VERSION_RESOURCE_ID	12346


	// This routine gets the version resource given the HINSTANCE.
	// Returns LT_OK if it gets it and LT_ERROR if it can't find it.
	LTRESULT GetLTExeVersion(
		HINSTANCE hInstance, 
		LTVersionInfo &info);
	
#endif




