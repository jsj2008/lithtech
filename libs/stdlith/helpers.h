//------------------------------------------------------------------
//
//  FILE      : Helpers.h
//
//  PURPOSE   : Defines the CHelpers class, which contains lots of
//              helpful little functions.
//
//  CREATED   : November 26 1996
//
//  COPYRIGHT : Microsoft 1996 All Rights Reserved
//
//------------------------------------------------------------------

#ifndef __HELPERS_H__
#define __HELPERS_H__


// Includes....
#ifndef __STDLITHDEFS_H__
#include "stdlithdefs.h"
#endif

class CHelpers
{
    public: 

                        CHelpers();         
        
        static LTBOOL       UpperStrcmp(const char *pStr1, const char *pStr2);    

        #ifdef _WINDOWS
        static LTBOOL       ExtractFullPath(const char *pInPath, char *pOutPath, uint32 outPathLen=256);
        #endif
        
        static LTBOOL       ExtractPathAndFileName(const char *pInputPath, char *pPathName, char *pFileName);
        static LTBOOL       ExtractFileNameAndExtension(const char *pInputFilename, char *pFilename, char *pExtension);
        static LTBOOL       ExtractNames(const char *pFullPath, char *pPathname, char *pFilename, char *pFiletitle, char *pExt);

		//determines if the file is relative, or is an absolute path (uses the : as a key)
		static LTBOOL		IsFileAbsolute(const char* pFilename);

		//takes a filename, and removes the extension from it (including the .)
		static void			RemoveExtension(char* pFilename);
	
		// on linux, lowcase, flip the dir delimiters, on windows upcase, smile contemtuously.
		static void			FormatFilename(const char *pFilename, char *pOut, int outLen);


        // Returns a pointer to the position in pIn where the next directory starts.
        // Fills in the string in pOut with the directory name that pIn points at.
        // Returns NULL if there is no directory name there.
        static char*    GetNextDirName(char *pIn, char *pOut);

};


#endif  // __HELPERS_H__

