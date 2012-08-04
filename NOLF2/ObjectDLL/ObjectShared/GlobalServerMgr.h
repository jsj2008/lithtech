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

class CServerSoundMgr;
class CAIButeMgr;
class CAIGoalButeMgr;
class CAttachButeMgr;
class CAnimationMgrList;
class CRelationMgr;
class CPropTypeMgr;
class CIntelMgr;
class CKeyMgr;
class CSearchItemMgr;
class CGadgetTargetMgr;
class CCommandButeMgr;
class CServerButeMgr;
class CInventoryButeMgr;
class CTransitionMgr;


class CGlobalServerMgr : public CGlobalMgr
{
	public :
		CGlobalServerMgr( );
		~CGlobalServerMgr( ) { Term( ); }

        LTBOOL Init();
		void Term( );

	protected :

		virtual void	ShutdownWithError(char* pMgrName, char* pButeFilePath);

	private :

		CServerSoundMgr*	m_pServerSoundMgr;		// Same as g_pServerSoundMgr
		CAIButeMgr*			m_pAIButeMgr;			// Same as g_pAIButeMgr
		CAIGoalButeMgr*		m_pAIGoalButeMgr;		// Same as g_pAIGoalButeMgr
		CAttachButeMgr*		m_pAttachButeMgr;		// Same as g_pAttachButeMgr
		CAnimationMgrList*	m_pAnimationMgrList;	// Same as g_pAnimationMgrList
		CPropTypeMgr*		m_pPropTypeMgr;			// Same as g_pPropTypeMgr
		CIntelMgr*			m_pIntelMgr;			// Same as g_pIntelMgr
		CKeyMgr*			m_pKeyMgr;				// Same as g_pKeyMgr
		CSearchItemMgr*		m_pSearchItemMgr;		// Same as g_pSearchItemMgr
		CGadgetTargetMgr*	m_pGadgetTargetMgr;		// Same as g_pGadgetTargetMgr
		CCommandButeMgr*	m_pCommandButeMgr;		// Same ar g_pCommandButeMgr
		CServerButeMgr*		m_pServerButeMgr;		// Same as g_pServerButeMgr
		CInventoryButeMgr*	m_pInventoryButeMgr;	// Same as g_pInventoryButeMgr
		CTransitionMgr*		m_pTransitionMgr;		// Same as g_pTransMgr.
};


#endif // __GLOBAL_SERVER_MGR_H__
