// ----------------------------------------------------------------------- //
//
// MODULE  : GameButeMgr.cpp
//
// PURPOSE : GameButeMgr - Implementation
//
// CREATED : 03/30/99
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "GameButeMgr.h"

#define BUTE_DEBUG_LEVEL		5

void GBM_DisplayError(const char* szMsg)
{
    LTFLOAT fVal = 0.0f;

#ifdef _CLIENTBUILD

    if (!g_pLTClient) return;

    HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar("DebugLevel");
	if(hVar)
	{
        fVal = g_pLTClient->GetVarValueFloat(hVar);
	}
#else

    if (!g_pLTServer) return;

    HCONVAR hVar = g_pLTServer->GetGameConVar("DebugLevel");
	if (hVar)
	{
        fVal = g_pLTServer->GetVarValueFloat(hVar);
	}
#endif

	if (fVal >= BUTE_DEBUG_LEVEL)
	{
#ifdef _CLIENTBUILD
        if (g_pLTClient)
		{
            g_pLTClient->CPrint((char*)szMsg);
		}
#else
        if (g_pLTServer)
		{
            g_pLTServer->CPrint((char*)szMsg);
		}
#endif
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameButeMgr::Parse()
//
//	PURPOSE:	Parse from a rez file
//
// ----------------------------------------------------------------------- //

LTBOOL CGameButeMgr::Parse(ILTCSBase *pInterface, const char* sButeFile)
{
	// Sanity checks...

    if (!sButeFile)	return(LTFALSE);


	BOOL bRet = TRUE;

	// NOTE!!! When _REZFILE is defined, this code will need to be 
	// updated to support being called from DEdit!!!!
	// (from a IObjectPlugin::PreHook_EditStringList() call)...

#if !defined(_REZFILE)

	// If we're going to allow the bute file to be saved by the game, it must
	// be read in from a file (not a .rez file)...

	// Append the NOLF directory onto the filename if this file is normally
	// stored in the .rez file...

	if (m_bInRezFile)
	{
		m_strAttributeFile.Format("NOLF\\%s", sButeFile);
	}
	else
	{
		m_strAttributeFile.Format("%s", sButeFile);
	}


	if (m_pCryptKey)
	{
		bRet = m_buteMgr.Parse(m_strAttributeFile, m_pCryptKey);
	}
	else
	{
		bRet = m_buteMgr.Parse(m_strAttributeFile);
	}

	return bRet;

#endif  // !_REZFILE

	m_strAttributeFile = sButeFile;

	// Open the file...

	char sConstFile[256];
	strncpy(sConstFile, sButeFile, 255);

    ILTStream* pDStream;

    LTRESULT dr = pInterface->OpenFile(sConstFile, &pDStream);

    if (dr != LT_OK)
	{
		char sError[512];
		sprintf(sError,"ERROR CGameButeMgr couldn't open file %s!",sButeFile);
		GBM_DisplayError(sError);

#ifdef _CLIENTBUILD
        g_pLTClient->ShutdownWithMessage(sError);
#endif
		return(FALSE);
	}


	// Read the file...

	unsigned long uLen = pDStream->GetLen();

	char* pData = debug_newa(char, uLen);
	if (!pData)
	{
		pDStream->Release();
		GBM_DisplayError("CGameButeMgr couldn't allocate data for stream.");
		return(FALSE);
	}

	pDStream->Read(pData, uLen);


	// Parse the file...

	if (m_pCryptKey)
	{
		bRet = m_buteMgr.Parse(pData, uLen, m_pCryptKey);
	}
	else
	{
		bRet = m_buteMgr.Parse(pData, uLen);
	}


	// Clean up...

	pDStream->Release();
	debug_deletea(pData);


	// Check for an error...

	if (!bRet)
	{
		TRACE("CGameButeMgr::Parse() ERROR!\n");
		return(FALSE);
	}


	// All done...

	return(TRUE);
}