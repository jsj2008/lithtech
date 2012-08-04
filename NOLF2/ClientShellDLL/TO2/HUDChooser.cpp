// ----------------------------------------------------------------------- //
//
// MODULE  : HUDChooser.cpp
//
// PURPOSE : HUDItem to display weapon and ammo choices
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "HUDMgr.h"
#include "HUDChooser.h"
#include "PlayerStats.h"
#include "InterfaceMgr.h"
#include "PlayerMgr.h"
#include "ClientWeaponMgr.h"

//******************************************************************************************
//**
//** HUD Weapon Chooser display
//**
//******************************************************************************************
CHUDWpnChooser::CHUDWpnChooser()
{
	m_UpdateFlags = kHUDChooser;
	m_bDraw = LTFALSE;
	m_fTexScale = 0.0f;
	m_fScale = 0.0f;
	m_nScreenWidth = 0;
	m_pStr = LTNULL;
}

CHUDWpnChooser::~CHUDWpnChooser()
{
 	if (m_pStr)
	{
		g_pFontManager->DestroyPolyString(m_pStr);
        m_pStr=LTNULL;
	}

}


LTBOOL CHUDWpnChooser::Init()
{
	// get first and last weapon indices
	uint8 nFirstWeaponCommandId = g_pWeaponMgr->GetFirstWeaponCommandId();
	uint8 nLastWeaponCommandId = g_pWeaponMgr->GetLastWeaponCommandId();
	uint8 nWeaponCount[kMaxClasses];
	memset(nWeaponCount,0,sizeof(nWeaponCount));
	for (uint8 nWpnCommandId = nFirstWeaponCommandId; nWpnCommandId <= nLastWeaponCommandId; nWpnCommandId++)
	{
		uint8 nWpn = g_pWeaponMgr->GetWeaponId( nWpnCommandId );

		if (g_pWeaponMgr->IsPlayerWeapon(nWpn))
		{
			// get the weapon data struct
			uint8 nClass = g_pWeaponMgr->GetWeaponClass( nWpn ) - 1;
			if (nClass < kMaxClasses && nWeaponCount[nClass] < kMaxItemsPerClass)
			{
				m_nWeaponID[nClass][nWeaponCount[nClass]] = nWpn;
				nWeaponCount[nClass]++;
			}

		}
	}

	UpdateLayout();


	for( uint8 i = 0; i < kMaxClasses; ++i )
	{
		g_pDrawPrim->SetRGBA( &m_ClassPoly[i], argbWhite );
		
		for (uint8 n = nWeaponCount[i]; n < kMaxItemsPerClass; n++)
		{
			m_nWeaponID[i][n] = WMGR_INVALID_ID;
		}
	}
	for( uint8 i = 0; i < kMaxItemsPerClass; ++i )
	{
		g_pDrawPrim->SetRGBA( &m_ItemPoly[i], argbWhite );

		m_hItemIcon[i] = NULL;

	}

	uint8 nFont = g_pLayoutMgr->GetHUDFont();
	CUIFont* pFont = g_pInterfaceResMgr->GetFont(nFont);

	m_pStr = g_pFontManager->CreateFormattedPolyString(pFont,"",0.0f, 0.0f);
	m_pStr->SetColor(m_TextColor);



	return LTTRUE;
}


void CHUDWpnChooser::Render()
{
	if (!m_bDraw) return;

	SetRenderState();

	for (uint8 n = 0; n < kMaxClasses; n++)
	{
//		if (n != m_nClass)
//		{
			g_pDrawPrim->SetTexture( m_hClassIcon[n] );
			SetupQuadUVs(m_ClassPoly[n], m_hClassIcon[n], 0.0f, 0.0f, m_fTexScale, m_fTexScale);
			g_pDrawPrim->DrawPrim( &m_ClassPoly[n], 1 );
//		}
	}

	for (uint8 n = 0; n < kMaxItemsPerClass && m_hItemIcon[n]; n++)
	{
		g_pDrawPrim->SetTexture( m_hItemIcon[n] );
		SetupQuadUVs(m_ItemPoly[n], m_hItemIcon[n], 0.0f, 0.0f, m_fTexScale, m_fTexScale);
		g_pDrawPrim->DrawPrim( &m_ItemPoly[n], 1 );
	}

	m_pStr->Render();
}


void CHUDWpnChooser::Update()
{
	m_bDraw = !!g_pInterfaceMgr->IsChoosingWeapon();
	if (!m_bDraw) return;

	if (g_pInterfaceResMgr->GetScreenWidth() != m_nScreenWidth)
	{
		m_nScreenWidth = g_pInterfaceResMgr->GetScreenWidth();
		if (m_nScreenWidth < 1024)
			m_fScale = (float)m_nScreenWidth / 1024.0f;
		else
			m_fScale = 1.0f;
		uint8 h = (uint8)(m_fTextHeight * m_fScale);
		m_pStr->SetCharScreenHeight(h);

		uint16 w = ((uint16)(m_nScreenWidth) / 2) - 20;
		m_pStr->SetWrapWidth(w);


		float cw = m_fScale * m_fIconHt;
		float x = ((float)m_nScreenWidth - ((float)kMaxClasses * cw)) / 2.0f;
		x += cw / 2.0f;
		

		for( uint8 i = 0; i < kMaxClasses; ++i )
			m_fColumnXPos[i] = x + (float)i * cw;

	}

	uint8 nClass = g_pInterfaceMgr->GetWeaponChooser()->GetCurrentClass();
	uint8 nWpn = g_pInterfaceMgr->GetWeaponChooser()->GetCurrentSelection();

	if (nClass > 0)
		m_nClass = nClass - 1;
	else
	{
		m_nClass = g_pWeaponMgr->GetWeaponClass( nWpn ) - 1;
	}

	if (m_nClass >= kMaxClasses)
	{
		m_bDraw = LTFALSE;
		g_pInterfaceMgr->GetWeaponChooser()->Close();
		return;
	}

	char szTmp[64] = "";
	float fHeight = m_fIconHt * m_fScale;
	sprintf(szTmp,"interface\\hud\\WpnClass%d.dtx",(m_nClass+1));
	m_hClassIcon[m_nClass] = g_pInterfaceResMgr->GetTexture( szTmp );

	uint32 tw,th;
	g_pTexInterface->GetTextureDims(m_hClassIcon[m_nClass],tw,th);

	float fBaseWidth = (float)tw * ( fHeight / (float)th );
	float fBaseX = (float)m_fColumnXPos[m_nClass] - (fBaseWidth / 2.0f);

	g_pDrawPrim->SetXYWH( &m_ClassPoly[m_nClass], fBaseX, 0.0f, fBaseWidth, fHeight );

	float x = fBaseX;
	for (uint8 n = m_nClass; n > 0; n--)
	{
		x -= fHeight;
		g_pDrawPrim->SetXYWH( &m_ClassPoly[n-1], x, 0.0f, fHeight, fHeight );
	}
	x = fBaseX + fBaseWidth;
	for (uint8 n = m_nClass+1; n < kMaxClasses; n++)
	{
		g_pDrawPrim->SetXYWH( &m_ClassPoly[n], x, 0.0f, fHeight, fHeight );
		x += fHeight;
	}

	for (uint8 n = 0; n < kMaxClasses; n++)
	{
		if (n == m_nClass) continue;

		bool bHasOne = false;
		for (uint8 i = 0; i < kMaxItemsPerClass && !bHasOne; i++)
		{
			bHasOne = (g_pPlayerStats->HaveWeapon( m_nWeaponID[n][i] ) && g_pPlayerMgr->GetClientWeaponMgr()->CanChangeToWeapon(m_nWeaponID[n][i]));
		}

		if (bHasOne)
			sprintf(szTmp,"interface\\hud\\WpnClass%dU.dtx",(n+1));
		else
			sprintf(szTmp,"interface\\hud\\WpnClass%dD.dtx",(n+1));
		m_hClassIcon[n] = g_pInterfaceResMgr->GetTexture( szTmp );
				
	}



	WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon( nWpn );
	if (pWeapon)
		m_pStr->SetText(pWeapon->szShortName);
	else
		m_pStr->SetText("");
	

	uint8 nSlot = 0;
	float y = fHeight;
	for (uint8 i = 0; i < kMaxItemsPerClass; i++)
	{
		m_hItemIcon[nSlot] = LTNULL;
		if (g_pPlayerStats->HaveWeapon( m_nWeaponID[m_nClass][i] ))
		{
			WEAPON const *pWeapon = g_pWeaponMgr->GetWeapon( m_nWeaponID[m_nClass][i] );
			if (pWeapon)
			{
				std::string icon;

				if (!g_pPlayerMgr->GetClientWeaponMgr()->CanChangeToWeapon(m_nWeaponID[m_nClass][i]))
				{
					icon = pWeapon->GetDisabledIcon();
				}
				else if (m_nWeaponID[m_nClass][i] == nWpn)
				{
					icon = pWeapon->GetNormalIcon();
				}
				else
				{
					icon = pWeapon->GetUnselectedIcon();
				}
				m_hItemIcon[nSlot] = g_pInterfaceResMgr->GetTexture( icon.c_str() );

				uint32 tw,th;
				g_pTexInterface->GetTextureDims(m_hItemIcon[nSlot],tw,th);

				float fWidth = (float)tw * ( fHeight / (float)th );
				float x = (float)m_fColumnXPos[m_nClass] - (fWidth / 2.0f);

				g_pDrawPrim->SetXYWH( &m_ItemPoly[nSlot], x, y, fWidth, fHeight );

				if (m_nWeaponID[m_nClass][i] == nWpn)
				{
					if (m_nClass >= kMaxClasses / 2)
					{
						m_pStr->SetPosition(x-2.0f,y+2.0f);
						m_pStr->SetAlignmentH(CUI_HALIGN_RIGHT);
					}
					else
					{
						m_pStr->SetPosition(x+fWidth+2.0f,y+2.0f);
						m_pStr->SetAlignmentH(CUI_HALIGN_LEFT);
					}
				}


				nSlot++;
				y += fHeight;

			}
		}
		
	}
		
}

void CHUDWpnChooser::UpdateLayout()
{
	int nCurrentLayout = GetConsoleInt("HUDLayout",0);


	m_fIconHt = (float)g_pLayoutMgr->GetChooserIconHeight(nCurrentLayout,48);
	m_TextColor = g_pLayoutMgr->GetChooserTextColor(nCurrentLayout,argbWhite);
	m_fTextHeight = (float)g_pLayoutMgr->GetChooserTextSize(nCurrentLayout,20);
	m_fTexScale = g_pLayoutMgr->GetChooserTextureScale(nCurrentLayout,0.75f);


}



//******************************************************************************************
//**
//** HUD Weapon Chooser display
//**
//******************************************************************************************

CHUDAmmoChooser::CHUDAmmoChooser()
{
	m_UpdateFlags = kHUDChooser;
	m_bDraw = LTFALSE;
	m_fScale = 0.0f;
	m_fTexScale = 0.0f;
	m_nScreenWidth = 0;
	m_pStr = LTNULL;
	m_pWpnStr = LTNULL;
}

CHUDAmmoChooser::~CHUDAmmoChooser()
{
 	if (m_pStr)
	{
		g_pFontManager->DestroyPolyString(m_pStr);
        m_pStr=LTNULL;
	}
 	if (m_pWpnStr)
	{
		g_pFontManager->DestroyPolyString(m_pWpnStr);
        m_pWpnStr=LTNULL;
	}

}


LTBOOL CHUDAmmoChooser::Init()
{

	UpdateLayout();


	g_pDrawPrim->SetRGBA( &m_WeaponPoly, argbWhite );

	m_hWeaponIcon = NULL;

	for (int i = 0; i < kMaxAmmoTypes; ++i )
	{
		g_pDrawPrim->SetRGBA( &m_AmmoPoly[i], argbWhite );

		m_hAmmoIcon[i] = NULL;

	}

	uint8 nFont = g_pLayoutMgr->GetHUDFont();
	CUIFont* pFont = g_pInterfaceResMgr->GetFont(nFont);

	m_pStr = g_pFontManager->CreateFormattedPolyString(pFont,"",0.0f, 0.0f);
	m_pStr->SetColor(m_TextColor);

	m_pWpnStr = g_pFontManager->CreateFormattedPolyString(pFont,"",0.0f, 0.0f);
	m_pWpnStr->SetColor(m_TextColor);


	return LTTRUE;
}


void CHUDAmmoChooser::Render()
{
	if (!m_bDraw) return;

	SetRenderState();

	g_pDrawPrim->SetTexture( m_hWeaponIcon );
	SetupQuadUVs(m_WeaponPoly, m_hWeaponIcon, 0.0f, 0.0f, m_fTexScale, m_fTexScale);
	g_pDrawPrim->DrawPrim( &m_WeaponPoly, 1 );

	for (uint8 n = 0; m_hAmmoIcon[n] && n < kMaxAmmoTypes; n++)
	{
		g_pDrawPrim->SetTexture( m_hAmmoIcon[n] );
		SetupQuadUVs(m_AmmoPoly[n], m_hAmmoIcon[n], 0.0f, 0.0f, m_fTexScale, m_fTexScale);
		g_pDrawPrim->DrawPrim( &m_AmmoPoly[n], 1 );
	}

	m_pStr->Render();
	m_pWpnStr->Render();
}


void CHUDAmmoChooser::Update()
{
	m_bDraw = !!g_pInterfaceMgr->IsChoosingAmmo();
	if (!m_bDraw) return;
	IClientWeaponBase *pClientWeapon = g_pPlayerMgr->GetCurrentClientWeapon();
	if ( !pClientWeapon )
	{
		m_bDraw = LTFALSE;
		g_pInterfaceMgr->GetAmmoChooser()->Close();
		return;
	}
	
	WEAPON const *pWeapon = pClientWeapon->GetWeapon();
	if (!pWeapon || pWeapon->nNumAmmoIds <= 1)
	{
		m_bDraw = LTFALSE;
		g_pInterfaceMgr->GetAmmoChooser()->Close();
		return;
	}

	if (g_pInterfaceResMgr->GetScreenWidth() != m_nScreenWidth)
	{
		m_nScreenWidth = g_pInterfaceResMgr->GetScreenWidth();
		if (m_nScreenWidth < 1024)
			m_fScale = (float)m_nScreenWidth / 1024.0f;
		else
			m_fScale = 1.0f;

		uint8 h = (uint8)(m_fTextHeight * m_fScale);
		m_pStr->SetCharScreenHeight(h);
		m_pWpnStr->SetCharScreenHeight(h);

		uint16 w = ((uint16)(m_nScreenWidth) / 2) - 20;
		m_pStr->SetWrapWidth(w);
		m_pWpnStr->SetWrapWidth(w);

	}


	float fHeight = m_fIconHt * m_fScale;

	std::string icon = pWeapon->GetNormalIcon();
	m_hWeaponIcon = g_pInterfaceResMgr->GetTexture( icon.c_str() );

	uint32 tw,th;
	g_pTexInterface->GetTextureDims(m_hWeaponIcon,tw,th);

	float fWidth = (float)tw * ( fHeight / (float)th );
	float x = ((float)m_nScreenWidth / 2.0f);
	g_pDrawPrim->SetXYWH( &m_WeaponPoly, x-fWidth, 0.0f, fWidth, fHeight );

	uint8 nSlot = 0;
	uint8 nCurrAmmo = g_pInterfaceMgr->GetAmmoChooser()->GetCurrentSelection();
	float y = fHeight;
	

	m_pWpnStr->SetText(pWeapon->szShortName);
	m_pWpnStr->SetPosition(x+2.0f,2.0f);

	for (uint8 n = 0; n < pWeapon->nNumAmmoIds && n < kMaxAmmoTypes; n++)
	{
		uint8 nAmmoID = pWeapon->aAmmoIds[n];
		if (g_pPlayerStats->GetAmmoCount(nAmmoID) <= 0) continue;
		AMMO const *pAmmo = g_pWeaponMgr->GetAmmo(nAmmoID);
		if (!pAmmo) continue;

	
		icon = "";
		if (nAmmoID == nCurrAmmo)
		{
			icon = pAmmo->GetNormalIcon();
		}
		else
		{
			icon = pAmmo->GetUnselectedIcon();
		}

		m_hAmmoIcon[nSlot] = g_pInterfaceResMgr->GetTexture( icon.c_str() );
		if (!m_hAmmoIcon[nSlot]) continue;

		uint32 tw,th;
		g_pTexInterface->GetTextureDims(m_hAmmoIcon[nSlot],tw,th);
		float fWidth = (float)tw * ( fHeight / (float)th );

		g_pDrawPrim->SetXYWH( &m_AmmoPoly[nSlot], x-fWidth, y, fWidth, fHeight );

		
		char szTmp[256] = "";

		if (pWeapon->bInfiniteAmmo)
			LTStrCpy(szTmp,pAmmo->szShortName,sizeof(szTmp));
		else
			sprintf(szTmp,"(%d/%d) %s",g_pPlayerStats->GetAmmoCount(nAmmoID), pAmmo->GetMaxAmount(LTNULL), pAmmo->szShortName );
		if (nAmmoID == nCurrAmmo)
		{
			m_pStr->SetText( szTmp);
			m_pStr->SetPosition(x+2.0f,y+2.0f);
		}

		nSlot++;
		y += fHeight;

	}
	m_hAmmoIcon[nSlot] = NULL;
		
}

void CHUDAmmoChooser::UpdateLayout()
{
	int nCurrentLayout = GetConsoleInt("HUDLayout",0);


	m_fIconHt = (float)g_pLayoutMgr->GetChooserIconHeight(nCurrentLayout,48);
	m_TextColor = g_pLayoutMgr->GetChooserTextColor(nCurrentLayout,argbWhite);
	m_fTextHeight = (float)g_pLayoutMgr->GetChooserTextSize(nCurrentLayout,20);
	m_fTexScale = g_pLayoutMgr->GetChooserTextureScale(nCurrentLayout,0.75f);


}

