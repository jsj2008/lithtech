// MessageBox.cpp: implementation of the CMessageBox class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MessageBox.h"
#include "InterfaceMgr.h"
#include "ClientRes.h"
#include "GameClientShell.h"

extern CGameClientShell* g_pGameClientShell;

#define DLG_OK		1
#define DLG_CANCEL	0

int nVKYes = 0;
int nVKNo = 0;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMessageBox::CMessageBox()
{
    m_pCallback = LTNULL;
    m_bVisible  = LTFALSE;
    m_bGameWasPaused = LTFALSE;
    m_bCursorWasUsed = LTFALSE;
}

CMessageBox::~CMessageBox()
{
	Term();
}

LTBOOL CMessageBox::Init()
{
	if (g_pInterfaceResMgr->IsEnglish())
	{
		HSTRING hVK = g_pLTClient->FormatString(IDS_YES_VK_CODE);
		nVKYes = atoi(g_pLTClient->GetStringData(hVK));
		g_pLTClient->FreeString(hVK);

		hVK = g_pLTClient->FormatString(IDS_NO_VK_CODE);
		nVKNo = atoi(g_pLTClient->GetStringData(hVK));
		g_pLTClient->FreeString(hVK);
	}
    return LTTRUE;
}
void CMessageBox::Term()
{
}

void CMessageBox::Show(HSTRING hString, eMBType eType, MBCallBackFn pFn, void *pData, LTBOOL bLargeFont, LTBOOL bDefaultReturn)
{
	if (m_bVisible)
		return;

	m_bCursorWasUsed = g_pInterfaceMgr->IsCursorUsed();
    g_pInterfaceMgr->UseCursor(LTTRUE);


	m_bGameWasPaused = g_pGameClientShell->IsGamePaused();
	if (!m_bGameWasPaused)
        g_pGameClientShell->PauseGame(LTTRUE,LTTRUE);

	char szBack[128] = "";
	g_pLayoutMgr->GetMessageBoxBackground(szBack,sizeof(szBack));
    LTFLOAT fAlpha = g_pLayoutMgr->GetMessageBoxAlpha();

    HSURFACE hSurf = g_pLTClient->CreateSurfaceFromBitmap(szBack);
    g_pLTClient->SetSurfaceAlpha(hSurf,fAlpha);
    g_pLTClient->OptimizeSurface(hSurf,LTNULL);
	CLTGUIFont *pFont;
	if (bLargeFont)
	{
		pFont = g_pInterfaceResMgr->GetLargeFont();
	}
	else
	{
		pFont = g_pInterfaceResMgr->GetSmallFont();
	}
    m_MsgBox.Create(g_pLTClient, hSurf, pFont, hString, this);
    m_MsgBox.SetPos(LTGUI_MB_CENTER,LTGUI_MB_CENTER,g_pLTClient->GetScreenSurface());
	m_MsgBox.SetTextButtonColor(kBlack, kWhite);

	m_MsgBox.SetTextButtonHorzSpace(32);

	m_pCallback = pFn;
	m_pData = pData;

    HSTRING hText = LTNULL;

	if (eType == LTMB_YESNO)
	{
        hText = g_pLTClient->FormatString(IDS_YES);
		m_MsgBox.AddMessageButton(hText, DLG_OK);
        g_pLTClient->FreeString(hText);

		m_MsgBox.AddKey(VK_RETURN, DLG_OK);
		if (g_pInterfaceResMgr->IsEnglish())
		{
			m_MsgBox.AddKey(nVKYes, DLG_OK);
		}

        hText = g_pLTClient->FormatString(IDS_NO);
		m_MsgBox.AddMessageButton(hText, DLG_CANCEL);
        g_pLTClient->FreeString(hText);

		m_MsgBox.AddKey(VK_ESCAPE, DLG_CANCEL);
		if (g_pInterfaceResMgr->IsEnglish())
		{
			m_MsgBox.AddKey(nVKNo, DLG_CANCEL);
		}

		m_MsgBox.SetSelection(!bDefaultReturn);

	}
	else
	{
        hText = g_pLTClient->FormatString(IDS_OK);
		m_MsgBox.AddMessageButton(hText, DLG_OK);
        g_pLTClient->FreeString(hText);

		m_MsgBox.AddKey(VK_RETURN, DLG_OK);
		m_MsgBox.AddKey(VK_ESCAPE, DLG_OK);
	}

    m_bVisible = LTTRUE;
}


void CMessageBox::Close(LTBOOL bReturn)
{
	if (!m_bVisible)
		return;

	if (m_pCallback)
	{
		m_pCallback(bReturn, m_pData);
	}

	//hack to prevent cursor from showing up after state transition
	if (g_pInterfaceMgr->GetGameState() == GS_PLAYING)
		m_bCursorWasUsed = LTFALSE;

//	g_pInterfaceMgr->UseCursor(m_bCursorWasUsed);
	if (!m_bGameWasPaused)
        g_pGameClientShell->PauseGame(LTFALSE);
	m_MsgBox.Destroy();

    m_bVisible = LTFALSE;
	g_pInterfaceMgr->UpdateCursorState();
}

void CMessageBox::Draw(HSURFACE hDestSurf)
{
	if (m_bVisible)
	{
		m_MsgBox.Render(hDestSurf);
	}
}

LTBOOL CMessageBox::HandleKeyDown(int key, int rep)
{
	if (m_bVisible)
	{
		return m_MsgBox.HandleKeyDown(key,rep);
	}
    return LTFALSE;
}

LTBOOL CMessageBox::OnLButtonDown(int x, int y)
{
	if (m_bVisible)
	{
		return m_MsgBox.OnLButtonDown( x, y);
	}
    return LTFALSE;
}

LTBOOL CMessageBox::OnLButtonUp(int x, int y)
{
	if (m_bVisible)
	{
		return m_MsgBox.OnLButtonUp( x, y);
	}
    return LTFALSE;
}


LTBOOL CMessageBox::OnMouseMove(int x, int y)
{
	if (m_bVisible)
	{
		return m_MsgBox.OnMouseMove(x,y);
	}
    return LTFALSE;
}

uint32 CMessageBox::OnCommand(uint32 dwCommand, uint32 dwParam1, uint32 dwParam2)
{
	switch(dwCommand)
	{
	case DLG_OK:
        Close(LTTRUE);
        return LTTRUE;
		break;
	case DLG_CANCEL:
        Close(LTFALSE);
        return LTTRUE;
		break;
	default:
        return LTFALSE;
		break;
	}
}