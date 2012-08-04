// MenuBloodBath.h: interface for the CMenuBloodBath class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MENUBLOODBATH_H__D7668B34_57D4_11D2_BDA0_0060971BDC6D__INCLUDED_)
#define AFX_MENUBLOODBATH_H__D7668B34_57D4_11D2_BDA0_0060971BDC6D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "MenuBase.h"

class CMenuBloodBath : public CMenuBase  
{
public:
	CMenuBloodBath();
	virtual ~CMenuBloodBath();

	// Build the menu
	void	Build();

protected:
	// Verifies that we are in atleast 640x480 and pops up a confirmation dialog otherwise
	// to switch to 640x480
	void	DoResolutionVerify();

	// Switches video resolutions to 640x480
	DBOOL	SwitchTo640x480();

protected:
	DDWORD	OnCommand(DDWORD dwCommand, DDWORD dwParam1, DDWORD dwParam2);
};

#endif // !defined(AFX_MENUBLOODBATH_H__D7668B34_57D4_11D2_BDA0_0060971BDC6D__INCLUDED_)
