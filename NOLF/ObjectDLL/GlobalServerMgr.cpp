// ----------------------------------------------------------------------- //
//
// MODULE  : GlobalServerMgr.cpp
//
// PURPOSE : Implementations of server global definitions
//
// CREATED : 7/07/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "GlobalServerMgr.h"

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGlobalServerMgr::Init()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

LTBOOL CGlobalServerMgr::Init()
{
	m_ServerSoundMgr.Init(g_pLTServer);

    // Since Client & Server are the same on PS2, we only need to do this once (when called by the client)
    if (!CGlobalMgr::Init(g_pLTServer)) return LTFALSE;

    m_AIButeMgr.Init(g_pLTServer);
    m_AttachButeMgr.Init(g_pLTServer);
    m_ServerButeMgr.Init(g_pLTServer);
    m_AnimationMgrHuman.Init();

	m_PropTypeMgr.Init(g_pLTServer);
	m_IntelMgr.Init(g_pLTServer);

    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGlobalServerMgr::~CGlobalServerMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CGlobalServerMgr::~CGlobalServerMgr()
{
	m_ServerSoundMgr.Term();
	m_AIButeMgr.Term();
	m_AttachButeMgr.Term();
	m_ServerButeMgr.Term();
	m_AnimationMgrHuman.Term();

	m_PropTypeMgr.Term();
	m_IntelMgr.Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGlobalServerMgr::ShutdownWithError()
//
//	PURPOSE:	Shutdown all clients with an error
//
// ----------------------------------------------------------------------- //

void CGlobalServerMgr::ShutdownWithError(char* pMgrName, char* pButeFilePath)
{
	char errorBuf[256];
	sprintf(errorBuf, "ERROR in CGlobalServerMgr::Init()\n\nCouldn't initialize %s.  Make sure the %s file is valid!", pMgrName, pButeFilePath);
    g_pLTServer->CPrint(errorBuf);

	// TO DO:
	// Send a message to all clients to shut down (also send error string!!!)
	//
}