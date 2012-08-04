// ----------------------------------------------------------------------- //
//
// MODULE  : MissionObjectives.cpp
//
// PURPOSE : Riot's Mission Objective system - Implementation
//
// CREATED : 3/22/98
//
// ----------------------------------------------------------------------- //

#include "RiotClientShell.h"
#include "iltclient.h"
#include "TextHelper.h"
#include "ClientRes.h"
#include "MissionObjectives.h"
#include "ClientRes.h"

#define TEXT_AREA_WIDTH 186
#define OPEN_ANIM_RATE 1000
#define X_LOCATION 25
#define Y_LOCATION 127
#define SCROLL_SPEED 200

CMissionObjectives::CMissionObjectives()
{
	m_pClientDE = LTNULL;
	m_pClientShell = LTNULL;
	m_pObjectives = LTNULL;
	m_pTopObjective = LTNULL;
	m_hDisplay = LTNULL;
	m_hSeparator = LTNULL;
	m_cxSeparator = 0;
	m_cySeparator = 0;
	m_bScrollable = LTFALSE;

	m_bOpenAnimating = LTFALSE;
	m_bCloseAnimating = LTFALSE;
	m_rcTop.left = m_rcTop.top = m_rcTop.right = m_rcTop.bottom = 0;
	m_rcBottom.left = m_rcBottom.top = m_rcBottom.right = m_rcBottom.bottom = 0;

	m_bScrollingUp = LTFALSE;
	m_bScrollingDown = LTFALSE;
	m_fScrollOffset = 0.0f;
	m_fScrollOffsetTarget = 0.0f;
}

CMissionObjectives::~CMissionObjectives()
{
	OBJECTIVE* pObjective = m_pObjectives;
	while (pObjective)
	{
		if (pObjective->hSurface && m_pClientDE) m_pClientDE->DeleteSurface (pObjective->hSurface);

		OBJECTIVE* pNext = pObjective->pNext;
		delete pObjective;
		pObjective = pNext;
	}

	if (m_hDisplay) m_pClientDE->DeleteSurface (m_hDisplay);
	if (m_hSeparator) m_pClientDE->DeleteSurface (m_hSeparator);
}

void CMissionObjectives::Init (ILTClient* pClientDE, CRiotClientShell* pClientShell)
{
	m_pClientDE = pClientDE;
	m_pClientShell = pClientShell;

	// create display surface

	m_hDisplay = m_pClientDE->CreateSurfaceFromBitmap ("interface/MissionLog.pcx");
	if (!m_hDisplay) return;

	HLTCOLOR hTransColor = m_pClientDE->SetupColor2(0.0f, 0.0f, 0.0f, LTTRUE);

	// load separator

	m_hSeparator = m_pClientDE->CreateSurfaceFromBitmap ("interface/MissionLogSeparator.pcx");
	if (m_hSeparator)
	{
		m_pClientDE->GetSurfaceDims (m_hSeparator, &m_cxSeparator, &m_cySeparator);
//		m_pClientDE->OptimizeSurface (m_hSeparator, hTransColor);
	}

	// draw mission log title on surface

	HSTRING hstrFont = m_pClientDE->FormatString (IDS_INGAMEFONT);
	FONT fontdef (const_cast<char *>(m_pClientDE->GetStringData(hstrFont)), 
				  TextHelperGetIntValFromStringID(m_pClientDE, IDS_MISSIONOBJECTIVETITLEWIDTH, 10),
				  TextHelperGetIntValFromStringID(m_pClientDE, IDS_MISSIONOBJECTIVETITLEHEIGHT, 26),
				  LTFALSE, LTFALSE, LTTRUE);
	m_pClientDE->FreeString (hstrFont);

	HLTCOLOR foreColor = m_pClientDE->SetupColor1 (0.98f, 0.317647f, 0.0f, LTFALSE);
	HSURFACE hStringSurface = CTextHelper::CreateSurfaceFromString (m_pClientDE, &fontdef, IDS_MISSIONLOG, foreColor);
	if (hStringSurface)
	{
		hStringSurface = CropSurface (hStringSurface, LTNULL);

		uint32 nWidth = 0;
		uint32 nHeight = 0;
		m_pClientDE->GetSurfaceDims (hStringSurface, &nWidth, &nHeight);

		int x = 35 + ((145 - (int)nWidth) / 2);
		int y = 11 + ((19 - (int)nHeight) / 2);
		m_pClientDE->DrawSurfaceToSurfaceTransparent (m_hDisplay, hStringSurface, LTNULL, x, y, LTNULL);

		m_pClientDE->DeleteSurface (hStringSurface);
		hStringSurface = LTNULL;
	}

	// draw "primary objectives:" on surface

	fontdef.nWidth = 6;
	fontdef.nHeight = 13;
	fontdef.bBold = LTFALSE;
	hStringSurface = CTextHelper::CreateSurfaceFromString (m_pClientDE, &fontdef, IDS_PRIMARY_OBJECTIVES, foreColor);
	if (hStringSurface)
	{
		hStringSurface = CropSurface (hStringSurface, LTNULL);

		uint32 nWidth = 0;
		uint32 nHeight = 0;
		m_pClientDE->GetSurfaceDims (hStringSurface, &nWidth, &nHeight);

		int x = 13;
		int y = 59 + ((10 - (int)nHeight) / 2);
		m_pClientDE->DrawSurfaceToSurfaceTransparent (m_hDisplay, hStringSurface, LTNULL, x, y, LTNULL);

		m_pClientDE->DeleteSurface (hStringSurface);
		hStringSurface = LTNULL;
	}

//	m_pClientDE->OptimizeSurface (m_hDisplay, hTransColor);
}

void CMissionObjectives::AddObjective (uint32 nID, LTBOOL bCompleted)
{
	if (!m_pClientDE) return;

	// create the surface from the string

	HSTRING hstrFont = m_pClientDE->FormatString (IDS_INGAMEFONT);
	FONT fontdef (const_cast<char *>(m_pClientDE->GetStringData (hstrFont)), 
				  TextHelperGetIntValFromStringID(m_pClientDE, IDS_MISSIONOBJECTIVETEXTWIDTH, 6),
				  TextHelperGetIntValFromStringID(m_pClientDE, IDS_MISSIONOBJECTIVETEXTHEIGHT, 13));

	m_pClientDE->FreeString (hstrFont);

	HLTCOLOR foreColor = m_pClientDE->SetupColor1 (1.0f, 1.0f, 1.0f, LTFALSE);
	HSURFACE hSurf = CTextHelper::CreateWrappedStringSurface (m_pClientDE, TEXT_AREA_WIDTH, &fontdef, nID, foreColor);
	
	if (!hSurf) return;

	hSurf = CropSurface (hSurf, LTNULL);

//	HLTCOLOR hTransColor = m_pClientDE->SetupColor1(0.0f, 0.0f, 0.0f, LTTRUE);

//	m_pClientDE->OptimizeSurface (hSurf, hTransColor);
	
	// create a new objective

	OBJECTIVE* pNew = new OBJECTIVE;
	if (!pNew) return;

	pNew->hSurface	 = hSurf;
	pNew->nID		 = nID;
	pNew->bCompleted = bCompleted;

	// add it to the list

	if (!m_pObjectives)
	{
		m_pObjectives = pNew;
	}
	else
	{
		pNew->pNext = m_pObjectives;
		m_pObjectives->pPrev = pNew;
		m_pObjectives = pNew;
	}

	m_pTopObjective = m_pObjectives;
}

void CMissionObjectives::RemoveObjective (uint32 nID)
{
	if (!m_pClientDE) return;

	// find the objective

	OBJECTIVE* pObjective = m_pObjectives;
	while (pObjective)
	{
		if (pObjective->nID == nID) break;
		pObjective = pObjective->pNext;
	}
	if (!pObjective) return;

	// now remove it from the list

	if (pObjective->hSurface) m_pClientDE->DeleteSurface (pObjective->hSurface);
	if (pObjective->pPrev) pObjective->pPrev->pNext = pObjective->pNext;
	if (pObjective->pNext) pObjective->pNext->pPrev = pObjective->pPrev;
	if (m_pObjectives == pObjective) m_pObjectives = pObjective->pNext;

	delete pObjective;
	
	m_pTopObjective = m_pObjectives;
}

void CMissionObjectives::CompleteObjective (uint32 nID)
{
	if (!m_pClientDE) return;

	// find the objective

	OBJECTIVE* pObjective = m_pObjectives;
	while (pObjective)
	{
		if (pObjective->nID == nID) break;
		pObjective = pObjective->pNext;
	}
	if (!pObjective) return;

	// mark it as completed

	pObjective->bCompleted = LTTRUE;
}

void CMissionObjectives::ScrollUp()
{
	if (!m_pClientDE || m_bScrollingUp) return;

	if (m_pTopObjective && m_pTopObjective->pPrev) 
	{
		m_bScrollingUp = LTTRUE;

		uint32 nWidth = 0;
		uint32 nHeight = 0;
		m_pClientDE->GetSurfaceDims (m_pTopObjective->pPrev->hSurface, &nWidth, &nHeight);
		
		m_fScrollOffset = (LTFLOAT) (-((int)nHeight) - (int)m_cySeparator);
		m_fScrollOffsetTarget = 0.0f;
		
		m_pTopObjective = m_pTopObjective->pPrev;
	}

}

void CMissionObjectives::ScrollDown()
{
	if (!m_pClientDE || m_bScrollingDown) return;

	if (m_pTopObjective && m_pTopObjective->pNext)
	{
		m_bScrollingDown = LTTRUE;
		
		uint32 nWidth = 0;
		uint32 nHeight = 0;
		m_pClientDE->GetSurfaceDims (m_pTopObjective->hSurface, &nWidth, &nHeight);
		
		m_fScrollOffset = 0;
		m_fScrollOffsetTarget = (LTFLOAT) (-((int)nHeight) - (int)m_cySeparator);
	}
}

void CMissionObjectives::Reset()
{
	OBJECTIVE* pObjective = m_pObjectives;
	while (pObjective)
	{
		if (pObjective->hSurface && m_pClientDE) m_pClientDE->DeleteSurface (pObjective->hSurface);

		OBJECTIVE* pNext = pObjective->pNext;
		delete pObjective;
		pObjective = pNext;
	}

	m_pObjectives = LTNULL;
	m_pTopObjective = LTNULL;
	m_bScrollable = LTFALSE;
}

void CMissionObjectives::ResetTop()
{
	m_pTopObjective = m_pObjectives;
}

void CMissionObjectives::SetLevelName (char* strLevelName)
{
}

void CMissionObjectives::StartOpenAnimation()
{
	m_bOpenAnimating = LTTRUE;
	
	m_rcTop.left = 0;
	m_rcTop.top = 0;
	m_rcTop.right = 217;
	m_rcTop.bottom = 53;

	m_rcBottom.left = 0;
	m_rcBottom.top = 185;
	m_rcBottom.right = 217;
	m_rcBottom.bottom = 200;
}

void CMissionObjectives::StartCloseAnimation()
{
	m_bCloseAnimating = LTTRUE;

	m_rcTop.left = 0;
	m_rcTop.top = 0;
	m_rcTop.right = 217;
	m_rcTop.bottom = 119;

	m_rcBottom.left = 0;
	m_rcBottom.top = 119;
	m_rcBottom.right = 217;
	m_rcBottom.bottom = 200;
}

void CMissionObjectives::Draw()
{
	if (!m_hDisplay || !m_pClientDE || !m_pClientShell) return;

	HSURFACE hScreen = m_pClientDE->GetScreenSurface();
	
	uint32 nScreenWidth, nScreenHeight;
	m_pClientDE->GetSurfaceDims (hScreen, &nScreenWidth, &nScreenHeight);
	
	uint32 nMOWidth, nMOHeight;
	m_pClientDE->GetSurfaceDims (m_hDisplay, &nMOWidth, &nMOHeight);

	int nOriginX = (int) ((LTFLOAT)X_LOCATION * (((LTFLOAT)nScreenWidth - (LTFLOAT)nMOWidth) / (640.0f - (LTFLOAT)nMOWidth)));
	int nOriginY = (int) ((LTFLOAT)Y_LOCATION * (((LTFLOAT)nScreenHeight - (LTFLOAT)nMOHeight) / (480.0f - (LTFLOAT)nMOHeight)));;

	// if we're doing the opening animation, draw it and increment the rectangles, then return

	if (m_bOpenAnimating)
	{
		LTFLOAT nFrameTime = m_pClientDE->GetFrameTime();

		int y = nOriginY + 119 - (m_rcTop.bottom - m_rcTop.top);
		m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_hDisplay, &m_rcTop, nOriginX, y, LTNULL);

		y = nOriginY + 119;
		m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_hDisplay, &m_rcBottom, nOriginX, y, LTNULL);

		m_rcTop.bottom += (int)(nFrameTime * (LTFLOAT)OPEN_ANIM_RATE);
		m_rcBottom.top -= (int)(nFrameTime * (LTFLOAT)OPEN_ANIM_RATE);

		if (m_rcBottom.top <= m_rcTop.bottom) m_bOpenAnimating = LTFALSE;
		return;
	}

	// if we're doing the closing animation, draw it and increment the rectangles, then return

	if (m_bCloseAnimating)
	{
		m_pClientShell->AddToClearScreenCount();

		LTFLOAT nFrameTime = m_pClientDE->GetFrameTime();

		int y = nOriginY + 119 - (m_rcTop.bottom - m_rcTop.top);
		m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_hDisplay, &m_rcTop, nOriginX, y, LTNULL);

		y = nOriginY + 119;
		m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_hDisplay, &m_rcBottom, nOriginX, y, LTNULL);

		m_rcTop.bottom -= (int)(nFrameTime * (LTFLOAT)OPEN_ANIM_RATE);
		m_rcBottom.top += (int)(nFrameTime * (LTFLOAT)OPEN_ANIM_RATE);

		if (m_rcBottom.top >= 185) m_bCloseAnimating = LTFALSE;
		return;
	}

	// set the initial coordinates

	int x = 15 + nOriginX;
	int y = 76 + nOriginY;
	
	// adjust the coordinates if we are scrolling up or down

	if (m_bScrollingUp)
	{
		LTFLOAT nFrameTime = m_pClientDE->GetFrameTime();

		m_fScrollOffset += nFrameTime * SCROLL_SPEED;
		if (m_fScrollOffset > m_fScrollOffsetTarget)
		{
			m_fScrollOffset = 0.0f;
			m_fScrollOffsetTarget = 0.0f;
			m_bScrollingUp = LTFALSE;
		}

		y += (int)m_fScrollOffset;
	}
	else if (m_bScrollingDown)
	{
		LTFLOAT nFrameTime = m_pClientDE->GetFrameTime();

		m_fScrollOffset -= nFrameTime * SCROLL_SPEED;
		if (m_fScrollOffset < m_fScrollOffsetTarget)
		{
			m_fScrollOffset = 0.0f;
			m_fScrollOffsetTarget = 0.0f;
			m_bScrollingDown = LTFALSE;

			m_pTopObjective = m_pTopObjective->pNext;
		}

		y += (int)m_fScrollOffset;
	}

	// first draw the display to the screen

	m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_hDisplay, LTNULL, nOriginX, nOriginY, LTNULL);

	// now draw the objectives to the screen

	OBJECTIVE* pObjective = m_pTopObjective;
	while (pObjective && y < 188 + nOriginY)
	{
		uint32 nWidth, nHeight;
		m_pClientDE->GetSurfaceDims (pObjective->hSurface, &nWidth, &nHeight);

		if (y < 74 + nOriginY)
		{
			LTRect rcSrc;
			rcSrc.left = 0;
			rcSrc.top = (74 + nOriginY) - y;
			rcSrc.right = nWidth;
			rcSrc.bottom = nHeight;

			if (rcSrc.bottom >= rcSrc.top)
			{
				DrawObjective (hScreen, pObjective, &rcSrc, x, 74 + nOriginY);
			}
		}
		else if (y + (int)nHeight > 188 + nOriginY)
		{
			LTRect rcSrc;
			rcSrc.left = 0;
			rcSrc.top = 0;
			rcSrc.right = nWidth;
			rcSrc.bottom = nHeight - ((y + nHeight) - (188 + nOriginY));

			DrawObjective (hScreen, pObjective, &rcSrc, x, y);
		}
		else
		{
			DrawObjective (hScreen, pObjective, LTNULL, x, y);
		}

		y += nHeight;

		if (pObjective->pNext && y < 188 + nOriginY)
		{
			if (y < 74 + nOriginY)
			{
				LTRect rcSrc;
				rcSrc.left = 0;
				rcSrc.top = (74 + nOriginY) - y;
				rcSrc.right = m_cxSeparator;
				rcSrc.bottom = m_cySeparator;

				if (rcSrc.bottom >= rcSrc.top)
				{
					m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_hSeparator, &rcSrc, 51, 74 + nOriginY, LTNULL);
				}
			}
			else if (y + (int)m_cySeparator > 188 + nOriginY)
			{
				LTRect rcSrc;
				rcSrc.left = 0;
				rcSrc.top = 0;
				rcSrc.right = m_cxSeparator;
				rcSrc.bottom = m_cySeparator - ((y + m_cySeparator) - (188 + nOriginY));

				m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_hSeparator, &rcSrc, 51, y, LTNULL);
			}
			else
			{
				m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_hSeparator, LTNULL, 51, y, LTNULL);
			}

			y += m_cySeparator;
		}

		pObjective = pObjective->pNext;
	}

	// see if the text should be scrollable

	if (y > 188 + nOriginY) 
	{
		m_bScrollable = LTTRUE;
	}
	else
	{
		m_bScrollable = LTFALSE;
	}
}

void CMissionObjectives::DrawObjective (HSURFACE hScreen, OBJECTIVE* pObjective, LTRect* rcSrc, int x, int y)
{
	if (!m_pClientDE || !hScreen || !pObjective) return;

	if (!pObjective->bCompleted)
	{
		m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, pObjective->hSurface, rcSrc, x, y, LTNULL);
	}
	else
	{
		HLTCOLOR hFillColor = m_pClientDE->SetupColor1 (0.4f, 0.4f, 0.4f, LTFALSE);
		m_pClientDE->DrawSurfaceSolidColor (hScreen, pObjective->hSurface, rcSrc, x, y, LTNULL, hFillColor);
	}
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionObjectives::Save
//
//	PURPOSE:	Save the mission objectives info
//
// --------------------------------------------------------------------------- //

void CMissionObjectives::Save(ILTMessage_Write* hWrite)
{
	if (!m_pClientDE) return;

	uint8 nNumObjectives = 0;

	OBJECTIVE* pObjective = m_pObjectives;
	while (pObjective)
	{
		nNumObjectives++;
		pObjective = pObjective->pNext;
	}	
	
	hWrite->Writeuint8(nNumObjectives);

	pObjective = m_pObjectives;
	while (pObjective)
	{
		hWrite->Writeuint32(pObjective->nID);
		hWrite->Writeuint8(pObjective->bCompleted);

		pObjective = pObjective->pNext;
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CMissionObjectives::Load
//
//	PURPOSE:	Load the mission objectives info
//
// --------------------------------------------------------------------------- //

void CMissionObjectives::Load(ILTMessage_Read* hRead)
{
	if (!m_pClientDE) return;

	uint8 nNumObjectives = hRead->Readuint8();

	uint32 dwId;
	LTBOOL  bCompleted;

	for (int i=0; i < nNumObjectives; i++)
	{
		dwId		= hRead->Readuint32();
		bCompleted	= (LTBOOL) hRead->Readuint8();

		AddObjective(dwId, bCompleted);
	}
}