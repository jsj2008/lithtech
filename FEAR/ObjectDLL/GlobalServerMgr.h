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
class CTransitionMgr;


class CGlobalServerMgr
{
	public :
		CGlobalServerMgr( );
		~CGlobalServerMgr( ) { Term( ); }

        bool Init();
		void Term( );

	protected :

		class CErrorHandler : public CGlobalMgr::CErrorHandler
		{
		public:
			virtual void ShutdownWithError(char* pMgrName, const char* pButeFilePath) const;
		};

	private :

		CServerSoundMgr*	m_pServerSoundMgr;		// Same as g_pServerSoundMgr
		CTransitionMgr*		m_pTransitionMgr;		// Same as g_pTransMgr.
};


#endif // __GLOBAL_SERVER_MGR_H__
