// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUIColumnCtrl.cpp
//
// PURPOSE : Control to display columns of text
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //


#include "ltguimgr.h"
#include "ltguiColumnCtrl.h"

const uint8	CLTGUIColumnCtrl::kMaxNumColumns = 255;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLTGUIColumnCtrl::CLTGUIColumnCtrl()
{
    m_pFont=LTNULL;
	m_nFontSize		= 0;
	m_nBaseFontSize	= 0;
	m_nWidth		= 0;
	m_nBaseWidth	= 0;
	m_nHeight		= 0;

    m_pCommandHandler=LTNULL;

}

CLTGUIColumnCtrl::~CLTGUIColumnCtrl()
{
	Destroy();
}

// Create the control
// nCommandID	- The command ID which is sent when "enter" is pressed.
// pFont		- The font to use for rendering the strings.
// pMessageCtrl - control which receives messages when the "enter" key is pressed.
LTBOOL CLTGUIColumnCtrl::Create (	uint32 nCommandID, 
									uint32 nHelpID,
									CUIFont *pFont, 
									uint8 nFontSize,
									CLTGUICommandHandler *pCommandHandler, 
									uint32 nParam1, 
									uint32 nParam2 )
{
	m_pCommandHandler=pCommandHandler;

	SetFont(pFont,nFontSize,LTFALSE);

	CLTGUICtrl::Create(nCommandID,nHelpID,nParam1,nParam2);
    return LTTRUE;
}

// Destroys the control
void CLTGUIColumnCtrl::Destroy ( )
{
	// Remove the columns
	RemoveAllColumns();
}

// Adds a column to the control
// nWidth	  - Width of the column
// hString	  - The initial text for the column
uint8 CLTGUIColumnCtrl::AddColumn(const char *pString, uint16 nWidth, LTBOOL bClip)
{
	if ( !pString )
	{
        assert(LTFALSE);
        return -1;
	}

	CLTGUITextCtrl *pCtrl = debug_new(CLTGUITextCtrl);
	if (!pCtrl->Create(pString,LTNULL,LTNULL,m_pFont,m_nBaseFontSize,LTNULL))
	{
		debug_delete(pCtrl);
		return -1;
	}
	m_columnArray.push_back(pCtrl);

	//force a little extra space between columns, if we can
	nWidth -= (m_nBaseFontSize/2);
	if (nWidth < m_nBaseFontSize + 1)
		nWidth = m_nBaseFontSize + 1 ;

	pCtrl->SetFixedWidth(nWidth,bClip);

	

	LTIntPt pos = m_basePos;
	pos.x += m_nBaseWidth;
    pCtrl->SetColors(m_argbSelected, m_argbNormal, m_argbDisabled);
	pCtrl->SetBasePos(pos);
	pCtrl->SetScale(m_fScale);


	CalculateSize();


    return m_columnArray.size() - 1;
}


// Gets a string at a specific column index.  This returns a copy (new handle).
CLTGUITextCtrl*	 CLTGUIColumnCtrl::GetColumn(uint8 nColumnIndex) const
{
	if (nColumnIndex > kMaxNumColumns || nColumnIndex >= m_columnArray.size())
		return LTNULL;
	return m_columnArray[nColumnIndex];
}

CUIFormattedPolyString*	CLTGUIColumnCtrl::GetPolyString(uint8 nColumnIndex) const
{
	if (nColumnIndex > kMaxNumColumns || nColumnIndex >= m_columnArray.size())
		return LTNULL;
	return m_columnArray[nColumnIndex]->GetString();

}

const char*	CLTGUIColumnCtrl::GetString(uint8 nColumnIndex) const
{
	CUIFormattedPolyString*	pString = GetPolyString(nColumnIndex);
	if (pString)
		return pString->GetText();
	else
		return LTNULL;
}

// Sets a string for a column
void CLTGUIColumnCtrl::SetString(uint8 nColumnIndex, const char *pString)
{
	if (nColumnIndex >= m_columnArray.size() || !pString)
		return;

	// Allocate the new string
	m_columnArray[nColumnIndex]->SetString(pString);
}


// Removes a column
void CLTGUIColumnCtrl::RemoveColumn(uint8 nIndex)
{
	if (nIndex >= m_columnArray.size())
		return;

	TextControlArray::iterator iter = m_columnArray.begin();
	iter += nIndex;

	if (iter == m_columnArray.end())
		return;

	debug_delete(*iter);
	m_columnArray.erase(iter);
}

// Removes all of the columns
void CLTGUIColumnCtrl::RemoveAllColumns()
{
	TextControlArray::iterator iter = m_columnArray.begin();

	while (iter != m_columnArray.end())
	{
		debug_delete(*iter);
		iter++;
	}
	m_columnArray.clear();

}

// Render the control
void CLTGUIColumnCtrl::Render ()
{
	if (!IsVisible()) return;

	for (int i=0; i < GetNumColumns(); i++)
	{
		if (m_columnArray[i]->IsVisible())
			m_columnArray[i]->Render();
	}
}

// Render the control
void CLTGUIColumnCtrl::OnSelChange()
{

	for (int i=0; i < GetNumColumns(); i++)
	{
		m_columnArray[i]->Select(IsSelected());
	}
}

// Set the font
void CLTGUIColumnCtrl::SetFont(CUIFont *pFont, uint8 nFontSize, LTBOOL bSetForAll)
{
	if (!pFont && !nFontSize)
	{
        return;
	}

	if (pFont)
		m_pFont = pFont;
	if (nFontSize)
		m_nBaseFontSize = nFontSize;

	m_nFontSize = (uint8)(m_fScale * (float)m_nBaseFontSize);

	if (bSetForAll)
	{
		for (int i=0; i < GetNumColumns(); i++)
		{
			m_columnArray[i]->SetFont(pFont,nFontSize);
		}
		CalculateSize();
	}

}

// Handle the Enter key being pressed
LTBOOL CLTGUIColumnCtrl::OnEnter ( )
{
	// Send the command
	if ( m_pCommandHandler )
	{
		if	(m_pCommandHandler->SendCommand(m_nCommandID, m_nParam1, m_nParam2))
            return LTTRUE;
	}
    return LTFALSE;
}


void CLTGUIColumnCtrl::CalculateSize()
{
	m_nWidth = 0;
	m_nBaseWidth = 0;
	m_nHeight = 0;

	// find the size
	for (int i=0; i < GetNumColumns(); i++)
	{
		m_nBaseWidth += (m_columnArray[i]->GetFixedWidth() + (m_nBaseFontSize/2));
		if (m_columnArray[i]->GetHeight() > m_nHeight)
		{
			m_nHeight = m_columnArray[i]->GetHeight();
		}
	}

	m_nWidth = (uint16) ((float)m_nBaseWidth * m_fScale);

	//adjust vertical positions
	for (int i=0; i < GetNumColumns(); i++)
	{
		uint16 nHt = m_columnArray[i]->GetHeight();
		if (nHt < m_nHeight)
		{
			LTIntPt pos = m_columnArray[i]->GetBasePos();
			uint16 nOffset = (uint16) ( (LTFLOAT)(m_nHeight - nHt) / m_fScale );
			pos.y = m_basePos.y + nOffset / 2;
			m_columnArray[i]->SetBasePos(pos);
		}
	}
}


void CLTGUIColumnCtrl::SetBasePos ( LTIntPt pos )
{ 
	CLTGUICtrl::SetBasePos(pos);
	LTIntPt tmpPos = m_basePos;
	for (int i=0; i < GetNumColumns(); i++)
	{
		m_columnArray[i]->SetBasePos(tmpPos);
		tmpPos.x += (m_columnArray[i]->GetFixedWidth() + (m_nBaseFontSize/2) );
	}

	CalculateSize();

}

void CLTGUIColumnCtrl::SetScale(float fScale)
{
	CLTGUICtrl::SetScale(fScale);
	m_nFontSize = (uint8)(m_fScale * (float)m_nBaseFontSize);
	for (int i=0; i < GetNumColumns(); i++)
	{
		m_columnArray[i]->SetScale(fScale);
	}
	CalculateSize();

}

void CLTGUIColumnCtrl::SetColors(uint32 argbSelected, uint32 argbNormal, uint32 argbDisabled)
{	
	CLTGUICtrl::SetColors(argbSelected, argbNormal, argbDisabled);
	for (int i=0; i < GetNumColumns(); i++)
	{
		m_columnArray[i]->SetColors(m_argbSelected, m_argbNormal, m_argbDisabled);
	}
}

void CLTGUIColumnCtrl::Enable ( LTBOOL bEnabled )
{ 
	CLTGUICtrl::Enable (bEnabled);
	for (uint8 i = 0; i < m_columnArray.size(); i++ )
	{
		m_columnArray[i]->Enable (bEnabled);
	}

}
