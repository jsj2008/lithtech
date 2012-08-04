// LTGUIEditCtrl.cpp: implementation of the CLTGUIEditCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "ltguimgr.h"
#include "ltguieditctrl.h"
#if !defined(PLATFORM_XENON)
#include "vkdefs.h"
#endif // !PLATFORM_XENON
#include "ctype.h"
#include <locale.h>
#include "EngineTimer.h"

const uint32 CLTGUIEditCtrl::kMaxLength = 128;


namespace
{
	wchar_t szString[CLTGUIEditCtrl::kMaxLength];
	
	bool IsFileFriendly(wchar_t c, bool isFirst)
	{
		static const wchar_t* pszOKChars = L" _";
		if (isalnum(c)) return true;
		if (wcschr(pszOKChars,c) && !isFirst) return true;
		return false;
	}
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLTGUIEditCtrl::CLTGUIEditCtrl()
{
    m_pLTClient     = NULL;
	m_nBaseFontSize = 0;

	m_nMaxLength = 0;
	m_nCaretPos = 0;
	m_pszValue = NULL;

    m_bCaretEnabled = false;
	m_fCaretTime = 0.0f;

	m_bUseFrame			= false;

	m_eInputMode		= kInputAll;
	m_pFn				= NULL;

}

CLTGUIEditCtrl::~CLTGUIEditCtrl()
{
	Destroy();
}

// Create the control
bool CLTGUIEditCtrl::Create(ILTClient *pLTClient, const CFontInfo& Font, const CLTGUIEditCtrl_create& cs)
{
	LTASSERT((cs.nMaxLength > 0 && cs.nMaxLength <= kMaxLength),"CLTGUIEditCtrl::Create() invalid buffer length");
	if (!pLTClient || !Font.m_nHeight || cs.nMaxLength <= 0)
        return false;



	m_pLTClient			= pLTClient;
	g_pLTBase			= pLTClient;
	m_pCommandHandler	= cs.pCommandHandler;
	m_pszValue			= cs.pszValue;
	m_bPreventEmptyString = cs.bPreventEmptyString;

	if (cs.nMaxLength <= kMaxLength)
		SetMaxLength(cs.nMaxLength);
	else
		SetMaxLength(kMaxLength);

	if (!SetFont(Font))
	{
        return false;
	}


	// Add the string
	if (cs.pszValue)
	{
		SetText(cs.pszValue);
	}

	DrawPrimSetRGBA(m_Caret,    0xFF, 0xFF, 0xFF, 0xFF);
	DrawPrimSetRGBA(m_Frame[0], 0x00, 0x00, 0x00, 0x7F);
	DrawPrimSetRGBA(m_Frame[1], 0xFF, 0xFF, 0xFF, 0x7F);
	DrawPrimSetRGBA(m_Frame[2], 0xFF, 0xFF, 0xFF, 0x7F);
	DrawPrimSetRGBA(m_Frame[3], 0x00, 0x00, 0x00, 0x7F);

	m_bUseFrame = cs.bUseFrame;

	EnableCaret(cs.bUseCaret, cs.fCaretTime, cs.argbCaretColor);
	SetInputMode(cs.eMode);

	m_Text.SetGlowParams(cs.bGlowEnable,cs.fGlowAlpha,cs.vGlowSize);

	CLTGUICtrl::Create(cs);
    return true;
}

// Destroys the control
void CLTGUIEditCtrl::Destroy ( )
{
	FlushTextureStrings();
}

// Update data
void CLTGUIEditCtrl::UpdateData(bool bSaveAndValidate)
{
	if (!m_pszValue)
		return;

	// Save the string out
	if (bSaveAndValidate)
	{
		LTStrCpy(m_pszValue, m_Text.GetText() ,m_nMaxLength);
	}
	else
		SetText(m_pszValue);
}

// Render the control
void CLTGUIEditCtrl::Render ()
{
	if ( !IsVisible())
		return;


	// Render the text
	m_Text.SetColor(GetCurrentColor());
	m_Text.SetGlow(IsSelected());
	m_Text.Render( );

	g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_NoBlend);

	// draw the caret
	if (IsCaretOn() && IsEnabled())
	{
		LTRect2n rRect;

		uint32 len = LTStrLen(GetText());
		if (m_nCaretPos > len)
			m_nCaretPos = len;
		
		float x = m_rfRect.Left();
		float y = m_rfRect.Top();
		float w = 2.0f;
		float h = GetHeight();
		if (m_nCaretPos > 0)
		{

			LTRect2n rRect;
			if (m_Text.GetCharRect(m_nCaretPos-1,rRect) == LT_OK)
			{
				x = (float)rRect.Right();
			}
		}
		DrawPrimSetXYWH(m_Caret,x,y,w,h);
		g_pDrawPrim->DrawPrim(&m_Caret);
	}

	g_pDrawPrim->SetRenderMode(eLTDrawPrimRenderMode_Modulate_Translucent);


	// draw our frame
	if (m_bUseFrame)
		g_pDrawPrim->DrawPrim(m_Frame,4);

}

// Check for caret use
bool CLTGUIEditCtrl::IsCaretOn()
{
    if (!m_bCaretEnabled) return(false);

	static float fCaretTimer = 0.0f;
	static double fLastTime    = 0.0f;
    static bool bCaretOn    = true;

	double fCurTime   = RealTimeTimer::Instance().GetTimerAccumulatedS();
	float fDeltaTime = (float)(fCurTime - fLastTime);

	if (fDeltaTime >= fCaretTimer)
	{
		fCaretTimer  = m_fCaretTime;
		bCaretOn    ^= 1;
	}
	else
		fCaretTimer -= fDeltaTime;

	fLastTime = fCurTime;

	return(bCaretOn);
}

// Enable the caret
void CLTGUIEditCtrl::EnableCaret(bool bUseCaret, float fToggleTime, uint32 argbColor)
{
    m_bCaretEnabled = bUseCaret;
	m_fCaretTime    = fToggleTime;

	DrawPrimSetRGBA(m_Caret,argbColor);
}


// Set the font, note this always expects an unscaled height
bool CLTGUIEditCtrl::SetFont(const CFontInfo& Font)
{
	if (!Font.m_nHeight)
	{
		return false;
	}

	ASSERT(Font.m_nHeight <= 255);
	m_nBaseFontSize = (uint8)LTMIN(Font.m_nHeight,255);

	CFontInfo tmpFont = Font;
	tmpFont.m_nHeight = (uint32)(m_vfScale.y * (float)m_nBaseFontSize);

	m_Text.SetFont(tmpFont);

	return true;
}

// Set the text
void CLTGUIEditCtrl::SetText(const wchar_t *pString)
{
	m_Text.SetText(pString);
	m_nCaretPos = LTStrLen(pString);
}

const wchar_t*  CLTGUIEditCtrl::GetText() const
{
	return m_Text.GetText();
}

void CLTGUIEditCtrl::SetBasePos ( const LTVector2n& pos )
{ 
	CLTGUICtrl::SetBasePos(pos);
	m_Text.SetPos(m_rfRect.m_vMin);

}

void CLTGUIEditCtrl::SetScale(const LTVector2& vfScale)
{
	if (m_vfScale == vfScale && m_rfRect.GetWidth() > 0.0)
		return;

	CLTGUICtrl::SetScale(vfScale);

	m_Text.SetFontHeight((uint32)(m_vfScale.y * (float)m_nBaseFontSize));
	m_Text.SetPos(m_rfRect.m_vMin);
}

void CLTGUIEditCtrl::SetSize(const LTVector2n& sz)
{
	CLTGUICtrl::SetSize(sz);
}


// Add a character to the end
void CLTGUIEditCtrl::AddCharacter(wchar_t c)
{
	//add 1 for trailing 0
	uint32 len = LTStrLen(m_Text.GetText()) + 1;

	// Check to see are at our buffer limit
	if (len >= m_nMaxLength)
		return;

	// strip out leading whitespace which confuses things
	if (LTStrEmpty(m_Text.GetText( )) && c == ' ')
		return;

	
	LTStrCpy(szString, m_Text.GetText(), LTARRAYSIZE(szString));
	uint32 nIndex = LTStrLen(szString);

	while (nIndex > m_nCaretPos)
	{
		szString[nIndex] = szString[nIndex-1];
		nIndex--;
	}

	szString[nIndex]	= c;
	m_nCaretPos++;
	m_Text.SetText(szString);

	LTRect2n rExt;
	LTRESULT res = m_Text.GetExtents(rExt);
	if (res == LT_OK)
	{
		if (rExt.GetWidth() > int32(GetWidth()) )
		{
			m_nCaretPos--;
			RemoveCharacter();
		}
	}
}

// Remove a character from the end
void CLTGUIEditCtrl::RemoveCharacter()
{

	// Check to see have any chars
	if (m_Text.IsEmpty())
		return;

	LTStrCpy(szString,m_Text.GetText(), LTARRAYSIZE(szString));

	uint32 nEnd=LTStrLen(szString);
	if (nEnd > m_nCaretPos)
	{
		uint32 nIndex = m_nCaretPos;

		while (nIndex < nEnd)
		{
			szString[nIndex] = szString[nIndex+1];
			nIndex++;
		}

		szString[nIndex]=L'\0';
		m_Text.SetText(szString);
	}

}

// Handles a key press
bool CLTGUIEditCtrl::HandleKeyDown(int key, int rep)
{
	if (CLTGUICtrl::HandleKeyDown(key, rep))
        return true;

	switch (key)
	{
	case VK_BACK:
		if (m_nCaretPos > 0)
		{
			m_nCaretPos--;
			RemoveCharacter();
		}
		break;
	case VK_DELETE:
		RemoveCharacter();
		break;
	case VK_HOME:
		m_nCaretPos = 0;
		break;
	case VK_END:
		m_nCaretPos = LTStrLen(m_Text.GetText());
		break;
	case VK_LEFT:
		if (m_nCaretPos > 0) m_nCaretPos--;
		break;
	case VK_RIGHT:
		if (m_nCaretPos < LTStrLen(m_Text.GetText())) m_nCaretPos++;
		break;
	default:
		return false;
	}

    return true;
}

// Handles a key press
bool CLTGUIEditCtrl::HandleChar(wchar_t c)
{
    if (CLTGUICtrl::HandleChar(c)) return true;
    if (c < ' ') return false;

	if (m_pFn)
	{
		c = (*m_pFn)(c,m_nCaretPos);
		if (!c)
			return LTFALSE;
	}

	switch	(m_eInputMode)
	{
	case kInputAlphaNumeric:
		if (!isalnum(c)) return false;
		break;
	case kInputAlphaOnly:
		if (!isalpha(c)) return false;
		break;
	case kInputNumberOnly:
		if (!isdigit(c)) return false;
		break;
	case kInputFileFriendly:
		if (!IsFileFriendly(c,(m_nCaretPos == 0))) return false;
		break;
	};


	AddCharacter(c);
    return true;
}

// Handle the Enter key being pressed
bool CLTGUIEditCtrl::OnEnter ( )
{
	// Send the command
	if ( m_pCommandHandler )
	{
		// Prevent entry of empty strings.
		if( m_bPreventEmptyString )
		{
			// If we have an empty string, swallow the 
			// the enter key press, but do not send our
			// command
			if( m_Text.IsEmpty() )
				return true;
		}

		if	(m_pCommandHandler->SendCommand(m_nCommandID, m_nParam1, m_nParam2))
			return true;
	}

    return false;
}


void CLTGUIEditCtrl::CalculateSize()
{

	if (m_bUseFrame)
	{
		float fx = m_rfRect.Left() - 1.0f;
		float fy = m_rfRect.Top() - 1.0f;
		float fw = m_rfRect.GetWidth() + 1.0f;
		float fh = 1.0f;
		DrawPrimSetXYWH(m_Frame[0],fx,fy,fw,fh);

		fx = (m_rfRect.Left() + m_rfRect.GetWidth());
		fy = m_rfRect.Top() - 1.0f;
		fw = 1.0f;
		fh = m_rfRect.GetHeight() + 1.0f;
		DrawPrimSetXYWH(m_Frame[1],fx,fy,fw,fh);

		fx = m_rfRect.Left();
		fy = (m_rfRect.Top() + m_rfRect.GetHeight());
		fw = m_rfRect.GetWidth() + 1.0f;
		fh = 1.0f;
		DrawPrimSetXYWH(m_Frame[2],fx,fy,fw,fh);

		fx = m_rfRect.Left() - 1.0f;
		fy = m_rfRect.Top();
		fw = 1.0f;
		fh = m_rfRect.GetHeight() + 1.0f;
		DrawPrimSetXYWH(m_Frame[3],fx,fy,fw,fh);
	}

}

/*
void CLTGUIEditCtrl::SetFixedWidth(uint16 nWidth, bool bUseFrame)
{
	uint16 nMinWidth = m_nBaseFontSize + 1;
	ASSERT(nWidth >= nMinWidth);
	if (nWidth < nMinWidth)
		nWidth = nMinWidth;
	m_nFixedWidth = nWidth;
	CalculateSize();

	if (m_nFixedWidth)
	{
		m_bUseFrame = bUseFrame;
		if (m_pText)
		{
			float testWidth = m_fScale * m_nFixedWidth;
			int nOldCaret = m_nCaretPos;
			m_nCaretPos = m_pText->GetLength();
			while (m_pText->GetLength() && m_pText->GetWidth() > testWidth )
			{
				m_nCaretPos--;
				RemoveCharacter();				
			}
			if (nOldCaret > m_pText->GetLength())
				m_nCaretPos = m_pText->GetLength();
			else
				m_nCaretPos = nOldCaret;
		}
	}
	else
		m_bUseFrame = false;
}
*/

void CLTGUIEditCtrl::SetMaxLength(uint32 nMaxLength)
{
	if (nMaxLength > kMaxLength)
		nMaxLength = kMaxLength;
	LTASSERT(nMaxLength > 0,"Edit ctrl max length set to 0");
	if (nMaxLength == 0)
		nMaxLength = 1;
	m_nMaxLength = nMaxLength;

	if (m_nMaxLength <= LTStrLen(m_Text.GetText()))
	{
		LTStrCpy(szString,m_Text.GetText(), LTARRAYSIZE(szString));
		szString[m_nMaxLength-1] = '\0';
		m_Text.SetText(szString);
	}
}


bool CLTGUIEditCtrl::OnLButtonUp(int x, int y)
{
	if (!IsOnMe(x,y)) return false;

	uint32 nMax = LTStrLen(m_Text.GetText());

	uint32 nIndex = 0;
	bool bFound = false;
	while (nIndex < nMax && !bFound)
	{
		LTRect2n rRect;
		if (m_Text.GetCharRect(m_nCaretPos-1,rRect) == LT_OK)
		{
			bFound = (x < rRect.Left());
		}
		
		nIndex++;
	}

	m_nCaretPos = nIndex;

	return true;
}

// free texture memory by flushing any texture strings owned by the control
void CLTGUIEditCtrl::FlushTextureStrings()
{
	m_Text.FlushTexture();
}

// rebuild any texture strings owned by the control
void CLTGUIEditCtrl::RecreateTextureStrings()
{
	if (m_Text.IsEmpty())
		return;
	m_Text.CreateTexture();
}
