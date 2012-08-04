// MenuKeyboard.cpp: implementation of the CMenuKeyboard class.
//
//////////////////////////////////////////////////////////////////////

#include "MainMenus.h"
#include "MenuKeyboard.h"
#include "BloodClientShell.h"
#include "ClientRes.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMenuKeyboard::CMenuKeyboard()
{
	m_nKeyboardTurnRate=100;
}

CMenuKeyboard::~CMenuKeyboard()
{

}

// Build the menu
void CMenuKeyboard::Build()
{
	// Make sure to call the base class
	CMenuBase::Build();

	CreateTitle("interface\\mainmenus\\options.pcx", IDS_MENU_TITLE_OPTIONS, m_pMainMenus->GetTitlePos());		
	SetOptionPos(m_pMainMenus->GetOptionsPos());
	SetItemSpacing(5);
	SetScrollWrap(DFALSE);	

	m_nKeyboardTurnRate=(int)(g_pBloodClientShell->GetKeyboardTurnRate()*100.0f);

	// Add the controls
	CLTGUISliderCtrl *pCtrl=AddSliderOption(IDS_MENU_KEYBOARD_TURNSPEED, m_pMainMenus->GetSmallFont(), 100, m_pMainMenus->GetSurfaceSliderBar(), m_pMainMenus->GetSurfaceSliderTab(), &m_nKeyboardTurnRate);
	pCtrl->SetSliderRange(15, 400);
	pCtrl->SetSliderIncrement(15);

	UpdateData(DFALSE);
}

// Change in focus
void CMenuKeyboard::OnFocus(DBOOL bFocus)
{
	if (!bFocus)
	{
		UpdateData();
		g_pBloodClientShell->SetKeyboardTurnRate((float)m_nKeyboardTurnRate/100.0f);
	}
}