// ----------------------------------------------------------------------- //
//
// MODULE  : GlobalClientMgr.cpp
//
// PURPOSE : Implementations of client global definitions
//
// CREATED : 7/07/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "GlobalClientMgr.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGlobalClientMgr::Init()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

LTBOOL CGlobalClientMgr::Init()
{
    if (!m_ClientSoundMgr.Init(g_pLTClient))
	{
		char errorBuf[256];
		sprintf(errorBuf, "ERROR in CGlobalClientMgr::Init()\n\nCouldn't initialize ClientSoundMgr.  Make sure the %s file is valid!", CSNDMGR_DEFAULT_FILE);
        g_pLTClient->ShutdownWithMessage(errorBuf);
        return LTFALSE;
	}

    if (!CGlobalMgr::Init(g_pLTClient)) return LTFALSE;

	if (!m_ClientButeMgr.Init(g_pLTClient))
	{
		char errorBuf[256];
		sprintf(errorBuf, "ERROR in CGlobalClientMgr::Init()\n\nCouldn't initialize ClientButeMgr.  Make sure the %s file is valid!", CBMGR_DEFAULT_FILE);
        g_pLTClient->ShutdownWithMessage(errorBuf);
        return LTFALSE;
	}

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGlobalClientMgr::~CGlobalClientMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CGlobalClientMgr::~CGlobalClientMgr()
{
	m_ClientSoundMgr.Term();

    m_ClientButeMgr.Term();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGlobalClientMgr::ShutdownWithError()
//
//	PURPOSE:	Shutdown the client with an error
//
// ----------------------------------------------------------------------- //

void CGlobalClientMgr::ShutdownWithError(char* pMgrName, char* pButeFilePath)
{
	char errorBuf[256];
	sprintf(errorBuf, "ERROR in CGlobalClientMgr::Init()\n\nCouldn't initialize %s.  Make sure the %s file is valid!", pMgrName, pButeFilePath);
    g_pLTClient->ShutdownWithMessage(errorBuf);
}