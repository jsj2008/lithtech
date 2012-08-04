//------------------------------------------------------------------
//
//	FILE	  : Helpers.h
//
//	PURPOSE	  : Defines the CHelpers class, which contains lots of
//              helpful little functions.
//
//	CREATED	  : November 26 1996
//
//	COPYRIGHT : Microsoft 1996 All Rights Reserved
//
//------------------------------------------------------------------

#ifndef __HELPERS_H__
	#define __HELPERS_H__


	// Includes....
	#include "StdLithDefs.h"


	class CHelpers
	{
		public:

							CHelpers();			
			
			static BOOL		UpperStrcmp( const char *pStr1, const char *pStr2 );	

#ifdef _WINDOWS
			static BOOL		ExtractFullPath( const char *pInPath, char *pOutPath, DWORD outPathLen=256 );
#endif
			
			static BOOL		ExtractPathAndFileName( const char *pInputPath, char *pPathName, char *pFileName );
			static BOOL		ExtractFileNameAndExtension( const char *pInputFilename, char *pFilename, char *pExtension );
			static BOOL		ExtractNames( const char *pFullPath, char *pPathname, char *pFilename, char *pFiletitle, char *pExt );

			// Returns a pointer to the position in pIn where the next directory starts.
			// Fills in the string in pOut with the directory name that pIn points at.
			// Returns NULL if there is no directory name there.
			static char*	GetNextDirName( char *pIn, char *pOut );

	};


#endif  // __HELPERS_H__

