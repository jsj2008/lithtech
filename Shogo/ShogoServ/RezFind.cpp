/****************************************************************************
;
;	 MODULE:		REZFIND (.CPP)
;
;	PURPOSE:		Routines to find the rez files
;
;	HISTORY:		08/11/98 [blg] This file was created
;
;	COMMENT:		Copyright (c) 1998, Monolith Productions Inc.
;
****************************************************************************/


// Includes...

#include "StdAfx.h"
#include "Io.h"
#include "RezFind.h"
#include "Resource.h"


// Globals...

char	g_sGameRez[128] = { "" };
char	g_sSoundRez[128] = { "" };


// Prototypes...

BOOL ExistRezFile(const char* sFile);
BOOL ExistRezDir(const char* sDir);


// Functions...

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FindRezFiles
//
//	PURPOSE:	Looks for game and sound rez files
//
// ----------------------------------------------------------------------- //

BOOL FindRezFiles(HINSTANCE hInst)
{
	// Sanity checks...

	if (!hInst) return(FALSE);


	// Clear the strings...

	g_sGameRez[0] = '\0';
	g_sSoundRez[0] = '\0';


	// Load the rez file base name to use...

	CString sGameRezBase;
	CString sGameRezFile;
	CString sGameRezDir;

	if (!sGameRezBase.LoadString(IDS_REZBASE))
	{
		return(FALSE);
	}

	sGameRezFile = sGameRezBase + ".rez";
	sGameRezDir  = sGameRezBase;


	// Look for game rez in the current directory...

	if (ExistRezFile(sGameRezFile))
	{
		strcpy(g_sGameRez, sGameRezFile);
	}


	// If necessary, look for a rez directory in the current directory...

	if (g_sGameRez[0] == '\0')
	{
		if (ExistRezDir(sGameRezDir))
		{
			strcpy(g_sGameRez, sGameRezDir);
		}
	}


	// Look for sound.rez in the current directory...

	if (ExistRezFile("sound.rez"))
	{
		strcpy(g_sSoundRez, "sound.rez");
	}


	// If necessary, look for a sound directory...

	if (g_sSoundRez[0] == '\0')
	{
		if (ExistRezDir("sound"))
		{
			strcpy(g_sSoundRez, "sound");
		}
	}


	// Determine if we have enough rez info...

	if (g_sGameRez[0] == '\0')
	{
		return(FALSE);
	}


	// All done...

	return(TRUE);
}

char* GetGameRezFile()
{
	return(g_sGameRez);
}

char* GetSoundRezFile()
{
	return(g_sSoundRez);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExistRezFile
//
//	PURPOSE:	Determines if the given file exists
//
// ----------------------------------------------------------------------- //

BOOL ExistRezFile(const char* sFile)
{
	// Sanity checks...

	if (!sFile) return(FALSE);
	if (sFile[0] == '\0') return(FALSE);


	// Check if this file exists...

    struct _finddata_t c_file;
    long   hFile;

    hFile = _findfirst(sFile, &c_file);
	if (hFile == -1L) return(FALSE);

	if (c_file.attrib & _A_SUBDIR)
	{
		return(FALSE);
	}


	// All done...

	return(TRUE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	ExistRezDir
//
//	PURPOSE:	Determines if the given directory exists
//
// ----------------------------------------------------------------------- //

BOOL ExistRezDir(const char* sDir)
{
	// Sanity checks...

	if (!sDir) return(FALSE);
	if (sDir[0] == '\0') return(FALSE);


	// Check if this directory exists...

    struct _finddata_t c_file;
    long   hFile;

    hFile = _findfirst(sDir, &c_file);
	if (hFile == -1L) return(FALSE);

	if (!(c_file.attrib & _A_SUBDIR))
	{
		return(FALSE);
	}


	// All done...

	return(TRUE);
}



