// ServerOptionMgr.h: interface for the CServerOptionMgr class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _SERVER_OPTION_MGR_
#define _SERVER_OPTION_MGR_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GameButeMgr.h"
#include "ltbasetypes.h"
#include "TemplateList.h"
#include "NetDefs.h"

class CServerOptionMgr;
extern CServerOptionMgr* g_pServerOptionMgr;

struct OPTION : public ServerOption
{
	OPTION();

    LTBOOL   Init(CButeMgr & buteMgr, char* aTagName);
    LTFLOAT  GetValue();
    void    SetValue(LTFLOAT val);
};

typedef CTList<OPTION*> OptionList;

class CServerOptionMgr : public CGameButeMgr
{
public:
	CServerOptionMgr();
	virtual ~CServerOptionMgr();

    LTBOOL       Init(ILTCSBase *pInterface, const char* szAttributeFile=SO_DEFAULT_FILE);
	void		Term();

    LTBOOL       WriteFile() { return m_buteMgr.Save(); }
	void		Reload()    { m_buteMgr.Parse(m_strAttributeFile); }

	int			GetNumOptions()	const { return m_OptionList.GetLength(); }

	OPTION*		GetOption(int nIndex);
	OPTION*		GetOption(char* pVariableName);


protected:
	OptionList			m_OptionList;			// Global Options

};

#endif // _SERVER_OPTION_MGR_