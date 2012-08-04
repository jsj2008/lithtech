// MenuOptions.h: interface for the CMenuOptions class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MENUOPTIONS_H__D7668B35_57D4_11D2_BDA0_0060971BDC6D__INCLUDED_)
#define AFX_MENUOPTIONS_H__D7668B35_57D4_11D2_BDA0_0060971BDC6D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "MenuBase.h"

class CMenuOptions : public CMenuBase  
{
public:
	CMenuOptions();
	virtual ~CMenuOptions();

	// Build the menu
	void	Build();		

protected:
	DDWORD	OnCommand(DDWORD dwCommand, DDWORD dwParam1, DDWORD dwParam2);
};

#endif // !defined(AFX_MENUOPTIONS_H__D7668B35_57D4_11D2_BDA0_0060971BDC6D__INCLUDED_)
