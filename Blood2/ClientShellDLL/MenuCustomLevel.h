// MenuCustomLevel.h: interface for the CMenuCustomLevel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MENUCUSTOMLEVEL_H__08A492F2_5A47_11D2_BDA0_0060971BDC6D__INCLUDED_)
#define AFX_MENUCUSTOMLEVEL_H__08A492F2_5A47_11D2_BDA0_0060971BDC6D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "MenuBase.h"

class CMenuCustomLevel : public CMenuBase  
{
public:
	CMenuCustomLevel();
	virtual ~CMenuCustomLevel();

	// Build the menu
	void	Build();	

	// Add the custom levels to the menu
	void	InitCustomLevels();

	void	OnFocus(DBOOL bFocus);

protected:
	// Add custom levels to the menu
	// lpszPath		  - rez file path
	// lpszItemPrefix - string to add to the beginning of each menu item
	void	AddCustomLevels(char *lpszPath, char *lpszItemPrefix);

	DDWORD	OnCommand(DDWORD dwCommand, DDWORD dwParam1, DDWORD dwParam2);
};

#endif // !defined(AFX_MENUCUSTOMLEVEL_H__08A492F2_5A47_11D2_BDA0_0060971BDC6D__INCLUDED_)
