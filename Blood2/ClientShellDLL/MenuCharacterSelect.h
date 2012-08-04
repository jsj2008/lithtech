// MenuCharacterSelect.h: interface for the CMenuCharacterSelect class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MENUCHARACTERSELECT_H__C8F72682_63A1_11D2_BDA9_0060971BDC6D__INCLUDED_)
#define AFX_MENUCHARACTERSELECT_H__C8F72682_63A1_11D2_BDA9_0060971BDC6D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "MenuBase.h"

class CMenuCharacterSelect : public CMenuBase  
{
public:
	CMenuCharacterSelect();
	virtual ~CMenuCharacterSelect();

	// Build the menu
	void	Build();

protected:
	// Switches to the difficulty menu
	void	SwitchToDifficultyMenu(int nCharacter);

	DDWORD	OnCommand(DDWORD dwCommand, DDWORD dwParam1, DDWORD dwParam2);
};

#endif // !defined(AFX_MENUCHARACTERSELECT_H__C8F72682_63A1_11D2_BDA9_0060971BDC6D__INCLUDED_)
