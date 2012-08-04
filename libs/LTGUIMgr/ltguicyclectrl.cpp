// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUICycleCtrl.cpp
//
// PURPOSE : Control which can cycle through a set of strings based on 
//				user input.
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "ltguimgr.h"
#include "ltguicyclectrl.h"


const uint8	CLTGUICycleCtrl::kMaxNumStrings = 254;
const uint8	CLTGUICycleCtrl::kNoSelection = 255;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLTGUICycleCtrl::CLTGUICycleCtrl()
{
    m_pHeaderText   = LTNULL;
	m_nHeaderWidth	= 0;
	m_nSelIndex			= kNoSelection;
	m_pCommandHandler	= LTNULL;

}

CLTGUICycleCtrl::~CLTGUICycleCtrl()
{
	Destroy();
}

// Create the control
// pText			- The initial text that is displayed for this control. Pass in 
//						LTNULL if you do not want initial text. A copy of this text
//						is made so the string may be discarded after making this call.
// pFont			- The font to use for this string.
// nFontSize		- The font size to use for this string.
// nHeaderWidth	- The width to use for the header string
// pnValue		- Pointer to the value which receives the selection index when UpdateData is called.
LTBOOL CLTGUICycleCtrl::Create ( const char *pText, uint32 nHelpID, 
								CUIFont *pFont, uint8 nFontSize, uint16 nHeaderWidth, uint8 *pnValue)
{

	if (!SetFont(pFont,nFontSize))
	{
        return LTFALSE;
	}

	if ( pText)
	{
		if (m_pHeaderText)
			m_pHeaderText->SetText((char *)pText);
		else
		{
			m_pHeaderText = g_pFontManager->CreateFormattedPolyString(m_pFont,(char *)pText,(float)m_pos.x,(float)m_pos.y);
			m_pHeaderText->SetCharScreenHeight(m_nFontSize);

		}
	}

	if (nHeaderWidth)
		m_nHeaderWidth = nHeaderWidth;
	else if (m_pHeaderText)
		m_nHeaderWidth = (uint16)m_pHeaderText->GetWidth() + m_nFontSize;
	

	m_nSelIndex=kNoSelection;		
	m_pnValue = pnValue;

    CLTGUICtrl::Create(LTNULL, nHelpID);

	return LTTRUE;
}

// Destroys the control
void CLTGUICycleCtrl::Destroy ( )
{
	if (m_pHeaderText)
	{
		g_pFontManager->DestroyPolyString(m_pHeaderText);
		m_pHeaderText = LTNULL;
	}

	// Remove the strings
	RemoveAll();

}

// Update data
void CLTGUICycleCtrl::UpdateData (LTBOOL bSaveAndValidate)
{
	if (!m_pnValue)
	{
		return;
	}

	if (bSaveAndValidate)
	{
		*m_pnValue=GetSelIndex();
	}
	else
	{
		SetSelIndex(*m_pnValue);
	}
}


// Add more strings to the control.  These are cycled when OnLeft() and OnRight() are called
void CLTGUICycleCtrl::AddString(const char *pString)
{
	
	if ( !pString || !m_pFont || !m_nFontSize || m_stringArray.size() >= kMaxNumStrings)
	{
		ASSERT(0);
		return;
	}



	int x = m_pos.x + (int)((float)m_nHeaderWidth * m_fScale);
	CUIFormattedPolyString *pPolyStr = g_pFontManager->CreateFormattedPolyString(m_pFont,(char *)pString,(float)x,(float)m_pos.y);


	if (pPolyStr)
	{

		pPolyStr->SetCharScreenHeight(m_nFontSize);

		m_stringArray.push_back(pPolyStr);

		//if this is the first string, select it
		if (m_nSelIndex == kNoSelection)
		{
			m_nSelIndex = 0;
			CalculateSize();
		}

	}
}

// Return a string at a specific index
CUIFormattedPolyString* CLTGUICycleCtrl::GetString(uint8 nIndex)
{
	if (nIndex > kMaxNumStrings || nIndex >= m_stringArray.size())
		return LTNULL;

	return m_stringArray.at(nIndex);
}

// Remove a string at a specific index
void CLTGUICycleCtrl::RemoveString(uint8 nIndex)
{
	if (nIndex > kMaxNumStrings || nIndex >= m_stringArray.size())
		return;
	g_pFontManager->DestroyPolyString(m_stringArray.at(nIndex));

	FPStringArray::iterator iter = m_stringArray.begin() + nIndex;
	m_stringArray.erase(iter);

	if (m_nSelIndex >= (int)m_stringArray.size())
	{
		m_nSelIndex = (int)m_stringArray.size()-1;
		CalculateSize();
	}
	else if (m_nSelIndex > nIndex)
	{
		m_nSelIndex--;
		CalculateSize();
	}
}

// Remove all strings
void CLTGUICycleCtrl::RemoveAll()
{
	FPStringArray::iterator iter = m_stringArray.begin();
	while ( iter != m_stringArray.end())
	{
		g_pFontManager->DestroyPolyString(*iter);
		iter++;
	}
	m_stringArray.clear();
	m_nSelIndex=kNoSelection;		

}

// Sets the currently selected index
void CLTGUICycleCtrl::SetSelIndex(uint8 nIndex)
{
	if (nIndex > kMaxNumStrings || nIndex >= m_stringArray.size())
		return;
	m_nSelIndex=nIndex;
	CalculateSize();
}

// Render the control
void CLTGUICycleCtrl::Render ()
{
	// Sanity checks...
	if (!IsVisible()) return;

    uint32 argbColor=GetCurrentColor();
	if (m_pHeaderText)
	{
		m_pHeaderText->SetColor(argbColor);
		m_pHeaderText->Render();
	}

	if (m_nSelIndex > m_stringArray.size() || m_stringArray.size() == 0)
	{
		return;
	}

	m_stringArray[m_nSelIndex]->SetColor(argbColor);
	m_stringArray[m_nSelIndex]->Render();

}


void CLTGUICycleCtrl::CalculateSize()
{
	if (!m_pFont ||  m_nSelIndex >= m_stringArray.size())
	{
		m_nWidth=0;
		m_nHeight=0;
	}
	else
	{
		m_nWidth = (uint16)m_stringArray[m_nSelIndex]->GetWidth();
		m_nHeight = (uint16)m_stringArray[m_nSelIndex]->GetHeight();
	}
	m_nWidth += (uint16)((float)m_nHeaderWidth * m_fScale);
	if (m_pHeaderText && m_pHeaderText->GetHeight() > m_nHeight)
	{
		m_nHeight = (uint16)m_pHeaderText->GetHeight();
	}
}


//this function sets up a notification when the control's value changes
void CLTGUICycleCtrl::NotifyOnChange(uint32 nCommandID,CLTGUICommandHandler *pCommandHandler, uint32 nParam1, uint32 nParam2)
{
	m_pCommandHandler	= pCommandHandler;
	m_nCommandID		= nCommandID;
	m_nParam1			= nParam1;
	m_nParam2			= nParam2;
}

// Set the font
LTBOOL CLTGUICycleCtrl::SetFont(CUIFont *pFont, uint8 nFontSize)
{
	if (!pFont && !nFontSize)
	{
        return LTFALSE;
	}

	LTBOOL bApply = LTFALSE;
	if (pFont && m_pFont != pFont)
	{
		m_pFont = pFont;
		bApply = LTTRUE;
	}
	if (nFontSize && m_nBaseFontSize != nFontSize)
	{
		m_nBaseFontSize = nFontSize;
		bApply = LTTRUE;
	}

	m_nFontSize = (uint8)(m_fScale * (float)m_nBaseFontSize);

	if (bApply)
	{
		for (uint8 i = 0; i < m_stringArray.size(); i++)
		{
			m_stringArray[i]->SetFont(m_pFont);
			m_stringArray[i]->SetCharScreenHeight(m_nFontSize);
		}
		if (m_pHeaderText)
		{
			m_pHeaderText->SetFont(m_pFont);
			m_pHeaderText->SetCharScreenHeight(m_nFontSize);
		}
		CalculateSize();
	}


    return LTTRUE;
}

// Left was pressed
LTBOOL CLTGUICycleCtrl::OnLeft ( )
{
	int oldSel = m_nSelIndex;
	if ( m_stringArray.size() <= 1 )
	{
        return LTFALSE;
	}

	uint8 newSel = m_nSelIndex;
	if ( newSel == 0 )
	{
		newSel=m_stringArray.size()-1;
	}
	else
		newSel--;

	if (newSel != m_nSelIndex)
	{
		SetSelIndex(newSel);
		CalculateSize();

	}
	if (m_pCommandHandler && m_nSelIndex != oldSel)
	{
		m_pCommandHandler->SendCommand(m_nCommandID, m_nParam1, m_nParam2);
	}
    return LTTRUE;
}

// Right was pressed
LTBOOL CLTGUICycleCtrl::OnRight ( )
{
	int oldSel = m_nSelIndex;
	if ( m_stringArray.size() <= 1 )
	{
        return LTFALSE;
	}

	uint8 newSel = m_nSelIndex;
	newSel++;
	if ( newSel >= m_stringArray.size() )
	{
		newSel = 0;
	}
	if (newSel != m_nSelIndex)
	{
		SetSelIndex(newSel);
		CalculateSize();

	}
	if (m_pCommandHandler && m_nSelIndex != oldSel)
	{
		m_pCommandHandler->SendCommand(m_nCommandID, m_nParam1, m_nParam2);
	}
    return LTTRUE;
}



void CLTGUICycleCtrl::SetBasePos ( LTIntPt pos )
{ 
	CLTGUICtrl::SetBasePos(pos);


	if (m_pHeaderText)
	{
		m_pHeaderText->SetPosition((float)m_pos.x,(float)m_pos.y);
	}
	float x = (float)m_pos.x + ((float)m_nHeaderWidth * m_fScale);
	for (uint8 i = 0; i < m_stringArray.size(); i++)
	{
		m_stringArray[i]->SetPosition(x,(float)m_pos.y);
	}
}

void CLTGUICycleCtrl::SetScale(float fScale)
{
	CLTGUICtrl::SetScale(fScale);
	m_nFontSize = (uint8)(m_fScale * (float)m_nBaseFontSize);

	if (m_pHeaderText)
	{
		m_pHeaderText->SetPosition((float)m_pos.x,(float)m_pos.y);
		m_pHeaderText->SetCharScreenHeight(m_nFontSize);
	}
	float x = (float)m_pos.x + ((float)m_nHeaderWidth * m_fScale);
	for (uint8 i = 0; i < m_stringArray.size(); i++)
	{
		m_stringArray[i]->SetPosition(x,(float)m_pos.y);
		m_stringArray[i]->SetCharScreenHeight(m_nFontSize);
	}
	CalculateSize();

}


