// ----------------------------------------------------------------------- //
//
// MODULE  : LTGUICycleCtrl.cpp
//
// PURPOSE : Control which can cycle through a set of strings based on 
//				user input.
//
// (c) 2001-2003 Monolith Productions, Inc.  All Rights Reserved
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
	m_nHeaderWidth	= 0;
	m_nSelIndex			= kNoSelection;
	m_pCallback = NULL;
}

CLTGUICycleCtrl::~CLTGUICycleCtrl()
{
	Destroy();
}

// Create the control
bool CLTGUICycleCtrl::Create ( const wchar_t *pHeader, const CFontInfo& Font, const CLTGUICycleCtrl_create& cs)
{

	if (!SetFont(Font))
	{
        return false;
	}

	if ( pHeader)
	{
		m_Header.SetText(pHeader);
	}

	m_nHeaderWidth = cs.nHeaderWidth;

	m_nSelIndex=kNoSelection;		
	m_pnValue = cs.pnValue;

	m_bGlowEnable = cs.bGlowEnable;
	m_fGlowAlpha = cs.fGlowAlpha;
	m_vGlowSize = cs.vGlowSize;
	m_Header.SetGlowParams(m_bGlowEnable,m_fGlowAlpha,m_vGlowSize);


	CLTGUICtrl::Create( (CLTGUICtrl_create)cs );

	SetBasePos(cs.rnBaseRect.m_vMin);

	return true;
}

// Destroys the control
void CLTGUICycleCtrl::Destroy ( )
{
	m_Header.FlushTexture();

	// Remove the strings
	RemoveAll();

}

// Update data
void CLTGUICycleCtrl::UpdateData (bool bSaveAndValidate)
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
void CLTGUICycleCtrl::AddString(const wchar_t *pString)
{
	
	if ( !pString || !m_Font.m_nHeight || m_StringArray.size() >= kMaxNumStrings)
	{
		ASSERT(0);
		return;
	}
	LTVector2 vPos = m_rfRect.m_vMin;
	float fi = (float)m_nHeaderWidth * m_vfScale.x;
	vPos.x += fi;

	CLTGUIString* pGUIString = debug_new(CLTGUIString);

	
	CFontInfo Font = m_Font;
	Font.m_nHeight = (uint32)(m_vfScale.y * (float)m_nBaseFontSize);
	pGUIString->SetFont(Font);
	pGUIString->SetText(pString);
	pGUIString->SetPos(vPos);
	pGUIString->SetGlowParams(m_bGlowEnable,m_fGlowAlpha,m_vGlowSize);

	m_StringArray.push_back(pGUIString);

	//if this is the first string, select it
	if (m_nSelIndex == kNoSelection)
	{
		m_nSelIndex = 0;
	}



}

// Return a string at a specific index
CLTGUIString* CLTGUICycleCtrl::GetString(uint8 nIndex)
{
	if (nIndex > kMaxNumStrings || nIndex >= m_StringArray.size())
		return NULL;

	return m_StringArray.at(nIndex);
}

// Remove a string at a specific index
void CLTGUICycleCtrl::RemoveString(uint8 nIndex)
{
	if (nIndex > kMaxNumStrings || nIndex >= m_StringArray.size())
		return;

	CLTGUIString* pString = GetString(nIndex);
	debug_delete(pString);

	LTGUIStringArray::iterator iter = m_StringArray.begin() + nIndex;
	m_StringArray.erase(iter);

	ASSERT( m_StringArray.size() <= 255 );
	if (m_nSelIndex >= (uint8)m_StringArray.size())
	{
		m_nSelIndex = (uint8)m_StringArray.size()-1;
	}
	else if (m_nSelIndex > nIndex)
	{
		m_nSelIndex--;
	}
}

// Remove all strings
void CLTGUICycleCtrl::RemoveAll()
{
	LTGUIStringArray::iterator iter = m_StringArray.begin();
	while ( iter != m_StringArray.end())
	{
		debug_delete(*iter);
		iter++;
	}
	m_StringArray.clear();
	m_nSelIndex=kNoSelection;		
}

// Sets the currently selected index
void CLTGUICycleCtrl::SetSelIndex(uint8 nIndex)
{
	if (nIndex > kMaxNumStrings || nIndex >= m_StringArray.size())
		return;
	m_nSelIndex=nIndex;
}

// Render the control
void CLTGUICycleCtrl::Render ()
{
	// Sanity checks...
	if (!IsVisible()) return;

    uint32 argbColor=GetCurrentColor();
	m_Header.SetColor(argbColor);
	m_Header.SetGlow(IsSelected());
	m_Header.Render();

	if (m_nSelIndex > m_StringArray.size() || m_StringArray.size() == 0)
	{
		return;
	}

	if (m_StringArray[m_nSelIndex])
	{
		m_StringArray[m_nSelIndex]->SetColor(argbColor);
		m_StringArray[m_nSelIndex]->SetGlow(IsSelected());
		m_StringArray[m_nSelIndex]->Render();
	}
}

// Render the control
void CLTGUICycleCtrl::RenderTransition(float fTrans)
{
	// Sanity checks...
	if (!IsVisible()) return;

	uint32 argbColor=GetCurrentColor();
	m_Header.SetColor(argbColor);
	m_Header.RenderTransition(fTrans);

	if (m_nSelIndex > m_StringArray.size() || m_StringArray.size() == 0)
	{
		return;
	}

	if (m_StringArray[m_nSelIndex])
	{
		m_StringArray[m_nSelIndex]->SetColor(argbColor);
		m_StringArray[m_nSelIndex]->RenderTransition(fTrans);
	}
}


// Set the font
bool CLTGUICycleCtrl::SetFont(const CFontInfo& Font)
{
	if (!Font.m_nHeight)
	{
		return false;
	}

	m_Font = Font;
	m_nBaseFontSize = Font.m_nHeight;
	m_Font.m_nHeight = (uint32)(m_vfScale.y * (float)m_nBaseFontSize);

	m_Header.SetFont(m_Font);

	LTGUIStringArray::iterator iter = m_StringArray.begin();
	while (iter != m_StringArray.end())
	{
		if (*iter)
		{
			(*iter)->SetFont(m_Font);
		}
		iter++;
	}
	return true;
}

// Set the font
bool CLTGUICycleCtrl::SetFontHeight(uint32 nFontHeight)
{
	if (!nFontHeight)
	{
		return false;
	}

	m_nBaseFontSize = nFontHeight;
	m_Font.m_nHeight = (uint32)(m_vfScale.y * (float)m_nBaseFontSize);

	m_Header.SetFontHeight(m_Font.m_nHeight);

	LTGUIStringArray::iterator iter = m_StringArray.begin();
	while (iter != m_StringArray.end())
	{
		if (*iter)
		{
			(*iter)->SetFontHeight(m_Font.m_nHeight);
		}
		iter++;
	}
	return true;
}


// Left was pressed
bool CLTGUICycleCtrl::OnLeft ( )
{
	int oldSel = m_nSelIndex;
	if ( m_StringArray.size() <= 1 )
	{
        return false;
	}

	uint8 newSel = m_nSelIndex;
	if (m_pCallback)
	{
		newSel = m_pCallback(this,m_nSelIndex,false);
	}
	else
	{
		if ( newSel == 0 )
		{
			ASSERT( m_StringArray.size()-1 <= 255 );
			newSel= ( uint8 )(m_StringArray.size()-1);
		}
		else
			newSel--;

	}

	if (newSel != m_nSelIndex)
	{
		SetSelIndex(newSel);
	}
	if (m_pCommandHandler && m_nSelIndex != oldSel)
	{
		m_pCommandHandler->SendCommand(m_nCommandID, m_nParam1, m_nParam2);
	}
    return true;
}

// Right was pressed
bool CLTGUICycleCtrl::OnRight ( )
{
	int oldSel = m_nSelIndex;
	if ( m_StringArray.size() <= 1 )
	{
        return false;
	}

	uint8 newSel = m_nSelIndex;
	if (m_pCallback)
	{
		newSel = m_pCallback(this,m_nSelIndex,true);
	}
	else
	{
		newSel++;
		if ( newSel >= m_StringArray.size() )
		{
			newSel = 0;
		}
	}
	if (newSel != m_nSelIndex)
	{
		SetSelIndex(newSel);
	}

	if (m_pCommandHandler && m_nSelIndex != oldSel)
	{
		m_pCommandHandler->SendCommand(m_nCommandID, m_nParam1, m_nParam2);
	}
    return true;
}



void CLTGUICycleCtrl::SetBasePos ( const LTVector2n& pos )
{ 
	CLTGUICtrl::SetBasePos(pos);

	LTVector2 vPos = m_rfRect.m_vMin;
	m_Header.SetPos(vPos);

	float fi = (float)m_nHeaderWidth * m_vfScale.x;
	vPos.x += fi;

	LTGUIStringArray::iterator iter = m_StringArray.begin();
	while (iter != m_StringArray.end())
	{
		if (*iter)
		{
			(*iter)->SetFontHeight((uint32)(m_vfScale.y * (float)m_nBaseFontSize));
			(*iter)->SetPos(vPos);
		}
		iter++;
	}

}

void CLTGUICycleCtrl::SetScale(const LTVector2& vfScale )
{
	if (vfScale == m_vfScale)
		return;

	CLTGUICtrl::SetScale(vfScale);


	m_Header.SetFontHeight((uint32)(m_vfScale.y * (float)m_nBaseFontSize));

	LTVector2 vPos = m_rfRect.m_vMin;
	m_Header.SetPos(vPos);

	float fi = (float)m_nHeaderWidth * m_vfScale.x;
	vPos.x += fi;

	LTGUIStringArray::iterator iter = m_StringArray.begin();
	while (iter != m_StringArray.end())
	{
		if (*iter)
		{
			(*iter)->SetFontHeight((uint32)(m_vfScale.y * (float)m_nBaseFontSize));
			(*iter)->SetPos(vPos);
		}
		iter++;
	}

}


// free texture memory by flushing any texture strings owned by the control
void CLTGUICycleCtrl::FlushTextureStrings()
{
	m_Header.FlushTexture();

	LTGUIStringArray::iterator iter = m_StringArray.begin();
	while (iter != m_StringArray.end())
	{
		if (*iter)
		{
			(*iter)->FlushTexture();
		}
		iter++;
	}


}

// rebuild any texture strings owned by the control
void CLTGUICycleCtrl::RecreateTextureStrings()
{
	if ( m_Font.m_nHeight == 0)
		return;

	m_Header.CreateTexture();

	LTGUIStringArray::iterator iter = m_StringArray.begin();
	while (iter != m_StringArray.end())
	{
		if (*iter)
		{
			(*iter)->CreateTexture();
		}
		iter++;
	}



}