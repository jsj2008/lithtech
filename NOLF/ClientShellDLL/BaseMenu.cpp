// ----------------------------------------------------------------------- //
//
// MODULE  : BaseMenu.cpp
//
// PURPOSE : Base Class for In-game Menus
//
// CREATED : 3/20/00
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "BaseMenu.h"
#include "LTMenuWnd.h"

CBaseMenu::CBaseMenu()
{
    m_pMenuWnd = LTNULL;
}

void CBaseMenu::Show(LTBOOL bShow)
{
	if (!m_pMenuWnd)
		return;

	m_pMenuWnd->Reset();
	if (bShow)
	{
		Build();
		if (m_pMenuWnd->Open())
			m_pMenuWnd->SetMenu(this);
	}
	else
	{
		m_pMenuWnd->Close();
	}
}
