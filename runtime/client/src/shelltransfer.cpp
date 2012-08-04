//------------------------------------------------------------------
//
//	FILE	  : ShellTransfer.cpp
//
//	PURPOSE	  : 
//
//	CREATED	  : March 5 1997
//							 
//	COPYRIGHT : Microsoft 1997 All Rights Reserved
//
//------------------------------------------------------------------

#include "bdefs.h"

#include "clientshell.h"
#include "console.h"
#include "ltpvalue.h"


extern char g_CacheDir[];



bool CClientShell::DoYouHaveThisFile( const char *pFilename, uint32 size, void *pTimeMeasure )
{
	return true;	// THIS DISABLES ALL THE FILE TRANSFER STUFF...
}


bool CClientShell::OpenFile( const char *pFilename, CAbstractIO **ppFile )
{
	return false;
}


void CClientShell::CloseFile( const char *pFilename, uint8 *pFileTime, CAbstractIO *pFile, bool bValidFile )
{
}


void CClientShell::OnFileTransferDone(CBaseConn *connID)
{
	con_Printf( CONRGB(100,100,250), 1, "File transfer/list done" );
}


void CClientShell::OnKilledTransfer( CBaseConn *connID, int reason )
{
}


bool CClientShell::MakeDirectoryExist( const char *pHeader, const char *pDirName )
{
	return true;
}




