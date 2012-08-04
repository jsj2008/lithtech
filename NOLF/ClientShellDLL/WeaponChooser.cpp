// WeaponChooser.cpp: implementation of the CWeaponChooser class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "WeaponChooser.h"
#include "InterfaceMgr.h"
#include "GameClientShell.h"
#include "WinUtil.h"
#include "SoundMgr.h"
#include "LayoutMgr.h"

extern CGameClientShell* g_pGameClientShell;


namespace
{
	const int kLastWeapon = (NUM_WEAPON_ICONS - 1);
	const int kLastAmmo = (NUM_AMMO_ICONS - 1);
	const int kCurrWeapon = 1;
	const int kCurrAmmo = 0;
	const int kIconWidth	= 48;
	const int kIconSpacing  = 16;
    const float kfIconY     = 400.0f;
	const float kfDelayTime	= 3.0f;
}


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWeaponChooser::CWeaponChooser()
{
    memset(m_hWeaponSurf, LTNULL, sizeof(m_hWeaponSurf));
	memset(m_nWeapons, -1, sizeof(m_nWeapons));
    m_hWeaponStr = LTNULL;
    m_szWeaponCommand[0] = LTNULL;
    m_bIsOpen = LTFALSE;
	m_fStartTime = 0.0f;
}

CWeaponChooser::~CWeaponChooser()
{
}

void CWeaponChooser::Term()
{
	if (m_bIsOpen)
		Close();
}

LTBOOL CWeaponChooser::Open()
{
	if (m_bIsOpen)
        return LTTRUE;

	m_nWeapons[0] = g_pGameClientShell->GetWeaponModel()->PrevWeapon();
	m_nWeapons[1] = g_pGameClientShell->GetWeaponModel()->GetWeaponId();
	m_nWeapons[2] = g_pGameClientShell->GetWeaponModel()->NextWeapon();

	if (m_nWeapons[1] == m_nWeapons[2])
	{
		for (int i = 0; i < NUM_WEAPON_ICONS; i++)
			m_nWeapons[i] = -1;
        m_bIsOpen = LTFALSE;
        return LTFALSE;
	}
    m_bIsOpen = LTTRUE;

    WEAPON* pWeapon = LTNULL;
	for (int i = 0; i < NUM_WEAPON_ICONS; i++)
	{
		pWeapon = g_pWeaponMgr->GetWeapon(m_nWeapons[i]);
        if (pWeapon)
		{
            m_hWeaponSurf[i] = g_pLTClient->CreateSurfaceFromBitmap(pWeapon->szIcon);
            g_pLTClient->OptimizeSurface(m_hWeaponSurf[i],SETRGB_T(255,0,255));
		}
	}

	pWeapon = g_pWeaponMgr->GetWeapon(m_nWeapons[1]);
	if (pWeapon)
	{
        m_hWeaponStr = g_pLTClient->FormatString(pWeapon->nNameId);
        SetCommandStr(m_nWeapons[1]);
        g_pLTClient->SetSurfaceAlpha(m_hWeaponSurf[0],0.5f);
        g_pLTClient->SetSurfaceAlpha(m_hWeaponSurf[1],1.0f);
        g_pLTClient->SetSurfaceAlpha(m_hWeaponSurf[2],0.5f);
	}

    m_fStartTime = g_pLTClient->GetTime();

    return LTTRUE;

}

void CWeaponChooser::Close()
{
	if (!m_bIsOpen)
		return;
	for (int i = 0; i < NUM_WEAPON_ICONS; i++)
	{
		m_nWeapons[i] = -1;
		if (m_hWeaponSurf[i])
		{
            g_pLTClient->DeleteSurface(m_hWeaponSurf[i]);
            m_hWeaponSurf[i] = LTNULL;
		}
	}
	if (m_hWeaponStr)
	{
        g_pLTClient->FreeString(m_hWeaponStr);
        m_hWeaponStr = LTNULL;
	}
	m_szWeaponCommand[0] = LTNULL;

    m_bIsOpen = LTFALSE;
	m_fStartTime = 0.0f;
}

void CWeaponChooser::NextWeapon()
{
	if (m_hWeaponSurf[0])
	{
        g_pLTClient->DeleteSurface(m_hWeaponSurf[0]);
        m_hWeaponSurf[0] = LTNULL;
	}
    int i;
    for (i = 0; i < kLastWeapon; i++)
	{
		m_nWeapons[i] = m_nWeapons[i+1];
		m_hWeaponSurf[i] = m_hWeaponSurf[i+1];
	}
	m_nWeapons[kLastWeapon] = g_pGameClientShell->GetWeaponModel()->NextWeapon(m_nWeapons[kLastWeapon-1]);

	WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(m_nWeapons[kLastWeapon]);
	if (pWeapon)
	{
        m_hWeaponSurf[kLastWeapon] = g_pLTClient->CreateSurfaceFromBitmap(pWeapon->szIcon);
        g_pLTClient->OptimizeSurface(m_hWeaponSurf[kLastWeapon],SETRGB_T(255,0,255));
	}
	if (m_hWeaponStr)
	{
        g_pLTClient->FreeString(m_hWeaponStr);
        m_hWeaponStr = LTNULL;
	}
	m_szWeaponCommand[0] = LTNULL;

	pWeapon = g_pWeaponMgr->GetWeapon(m_nWeapons[1]);
	if (pWeapon)
	{
        m_hWeaponStr = g_pLTClient->FormatString(pWeapon->nNameId);
		SetCommandStr(m_nWeapons[1]);
        g_pLTClient->SetSurfaceAlpha(m_hWeaponSurf[0],0.5f);
        g_pLTClient->SetSurfaceAlpha(m_hWeaponSurf[1],1.0f);
        g_pLTClient->SetSurfaceAlpha(m_hWeaponSurf[2],0.5f);
	}

    g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundSelect());

    m_fStartTime = g_pLTClient->GetTime();

}

void CWeaponChooser::PrevWeapon()
{
	if (m_hWeaponSurf[kLastWeapon])
	{
        g_pLTClient->DeleteSurface(m_hWeaponSurf[kLastWeapon]);
        m_hWeaponSurf[kLastWeapon] = LTNULL;
	}
    int i;
    for (i = kLastWeapon; i > 0; i--)
	{
		m_nWeapons[i] = m_nWeapons[i-1];
		m_hWeaponSurf[i] = m_hWeaponSurf[i-1];
	}
	m_nWeapons[0] = g_pGameClientShell->GetWeaponModel()->PrevWeapon(m_nWeapons[1]);

	WEAPON* pWeapon = g_pWeaponMgr->GetWeapon(m_nWeapons[0]);
	if (pWeapon)
	{
        m_hWeaponSurf[0] = g_pLTClient->CreateSurfaceFromBitmap(pWeapon->szIcon);
        g_pLTClient->OptimizeSurface(m_hWeaponSurf[0],SETRGB_T(255,0,255));
	}
	if (m_hWeaponStr)
	{
        g_pLTClient->FreeString(m_hWeaponStr);
        m_hWeaponStr = LTNULL;
	}
	m_szWeaponCommand[0] = LTNULL;


	pWeapon = g_pWeaponMgr->GetWeapon(m_nWeapons[1]);
	if (pWeapon)
	{
        m_hWeaponStr = g_pLTClient->FormatString(pWeapon->nNameId);
		SetCommandStr(m_nWeapons[1]);
        g_pLTClient->SetSurfaceAlpha(m_hWeaponSurf[0],0.5f);
        g_pLTClient->SetSurfaceAlpha(m_hWeaponSurf[1],1.0f);
        g_pLTClient->SetSurfaceAlpha(m_hWeaponSurf[2],0.5f);
	}

    g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundSelect());

    m_fStartTime = g_pLTClient->GetTime();

}

void CWeaponChooser::Draw()
{
    float fTime = g_pLTClient->GetTime() - m_fStartTime;
	if (m_fStartTime > 0.0f && fTime > kfDelayTime)
	{
        g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundSelect());
		Close();
		return;
	}


    HSURFACE hScreen = g_pLTClient->GetScreenSurface();
    uint32 nScreenHeight, nScreenWidth;
    g_pLTClient->GetSurfaceDims (hScreen, &nScreenWidth, &nScreenHeight);
	float yRatio = (float)nScreenHeight / 480.0f;

	int x = (int)nScreenWidth / 2;
	int y = (int) (yRatio * kfIconY);

	CLTGUIFont* pFont = g_pInterfaceResMgr->GetChooserFont();
	if (pFont)
	{
		pFont->Draw(m_hWeaponStr,hScreen,x,y+(kIconWidth + kIconSpacing),LTF_JUSTIFY_CENTER,kWhite);
	}

	x -= (NUM_WEAPON_ICONS * (kIconWidth + kIconSpacing)) / 2;
/*
	if (m_hWeaponSurf[0] && m_nWeapons[0] != m_nWeapons[kLastWeapon])
        g_pLTClient->DrawSurfaceToSurfaceTransparent(hScreen, m_hWeaponSurf[0], LTNULL, x, y,SETRGB_T(255,0,255));
*/
	x += (kIconWidth + kIconSpacing);
	if (m_hWeaponSurf[1])
	{
        LTVector vColor = g_pLayoutMgr->GetChooserHighlightColor();

        LTRect rect(x-3,y-3,x+kIconWidth+3,y+kIconWidth+3);
        g_pLTClient->FillRect(hScreen,&rect,SETRGB_T(vColor.x,vColor.y,vColor.z));
        g_pLTClient->DrawSurfaceToSurface(hScreen, m_hWeaponSurf[1], LTNULL, x, y);
	}

/* commented out drawing of weapon command - jrg 9/22
	if (pFont && m_szWeaponCommand[0])
	{
		pFont->Draw(m_szWeaponCommand,hScreen,x,y,LTF_JUSTIFY_LEFT,SETRGB(255,255,0));
	}
*/
	x += (kIconWidth + kIconSpacing);

/*
	if (m_hWeaponSurf[2] && m_nWeapons[2] != m_nWeapons[1])
        g_pLTClient->DrawSurfaceToSurfaceTransparent(hScreen, m_hWeaponSurf[2], LTNULL, x, y,SETRGB_T(255,0,255));
*/
}

void CWeaponChooser::SetCommandStr(int nWeaponId)
{
	int commandId = g_pWeaponMgr->GetCommandId(nWeaponId);
	GetCommandKeyStr(commandId,m_szWeaponCommand,sizeof(m_szWeaponCommand));

}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAmmoChooser::CAmmoChooser()
{
    memset(m_hAmmoSurf, LTNULL, sizeof(m_hAmmoSurf));
	memset(m_nAmmo, -1, sizeof(m_nAmmo));
    m_hAmmoStr = LTNULL;
    m_bIsOpen = LTFALSE;
	m_fStartTime = 0.0f;
}

CAmmoChooser::~CAmmoChooser()
{
}

void CAmmoChooser::Term()
{
	if (m_bIsOpen)
		Close();
}

LTBOOL CAmmoChooser::Open()
{
	// Don't allow the chooser to be opened if we're selecting/deselecting a
	// weapon...

	WeaponState eState = g_pGameClientShell->GetWeaponModel()->GetState();
	if (W_DESELECT == eState || W_SELECT == eState) return LTFALSE;


	if (m_bIsOpen)
        return LTTRUE;

	m_nAmmo[kCurrAmmo] = g_pGameClientShell->GetWeaponModel()->GetAmmoId();
	m_nAmmo[kLastAmmo] = g_pGameClientShell->GetWeaponModel()->NextAmmo();

	if (m_nAmmo[kCurrAmmo] == m_nAmmo[kLastAmmo])
	{
		for (int i = 0; i < NUM_AMMO_ICONS; i++)
			m_nAmmo[i] = -1;
        m_bIsOpen = LTFALSE;
        return LTFALSE;
	}
    m_bIsOpen = LTTRUE;

    AMMO* pAmmo = LTNULL;

	for (int i = 0; i < NUM_AMMO_ICONS; i++)
	{
		pAmmo = g_pWeaponMgr->GetAmmo(m_nAmmo[i]);
		if (pAmmo)
		{
            m_hAmmoSurf[i] = g_pLTClient->CreateSurfaceFromBitmap(pAmmo->szIcon);
            g_pLTClient->OptimizeSurface(m_hAmmoSurf[i],SETRGB_T(255,0,255));
		}
	}

	pAmmo = g_pWeaponMgr->GetAmmo(m_nAmmo[kLastAmmo]);
	if (pAmmo)
	{
        m_hAmmoStr = g_pLTClient->FormatString(pAmmo->nNameId);
        g_pLTClient->SetSurfaceAlpha(m_hAmmoSurf[kCurrAmmo],1.0f);
        g_pLTClient->SetSurfaceAlpha(m_hAmmoSurf[kLastAmmo],0.5f);
	}

    m_fStartTime = g_pLTClient->GetTime();

    return LTTRUE;
}

void CAmmoChooser::Close()
{
	if (!m_bIsOpen)
		return;
	for (int i = 0; i < NUM_AMMO_ICONS; i++)
	{
		m_nAmmo[i] = -1;
		if (m_hAmmoSurf[i])
		{
            g_pLTClient->DeleteSurface(m_hAmmoSurf[i]);
            m_hAmmoSurf[i] = LTNULL;
		}
	}
	if (m_hAmmoStr)
	{
        g_pLTClient->FreeString(m_hAmmoStr);
        m_hAmmoStr = LTNULL;
	}
    m_bIsOpen = LTFALSE;
	m_fStartTime = 0.0f;
}

void CAmmoChooser::NextAmmo()
{
	if (m_hAmmoSurf[kCurrAmmo])
	{
        g_pLTClient->DeleteSurface(m_hAmmoSurf[kCurrAmmo]);
        m_hAmmoSurf[kCurrAmmo] = LTNULL;
	}
	m_nAmmo[kCurrAmmo] = m_nAmmo[kLastAmmo];
	m_hAmmoSurf[kCurrAmmo] = m_hAmmoSurf[kLastAmmo];

	m_nAmmo[kLastAmmo] = g_pGameClientShell->GetWeaponModel()->NextAmmo(m_nAmmo[kCurrAmmo]);

	AMMO* pAmmo = g_pWeaponMgr->GetAmmo(m_nAmmo[kLastAmmo]);
	if (pAmmo)
	{
        m_hAmmoSurf[kLastAmmo] = g_pLTClient->CreateSurfaceFromBitmap(pAmmo->szIcon);
        g_pLTClient->OptimizeSurface(m_hAmmoSurf[kLastAmmo],SETRGB_T(255,0,255));
	}
	if (m_hAmmoStr)
	{
        g_pLTClient->FreeString(m_hAmmoStr);
        m_hAmmoStr = LTNULL;
	}

	pAmmo = g_pWeaponMgr->GetAmmo(m_nAmmo[kCurrAmmo]);
	if (pAmmo)
	{
        m_hAmmoStr = g_pLTClient->FormatString(pAmmo->nNameId);
        g_pLTClient->SetSurfaceAlpha(m_hAmmoSurf[kCurrAmmo],1.0f);
        g_pLTClient->SetSurfaceAlpha(m_hAmmoSurf[kLastAmmo],0.5f);
	}

	g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundSelect());

    m_fStartTime = g_pLTClient->GetTime();

}


void CAmmoChooser::Draw()
{
    float fTime = g_pLTClient->GetTime() - m_fStartTime;
	if (m_fStartTime > 0.0f && fTime > kfDelayTime)
	{
		g_pClientSoundMgr->PlayInterfaceSound((char*)g_pInterfaceResMgr->GetSoundSelect());
		Close();
		return;
	}

	CPlayerStats *pStats = g_pInterfaceMgr->GetPlayerStats();

    HSURFACE hScreen = g_pLTClient->GetScreenSurface();
    uint32 nScreenHeight, nScreenWidth;
    g_pLTClient->GetSurfaceDims (hScreen, &nScreenWidth, &nScreenHeight);
	float yRatio = (float)nScreenHeight / 480.0f;

	int x = (int)nScreenWidth / 2;
	int y = (int) (yRatio * kfIconY);

	CLTGUIFont *pFont = g_pInterfaceResMgr->GetChooserFont();
	pFont->Draw(m_hAmmoStr,hScreen,x,y+(kIconWidth + kIconSpacing),LTF_JUSTIFY_CENTER,kWhite);

//	x -= (NUM_AMMO_ICONS * (kIconWidth + kIconSpacing)) / 2;
	x -= kIconWidth / 2;

	if (m_hAmmoSurf[kCurrAmmo])
	{
        LTRect rect(x-3,y-3,x+kIconWidth+3,y+kIconWidth+3);
        g_pLTClient->FillRect(hScreen,&rect,SETRGB_T(220,192,255));
        g_pLTClient->DrawSurfaceToSurface(hScreen, m_hAmmoSurf[kCurrAmmo], LTNULL, x, y);
		int count = pStats->GetAmmoCount(m_nAmmo[kCurrAmmo]);
		if (count > 0 && count < 1000)
		{
			char szStr[5] = "";
			sprintf(szStr,"%d",count);
			pFont->Draw(szStr,hScreen,(x+kIconWidth/2),y + kIconWidth - 16,LTF_JUSTIFY_CENTER,kWhite);
		}
	}
	x += (kIconWidth + kIconSpacing);
/*
	if (m_hAmmoSurf[kLastAmmo])
	{
        g_pLTClient->DrawSurfaceToSurfaceTransparent(hScreen, m_hAmmoSurf[kLastAmmo], LTNULL, x, y,SETRGB_T(255,0,255));
		int count = pStats->GetAmmoCount(m_nAmmo[kLastAmmo]);
		if (count > 0 && count < 1000)
		{
			char szStr[5] = "";
			sprintf(szStr,"%d",count);
			pFont->Draw(szStr,hScreen,(x+kIconWidth/2),y + kIconWidth - 16,LTF_JUSTIFY_CENTER);
		}
	}
*/
}