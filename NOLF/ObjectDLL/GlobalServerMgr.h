// ----------------------------------------------------------------------- //
//
// MODULE  : GlobalServerMgr.h
//
// PURPOSE : Definition of server global definitions
//
// CREATED : 7/07/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __GLOBAL_SERVER_MGR_H__
#define __GLOBAL_SERVER_MGR_H__

#include "GlobalMgr.h"
#include "ServerButeMgr.h"

#include "ServerSoundMgr.h"
#include "AIButeMgr.h"
#include "AttachButeMgr.h"
#include "AnimationMgrHuman.h"
#include "PropTypeMgr.h"
#include "IntelMgr.h"

class CGlobalServerMgr : public CGlobalMgr
{
	public :
		~CGlobalServerMgr();

        LTBOOL Init();

	protected :

		virtual void	ShutdownWithError(char* pMgrName, char* pButeFilePath);

	private :

		CServerSoundMgr		m_ServerSoundMgr;		// Same as g_pServerSoundMgr
		CAIButeMgr			m_AIButeMgr;			// Same as g_pAIButeMgr
		CAttachButeMgr		m_AttachButeMgr;		// Same as g_pAttachButeMgr
		CAnimationMgrHuman	m_AnimationMgrHuman;	// Same as g_pAnimationMgrHuman
		CPropTypeMgr		m_PropTypeMgr;			// Same as g_pPropTypeMgr
		CIntelMgr			m_IntelMgr;				// Same as g_pIntelMgr

		CServerButeMgr		m_ServerButeMgr;		// Same as g_pServerButeMgr
};


#endif // __GLOBAL_SERVER_MGR_H__