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
// SubMenu members
/////////////////////////////////////////////////////////////////////////////

CSubMenu::CSubMenu()
{
	m_hFrame = NULL;
	m_hFrameTip = NULL;
}

LTBOOL CSubMenu::Init(HTEXTURE hFrame,HTEXTURE hFrameTip, LTIntPt size)
{
	m_hFrame = hFrame;
	m_hFrameTip = hFrameTip;

	if (!Create(NULL,size.x,size.y)) 
		return LTFALSE;

	SetupQuadUVs(m_Poly[0], hFrame, 0.0f, 0.0f, 1.0f, 1.0f);
	SetupQuadUVs(m_Poly[1], hFrameTip, 0.0f, 0.0f, 1.0f, 1.0f);
	RotateQuadUVs(m_Poly[1], 1);

	g_pDrawPrim->SetRGBA(&m_Poly[0],argbWhite);
	g_pDrawPrim->SetRGBA(&m_Poly[1],argbWhite);

	return LTTRUE;

}


LTBOOL CSubMenu::HandleKeyUp (int vkey )
{
	if (vkey == VK_ESCAPE)
	{
		g_pInterfaceMgr->GetMenuMgr()->HideSubMenu(true);
		return LTTRUE;
	}
	return LTFALSE;
}

// Render the control
void CSubMenu::Render ( )
{
	if (!IsVisible()) return;


	g_pDrawPrim->SetTransformType(DRAWPRIM_TRANSFORM_SCREEN);
	g_pDrawPrim->SetZBufferMode(DRAWPRIM_NOZ); 
	g_pDrawPrim->SetClipMode(DRAWPRIM_NOCLIP);
	g_pDrawPrim->SetFillMode(DRAWPRIM_FILL);
	g_pDrawPrim->SetColorOp(DRAWPRIM_MODULATE);
	g_pDrawPrim->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
	g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);

	g_pDrawPrim->SetTexture(m_hFrame);
	g_pDrawPrim->DrawPrim(&m_Poly[0]);

	g_pDrawPrim->SetTexture(m_hFrameTip);
	g_pDrawPrim->DrawPrim(&m_Poly[1]);

	CLTGUIWindow::Render();

}

void CSubMenu::SetBasePos ( LTIntPt pos )
{ 
	CLTGUIWindow::SetBasePos(pos);
	UpdateFrame();
}


void CSubMenu::SetScale(float fScale)
{
	CLTGUIWindow::SetScale(fScale);
	UpdateFrame();
}

void CSubMenu::UpdateFrame()
{
	float fx = (float)m_pos.x;
	float fy = (float)m_pos.y;

	float fw = (float)m_nWidth * m_fScale * 0.75f;
	float fh = (float)m_nHeight * m_fScale;

	g_pDrawPrim->SetXYWH(&m_Poly[0],fx,fy,fw,fh);
	g_pDrawPrim->SetXYWH(&m_Poly[1],fx+fw,fy,(fh/2.0f),fh);


}


/////////////////////////////////////////////////////////////////////////////
// BaseMenu members
/////////////////////////////////////////////////////////////////////////////

//static members
LTIntPt CBaseMenu::s_Size;
uint16 CBaseMenu::s_Pos;

HTEXTURE CBaseMenu::s_Frame = LTNULL;
HTEXTURE CBaseMenu::s_FrameTip = LTNULL;
HTEXTURE CBaseMenu::s_Up = LTNULL;
HTEXTURE CBaseMenu::s_UpH = LTNULL;
HTEXTURE CBaseMenu::s_Down = LTNULL;
HTEXTURE CBaseMenu::s_DownH = LTNULL;

CBaseMenu::CBaseMenu()
{
	m_MenuID = MENU_ID_NONE;

	m_SelectedColor		= argbWhite;
	m_NonSelectedColor	= argbBlack;
	m_DisabledColor		= argbGray;

	m_FontSize = 12;
	m_FontFace = 0;
	m_TitleFontSize = 0;
	m_TitleFontFace = 16;
}

CBaseMenu::~CBaseMenu()
{
	Term();
}

LTBOOL CBaseMenu::Init()
{
	if (!s_Frame)
	{

		char szTmp[128] = "";
		g_pLayoutMgr->GetMenuFrame(szTmp,sizeof(szTmp));
		s_Frame = g_pInterfaceResMgr->GetTexture(szTmp);

		g_pLayoutMgr->GetMenuFrameTip(szTmp,sizeof(szTmp));
		s_FrameTip = g_pInterfaceResMgr->GetTexture(szTmp);

		s_Size	= g_pLayoutMgr->GetMenuSize();
		s_Pos	= g_pLayoutMgr->GetMenuPosition();

		g_pLayoutMgr->GetMenuUpArrow(szTmp,sizeof(szTmp));
		s_Up = g_pInterfaceResMgr->GetTexture(szTmp);
		g_pLayoutMgr->GetMenuUpArrowHighlight(szTmp,sizeof(szTmp));
		s_UpH = g_pInterfaceResMgr->GetTexture(szTmp);

		g_pLayoutMgr->GetMenuDownArrow(szTmp,sizeof(szTmp));
		s_Down = g_pInterfaceResMgr->GetTexture(szTmp);
		g_pLayoutMgr->GetMenuDownArrowHighlight(szTmp,sizeof(szTmp));
		s_DownH = g_pInterfaceResMgr->GetTexture(szTmp);
	}

	m_FontFace		= g_pLayoutMgr->GetMenuFontFace(m_MenuID);
	m_FontSize		= g_pLayoutMgr->GetMenuFontSize(m_MenuID);
	m_TitleFontFace	= g_pLayoutMgr->GetMenuTitleFontFace(m_MenuID);
	m_TitleFontSize	= g_pLayoutMgr->GetMenuTitleFontSize(m_MenuID);

	m_Indent		= g_pLayoutMgr->GetMenuIndent(m_MenuID);


	m_SelectedColor		= g_pLayoutMgr->GetMenuSelectedColor(m_MenuID);
	m_NonSelectedColor	= g_pLayoutMgr->GetMenuNonSelectedColor(m_MenuID);
	m_DisabledColor		= g_pLayoutMgr->GetMenuDisabledColor(m_MenuID);

	if (!Create(NULL,s_Size.x,s_Size.y)) return LTFALSE;

	SetupQuadUVs(m_Poly[0], s_Frame, 0.0f,0.0f,1.0f,1.0f);
	SetupQuadUVs(m_Poly[1], s_FrameTip, 0.0f,0.0f,1.0f,1.0f);
	g_pDrawPrim->SetRGBA(&m_Poly[0],argbWhite);
	g_pDrawPrim->SetRGBA(&m_Poly[1],argbWhite);


	LTIntPt pos = m_Indent;

	CUIFont* pFont = g_pInterfaceResMgr->GetFont(m_TitleFontFace);
	if (!pFont) return LTFALSE;

    if (!m_Title.Create("X", LTNULL, LTNULL, pFont, m_TitleFontSize, this))
	{
        return LTFALSE;
	}

	m_Title.SetColors(m_NonSelectedColor,m_NonSelectedColor,m_NonSelectedColor);
	m_Title.Enable(LTFALSE);

	pos.x = m_Indent.x + 24;
	CLTGUIWindow::AddControl(&m_Title,pos);

	m_Title.SetScale(1.0f);
	pos.x = m_Indent.x;
	pos.y += (m_Title.GetHeight() + 4);
	m_Title.SetScale(g_pInterfaceResMgr->GetXRatio());

	m_List.Create(s_Size.y - pos.y);
	uint16 nOffset = (s_Size.x-m_Indent.x*2)-16;
	m_List.UseArrows(nOffset ,1.0f,s_Up,s_UpH,s_Down,s_DownH);
	CLTGUIWindow::AddControl(&m_List,pos);

	m_Resume.Create(LoadTempString(IDS_RESUME),MC_CLOSE,NULL,pFont,m_TitleFontSize,this);

	pos.x = s_Size.x - m_Indent.x - m_Resume.GetWidth();
	pos.y = 12;
	CLTGUIWindow::AddControl(&m_Resume,pos);

	pos.x = s_Pos;
	pos.y = 0;
	SetBasePos(pos);


	return LTTRUE;
}

void CBaseMenu::Term()
{

	CLTGUIWindow::RemoveControl(&m_Title,LTFALSE);
	m_Title.Destroy();

	CLTGUIWindow::RemoveControl(&m_List,LTFALSE);
	m_List.Destroy();

	CLTGUIWindow::RemoveControl(&m_Resume,LTFALSE);
	m_Resume.Destroy();

	Destroy();
}

// This is called when the screen gets or loses focus
void CBaseMenu::OnFocus(LTBOOL bFocus)
{
	ClearSelection();
	m_List.ClearSelection();
	if (bFocus)
	{
		if (m_fScale != g_pInterfaceResMgr->GetXRatio())
		{
			SetScale(g_pInterfaceResMgr->GetXRatio());
		}

		SetSize(s_Size.x,s_Size.y);

		SetSelection(GetIndex(&m_List));
	}
}

LTBOOL CBaseMenu::OnUp()
{
	return CLTGUIWindow::OnUp();

}

LTBOOL CBaseMenu::OnDown()
{
	return CLTGUIWindow::OnDown();
}

LTBOOL CBaseMenu::OnMouseMove(int x, int y)
{
	uint16 listSelect = m_List.GetSelectedIndex( );
	LTBOOL bHandled = CLTGUIWindow::OnMouseMove(x,y);

	if (bHandled || listSelect != m_List.GetSelectedIndex( ))
	{
		g_pInterfaceMgr->RequestInterfaceSound(IS_CHANGE);
		return LTTRUE;
	}
	return LTFALSE;
}

void CBaseMenu::SetTitle (int stringID)
{
	m_Title.SetString(LoadTempString(stringID));
}

uint16 CBaseMenu::AddControl (int stringID, uint32 commandID, LTBOOL bStatic)
{
	return AddControl(LoadTempString(stringID),commandID,bStatic);
}

uint16 CBaseMenu::AddControl (char *pString, uint32 commandID, LTBOOL bStatic)
{
	CUIFont* pFont = g_pInterfaceResMgr->GetFont(m_FontFace);
	if (!pFont) return -1;

	CLTGUITextCtrl* pCtrl=debug_new(CLTGUITextCtrl);
    if (!pCtrl->Create(pString, commandID, LTNULL, pFont, m_FontSize, this))
	{
		debug_delete(pCtrl);
        return -1;
	}

	pCtrl->SetBasePos(m_nextPos);
	if (bStatic)
	{
		pCtrl->SetColors(m_NonSelectedColor,m_NonSelectedColor,m_NonSelectedColor);
		pCtrl->Enable(LTFALSE);
	}
	else
		pCtrl->SetColors(m_SelectedColor,m_NonSelectedColor,m_DisabledColor);
	pCtrl->SetScale(g_pInterfaceResMgr->GetXRatio());

	return m_List.AddControl(pCtrl);

}

uint32 CBaseMenu::OnCommand(uint32 nCommand, uint32 nParam1, uint32 nParam2)
{
	switch (nCommand)
	{
	case MC_CLOSE:
		g_pInterfaceMgr->GetMenuMgr()->SlideOut();
		break;
	case MC_LEFT:
		g_pInterfaceMgr->GetMenuMgr()->PreviousMenu();
		break;
	case MC_RIGHT:
		g_pInterfaceMgr->GetMenuMgr()->NextMenu();
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


	g_pDrawPrim->SetTransformType(DRAWPRIM_TRANSFORM_SCREEN);
	g_pDrawPrim->SetZBufferMode(DRAWPRIM_NOZ); 
	g_pDrawPrim->SetClipMode(DRAWPRIM_NOCLIP);
	g_pDrawPrim->SetFillMode(DRAWPRIM_FILL);
	g_pDrawPrim->SetColorOp(DRAWPRIM_MODULATE);
	g_pDrawPrim->SetAlphaTestMode(DRAWPRIM_NOALPHATEST);
	g_pDrawPrim->SetAlphaBlendMode(DRAWPRIM_BLEND_MOD_SRCALPHA);

	g_pDrawPrim->SetTexture(s_Frame);
	g_pDrawPrim->DrawPrim(&m_Poly[0]);

	g_pDrawPrim->SetTexture(s_FrameTip);
	g_pDrawPrim->DrawPrim(&m_Poly[1]);

	CLTGUIWindow::Render();

}


void CBaseMenu::SetBasePos ( LTIntPt pos )
{ 
	CLTGUIWindow::SetBasePos(pos);
	UpdateFrame();
}


void CBaseMenu::SetScale(float fScale)
{
	CLTGUIWindow::SetScale(fScale);
	UpdateFrame();
}

void CBaseMenu::UpdateFrame()
{
	float fx = (float)m_pos.x;
	float fy = (float)m_pos.y;

	float fw = (float)m_nWidth * m_fScale;
	float fh = (float)m_nHeight * m_fScale * 0.75f;
	g_pDrawPrim->SetXYWH(&m_Poly[0],fx,fy,fw,fh);
	g_pDrawPrim->SetXYWH(&m_Poly[1],fx,fy+fh,fw,(fw/2.0f));


}
