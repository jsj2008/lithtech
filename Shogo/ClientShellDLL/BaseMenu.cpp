#include "clientheaders.h"
#include "BaseMenu.h"
#include "RiotSoundTypes.h"
#include "RiotMenu.h"

#define TITLE_SPACING_MULTIPLIER	3

CBaseMenu::CBaseMenu()
{
	m_pClientDE = LTNULL;
	m_pRiotMenu = LTNULL;
	m_pParent = LTNULL;
	m_nSelection = 0;
	m_nTopItem = 0;
	m_hMenuTitle = LTNULL;
	m_nMenuTitleSpacing = 0;
	m_nMenuX = 0;
	m_nMenuY = 0;
	m_nMenuSpacing = 0;
	m_nGenericItems = 0;
}

CBaseMenu::~CBaseMenu()
{
	if (!m_pClientDE) return;
	UnloadSurfaces();
}

LTBOOL CBaseMenu::Init (ILTClient* pClientDE, CRiotMenu* pRiotMenu, CBaseMenu* pParent, int nScreenWidth, int nScreenHeight)
{
	if (!pClientDE || !pRiotMenu)
	{
		return LTFALSE;
	}

	m_pClientDE = pClientDE;
	m_pRiotMenu = pRiotMenu;
	m_pParent = pParent;

	m_szScreen.cx = nScreenWidth;
	m_szScreen.cy = nScreenHeight;

	m_szMenuArea.cx = m_szScreen.cx - 10;
	m_szMenuArea.cy = m_szScreen.cy - 20;

	return LTTRUE;
}

void CBaseMenu::ScreenDimsChanged (int nScreenWidth, int nScreenHeight)
{
	m_szScreen.cx = nScreenWidth;
	m_szScreen.cy = nScreenHeight;

	m_szMenuArea.cx = m_szScreen.cx - 10;
	m_szMenuArea.cy = m_szScreen.cy - 20;

	UnloadSurfaces();
	LoadSurfaces();			// this should cause CalculateMenuDims() to be called...
}

void CBaseMenu::OnEnterWorld()
{
	CalculateMenuDims();
}

void CBaseMenu::OnExitWorld()
{
	CalculateMenuDims();
}

void CBaseMenu::Up()
{
	if (m_nGenericItems)
	{
		for (int i = 0; i <= MAX_GENERIC_ITEMS; i++)
		{
			m_nSelection--;
			if (m_nSelection < 0) m_nSelection = MAX_GENERIC_ITEMS - 1;
			if (m_GenericItem[m_nSelection].hMenuItem) break;
		}
	}

	CheckSelectionOffMenuTop();
	CheckSelectionOffMenuBottom();		// if we allow wrap-around scrolling

	PlayUpSound();
}

void CBaseMenu::Down()
{
	if (m_nGenericItems)
	{
		for (int i = 0; i <= MAX_GENERIC_ITEMS; i++)
		{
			m_nSelection++;
			if (m_nSelection == MAX_GENERIC_ITEMS) m_nSelection = 0;
			if (m_GenericItem[m_nSelection].hMenuItem) break;
		}
	}

	CheckSelectionOffMenuBottom();
	CheckSelectionOffMenuTop();			// if we around wrap-around scrolling

	PlayDownSound();
}

void CBaseMenu::Right()
{
	PlayRightSound();
}

void CBaseMenu::Left()
{
	PlayLeftSound();
}

void CBaseMenu::PageUp()
{
	return;
	PlayPageUpSound();
}

void CBaseMenu::PageDown()
{
	return;
	PlayPageDownSound();
}

void CBaseMenu::Home()
{
	m_nSelection = 0;
	CheckSelectionOffMenuTop();
	PlayHomeSound();
}

void CBaseMenu::End()
{
	m_nSelection = m_nGenericItems - 1;
	CheckSelectionOffMenuBottom();
	PlayEndSound();
}

void CBaseMenu::Return()
{
	PlayReturnSound();
}

void CBaseMenu::Esc()
{
	if (!m_pClientDE || !m_pRiotMenu) return;

	CBaseMenu* pCurrentMenu = m_pRiotMenu->GetCurrentMenu();
	if (!pCurrentMenu) return;

	if (pCurrentMenu->m_pParent)
	{
		m_pRiotMenu->SetCurrentMenu (pCurrentMenu->m_pParent);
	}

	PlayEscSound();
}

void CBaseMenu::Draw (HSURFACE hScreen, int nScreenWidth, int nScreenHeight, int nTextOffset)
{
	if (!m_pClientDE || !m_pRiotMenu || !hScreen) return;

	// first draw the menu title if there is one

	int nCurrentY = m_nMenuY;
	if (m_hMenuTitle)
	{
		m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_hMenuTitle, LTNULL, m_nMenuX, nCurrentY, LTNULL);
		nCurrentY += m_szMenuTitle.cy + m_nMenuTitleSpacing;
	}

	if (m_nTopItem > 0)
	{
		m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_pRiotMenu->GetUpArrow(), LTNULL, m_nMenuX + 2, nCurrentY - m_pRiotMenu->GetArrowHeight() - 3, LTNULL);
	}

	LTBOOL bDrawDownArrow = LTFALSE;
	for (int i = m_nTopItem; i < MAX_GENERIC_ITEMS; i++)
	{
		if (m_GenericItem[i].hMenuItem)
		{
			m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, i == m_nSelection ? m_GenericItem[i].hMenuItemSelected : m_GenericItem[i].hMenuItem, LTNULL, m_nMenuX, nCurrentY, LTNULL);
			nCurrentY += m_GenericItem[i].szMenuItem.cy + m_nMenuSpacing;
			if (nCurrentY > GetMenuAreaBottom() - (int)m_GenericItem[i].szMenuItem.cy) 
			{
				if (i < m_nGenericItems - 1)
				{
					bDrawDownArrow = LTTRUE;
				}
				break;
			}
		}
	}

	if (bDrawDownArrow)
	{
		m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_pRiotMenu->GetDownArrow(), LTNULL, m_nMenuX + 2, nCurrentY, LTNULL);
	}
}

LTBOOL CBaseMenu::LoadSurfaces()
{
	if (!m_pClientDE) return LTFALSE;

	m_nGenericItems = 0;
	for (int i = 0; i < MAX_GENERIC_ITEMS; i++)
	{
		if (m_GenericItem[i].hMenuItem)
		{
			m_nGenericItems++;
		}
	}

	// crop the top item to make menu title spacing consistent

	m_GenericItem[0].hMenuItem = CropMenuItemTop (m_GenericItem[0].hMenuItem);
	m_GenericItem[0].hMenuItemSelected = CropMenuItemTop (m_GenericItem[0].hMenuItemSelected);
	m_pClientDE->GetSurfaceDims (m_GenericItem[0].hMenuItem, &m_GenericItem[0].szMenuItem.cx, &m_GenericItem[0].szMenuItem.cy);

	// crop the bottom of the menu title (if there is one) to keep title spacing consistent

	if (m_hMenuTitle)
	{
		m_hMenuTitle = CropMenuItemBottom (m_hMenuTitle);
		m_hMenuTitle = AddUnderline (m_hMenuTitle, 4);

		m_pClientDE->GetSurfaceDims (m_hMenuTitle, &m_szMenuTitle.cx, &m_szMenuTitle.cy);
	}

	CalculateMenuDims();

	return LTTRUE;
}

void CBaseMenu::UnloadSurfaces()
{
	m_szMenuTitle.cx = m_szMenuTitle.cy = 0;
	m_nMenuTitleSpacing = 0;

	m_nMenuX = 0;
	m_nMenuY = 0;
	m_nMenuSpacing = 0;
	m_nGenericItems = 0;
}

HSURFACE CBaseMenu::CropMenuItemTop (HSURFACE hSurf)
{
	if (!m_pClientDE) return LTNULL;

	if (!hSurf) return LTNULL;

	uint32 nWidth, nHeight;
	m_pClientDE->GetSurfaceDims (hSurf, &nWidth, &nHeight);

	LTRect rcBorders;
	memset (&rcBorders, 0, sizeof (LTRect));
	m_pClientDE->GetBorderSize (hSurf, LTNULL, &rcBorders);

	HSURFACE hCropped = m_pClientDE->CreateSurface (nWidth, nHeight - rcBorders.top);
	if (!hCropped) return hSurf;

	LTRect rcSrc;
	rcSrc.left = 0;
	rcSrc.right = nWidth;
	rcSrc.top = rcBorders.top;
	rcSrc.bottom = nHeight;

	m_pClientDE->DrawSurfaceToSurface (hCropped, hSurf, &rcSrc, 0, 0);
	
	m_pClientDE->DeleteSurface (hSurf);

	return hCropped;
}

HSURFACE CBaseMenu::CropMenuItemBottom (HSURFACE hSurf)
{
	if (!m_pClientDE) return LTNULL;

	if (!hSurf) return LTNULL;

	uint32 nWidth, nHeight;
	m_pClientDE->GetSurfaceDims (hSurf, &nWidth, &nHeight);

	LTRect rcBorders;
	memset (&rcBorders, 0, sizeof (LTRect));
	m_pClientDE->GetBorderSize (hSurf, LTNULL, &rcBorders);

	HSURFACE hCropped = m_pClientDE->CreateSurface (nWidth, nHeight - rcBorders.bottom);
	if (!hCropped) return hSurf;

	LTRect rcSrc;
	rcSrc.left = 0;
	rcSrc.right = nWidth;
	rcSrc.top = 0;
	rcSrc.bottom = nHeight - rcBorders.bottom;

	m_pClientDE->DrawSurfaceToSurface (hCropped, hSurf, &rcSrc, 0, 0);
	
	m_pClientDE->DeleteSurface (hSurf);

	return hCropped;
}

HSURFACE CBaseMenu::AddUnderline (HSURFACE hSurf, int nSpacing, int nThickness)
{
	if (!m_pClientDE) return LTNULL;

	if (!hSurf) return LTNULL;

	uint32 nWidth, nHeight;
	m_pClientDE->GetSurfaceDims (hSurf, &nWidth, &nHeight);

	HSURFACE hNew = m_pClientDE->CreateSurface (nWidth, nHeight + nSpacing + nThickness);
	if (!hNew) return hSurf;

	LTRect rcLine;
	rcLine.left = 0;
	rcLine.right = nWidth;
	rcLine.top = nHeight + nSpacing;
	rcLine.bottom = nHeight + nSpacing + nThickness;
	
	HLTCOLOR lineColor = m_pClientDE->SetupColor1 (0.98f, 0.317647f, 0.0f, LTFALSE);

	m_pClientDE->FillRect (hNew, &rcLine, lineColor);
	m_pClientDE->DrawSurfaceToSurface (hNew, hSurf, LTNULL, 0, 0);
	
	m_pClientDE->DeleteSurface (hSurf);

	return hNew;
}

void CBaseMenu::CalculateMenuDims()
{
	if (!m_pClientDE || !m_pRiotMenu) return;

	// get the dims of any menu title there might be

	m_szMenuTitle.cx = m_szMenuTitle.cy = 0;
	if (m_hMenuTitle)
	{
		m_pClientDE->GetSurfaceDims (m_hMenuTitle, &m_szMenuTitle.cx, &m_szMenuTitle.cy);
	}

	// get the total height of the menu, without any spacing as well as maximum width

	int nMenuHeight = 0;
	int nMenuMaxWidth = 0;
	for (int i = 0; i < MAX_GENERIC_ITEMS; i++)
	{
		nMenuHeight += m_GenericItem[i].szMenuItem.cy;
		if ((int)m_GenericItem[i].szMenuItem.cx > nMenuMaxWidth)
		{
			nMenuMaxWidth = m_GenericItem[i].szMenuItem.cx;
		}
	}

	// determine correct spacing
	// if we can _just_ fit without any spacing, set spacing to default value

	LTBOOL bOffScreen = LTFALSE;
	if (nMenuHeight + m_nMenuTitleSpacing + m_szMenuTitle.cy + ((m_nGenericItems - 1) * MAX_MENU_SPACING) > m_szMenuArea.cy &&
		nMenuHeight + m_nMenuTitleSpacing + m_szMenuTitle.cy + ((m_nGenericItems - 1) * MIN_MENU_SPACING) <= m_szMenuArea.cy)
	{
		for (int i = MIN_MENU_SPACING; i < MAX_MENU_SPACING; i++)
		{
			if (nMenuHeight + m_szMenuTitle.cy + ((m_nGenericItems - 1) * (i + 1)) > m_szMenuArea.cy)
			{
				m_nMenuSpacing = i;
				m_nMenuTitleSpacing = TITLE_SPACING_MULTIPLIER * m_nMenuSpacing;
				break;
			}
		}
	}
	else if (nMenuHeight + m_nMenuTitleSpacing + m_szMenuTitle.cy + ((m_nGenericItems - 1) * MIN_MENU_SPACING) > m_szMenuArea.cy)
	{
		bOffScreen = LTTRUE;
		m_nMenuSpacing = MAX_MENU_SPACING;
		m_nMenuTitleSpacing = TITLE_SPACING_MULTIPLIER * m_nMenuSpacing;
	}
	else
	{
		m_nMenuSpacing = MAX_MENU_SPACING;
		m_nMenuTitleSpacing = TITLE_SPACING_MULTIPLIER * m_nMenuSpacing;
	}

	// determine correct placement

	m_nMenuX = 0;
	//if (m_pRiotMenu->InWorld() || m_szScreen.cx < 512)
	//{
		m_nMenuX = GetMenuAreaLeft() + ((int)m_szMenuArea.cx - nMenuMaxWidth) / 2;
	//}
	//else
	//{
	//	m_nMenuX = GetMenuAreaLeft() + ((int)m_szMenuArea.cx / 2);
	//}
	
	m_nMenuY = GetMenuAreaTop();
	if (!bOffScreen)
	{
		m_nMenuY = GetMenuAreaTop() + (((int)m_szMenuArea.cy - (nMenuHeight + m_nMenuTitleSpacing + (int)m_szMenuTitle.cy + ((m_nGenericItems - 1) * m_nMenuSpacing))) / 2);
	}

	// give derived objects a chance to modify values...

	PostCalculateMenuDims();
}

void CBaseMenu::PostCalculateMenuDims()
{
}

void CBaseMenu::CheckSelectionOffMenuTop()
{
	if (m_nSelection < m_nTopItem)
	{
		m_nTopItem = m_nSelection;
	}
}

void CBaseMenu::CheckSelectionOffMenuBottom()
{
	int nCurrentY = m_nMenuY + m_szMenuTitle.cy + m_nMenuTitleSpacing;
	for (int i = m_nTopItem; i < MAX_GENERIC_ITEMS; i++)
	{
		if (m_GenericItem[i].hMenuItem)
		{
			nCurrentY += m_GenericItem[i].szMenuItem.cy;

			if (nCurrentY > GetMenuAreaBottom())
			{
				m_nTopItem++;
				if (m_nTopItem == MAX_GENERIC_ITEMS - 1)
				{
					// some strangeness happened...
					m_nTopItem = 0;
					return;
				}
				CheckSelectionOffMenuBottom();
				return;
			}

			if (m_nSelection == i) return;

			nCurrentY += m_nMenuSpacing;
		}
	}
}

