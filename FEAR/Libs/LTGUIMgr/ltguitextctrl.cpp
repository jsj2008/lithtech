// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUITextCtrl.cp
//
// PURPOSE : Simple text control which may be used as a menu item.
//
// (c) 2001-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "ltguimgr.h"
#include "ltguitextctrl.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLTGUITextCtrl::CLTGUITextCtrl() :
	m_nBaseFontSize(0), m_nIndent(0), m_bWordWrap(false), m_bClip(false)
{
}

CLTGUITextCtrl::~CLTGUITextCtrl()
{
	Destroy();
}

// Create the control
bool CLTGUITextCtrl::Create (  	const wchar_t*	pText,
								const CFontInfo& Font,
                                const CLTGUICtrl_create& cs,
								const bool bEllipsis /*= false*/ )
{

	if (!SetFont(Font))
	{
        return false;
	}

	// Add the string
	if (pText)
	{
		SetString(pText, bEllipsis);
	}

	CLTGUICtrl::Create(cs);

	m_Text.SetGlowParams(cs.bGlowEnable,cs.fGlowAlpha,cs.vGlowSize);


    return true;
}


// Destroys the control
void CLTGUITextCtrl::Destroy ( )
{
	FlushTextureStrings();
}


// Set the control's string
void CLTGUITextCtrl::SetString(const wchar_t *pString, bool bEllipsis)
{
	m_Text.SetText(pString, bEllipsis);
}


// Set the font, note this always expects an unscaled height
bool CLTGUITextCtrl::SetFont(const CFontInfo& Font)
{
	if (!Font.m_nHeight)
	{
		return false;
	}

	m_nBaseFontSize = Font.m_nHeight;

	CFontInfo tmpFont = Font;
	tmpFont.m_nHeight = (uint32)(m_vfScale.y * (float)m_nBaseFontSize);

	m_Text.SetFont(tmpFont);

	return true;
}

// Set the font, note this always expects an unscaled height
bool CLTGUITextCtrl::SetFontHeight(uint32 nFontHeight)
{
	if (!nFontHeight)
	{
		return false;
	}

	m_nBaseFontSize = nFontHeight;
	
	uint32 nHeight = (uint32)(m_vfScale.y * (float)m_nBaseFontSize);

	m_Text.SetFontHeight(nHeight);

	return true;
}


// Render the control
void CLTGUITextCtrl::Render()
{
	// Sanity checks...
	if (!IsVisible()) 
		return;

    m_Text.SetColor(GetCurrentColor());
	m_Text.SetGlow(IsSelected());
	
	if (m_bClip)
	{
		LTRect2n rClipRect;
		rClipRect.m_vMin.x = (int32)m_rfRect.m_vMin.x;
		rClipRect.m_vMin.y = (int32)m_rfRect.m_vMin.y;
		rClipRect.m_vMax.x = (int32)m_rfRect.m_vMax.x;
		rClipRect.m_vMax.y = (int32)m_rfRect.m_vMax.y;

		m_Text.RenderClipped( rClipRect );
	}
	else
	{
		m_Text.Render( );
	}
}

// Render the control
void CLTGUITextCtrl::RenderTransition(float fTrans)
{
	// Sanity checks...
	if (!IsVisible()) 
		return;

	m_Text.SetColor(GetCurrentColor());

	if (m_bClip)
	{
		LTRect2n rClipRect;
		rClipRect.m_vMin.x = (int32)m_rfRect.m_vMin.x;
		rClipRect.m_vMin.y = (int32)m_rfRect.m_vMin.y;
		rClipRect.m_vMax.x = (int32)m_rfRect.m_vMax.x;
		rClipRect.m_vMax.y = (int32)m_rfRect.m_vMax.y;

		m_Text.RenderTransitionClipped( rClipRect,fTrans );
	}
	else
	{
		m_Text.RenderTransition(fTrans);
	}
}



// Enter was pressed
bool CLTGUITextCtrl::OnEnter ( )
{
	// Send the command
	if ( m_pCommandHandler && m_pCommandHandler->SendCommand(m_nCommandID, m_nParam1, m_nParam2) )
		return true;
    return false;
}

void CLTGUITextCtrl::SetBasePos ( const LTVector2n& pos )
{ 
	CLTGUICtrl::SetBasePos(pos);
	float fi = (float)m_nIndent * m_vfScale.x;

	LTVector2 vPos;
	switch(m_Text.GetAlignment()) 
	{
	case kLeft:
	default:
		vPos = m_rfRect.m_vMin;
		vPos.x += fi;
		break;
	case kRight:
		vPos.y = m_rfRect.Top();
		vPos.x = m_rfRect.Right() - fi;
		break;
	case kCenter:
		vPos.y = m_rfRect.Top();
		vPos.x = (m_rfRect.Left() + m_rfRect.Right()) / 2;
		break;
	}

	m_Text.SetPos(vPos);

}

void CLTGUITextCtrl::SetScale(const LTVector2& vfScale)
{
	if (m_vfScale == vfScale && m_rfRect.GetWidth() > 0.0)
		return;

	CLTGUICtrl::SetScale(vfScale);

	m_Text.SetFontHeight((uint32)(m_vfScale.y * (float)m_nBaseFontSize));

	LTVector2 vPos;
	float fi = (float)m_nIndent * m_vfScale.x;
	switch(m_Text.GetAlignment()) 
	{
	case kLeft:
	default:
		vPos = m_rfRect.m_vMin;
		vPos.x += fi;
		break;
	case kRight:
		vPos.y = m_rfRect.Top();
		vPos.x = m_rfRect.Right() - fi;
		break;
	case kCenter:
		vPos.y = m_rfRect.Top();
		vPos.x = (m_rfRect.Left() + m_rfRect.Right()) / 2;
		break;
	}

	m_Text.SetPos(vPos);

	if (m_bWordWrap)
	{
		uint32 nWidth = (uint32)( GetWidth() - (fi * 2.0f) );
		m_Text.WordWrap(nWidth);
	}
}

void CLTGUITextCtrl::SetSize(const LTVector2n& sz)
{
	CLTGUICtrl::SetSize(sz);
	if (m_bWordWrap)
	{
		float fi = (float)m_nIndent * m_vfScale.x;
		uint32 nWidth = (uint32)( GetWidth() - (fi * 2.0f) );
		m_Text.WordWrap(nWidth);
	}
}


void CLTGUITextCtrl::SetIndent(uint32 nIndent)
{
	m_nIndent = nIndent;

	if (m_bWordWrap)
	{
		float fi = (float)m_nIndent * m_vfScale.x;
		uint32 nWidth = (uint32)( GetWidth() - (fi * 2.0f) );
		m_Text.WordWrap(nWidth);
	}

}

void CLTGUITextCtrl::SetWordWrap(bool bWrap)
{
	m_bWordWrap = bWrap;

	float fi = (float)m_nIndent * m_vfScale.x;
	uint32 nWidth = (uint32)( GetWidth() - (fi * 2.0f) );
	if (m_bWordWrap)
		m_Text.WordWrap(nWidth);
	else
		m_Text.WordWrap(0);

}

void CLTGUITextCtrl::SetDropShadow(uint8 drop) 
{
	m_Text.SetDropShadow(drop);
}


// free texture memory by flushing any texture strings owned by the control
void CLTGUITextCtrl::FlushTextureStrings()
{
	m_Text.FlushTexture();
}

// rebuild any texture strings owned by the control
void CLTGUITextCtrl::RecreateTextureStrings()
{
	if (m_Text.IsEmpty())
		return;
	m_Text.CreateTexture();
}


const wchar_t*  CLTGUITextCtrl::GetString() const
{
	return m_Text.GetText();
}


bool CLTGUITextCtrl::GetExtents(LTRect2n& rExtents)
{
	return (LT_OK == m_Text.GetExtents(rExtents));
}

void CLTGUITextCtrl::SetAlignment(eTextAlign align) 
{
	m_Text.SetAlignment(align);

	LTVector2 vPos;
	float fi = (float)m_nIndent * m_vfScale.x;
	switch(align) 
	{
	case kLeft:
	default:
		vPos = m_rfRect.m_vMin;
		vPos.x += fi;
		break;
	case kRight:
		vPos.y = m_rfRect.Top();
		vPos.x = m_rfRect.Right() - fi;
		break;
	case kCenter:
		vPos.y = m_rfRect.Top();
		vPos.x = (m_rfRect.Left() + m_rfRect.Right()) / 2;
		break;
	}

	m_Text.SetPos(vPos);
}