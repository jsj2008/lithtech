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

class CClientButeMgr;
class CClientSoundMgr;
class CKeyMgr;
class CSearchItemMgr;
class CPopupMgr;

class CGlobalClientMgr : public CGlobalMgr
{
	public :

		CGlobalClientMgr( );
		~CGlobalClientMgr();

        LTBOOL Init( );
		void Term( );

	protected :

		virtual void ShutdownWithError(char* pMgrName, char* pButeFilePath);

	private :

		CClientButeMgr*		m_pClientButeMgr;	// Same as g_pClientButeMgr
		CClientSoundMgr*	m_pClientSoundMgr;	// Same as g_pClientSoundMgr

		CKeyMgr*		m_pKeyMgr;			// stores key item data
		CSearchItemMgr*	m_pSearchItemMgr;	// stores search item data
		CPopupMgr*		m_pPopupMgr;			// stores popup item data

};

#endif // __GLOBAL_CLIENT_MGR_H__