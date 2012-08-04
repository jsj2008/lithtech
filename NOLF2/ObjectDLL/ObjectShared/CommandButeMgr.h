// ----------------------------------------------------------------------- //
//
// MODULE  : CommandButeMgr.h
//
// PURPOSE : The CommandButeMgr
//
// CREATED : 11/08/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __COMMAND_BUTE_MGR_H__
#define __COMMAND_BUTE_MGR_H__

//
// Includes...
//

	#include "GameButeMgr.h"

//
// Forwards...
//

	class CCommandMgrPlugin;
	class CCommandButeMgr;
	extern CCommandButeMgr *g_pCommandButeMgr;

//
// Defines...
//

	#define CBMGR_DEFAULT_FILE			"Attributes\\Commands.txt"
	#define CBMGR_MAX_COMMAND_LENGTH	256


class CCommandButeMgr : public CGameButeMgr 
{
	public : // Methods...

		CCommandButeMgr( );
		~CCommandButeMgr( );

		void	Reload() { Term(); m_buteMgr.Term(); Init(); }
		void	Pre_Reload( ILTPreInterface *pInterface ); 

		LTBOOL	Init( const char *szAttributeFile = CBMGR_DEFAULT_FILE );
		void	Term( );

		void	PostStartWorld( uint8 nLoadGameFlags );
		void	AllObjectsCreated();

		LTBOOL	FileChanged( );

		LTBOOL	Pre_CheckGlobalCmds( ILTPreInterface *pInterface, CCommandMgrPlugin *pCmdMgrPlugin, const char *pCmd = LTNULL );
		LTBOOL	Pre_CheckLevelCmds( ILTPreInterface *pInterface, CCommandMgrPlugin *pCmdMgrPlugin, const char *pCmd = LTNULL );

	private : // Methods...

		void	ProcessGlobalCmds( );
		void	ProcessLevelCmds( );

	private : // Members...

		uint32	m_dwLastChecksum;
		bool	m_bValid;
};

#endif // __COMMAND_BUTE_MGR_H__