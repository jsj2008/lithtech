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

class CClientSoundMgr;

class CGlobalClientMgr
{
	public :

		CGlobalClientMgr( );
		~CGlobalClientMgr();

		bool	Init( );
		void	Term( );

	protected :

		class CErrorHandler : public CGlobalMgr::CErrorHandler
		{
		public:
			virtual void ShutdownWithError(char* pMgrName, const char* pButeFilePath) const;
		};

	private :

		CClientSoundMgr*	m_pClientSoundMgr;	// Same as g_pClientSoundMgr

		PREVENT_OBJECT_COPYING( CGlobalClientMgr );
};

#endif // __GLOBAL_CLIENT_MGR_H__