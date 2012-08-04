// BitmapCtrl.cpp: implementation of the CBitmapCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BitmapCtrl.h"
#include "InterfaceResMgr.h"

namespace
{
	const int kHighlightWidth = 2;
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBitmapCtrl::CBitmapCtrl()
{
    m_sNormalSurface[0] = LTNULL;
    m_sSelectedSurface[0] = LTNULL;
    m_sDisabledSurface[0] = LTNULL;
    m_bFreeSurfaces = LTFALSE;
}

CBitmapCtrl::~CBitmapCtrl()
{
	Destroy();
}


LTBOOL CBitmapCtrl::Create(ILTClient *pClientDE,  char *lpszNormalBmp,char *lpszSelectedBmp,char *lpszDisabledBmp,
                            CLTGUICommandHandler *pCommandHandler, uint32 dwCommandID, uint32 dwParam1, uint32 dwParam2)
{
	m_pClientDE=pClientDE;
	m_pCommandHandler=pCommandHandler;
	// The transparent color
	SetTransparentColor(SETRGB_T(255,0,255));

	if (SetBitmap(lpszNormalBmp,lpszSelectedBmp,lpszDisabledBmp))
	{
		CLTGUICtrl::Create(dwCommandID,dwParam1,dwParam2);
        return LTTRUE;
	}
	else
        return LTFALSE;
}

LTBOOL CBitmapCtrl::SetBitmap(char *lpszNormalBmp,char *lpszSelectedBmp,char *lpszDisabledBmp)
{
    uint32 dwWidth=0;
    uint32 dwHeight=0;
	if (m_bFreeSurfaces)
		FreeSurfaces();
	HSURFACE hNormalSurface = g_pInterfaceResMgr->GetSharedSurface(lpszNormalBmp);
	if (!hNormalSurface)
        return LTFALSE;
	strncpy(m_sNormalSurface,lpszNormalBmp,sizeof(m_sNormalSurface));
	m_pClientDE->GetSurfaceDims (hNormalSurface, &dwWidth, &dwHeight);
	m_nNormalWidth = (int)dwWidth;
	m_nNormalHeight = (int)dwHeight;

	if (lpszSelectedBmp)
	{
		HSURFACE hSelectedSurface = g_pInterfaceResMgr->GetSharedSurface(lpszSelectedBmp);
		if (!hSelectedSurface)
		{
			Destroy();
            return LTFALSE;
		}
		strncpy(m_sSelectedSurface,lpszSelectedBmp,sizeof(m_sSelectedSurface));
		m_pClientDE->GetSurfaceDims (hSelectedSurface, &dwWidth, &dwHeight);
		m_nSelectedWidth = (int)dwWidth;
		m_nSelectedHeight = (int)dwHeight;
	}
	if (lpszDisabledBmp)
	{
		HSURFACE hDisabledSurface = g_pInterfaceResMgr->GetSharedSurface(lpszDisabledBmp);
		if (!hDisabledSurface)
		{
			Destroy();
            return LTFALSE;
		}
		strncpy(m_sDisabledSurface,lpszDisabledBmp,sizeof(m_sDisabledSurface));
		m_pClientDE->GetSurfaceDims (hDisabledSurface, &dwWidth, &dwHeight);
		m_nDisabledWidth = (int)dwWidth;
		m_nDisabledHeight = (int)dwHeight;

	}
    return LTTRUE;
}

void CBitmapCtrl::FreeSurfaces()
{
	if (strlen(m_sNormalSurface))
	{
		g_pInterfaceResMgr->FreeSharedSurface(m_sNormalSurface);
        m_sNormalSurface[0] = LTNULL;
	}
	if (strlen(m_sSelectedSurface))
	{
		g_pInterfaceResMgr->FreeSharedSurface(m_sSelectedSurface);
        m_sSelectedSurface[0] = LTNULL;
	}
	if (strlen(m_sDisabledSurface))
	{
		g_pInterfaceResMgr->FreeSharedSurface(m_sDisabledSurface);
        m_sDisabledSurface[0] = LTNULL;
	}
}


// Destruction
void CBitmapCtrl::Destroy()
{
	if (m_bFreeSurfaces)
		FreeSurfaces();
}

// Render the control
void CBitmapCtrl::Render ( HSURFACE hDestSurf )
{
	int state = GetState();
	if ((state == LGCS_DISABLED || !m_bEnabled) && m_sDisabledSurface[0])
	{
        m_pClientDE->DrawSurfaceToSurfaceTransparent(hDestSurf, g_pInterfaceResMgr->GetSharedSurface(m_sDisabledSurface), LTNULL, m_pos.x, m_pos.y, m_hTrans);
	}
	else if (state == LGCS_SELECTED && m_sSelectedSurface[0])
	{
        m_pClientDE->DrawSurfaceToSurfaceTransparent(hDestSurf, g_pInterfaceResMgr->GetSharedSurface(m_sSelectedSurface), LTNULL, m_pos.x, m_pos.y, m_hTrans);
	}

	else
	{
//		if (state == LGCS_SELECTED)
//		{
//          LTRect rect = {m_pos.x-2,m_pos.y-2,m_pos.x+m_nNormalWidth+2,m_pos.y+m_nNormalHeight+2};
//          g_pLTClient->FillRect(hDestSurf,&rect,SETRGB(255,255,255));
//		}
        m_pClientDE->DrawSurfaceToSurfaceTransparent(hDestSurf, g_pInterfaceResMgr->GetSharedSurface(m_sNormalSurface), LTNULL, m_pos.x, m_pos.y, m_hTrans);

		if (state == LGCS_SELECTED)
		{
			//left
            LTRect rectL(m_pos.x,m_pos.y,m_pos.x+kHighlightWidth,m_pos.y+m_nNormalHeight);

			//top
            LTRect rectT(m_pos.x,m_pos.y,m_pos.x+m_nNormalWidth,m_pos.y+kHighlightWidth);

			//right
            LTRect rectR(m_pos.x+m_nNormalWidth-kHighlightWidth,m_pos.y,m_pos.x+m_nNormalWidth,m_pos.y+m_nNormalHeight);

			//bottom
            LTRect rectB(m_pos.x,m_pos.y+m_nNormalHeight-kHighlightWidth,m_pos.x+m_nNormalWidth,m_pos.y+m_nNormalHeight);

            g_pLTClient->FillRect(hDestSurf,&rectL,kWhite);
            g_pLTClient->FillRect(hDestSurf,&rectT,kWhite);
            g_pLTClient->FillRect(hDestSurf,&rectR,kWhite);
            g_pLTClient->FillRect(hDestSurf,&rectB,kWhite);
		}
	}

}

// Width/Height calculations
int CBitmapCtrl::GetWidth ( )
{
	int state = GetState();
	if (state == LGCS_SELECTED && m_sSelectedSurface[0])
	{
		return m_nSelectedWidth;
	}
	else if (state == LGCS_DISABLED && m_sDisabledSurface[0])
	{
		return m_nDisabledWidth;
	}
	else
	{
		return m_nNormalWidth;
	}

}

int CBitmapCtrl::GetHeight ( )
{
	int state = GetState();
	if (state == LGCS_SELECTED && m_sSelectedSurface[0])
	{
		return m_nSelectedHeight;
	}
	else if (state == LGCS_DISABLED && m_sDisabledSurface[0])
	{
		return m_nDisabledHeight;
	}
	else
	{
		return m_nNormalHeight;
	}

}


// Enter was pressed
LTBOOL CBitmapCtrl::OnEnter ( )
{
	// Send the command
	if ( m_pCommandHandler )
	{
		if (m_pCommandHandler->SendCommand(m_dwCommandID, m_dwParam1, m_dwParam2))
            return LTTRUE;
	}
    return LTFALSE;
}


void CBitmapCtrl::GetNormalBitmap(char* pBuf, int nBufLen)
{
	strncpy(pBuf,m_sNormalSurface,nBufLen);
}

void CBitmapCtrl::GetSelectedBitmap(char* pBuf, int nBufLen)
{
	if (strlen(m_sSelectedSurface))
		strncpy(pBuf,m_sSelectedSurface,nBufLen);
	else
		strncpy(pBuf,m_sNormalSurface,nBufLen);
}

void CBitmapCtrl::GetDisabledBitmap(char* pBuf, int nBufLen)
{
	if (strlen(m_sDisabledSurface))
		strncpy(pBuf,m_sDisabledSurface,nBufLen);
	else
		strncpy(pBuf,m_sNormalSurface,nBufLen);
}
