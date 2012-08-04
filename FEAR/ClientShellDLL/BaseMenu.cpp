// ----------------------------------------------------------------------- //
//
// MODULE  : BaseMenu.cpp
//
// PURPOSE : Base class for in-game menus
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "stdafx.h"
#include "BaseMenu.h"
#include "InterfaceMgr.h"

/////////////////////////////////////////////////////////////////////////////
// BaseMenu members
/////////////////////////////////////////////////////////////////////////////

//static members


CBaseMenu::CBaseMenu()
{
	m_MenuID = MENU_ID_NONE;

	m_SelectedColor		= argbWhite;
	m_NonSelectedColor	= argbBlack;
	m_DisabledColor		= argbGray;

	m_FontSize = 12;

	m_pMenuMgr = NULL;

	m_bShowFrame = true;
}

CBaseMenu::~CBaseMenu()
{
	Term();
}

bool CBaseMenu::Init( CMenuMgr& menuMgr )
{
	HRECORD hMenuRec = GetMenuRecord();

	m_vDefaultSize	= g_pLayoutDB->GetPosition(hMenuRec,"Size");
	m_vDefaultPos	= g_pLayoutDB->GetPosition(hMenuRec,"Pos");

	m_Frame.Load(g_pLayoutDB->GetString(hMenuRec,"FrameTexture"));

	m_Up.Load(g_pLayoutDB->GetString(hMenuRec,"UpArrow"));
	m_UpH.Load(g_pLayoutDB->GetString(hMenuRec,"UpArrowHighlight"));

	m_Down.Load(g_pLayoutDB->GetString(hMenuRec,"DownArrow"));
	m_DownH.Load(g_pLayoutDB->GetString(hMenuRec,"DownArrowHighlight"));

	m_FontFace		= g_pLayoutDB->GetFont(hMenuRec,"FontFace");
	m_FontSize		= g_pLayoutDB->GetInt32(hMenuRec,"FontSize");

	m_Indent		= g_pLayoutDB->GetPosition(hMenuRec,"Indent");


	m_SelectedColor		= g_pLayoutDB->GetColor(hMenuRec,"SelectedColor");
	m_NonSelectedColor	= g_pLayoutDB->GetColor(hMenuRec,"NonSelectedColor");
	m_DisabledColor		= g_pLayoutDB->GetColor(hMenuRec,"DisabledColor");

	CLTGUICtrl_create cs;
	cs.rnBaseRect.m_vMax = m_vDefaultSize;
	if (!Create(NULL,cs)) return false;

	SetupQuadUVs(m_Poly, m_Frame, 0.0f,0.0f,1.0f,1.0f);
	DrawPrimSetRGBA(m_Poly,argbWhite);


	LTVector2n pos = m_Indent;
	CFontInfo Font(m_FontFace.c_str(),m_FontSize);

	cs.rnBaseRect.Bottom() = m_FontSize;

	CLTGUIListCtrl_create lcs;
	lcs.rnBaseRect.Bottom() = m_vDefaultSize.y - pos.y;
	lcs.bArrows = true;
	lcs.nArrowOffset = (m_vDefaultSize.x-m_Indent.x*2)-16;
	lcs.vnArrowSz.Init(16,16);
	lcs.hUpNormal = m_Up;
	lcs.hUpSelected = m_UpH;
	lcs.hDownNormal = m_Down;
	lcs.hDownSelected = m_DownH;


	m_List.Create(lcs);
	CLTGUIWindow::AddControl(&m_List,pos);

	m_List.SetItemSpacing( (m_FontSize/4) );

	cs.nCommandID = MC_CLOSE;
	cs.pCommandHandler = this;
	m_Resume.Create(LoadString("IDS_RESUME"), Font, cs);
	m_Resume.SetAlignment(kRight);

	pos.x = m_vDefaultSize.x - m_Indent.x;
	pos.y = 12;
	CLTGUIWindow::AddControl(&m_Resume,pos);

	SetBasePos(m_vDefaultPos);

	m_pMenuMgr = &menuMgr;

	return true;
}

void CBaseMenu::Term()
{
	CLTGUIWindow::RemoveControl(&m_List,false);
	m_List.Destroy();

	CLTGUIWindow::RemoveControl(&m_Resume,false);
	m_Resume.Destroy();

	Destroy();
}

// This is called when the screen gets or loses focus
void CBaseMenu::OnFocus(bool bFocus)
{
	ClearSelection();
	m_List.ClearSelection();
	if (bFocus)
	{
		if (m_vfScale != g_pInterfaceResMgr->GetScreenScale())
		{
			SetScale(g_pInterfaceResMgr->GetScreenScale());
		}

		SetSize(m_vDefaultSize);

		SetSelection(GetIndex(&m_List));
	}
}

bool CBaseMenu::OnUp()
{
	return CLTGUIWindow::OnUp();

}

bool CBaseMenu::OnDown()
{
	return CLTGUIWindow::OnDown();
}

bool CBaseMenu::OnMouseMove(int x, int y)
{
	uint32 listSelect = m_List.GetSelectedIndex( );
	bool bHandled = CLTGUIWindow::OnMouseMove(x,y);

	if (bHandled || listSelect != m_List.GetSelectedIndex( ))
	{
		g_pInterfaceMgr->RequestInterfaceSound(IS_CHANGE);
		return true;
	}
	return false;
}


uint16 CBaseMenu::AddControl (const char* szStringID, uint32 commandID, bool bStatic)
{
	return AddControl(LoadString(szStringID),commandID,bStatic);
}

uint16 CBaseMenu::AddControl (const wchar_t *pString, uint32 commandID, bool bStatic /* = false */)
{
	CLTGUITextCtrl* pCtrl=debug_new(CLTGUITextCtrl);
	CLTGUICtrl_create cs;
	cs.nCommandID = commandID;
	cs.pCommandHandler = this;
	cs.rnBaseRect.Bottom() = m_FontSize;
	cs.rnBaseRect.Right() = m_vDefaultSize.x - 2*m_Indent.x;
	cs.bGlowEnable = true;
	cs.fGlowAlpha = g_pLayoutDB->GetHighlightGlowAlpha();
	cs.vGlowSize = g_pLayoutDB->GetHighlightGlowSize();
    if (!pCtrl->Create(pString, CFontInfo(m_FontFace.c_str(),m_FontSize), cs))
	{
		debug_delete(pCtrl);
        return -1;
	}

	pCtrl->SetBasePos(m_nextPos);
	if (bStatic)
	{
		pCtrl->SetColor(m_NonSelectedColor);
		pCtrl->Enable(false);
	}
	else
		pCtrl->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
	pCtrl->SetScale(g_pInterfaceResMgr->GetScreenScale());

	return m_List.AddControl(pCtrl);

}

uint32 CBaseMenu::OnCommand(uint32 nCommand, uint32 nParam1, uint32 nParam2)
{
	switch (nCommand)
	{
	case MC_CLOSE:
		m_pMenuMgr->Close();
		break;
	default:
		return 0;
	}
	return 1;
}


// Render the control
void CBaseMenu::Render ( )
{
	if (!IsVisible()) return;

	if( m_bShowFrame )
	{
		g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Translucent);
		g_pDrawPrim->SetTexture(m_Frame);
		g_pDrawPrim->DrawPrim(&m_Poly);
	}

	CLTGUIWindow::Render();
}


void CBaseMenu::SetBasePos(const LTVector2n& pos )
{ 
	CLTGUIWindow::SetBasePos(pos);
	UpdateFrame();
}


void CBaseMenu::SetScale(const LTVector2& vfScale)
{
	CLTGUIWindow::SetScale(vfScale);
	UpdateFrame();
}

void CBaseMenu::UpdateFrame()
{
	float fx = m_rfRect.Left();
	float fy = m_rfRect.Top();

	float fw = m_rfRect.GetWidth();
	float fh = m_rfRect.GetHeight();
	DrawPrimSetXYWH(m_Poly,fx,fy,fw,fh);
}

void CBaseMenu::FreeSharedTextures()
{
	m_Frame.Free();
	m_Up.Free();
	m_UpH.Free();
	m_Down.Free();
	m_DownH.Free();
}

HRECORD CBaseMenu::GetMenuRecord()
{ 
	return g_pLayoutDB->GetMenuRecord( GetMenuID() ); 
}