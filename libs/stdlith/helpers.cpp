//------------------------------------------------------------------
//
//	FILE	  : Helpers.cpp
//
//	PURPOSE	  : Implements the CHelpers class.
//
//	CREATED	  : November 26 1996
//
//	COPYRIGHT : Microsoft 1996 All Rights Reserved
//
//------------------------------------------------------------------

// Includes....
#ifdef _WINDOWS
	#include <windows.h>
#endif

#include <ctype.h>
#include "helpers.h"
#include "stdlithdefs.h"


static char		g_UpperTable[256];
static CHelpers	g_Helpers;



CHelpers::CHelpers()
{
	for( uint32 i=0; i < 256; i++ )
		g_UpperTable[i] = (char)toupper( i );
}


LTBOOL CHelpers::UpperStrcmp( const char *pInputString1, const char *pInputString2 )
{
	int				curPos=0;
	unsigned char	*pStr1 = (unsigned char*)pInputString1, *pStr2 = (unsigned char*)pInputString2;

	while(1)
	{
		if( g_UpperTable[pStr1[curPos]] != g_UpperTable[pStr2[curPos]] )
			return FALSE;

		if( pStr1[curPos] == 0 )
			return TRUE;

		++curPos;
	}

	return FALSE;
}


#ifdef _WINDOWS
	LTBOOL CHelpers::ExtractFullPath( const char *pInPath, char *pOutPath, uint32 outPathLen )
	{
		char		*pFilePart;
		
		if( ::GetFullPathName(pInPath, outPathLen, pOutPath, &pFilePart) == 0 )
			strcpy( pOutPath, pInPath );

		return TRUE;
	}
#endif


LTBOOL CHelpers::ExtractPathAndFileName( const char *pInputPath, char *pPathName, char *pFileName )
{
	char delimiters[2];
	int i, len, lastDelimiter;

	
	delimiters[0] = '\\';
	delimiters[1] = '/';
		
	pPathName[0] = pFileName[0] = 0;

	len = strlen(pInputPath);
	lastDelimiter = -1;
	
	for( i=0; i < len; i++ )
	{
		if( pInputPath[i] == delimiters[0] || pInputPath[i] == delimiters[1] )
		{
			lastDelimiter = i;
		}
	}

	if( lastDelimiter == -1 )
	{
		pPathName[0] = 0;
		strcpy( pFileName, pInputPath );
	}
	else
	{
		memcpy( pPathName, pInputPath, lastDelimiter );
		pPathName[lastDelimiter] = 0;
	
		memcpy( pFileName, &pInputPath[lastDelimiter+1], len-lastDelimiter );
		pFileName[len-lastDelimiter] = 0;
	}

	return TRUE;
}


LTBOOL CHelpers::ExtractFileNameAndExtension( const char *pInputFilename, char *pFilename, char *pExtension )
{
	int		i, len, lastDot;
	char	delimiter = '.';

	len = strlen(pInputFilename);
	lastDot = len;

	for( i=0; i < len; i++ )
	{
		if( pInputFilename[i] == delimiter )
		{
			lastDot = i;
		}
	}
	
	memcpy( pFilename, pInputFilename, lastDot );
	pFilename[lastDot] = 0;

	memcpy( pExtension, &pInputFilename[lastDot+1], len-lastDot );
	pExtension[len-lastDot] = 0;

	return TRUE;
}


LTBOOL CHelpers::ExtractNames( const char *pFullPath, char *pPathname, char *pFilename, char *pFileTitle, char *pExt )
{
	char		pathName[256], fileName[256], fileTitle[256], ext[256];


	ExtractPathAndFileName( pFullPath, pathName, fileName );
	ExtractFileNameAndExtension( fileName, fileTitle, ext );

	if( pPathname )
		strcpy( pPathname, pathName );

	if( pFilename )
		strcpy( pFilename, fileName );

	if( pFileTitle )
		strcpy( pFileTitle, fileTitle );

	if( pExt )
		strcpy( pExt, ext );
	
	return TRUE;
}


char* CHelpers::GetNextDirName( char *pIn, char *pOut )
{
	uint32		len = strlen(pIn);
	uint32		inPos=0, outPos=0;


	// Skip \ characters.
	while( (pIn[inPos] == '/') || (pIn[inPos] == '\\') )
		inPos++;

	if( pIn[inPos] == 0 )
		return NULL;

	while( (pIn[inPos] != '/') && (pIn[inPos] != '\\') && (pIn[inPos] != 0) )
		pOut[outPos++] = pIn[inPos++];

	pOut[outPos] = 0;
	return &pIn[inPos];
}


//determines if the file is relative, or is an absolute path (uses the : as a key)
LTBOOL CHelpers::IsFileAbsolute(const char* pFilename)
{
	for(uint32 nCurrChar = 0; (pFilename[nCurrChar] != '\0') && (pFilename[nCurrChar] != '\\') && (pFilename[nCurrChar] != '/'); nCurrChar++)
	{
		//we hit a colon before the slash or end of the string,
		//this means that it is absolute
		if(pFilename[nCurrChar] == ':')
		{
			return TRUE;
		}
	}

	return FALSE;
}

//takes a filename, and removes the extension from it (including the .)
void CHelpers::RemoveExtension(char* pFilename)
{
	//go to the very end of the string
	int32 nCurrPos = strlen(pFilename) - 1;

	//now we go backwards looking for a dot, or a slash
	for(;nCurrPos >= 0; nCurrPos--)
	{
		//if we have hit a . we need to truncate there
		if(pFilename[nCurrPos] == '.')
		{
			pFilename[nCurrPos] = '\0';
		}

		//see if we have hit a slash and need to end
		if( (pFilename[nCurrPos] == '\\') ||
			(pFilename[nCurrPos] == '/'))
		{
			break;
		}
	}
}

// ----------------------------------------------------------------
// FormatFilename( in-name, out-name, out-name-length )
//
// Convert all the dos like path delimiters to unix like on unix
// platform and vis-versa on winX platforms.
// ----------------------------------------------------------------
void CHelpers::FormatFilename(const char *pFilename, char *pOut, int outLen)
{
	if( !pFilename )
	{
		pOut[0] = '\0';
		return;
	}

	strncpy(pOut, pFilename, outLen);
	pOut[outLen-1] = 0;
	#if defined(__LINUX)
	while(*pOut != 0)
	{
//[dlj] stay with the correct case	*pOut = tolower(*pOut);
		if(*pOut == '\\')
			*pOut = '/';

		++pOut;
	}
	#else
	strupr(pOut);
	while(*pOut != 0)
	{
		if(*pOut == '/')
			*pOut = '\\';

		++pOut;
	}
	#endif
}
