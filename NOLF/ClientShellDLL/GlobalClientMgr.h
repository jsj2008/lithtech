// ----------------------------------------------------------------------- //
//
// MODULE  : GlobalClientMgr.h
//
// PURPOSE : Definition of client global definitions
//
// CREATED : 7/07/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __GLOBAL_CLIENT_MGR_H__
#define __GLOBAL_CLIENT_MGR_H__

#include "GlobalMgr.h"
#include "ClientButeMgr.h"
#include "ClientSoundMgr.h"

class CGlobalClientMgr : public CGlobalMgr
{
	public :

		~CGlobalClientMgr();

        LTBOOL Init();

	protected :

		virtual void ShutdownWithError(char* pMgrName, char* pButeFilePath);

	private :

		CClientButeMgr	m_ClientButeMgr;	// Same as g_pClientButeMgr
		CClientSoundMgr	m_ClientSoundMgr;	// Same as g_pClientSoundMgr
};

#endif // __GLOBAL_CLIENT_MGR_H__