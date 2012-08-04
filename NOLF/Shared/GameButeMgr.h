// ----------------------------------------------------------------------- //
//
// MODULE  : GameButeMgr.h
//
// PURPOSE : GameButeMgr definition - Base class for all bute mgrs
//
// CREATED : 3/30/99
//
// ----------------------------------------------------------------------- //

#ifndef __GAME_BUTE_MGR_H__
#define __GAME_BUTE_MGR_H__

#include "ButeMgr.h"

void GBM_DisplayError(const char* szMsg);

class CGameButeMgr
{
	public :

        virtual LTBOOL Init(ILTCSBase *pInterface, const char* szAttributeFile="") = 0;

		CGameButeMgr()
		{
			m_buteMgr.Init(GBM_DisplayError);
            m_pCryptKey = LTNULL;
            m_bInRezFile = LTTRUE;
		}

		virtual ~CGameButeMgr() { m_buteMgr.Term(); }

        inline void SetInRezFile(LTBOOL bInRezFile) { m_bInRezFile = bInRezFile; }

	protected :

		CString		m_strAttributeFile;
		CButeMgr	m_buteMgr;

		char*		m_pCryptKey;
        LTBOOL       m_bInRezFile;

        LTBOOL       Parse(ILTCSBase *pInterface, const char* sButeFile);
};

#endif // __GAME_BUTE_MGR_H__