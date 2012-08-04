// MenuKeyboard.h: interface for the CMenuKeyboard class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MENUKEYBOARD_H__8B896D03_6918_11D2_BDAE_0060971BDC6D__INCLUDED_)
#define AFX_MENUKEYBOARD_H__8B896D03_6918_11D2_BDAE_0060971BDC6D__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "MenuBase.h"

class CMenuKeyboard : public CMenuBase  
{
public:
	CMenuKeyboard();
	virtual ~CMenuKeyboard();

	// Build the menu
	void	Build();			

	// Change in focus
	void	OnFocus(DBOOL bFocus);

protected:
	int		m_nKeyboardTurnRate;
};

#endif // !defined(AFX_MENUKEYBOARD_H__8B896D03_6918_11D2_BDAE_0060971BDC6D__INCLUDED_)
