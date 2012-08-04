// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_CMD_MGR_H__
#define __AI_CMD_MGR_H__

#include "AICmd.h"

class CAIPath;

// Externs

extern class CAICmdMgr* g_pAICmdMgr;

// Classes

class CAICmdMgr
{
	public :

		// Ctors/Dtors/etc

		CAICmdMgr();
		~CAICmdMgr();

		void Init();
		void Term();

		// Simple accessors

		uint32 GetNumCmds() { return m_cCmds; }
		LTBOOL IsInitialized() { return m_bInitialized; }

	protected :

		LTBOOL		m_bInitialized;
		uint32		m_cCmds;
		AICmd**		m_apCmds;
};

#endif // __AI_Cmd_MGR_H__