// StaticTextCtrl.cpp: implementation of the CStaticTextCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ltguimgr.h"
#include "StaticTextCtrl.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CStaticTextCtrl::CStaticTextCtrl()
{
    m_pFont             = LTNULL;
    m_pCommandHandler   = LTNULL;
	m_dwWidth			= 0;
	m_dwHeight			= 0;
    m_bFixedWidth       = LTFALSE;
    m_bFixedHeight      = LTFALSE;
    m_hString           = LTNULL;
    m_hSurface			= LTNULL;
}

CStaticTextCtrl::~CStaticTextCtrl()
{
	Destroy();
}

// Create the control
LTBOOL CStaticTextCtrl::Create ( ILTClient *pClientDE,
                                uint32 dwCommandID,
								HSTRING hText,
								CLTGUIFont *pFont,
								CLTGUICommandHandler *pCommandHandler,
                                uint32 dwWidth, uint32 dwHeight,
                                uint32 dwParam1, uint32 dwParam2)
{
	m_pClientDE			= pClientDE;
	m_pCommandHandler	= pCommandHandler;


	if (!SetFont(pFont))
	{
        return LTFALSE;
	}

	// Add the string
	if (hText)
	{
		SetString(hText);
	}

	if (dwWidth)
	{
        m_bFixedWidth = LTTRUE;
		m_dwWidth = dwWidth;
	}

	if (dwHeight)
	{
        m_bFixedHeight = LTTRUE;
		m_dwHeight = dwHeight;
	}

	CLTGUICtrl::Create(dwCommandID,dwParam1,dwParam2);

    return LTTRUE;
}

// Destroys the control
void CStaticTextCtrl::Destroy ( )
{
	// Remove the string
	RemoveString();
	//clear the surfaces
	Purge();

}

//clear the surfaces
void CStaticTextCtrl::Purge()
{
	if (m_hSurface)
	{
		m_pClientDE->DeleteSurface(m_hSurface);
           m_hSurface = LTNULL;
	}
}


void CStaticTextCtrl::SetString(HSTRING hString)
{
	// Copy the text into a new buffer
	if ( hString != NULL && m_pClientDE)
	{
		if (m_hString)
			RemoveString();
		m_hString = m_pClientDE->CopyString(hString);

		if (m_hSurface)
			CreateSurface();
	}
}

void CStaticTextCtrl::SetString(int messageCode)
{
	if (m_pClientDE)
	{
		HSTRING hString=m_pClientDE->FormatString(messageCode);
		SetString(hString);
		m_pClientDE->FreeString(hString);
	}
}

void CStaticTextCtrl::SetString(char *pszString)
{
	if (m_pClientDE)
	{
		HSTRING hString=m_pClientDE->CreateString(pszString);
		SetString(hString);
		m_pClientDE->FreeString(hString);
	}
}

// Return a string at a specific index
HSTRING CStaticTextCtrl::GetString()
{
	return m_hString;
}

// Remove a string at a specific index
void CStaticTextCtrl::RemoveString()
{
	if (m_pClientDE)
	{
		m_pClientDE->FreeString(m_hString);
        m_hString = LTNULL;
	}
	Purge();
}


// Set the font
LTBOOL CStaticTextCtrl::SetFont(CLTGUIFont *pFont)
{
    if ( pFont == LTNULL)
	{
        return LTFALSE;
	}

	m_pFont = pFont;

	if (m_hSurface)
		CreateSurface();

    return LTTRUE;
}

// Width/Height calculations
int CStaticTextCtrl::GetWidth ( )
{
	if (!m_hSurface)
		CreateSurface();
	return m_dwWidth;
}
int CStaticTextCtrl::GetHeight ( )
{
	if (!m_hSurface)
		CreateSurface();
	return m_dwHeight;
}


void CStaticTextCtrl::CalculateSize()
{
	assert(m_pFont);
	if (!m_pFont || !m_hString)
	{
		if (!m_bFixedWidth)
			m_dwWidth=0;
		m_dwHeight=0;
		return;
	}

    LTIntPt size;
	if (m_bFixedWidth)
		size=m_pFont->GetTextExtentsFormat(m_hString, m_dwWidth);
	else
	{
		size=m_pFont->GetTextExtents(m_hString);
		m_dwWidth=size.x;
	}

	if (!m_bFixedHeight)
		m_dwHeight=size.y;
}


// Render the control
void CStaticTextCtrl::Render ( HSURFACE hDestSurf )
{
	if (m_hString && !m_hSurface)
		CreateSurface();

	HLTCOLOR color = GetCurrentColor();
	if (m_hSurface)
	{
		if (m_pFont->IsChromaKeyed())
			g_pLTClient->DrawSurfaceToSurfaceTransparent(hDestSurf, m_hSurface, LTNULL, m_pos.x, m_pos.y, m_hTransColor);
		else
		{
			LTSurfaceBlend  oldBlend = LTSURFACEBLEND_ALPHA;
			HLTCOLOR oldColor = kWhite;
//			m_pClientDE->GetOptimized2DBlend(oldBlend);
//			m_pClientDE->GetOptimized2DColor(oldColor);

			if (color == kWhite)
			{
				m_pClientDE->SetOptimized2DBlend(LTSURFACEBLEND_ADD);
				g_pLTClient->DrawSurfaceToSurface(hDestSurf, m_hSurface, LTNULL, m_pos.x, m_pos.y);
			}
			else if (color == kBlack)
			{
				m_pClientDE->SetOptimized2DBlend(LTSURFACEBLEND_MASK);
				g_pLTClient->DrawSurfaceToSurface(hDestSurf, m_hSurface, LTNULL, m_pos.x, m_pos.y);
			}
			else
			{
				m_pClientDE->SetOptimized2DBlend(LTSURFACEBLEND_MASK);
				g_pLTClient->DrawSurfaceToSurface(hDestSurf, m_hSurface, LTNULL, m_pos.x, m_pos.y);
				m_pClientDE->SetOptimized2DBlend(LTSURFACEBLEND_ADD);
				m_pClientDE->SetOptimized2DColor(color);
				g_pLTClient->DrawSurfaceToSurface(hDestSurf, m_hSurface, LTNULL, m_pos.x, m_pos.y);
			}
			m_pClientDE->SetOptimized2DBlend(oldBlend);
			m_pClientDE->SetOptimized2DColor(oldColor);
		}
			
	}

}


void CStaticTextCtrl::CreateSurface()
{
	CalculateSize();
    HLTCOLOR backcolor = kBlack;
	if (m_pFont->IsChromaKeyed())
		backcolor = m_hTransColor;

	//if I have a surface get it's dims
	uint32 dwWidth = 0;
	uint32 dwHeight = 0;
	if (m_hSurface)
		m_pClientDE->GetSurfaceDims(m_hSurface, &dwWidth, &dwHeight);

	//if the surface dims are less than the text extents, create a new surface
	if (dwWidth < m_dwWidth || dwHeight < m_dwHeight)
	{
		dwWidth = Max(dwWidth,m_dwWidth);
		dwHeight = Max(dwHeight,m_dwHeight);
		if (m_hSurface)
			m_pClientDE->DeleteSurface(m_hSurface);
		m_hSurface = m_pClientDE->CreateSurface(dwWidth,dwHeight);

		//if I have a surface...
		if (m_hSurface)
		{
			//clear the surface
            LTRect rcSrc;
			rcSrc.left = rcSrc.top = 0;
			rcSrc.right = dwWidth;
			rcSrc.bottom = dwHeight;
			m_pClientDE->FillRect(m_hSurface, &rcSrc, backcolor);

			// and draw the text on it
			if (m_bFixedWidth)
			{
				m_pFont->DrawFormat(m_hString, m_hSurface, 0, 0, m_dwWidth, kWhite);
			}
			else
			{
				m_pFont->Draw(m_hString, m_hSurface, 0, 0, LTF_JUSTIFY_LEFT, kWhite);
			}
			m_pClientDE->OptimizeSurface(m_hSurface, backcolor);

		}
	}
}

// Enter was pressed
LTBOOL CStaticTextCtrl::OnEnter ( )
{
	// Send the command
	if ( m_pCommandHandler )
	{
		if	(m_pCommandHandler->SendCommand(m_dwCommandID, m_dwParam1, m_dwParam2))
            return LTTRUE;
	}
    return LTFALSE;
}


// The selection for this control has changed
void CStaticTextCtrl::OnSelChange()
{
	// Start the "timer" if we are unselected now
	if ( !IsSelected() )
	{
		StartFade();
	}
}