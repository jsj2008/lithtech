// PopupText.cpp: implementation of the CPopupText class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PopupText.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "ClientWeaponMgr.h"
#include "PopupMgr.h"


namespace
{
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPopupText::CPopupText()
{
	m_bVisible			= LTFALSE;
	m_bUsePopupState	= true;
	m_bWeaponsEnabled	= true;
}



CPopupText::~CPopupText()
{
	Term();
}


void CPopupText::Init(bool bUsePopupState/*=true*/)
{
	m_bVisible		= LTFALSE;
	m_bUsePopupState = bUsePopupState;

	m_Text.Create(" ",0,0,g_pInterfaceResMgr->GetFont(0),8,LTNULL);
	m_Frame.Create(g_pInterfaceResMgr->GetTexture("interface\\menu\\sprtex\\frame.dtx"),200,320,LTTRUE);

}

void CPopupText::Term()
{
	if (m_bVisible)
	{
		Close();
	}

}


void CPopupText::Close()
{

	m_bVisible = LTFALSE;


	//if we had our weapons out before, show them again now
	if (g_pInterfaceMgr && g_pPlayerMgr && m_bWeaponsEnabled)
	{
		g_pPlayerMgr->GetClientWeaponMgr()->EnableWeapons();
		g_pInterfaceMgr->EnableCrosshair(LTTRUE);
	}
}

void CPopupText::Draw()
{
	if (!m_bVisible) return;

	m_Frame.Render();
	m_Text.Render();

}


void CPopupText::DisplayPopup(uint8 nPopupId, char *pText, uint32 nTextId)
{
	POPUP* pPopup = g_pPopupMgr->GetPopup(nPopupId);
	if (!pPopup) return;

	CUIFont *pFont = g_pInterfaceResMgr->GetFont(pPopup->nFont);
	LTIntPt pos( (640 - pPopup->sSize.x) / 2, (480 - pPopup->sSize.y) / 2 );

	m_Frame.SetFrame(g_pInterfaceResMgr->GetTexture(pPopup->szFrame));
	m_Frame.SetSize(pPopup->sSize.x,pPopup->sSize.y);
	m_Frame.SetBasePos(pos);
	m_Frame.SetScale(g_pInterfaceResMgr->GetXRatio());


	pos.x += pPopup->sTextOffset.x;
	pos.y += pPopup->sTextOffset.y;

	m_Text.SetScale(1.0f);

	// If we were given a string use it, otherwise get the string from the ID...

	if( pText )
	{
		m_Text.SetString( pText );
	}
	else
	{
		char szString[1024] = "";
		LoadString(nTextId,szString,sizeof(szString));

		char *pBody = strchr(szString,'@');
		if (pBody)
		{
			++pBody;
			m_Text.SetString(pBody);
		}
		else
		{
			m_Text.SetString(szString);
		}
	}

	m_Text.SetFont(pFont,pPopup->nFontSize);
	m_Text.SetColors(pPopup->argbTextColor,pPopup->argbTextColor,pPopup->argbTextColor);
	m_Text.SetFixedWidth(pPopup->nTextWidth);
	m_Text.SetBasePos(pos);
	m_Text.SetScale(g_pInterfaceResMgr->GetXRatio());

	m_bVisible = LTTRUE;

	//remember whether our weapons were out
	m_bWeaponsEnabled = g_pPlayerMgr->GetClientWeaponMgr()->WeaponsEnabled();

	if (m_bUsePopupState)
	{
		g_pInterfaceMgr->ChangeState(GS_POPUP);
	}
}


void CPopupText::Update()
{
}

LTBOOL CPopupText::OnKeyDown(int key, int rep)
{
	// They pressed escape - close the popup
	if (key == VK_ESCAPE)
	{
		Close();
        return LTTRUE;
	}
	return LTFALSE;
}
