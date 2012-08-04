// ----------------------------------------------------------------------- //
//
// MODULE  : GameButeMgr.cpp
//
// PURPOSE : GameButeMgr - Implementation
//
// CREATED : 03/30/99
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "GameButeMgr.h"

#define BUTE_DEBUG_LEVEL		5

void GBM_DisplayError(const char* szMsg)
{
    LTFLOAT fVal = 0.0f;

#ifdef __PSX2
    if (!g_pLTClient) return;

    HCONSOLEVAR hVar = g_pLTClient->GetConsoleVar("DebugLevel");
	if(hVar)
	{
        fVal = g_pLTClient->GetVarValueFloat(hVar);
	}
	if (fVal >= BUTE_DEBUG_LEVEL)
	{
        printf(szMsg);
    }

#else

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
#endif
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameButeMgr::Parse()
//
//	PURPOSE:	Parse from a rez file
//
// ----------------------------------------------------------------------- //

LTBOOL CGameButeMgr::Parse(const char* sButeFile)
{
	// Sanity checks...

    if (!sButeFile)	return(LTFALSE);


	BOOL bRet = TRUE;


	//if there is no g_pLTBase, then we can't read from the stream
	if (!g_pLTBase || !m_bInRezFile)
	{

		// Append the GAME directory onto the filename if this file is normally
		// stored in the .rez file...

		if (m_bInRezFile)
		{
			m_strAttributeFile.Format("Game\\%s", sButeFile);
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
	}


	// Open the file...

	m_strAttributeFile = sButeFile;

    ILTStream* pDStream = LTNULL;

    LTRESULT dr = g_pLTBase->OpenFile(m_strAttributeFile, &pDStream);

    if (dr != LT_OK || !pDStream)
	{
		char sError[512];
		sprintf(sError,"ERROR CGameButeMgr couldn't open file %s!",m_strAttributeFile);
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
		GBM_DisplayError("ERROR CGameButeMgr couldn't allocate data for stream.");
		return(FALSE);
	}

	pDStream->Read(pData, uLen);


	// Setup the save file name.  This is for saving the attribute file from
	// the game and is ONLY used during development...

	CString strSaveFilename = "";

#ifndef _FINAL

	strSaveFilename = sButeFile;

	if (m_bInRezFile)
	{
		strSaveFilename.Format("Game\\%s", sButeFile);
	}

#endif // _FINAL


	// Parse the file...

	if (m_pCryptKey)
	{
		bRet = m_buteMgr.Parse(pData, uLen, m_pCryptKey, strSaveFilename);
	}
	else
	{
		bRet = m_buteMgr.Parse(pData, uLen, 0, strSaveFilename);
	}


	// Clean up...

	pDStream->Release();
	debug_deletea(pData);


	// Check for an error...

	if (!bRet)
	{
		TRACE("ERROR CGameButeMgr::Parse() (%s)!\n", sButeFile);
		return(FALSE);
	}


	// All done...

	return(TRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGameButeMgr::Save()
//
//	PURPOSE:	Save to a file
//
// ----------------------------------------------------------------------- //

void CGameButeMgr::Save()
{

	if (m_bInRezFile)
	{
		ASSERT(!"ERROR CGameButeMgr can't save a rezzed file!");
		return;
	}

	m_buteMgr.Save(m_strAttributeFile);
}