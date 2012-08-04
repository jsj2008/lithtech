// ----------------------------------------------------------------------- //
//
// MODULE  : MessageBox.cpp
//
// PURPOSE : Handle the display of a simple message box
//
// (c) 1999-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "MessageBox.h"
#include "ScreenCommands.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"

extern CGameClientShell* g_pGameClientShell;

// KLS - Added new control type to allow for sounds when control is selected...
class CLTGUIAutoSoundTextCtrl : public CLTGUITextCtrl
{
  protected:

	// Override virtual function that is called when the state of the control
	// is changed.
	virtual void OnSelChange()
	{
		g_pInterfaceMgr->RequestInterfaceSound(IS_CHANGE);
	}
};

namespace
{
	const wchar_t* szOK = L"";
	const wchar_t* szCancel = L"";
	const wchar_t* szYes = L"";
	const wchar_t* szNo = L"";
	const int32 kIndent = 8;
	const int32 kBaseWidth = 320;
	const int32 kBaseHeight = 200;
	const int32 kBigWidth = 480;
	const int32 kMinWidth = 160;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMessageBox::CMessageBox()
{
    m_pCallback = NULL;
	m_pUserData = NULL;
    m_bVisible  = false;
    m_bGameWasPaused = false;

	m_pText = NULL;
	m_pEdit = NULL;
	m_pOK = NULL;
	m_pCancel = NULL;

	m_pData = NULL;

	m_bIgnoreEsc = false;
	m_bPreventEmptyString = false;
}

CMessageBox::~CMessageBox()
{
	Term();
}

bool CMessageBox::Init()
{


	LTStrCpy(m_Font.m_szTypeface,g_pLayoutDB->GetDialogFontFace(),CFontInfo::knMaxTypefaceLen);
	m_Font.m_nHeight = g_pLayoutDB->GetDialogFontSize();

	HRECORD hLayout = g_pLayoutDB->GetScreenRecord(SCREEN_ID_MAIN);
	uint32 selColor = g_pLayoutDB->GetColor(hLayout,LDB_ScreenSelectedColor);
	uint32 nonColor = g_pLayoutDB->GetColor(hLayout,LDB_ScreenNonSelectedColor);
	uint32 disColor = g_pLayoutDB->GetColor(hLayout,LDB_ScreenDisabledColor);

	szOK = LoadString("IDS_OK" );
	szCancel = LoadString("IDS_CANCEL");
	szYes = LoadString("IDS_YES");
	szNo = LoadString("IDS_NO");
	
	CLTGUICtrl_create cs;
	cs.pCommandHandler = this;
	cs.rnBaseRect.Bottom() = m_Font.m_nHeight;
	cs.rnBaseRect.Right() = kBaseWidth / 3;
	cs.bGlowEnable = true;
	cs.fGlowAlpha = g_pLayoutDB->GetHighlightGlowAlpha();
	cs.vGlowSize = g_pLayoutDB->GetHighlightGlowSize();


	m_pOK = debug_new(CLTGUIAutoSoundTextCtrl);
	cs.nCommandID = CMD_OK;
    if (!m_pOK->Create(szOK, m_Font, cs))
	{
		debug_delete(m_pOK);
		m_pOK = NULL;
        return false;
	}
	m_pOK->SetColors(selColor, nonColor, disColor);

	m_pCancel = debug_new(CLTGUIAutoSoundTextCtrl);
	cs.nCommandID = CMD_CANCEL;
    if (!m_pCancel->Create(szNo, m_Font, cs))
	{
		debug_delete(m_pCancel);
		m_pCancel = NULL;
        return false;
	}
	m_pCancel->SetColors(selColor, nonColor, disColor);
	m_pCancel->SetAlignment(kRight);

	m_pText = debug_new(CLTGUITextCtrl);
	cs.nCommandID = NULL;
	cs.rnBaseRect.Right() = kBaseWidth - 2*kIndent;
	cs.pCommandHandler = NULL;

    if (!m_pText->Create(L"", m_Font,cs))
	{
		debug_delete(m_pText);
		m_pText = NULL;
        return false;
	}
	m_pText->SetColor(selColor);
	m_pText->SetWordWrap(true);
	m_pText->Enable(false);

	CLTGUIEditCtrl_create ecs;
	ecs.bUseCaret = true;
	ecs.pCommandHandler = this;
	ecs.nMaxLength = CLTGUIEditCtrl::kMaxLength;
	ecs.rnBaseRect.Right() = kBaseWidth - 2*kIndent;
	ecs.rnBaseRect.Bottom() = m_Font.m_nHeight;
	ecs.nCommandID = CMD_OK;
	ecs.argbCaretColor = nonColor;
	ecs.bGlowEnable = true;
	ecs.fGlowAlpha = g_pLayoutDB->GetHighlightGlowAlpha();
	ecs.vGlowSize = g_pLayoutDB->GetHighlightGlowSize();

	m_pEdit = debug_new(CLTGUIEditCtrl);
    if (!m_pEdit->Create(g_pLTClient, m_Font, ecs))
	{
		debug_delete(m_pEdit);
		m_pEdit = NULL;
        return false;
	}
//	m_pEdit->SetFixedWidth(kBaseWidth-2*kIndent,true);
	m_pEdit->SetColor(nonColor);


	cs.rnBaseRect.Right() = kBaseWidth;
	cs.rnBaseRect.Bottom() = kBaseHeight;
	TextureReference hFrame(g_pLayoutDB->GetDialogFrame());
	m_Dlg.Create(hFrame,cs);
	LTVector2n tmp(0,0);
	m_Dlg.AddControl(m_pText, tmp);
	m_Dlg.AddControl(m_pEdit, tmp);
	m_Dlg.AddControl(m_pOK, tmp);
	m_Dlg.AddControl(m_pCancel, tmp);

    return true;
}

void CMessageBox::Term()
{
	m_Dlg.Destroy();
}

void CMessageBox::Show(const char* szStringID, MBCreate* pCreate, uint32 nFontSize, bool bDefaultReturn)
{
	if (m_bVisible || !pCreate)
		return;

	Show(LoadString(szStringID),pCreate,nFontSize,bDefaultReturn);
}

void CMessageBox::Show(const wchar_t *pString, MBCreate* pCreate, uint32 nFontSize, bool bDefaultReturn)
{
	if (m_bVisible || !pCreate)
		return;

	m_eType = pCreate->eType;

	m_bGameWasPaused = g_pGameClientShell->IsGamePaused();
	if (!m_bGameWasPaused)
        g_pGameClientShell->PauseGame(true,true);

	uint32 nSize;
	if (nFontSize)
		nSize = nFontSize;
	else
		nSize = g_pLayoutDB->GetDialogFontSize();

	m_Dlg.SetScale(g_pInterfaceResMgr->GetScreenScale());

	m_pText->SetSize( LTVector2n(kBaseWidth-2*kIndent,nSize) );
	m_pText->SetString(pString);
	m_pText->SetFont(CFontInfo(m_Font.m_szTypeface,nSize));

	if (m_eType == LTMB_EDIT)
	{
		m_pEdit->SetText(pCreate->pString);
		m_pEdit->SetInputMode(pCreate->eInput);
		m_pEdit->SetInputFilter(pCreate->pFilterFn);
		m_pEdit->SetMaxLength(pCreate->nMaxChars);
		m_pEdit->SetPreventEmptyString(pCreate->bPreventEmptyString);

		m_pEdit->Show(true);
		m_pEdit->SetFont(CFontInfo(m_Font.m_szTypeface,nSize));
	}
	else
		m_pEdit->Show(false);


	//build the texture so we can find out how big it is
	m_pText->RecreateTextureStrings();
	LTRect2n rExtents;
	m_pText->GetExtents(rExtents);

	//if the wordwrapped text height is more than half it's width, go to a larger box
	if (rExtents.GetHeight() > (rExtents.GetWidth() / 2))
	{
		m_pText->SetSize(LTVector2n(kBigWidth-2*kIndent,nSize));
		m_pText->GetExtents(rExtents);

	}

	LTVector2n dlgSz;
	dlgSz.x = 2*kIndent + (int32)((float)rExtents.GetWidth()/ g_pInterfaceResMgr->GetXRatio());
	if (dlgSz.x < kMinWidth)
		dlgSz.x = kMinWidth;
	dlgSz.y = (uint32)((float)rExtents.GetHeight()/g_pInterfaceResMgr->GetYRatio());
	dlgSz.y += 3*kIndent + m_pOK->GetBaseHeight();

	if (m_eType == LTMB_EDIT)
	{
		dlgSz.y += (kIndent + m_pEdit->GetBaseHeight());
		if (dlgSz.x < kBaseWidth)
			dlgSz.x = kBaseWidth;
//		m_pEdit->SetFixedWidth(dlgSz.x-2*kIndent,true);
	}

	LTVector2n offset;

	uint32 textWidth = (uint32)( (float)rExtents.GetWidth() / g_pInterfaceResMgr->GetXRatio() );

	m_Dlg.SetSize(dlgSz);
	offset.x = (dlgSz.x - textWidth) / 2;
	offset.y = kIndent;
	m_Dlg.SetControlOffset(m_pText,offset);

	m_bIgnoreEsc = !!( pCreate->nFlags & eMBFlag_IgnoreESC );
	m_bPreventEmptyString = pCreate->bPreventEmptyString;

	switch (m_eType)
	{
	case LTMB_YESNO:
		{
			m_pOK->SetAlignment( kLeft );
			m_pOK->SetString(szYes);
			m_pOK->SetSize(LTVector2n(dlgSz.x/3,m_pOK->GetBaseHeight()));
			offset.x = 4*kIndent;
			offset.y = (dlgSz.y - kIndent) - m_pOK->GetBaseHeight();
			m_Dlg.SetControlOffset(m_pOK,offset);
			m_pOK->Show(true);

			m_pCancel->SetString(szNo);
			m_pCancel->SetSize(LTVector2n(dlgSz.x/3,m_pCancel->GetBaseHeight()));
			offset.x = (dlgSz.x) - 4*kIndent - m_pCancel->GetBaseWidth();
			offset.y = (dlgSz.y - kIndent) - m_pCancel->GetBaseHeight();
			m_Dlg.SetControlOffset(m_pCancel,offset);
			m_pCancel->Show(true);
		}	break;
	case LTMB_OK:
		{
			m_pOK->SetAlignment( kCenter );
			m_pOK->SetString(szOK);
			m_pOK->SetSize(LTVector2n(dlgSz.x/3,m_pOK->GetBaseHeight()));
			offset.x = (dlgSz.x - m_pOK->GetBaseWidth()) / 2;
			offset.y = (dlgSz.y - kIndent) - m_pOK->GetBaseHeight();
			m_Dlg.SetControlOffset(m_pOK,offset);
			m_pOK->Show(true);
			m_pCancel->Show(false);
		} break;
	case LTMB_EDIT:
		{
			offset.x = kIndent;
			offset.y = 2* kIndent + m_pText->GetBaseHeight();
			m_Dlg.SetControlOffset(m_pEdit,offset);

			m_pOK->SetAlignment( kLeft );
			m_pOK->SetString(szOK);
			m_pOK->SetSize(LTVector2n(dlgSz.x/3,m_pOK->GetBaseHeight()));
			offset.x = 4*kIndent;
			offset.y = (dlgSz.y - kIndent) - m_pOK->GetBaseHeight();
			m_Dlg.SetControlOffset(m_pOK,offset);

			UpdateEditOK();

			m_pCancel->SetString(szCancel);
			m_pCancel->SetSize(LTVector2n(dlgSz.x/3,m_pCancel->GetBaseHeight()));
			offset.x = (dlgSz.x) - 4*kIndent - m_pCancel->GetBaseWidth();
			offset.y = (dlgSz.y - kIndent) - m_pCancel->GetBaseHeight();
			m_Dlg.SetControlOffset(m_pCancel,offset);
			m_pCancel->Show(true);
		} break;
	};


	m_pCallback = pCreate->pFn;
	m_pUserData = pCreate->pUserData;
	m_pData = pCreate->pData;

	if (bDefaultReturn)
	{
		m_Dlg.SetSelection(m_Dlg.GetIndex(m_pOK));
	}
	else
	{
		m_Dlg.SetSelection(m_Dlg.GetIndex(m_pCancel));
	}

	offset.x =  (640 - dlgSz.x) / 2;
	offset.y = (480 - dlgSz.y) / 2;
	m_Dlg.SetBasePos(offset);

	m_Dlg.Show(true);
    m_bVisible = true;

    // Set the cursor to the appropriate state
	g_pInterfaceMgr->UpdateCursorState();

	m_nHotKey = pCreate->nHotKey;
}


void CMessageBox::Close(bool bReturn)
{
	if (!m_bVisible)
		return;
    m_bVisible = false;
	m_Dlg.Show(false);

	m_Dlg.FlushTextureStrings();

	if (m_eType == LTMB_EDIT)
		m_pData = (void*)m_pEdit->GetText();


	if (!m_bGameWasPaused)
        g_pGameClientShell->PauseGame(false);

	if (m_pCallback)
	{
		m_pCallback(bReturn, m_pData, m_pUserData);
	}

	// Play sound...
	g_pInterfaceMgr->RequestInterfaceSound(IS_SELECT);

	// Set the cursor to the appropriate state
	g_pInterfaceMgr->UpdateCursorState();
}

void CMessageBox::Draw()
{
	if (m_bVisible)
	{
		m_Dlg.Render();
	}
}

bool CMessageBox::HandleChar(wchar_t c)
{
	if (!m_bVisible) return false;
	if (m_eType == LTMB_EDIT)
	{
		if( m_pEdit->HandleChar(c) )
		{
			UpdateEditOK();
			return true;
		}
	}

	return false;
}

bool CMessageBox::HandleKeyDown(int key, int rep)
{
// XENON: Currently disabled in Xenon builds
#if !defined(PLATFORM_XENON)
	if (!m_bVisible) return false;
	if (m_eType == LTMB_EDIT)
	{
		//special handling for right and left arrows so that they may cycle through controls
		// when the edit box is not selected
		if (m_Dlg.GetSelectedControl() != m_pEdit)
		{
			if (key == VK_LEFT)
				return m_Dlg.OnUp();
			if (key == VK_RIGHT)
				return m_Dlg.OnDown();
		}

		if (m_pEdit->HandleKeyDown(key,rep))
		{
			UpdateEditOK();
			return true;
		}
	}

    bool handled = false;
	switch (key)
	{
	case VK_LEFT:
	case VK_UP:
		{
			handled = m_Dlg.OnUp();
			break;
		}
	case VK_RIGHT:
	case VK_DOWN:
		{
			handled = m_Dlg.OnDown();
			break;
		}
	case VK_RETURN:
		{
			handled = m_Dlg.OnEnter();
			break;
		}
	case VK_ESCAPE:
		{
			if (!m_bIgnoreEsc)
			{
				Close(false);
				handled = true;
			}
			break;
		}
	default:
		{
			if (m_nHotKey > 0 && key == m_nHotKey)
			{
				Close(true);
				handled = true;
			}
			else
			{
				CLTGUICtrl* pCtrl = m_Dlg.GetSelectedControl();
				if (pCtrl)
					handled = pCtrl->HandleKeyDown(key,rep);
				else
					handled = false;
				break;
			}
		}
	}

	// Handled the key
	return handled;
#else // PLATFORM_XENON
	return false;
#endif // PLATFORM_XENON
}

bool CMessageBox::OnLButtonDown(int x, int y)
{
	if (m_bVisible)
	{
		return m_Dlg.OnLButtonDown( x, y);
	}
    return false;
}

bool CMessageBox::OnLButtonUp(int x, int y)
{
	if (m_bVisible)
	{
		return m_Dlg.OnLButtonUp( x, y);
	}
    return false;
}


bool CMessageBox::OnMouseMove(int x, int y)
{
	if (m_bVisible)
	{
		return m_Dlg.OnMouseMove(x,y);
	}
    return false;
}

uint32 CMessageBox::OnCommand(uint32 dwCommand, uint32 /*dwParam1*/, uint32 /*dwParam2*/)
{
	switch(dwCommand)
	{
	case CMD_OK:
        Close(true);
        return true;
		break;
	case CMD_CANCEL:
        Close(false);
        return true;
		break;
	default:
        return false;
		break;
	}
}



void CMessageBox::UpdateEditOK()
{
	if( !m_bPreventEmptyString )
	{
		m_pOK->Show(true);
	}
	else
	{
		if( !m_pEdit->IsEmpty() )
		{
			// Be sure we turn the OK button back on.
			if( !m_pOK->IsVisible() )
			{
				m_pOK->Show(true);

				// If the mouse is over the OK button that just popped up, make 
				// the OK button the current selection.
				const LTVector2n cursor_pos = g_pInterfaceMgr->GetCursorPos();

				uint32 nControlUnderPoint;
				if( m_pOK == m_Dlg.GetControlUnderPoint(cursor_pos.x, cursor_pos.y, &nControlUnderPoint) )
				{
					m_Dlg.SetSelection(nControlUnderPoint);
				}
			}
		}
		else
		{
			m_pOK->Show(false);

			// If the OK button was the current selection, 
			// select the edit box instead.
			if( m_Dlg.GetSelectedControl() == m_pOK )
			{
				const uint32 nTextIndex = m_Dlg.GetIndex(m_pEdit);
				m_Dlg.SetSelection( nTextIndex );
			}
		}
	}
}