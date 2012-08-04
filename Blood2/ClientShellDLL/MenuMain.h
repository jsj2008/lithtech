// MenuMain.h: interface for the CMenuMain class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MENUMAIN_H__D7668B32_57D4_11D2_BDA0_0060971BDC6D__INCLUDED_)
#define AFX_MENUMAIN_H__D7668B32_57D4_11D2_BDA0_0060971BDC6D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "MenuBase.h"

class CMenuMain : public CMenuBase  
{
public:
	CMenuMain();
	virtual ~CMenuMain();	

	// Build the menu
	void	Build();		

protected:
	DDWORD	OnCommand(DDWORD dwCommand, DDWORD dwParam1, DDWORD dwParam2);
};

#endif // !defined(AFX_MENUMAIN_H__D7668B32_57D4_11D2_BDA0_0060971BDC6D__INCLUDED_)
