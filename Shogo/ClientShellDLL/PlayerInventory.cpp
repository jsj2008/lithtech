#include "RiotClientShell.h"
#include "iltclient.h"
#include "PlayerInventory.h"
#include "ClientRes.h"
#include "TextHelper.h"
#include "PlayerStats.h"
#include "WeaponStringDefs.h"
#include "ClientServerShared.h"
#include <stdio.h>

#define FIRST_GUN_ICON		IDS_PULSERIFLE

#define ORDINANCE_X			296
#define ORDINANCE_Y			128

CPlayerInventory::CPlayerInventory()
{
	m_pClientDE = LTNULL;
	m_pClientShell = LTNULL;

	memset (m_bHaveGun, 0, GUN_MAX_NUMBER * sizeof (LTBOOL));
	memset (m_bHaveShogoLetter, 0, sizeof (LTBOOL) * 5);
	memset (m_hShogoLetter, 0, sizeof (HSURFACE) * 5);
	
	m_bAmmoChanged = LTFALSE;
	m_nNewAmmoType = 0;
	m_nNewAmmoAmount = 0;

	m_hAmmoCountFont = LTNULL;

	m_hCurrentMessage = LTNULL;
	m_cxCurrentMessage = 0;
	m_cyCurrentMessage = 0;

	m_fDisplayTimeLeft = 0.0f;

	m_hOrdinance = LTNULL;

	memset (m_hGunIcon, 0, GUN_MAX_NUMBER * sizeof (HSURFACE));
	memset (m_cxGunIcon, 0, GUN_MAX_NUMBER * sizeof (uint32));
	memset (m_cyGunIcon, 0, GUN_MAX_NUMBER * sizeof (uint32));

	memset (m_hGunName, 0, GUN_MAX_NUMBER * sizeof (HSURFACE));
	memset (m_hAmmoCount, 0, GUN_MAX_NUMBER * sizeof (HSURFACE));
}

CPlayerInventory::~CPlayerInventory()
{
	Term();
}

LTBOOL CPlayerInventory::Init (ILTClient* pClientDE, CRiotClientShell* pClientShell)
{
	if (!pClientDE || !pClientShell) return LTFALSE;

	// set the ILTClient and ClientShell pointers

	m_pClientDE = pClientDE;
	m_pClientShell = pClientShell;

	// Set up the font...

	HSTRING hstrFont = m_pClientDE->FormatString (IDS_INGAMEFONT);
	m_hAmmoCountFont = m_pClientDE->CreateFont (m_pClientDE->GetStringData(hstrFont),
						TextHelperGetIntValFromStringID(m_pClientDE, IDS_ORDINANCETEXTWIDTH, 6),
						TextHelperGetIntValFromStringID(m_pClientDE, IDS_ORDINANCETEXTHEIGHT, 13),
						LTFALSE, LTFALSE, LTTRUE);
	m_pClientDE->FreeString (hstrFont);

	// init the ordinance surfaces

	if (!InitSurfaces())
	{
		Term();
		return LTFALSE;
	}

	// load the shogo letter surfaces

	m_hShogoLetter[0] = m_pClientDE->CreateSurfaceFromBitmap ("interface\\Ord_s.pcx");	
	m_hShogoLetter[1] = m_pClientDE->CreateSurfaceFromBitmap ("interface\\Ord_h.pcx");
	m_hShogoLetter[2] = m_pClientDE->CreateSurfaceFromBitmap ("interface\\Ord_o1.pcx");
	m_hShogoLetter[3] = m_pClientDE->CreateSurfaceFromBitmap ("interface\\Ord_g.pcx");
	m_hShogoLetter[4] = m_pClientDE->CreateSurfaceFromBitmap ("interface\\Ord_o2.pcx");

	// load the gun icons

	for (int i = GUN_FIRST_ID; i < GUN_MAX_NUMBER; i++)
	{
		HSTRING hGunFilename = m_pClientDE->FormatString (FIRST_GUN_ICON + i);
		if (hGunFilename)
		{
			m_hGunIcon[i] = m_pClientDE->CreateSurfaceFromBitmap (const_cast<char *>(m_pClientDE->GetStringData (hGunFilename)));
			if (m_hGunIcon[i])
			{
				m_pClientDE->GetSurfaceDims (m_hGunIcon[i], &m_cxGunIcon[i], &m_cyGunIcon[i]);
			}

			m_pClientDE->FreeString (hGunFilename);
		}
	}

	// Set us up with the basics...

	m_bHaveGun[GUN_ENERGYBATON_ID] = LTTRUE;
	m_bHaveGun[GUN_ENERGYBLADE_ID] = LTTRUE;
	m_bHaveGun[GUN_KATANA_ID] = LTTRUE;
	m_bHaveGun[GUN_MONOKNIFE_ID] = LTTRUE;
	m_bHaveGun[GUN_TANTO_ID] = LTTRUE;

	return LTTRUE;
}

void CPlayerInventory::Term()
{
	if (!m_pClientDE) return;

	m_pClientShell = LTNULL;

	memset (m_bHaveGun, 0, GUN_MAX_NUMBER * sizeof (LTBOOL));

	for (int i = 0; i < 5; i++)
	{
		if (m_hShogoLetter[i]) 
		{
			m_pClientDE->DeleteSurface (m_hShogoLetter[i]);
			m_hShogoLetter[i] = LTNULL;
		}
	}

	if (m_hAmmoCountFont)
	{
		m_pClientDE->DeleteFont (m_hAmmoCountFont);
		m_hAmmoCountFont = LTNULL;
	}
	
	if (m_hCurrentMessage) m_pClientDE->DeleteSurface (m_hCurrentMessage);
	if (m_hOrdinance) m_pClientDE->DeleteSurface (m_hOrdinance);
	
	for (int i = GUN_FIRST_ID; i < GUN_MAX_NUMBER; i++)
	{
		if (m_hGunIcon[i]) m_pClientDE->DeleteSurface (m_hGunIcon[i]);
		if (m_hGunName[i]) m_pClientDE->DeleteSurface (m_hGunName[i]);
		if (m_hAmmoCount[i]) m_pClientDE->DeleteSurface (m_hAmmoCount[i]);
	}
	
	m_hCurrentMessage = LTNULL;
	m_hOrdinance = LTNULL;

	memset (m_hGunIcon, 0, GUN_MAX_NUMBER * sizeof (HSURFACE));
	memset (m_cxGunIcon, 0, GUN_MAX_NUMBER * sizeof (uint32));
	memset (m_cyGunIcon, 0, GUN_MAX_NUMBER * sizeof (uint32));

	memset (m_hGunName, 0, GUN_MAX_NUMBER * sizeof (HSURFACE));
	memset (m_hAmmoCount, 0, GUN_MAX_NUMBER * sizeof (HSURFACE));
	
	m_cxCurrentMessage = 0;
	m_cyCurrentMessage = 0;

	m_fDisplayTimeLeft = 0.0f;


	m_pClientDE = LTNULL;
}

void CPlayerInventory::Reset()
{
	if (!m_pClientDE) return;

	memset (m_bHaveGun, 0, GUN_MAX_NUMBER * sizeof (LTBOOL));
	memset (m_bHaveShogoLetter, 0, sizeof (LTBOOL) * 5);

	m_bHaveGun[GUN_ENERGYBATON_ID] = LTTRUE;
	m_bHaveGun[GUN_ENERGYBLADE_ID] = LTTRUE;
	m_bHaveGun[GUN_KATANA_ID] = LTTRUE;
	m_bHaveGun[GUN_MONOKNIFE_ID] = LTTRUE;
	m_bHaveGun[GUN_TANTO_ID] = LTTRUE;

	HLTCOLOR hOrangeColor = m_pClientDE->SetupColor1 (0.98f, 0.317647f, 0.0f, LTFALSE);
	HLTCOLOR hTransColor = m_pClientDE->SetupColor2(0.0f, 0.0f, 0.0f, LTTRUE);
	
	for (uint32 i = GUN_FIRST_ID; i < GUN_MAX_NUMBER; i++)
	{
		if (m_hAmmoCount[i]) m_pClientDE->DeleteSurface (m_hAmmoCount[i]);
		m_hAmmoCount[i] = LTNULL;

		if (!UsesAmmo((RiotWeaponId) i)) continue;

		m_hAmmoCount[i] = CTextHelper::CreateSurfaceFromString (m_pClientDE, m_hAmmoCountFont, "0", hOrangeColor);
		m_pClientDE->OptimizeSurface (m_hAmmoCount[i], hTransColor);
	}
}

void CPlayerInventory::Update()
{
	if (m_bAmmoChanged)
	{
		if (UsesAmmo((RiotWeaponId) m_nNewAmmoType))
		{
			HLTCOLOR hOrangeColor = m_pClientDE->SetupColor1 (0.98f, 0.317647f, 0.0f, LTFALSE);
			HLTCOLOR hTransColor = m_pClientDE->SetupColor2(0.0f, 0.0f, 0.0f, LTTRUE);
			
			if (m_hAmmoCount[m_nNewAmmoType])
			{
				m_pClientDE->DeleteSurface (m_hAmmoCount[m_nNewAmmoType]);
				m_hAmmoCount[m_nNewAmmoType] = LTNULL;
			}

			char str[16];
			itoa (m_nNewAmmoAmount, str, 10);
			m_hAmmoCount[m_nNewAmmoType] = CTextHelper::CreateSurfaceFromString (m_pClientDE, m_hAmmoCountFont, str, hOrangeColor);
			m_pClientDE->OptimizeSurface (m_hAmmoCount[m_nNewAmmoType], hTransColor);
		}

		m_bAmmoChanged = LTFALSE;
		m_nNewAmmoType = 0;
		m_nNewAmmoAmount = 0;
	}
}

void CPlayerInventory::GunPickup (uint8 nType, LTBOOL bDisplayMessage)
{
	if (!m_pClientDE) return;

	m_bHaveGun[nType] = LTTRUE;

	if (!bDisplayMessage) return;

	if (m_hCurrentMessage) m_pClientDE->DeleteSurface (m_hCurrentMessage);
	m_hCurrentMessage = LTNULL;

	// build the pickup string surface

	HSTRING hStr = m_pClientDE->FormatString (IDS_GUNPICKUP, GetWeaponString ((RiotWeaponId)nType));
	
	HLTCOLOR foreColor = m_pClientDE->SetupColor1 (1.0f, 1.0f, 1.0f, LTFALSE);
	HSURFACE hPickup = CTextHelper::CreateSurfaceFromString (m_pClientDE, m_hAmmoCountFont, m_pClientDE->GetStringData (hStr), foreColor);

	m_pClientDE->FreeString (hStr);

	uint32 nPickupWidth = 0;
	uint32 nPickupHeight = 0;
	m_pClientDE->GetSurfaceDims (hPickup, &nPickupWidth, &nPickupHeight);

	// now build the entire display surface

	m_cxCurrentMessage = nPickupWidth > m_cxGunIcon[nType] ? nPickupWidth : m_cxGunIcon[nType];
	m_cyCurrentMessage = nPickupHeight + m_cyGunIcon[nType] + 3;
	m_hCurrentMessage = m_pClientDE->CreateSurface (m_cxCurrentMessage, m_cyCurrentMessage);
	m_pClientDE->DrawSurfaceToSurface (m_hCurrentMessage, hPickup, LTNULL, 0, 0);
	if (m_hGunIcon[nType])
	{
		HLTCOLOR hTrans = m_pClientDE->SetupColor1 (1.0f, 0.0f, 0.0f, LTFALSE);
		m_pClientDE->DrawSurfaceToSurfaceTransparent (m_hCurrentMessage, m_hGunIcon[nType], LTNULL, (nPickupWidth - m_cxGunIcon[nType]) >> 1, nPickupHeight + 3, hTrans);
	}	
	m_pClientDE->DeleteSurface (hPickup);

	HLTCOLOR hTransColor = m_pClientDE->SetupColor1(0.0f, 0.0f, 0.0f, LTTRUE);
	m_pClientDE->OptimizeSurface (m_hCurrentMessage, hTransColor);

	// keep around for 5 seconds

	m_fDisplayTimeLeft = 5.0f;
}

void CPlayerInventory::UpdateAmmo (uint32 nType, uint32 nAmount)
{
	if (!m_pClientDE) return;

	if (!m_hAmmoCount[nType]) return;

	if (UsesAmmo((RiotWeaponId) nType))
	{
		HLTCOLOR hOrangeColor = m_pClientDE->SetupColor1 (0.98f, 0.317647f, 0.0f, LTFALSE);
		HLTCOLOR hTransColor = m_pClientDE->SetupColor2(0.0f, 0.0f, 0.0f, LTTRUE);
			
		if (m_hAmmoCount[nType])
		{
			m_pClientDE->DeleteSurface (m_hAmmoCount[nType]);
			m_hAmmoCount[nType] = LTNULL;
		}

		char str[16];
		itoa (nAmount, str, 10);
		m_hAmmoCount[nType] = CTextHelper::CreateSurfaceFromString (m_pClientDE, m_hAmmoCountFont, str, hOrangeColor);
		m_pClientDE->OptimizeSurface (m_hAmmoCount[nType], hTransColor);
	}
	return;

	// if we've already received notification of an ammo change for
	// a different weapon, force that update before we save the new info

	if (m_bAmmoChanged && nType != m_nNewAmmoType)
	{
		Update();
	}

	m_bAmmoChanged = LTTRUE;
	m_nNewAmmoType = nType;
	m_nNewAmmoAmount = nAmount;
}

void CPlayerInventory::ShogoPowerupPickup (PickupItemType ePowerup)
{
	if (ePowerup == PIT_SHOGO_S)
	{
		m_bHaveShogoLetter[0] = LTTRUE;
	}
	else if (ePowerup == PIT_SHOGO_H)
	{
		m_bHaveShogoLetter[1] = LTTRUE;
	}
	else if (ePowerup == PIT_SHOGO_O && m_bHaveShogoLetter[4])
	{
		m_bHaveShogoLetter[2] = LTTRUE;
	}
	else if (ePowerup == PIT_SHOGO_O)
	{
		m_bHaveShogoLetter[4] = LTTRUE;
	}
	else if (ePowerup == PIT_SHOGO_G)
	{
		m_bHaveShogoLetter[3] = LTTRUE;
	}
}

void CPlayerInventory::ShogoPowerupClear()
{
	memset (m_bHaveShogoLetter, 0, sizeof (LTBOOL) * 5);
}

void CPlayerInventory::Draw (LTBOOL bDrawOrdinance)
{
	if (!m_pClientDE || !m_pClientShell) return;

	// get the screen surface and dims

	HSURFACE hScreen = m_pClientDE->GetScreenSurface();
	uint32 nScreenWidth = 0;
	uint32 nScreenHeight = 0;
	m_pClientDE->GetSurfaceDims (hScreen, &nScreenWidth, &nScreenHeight);

	// see if we're drawing a pickup message

	if (m_hCurrentMessage)
	{
		// draw the display message

		m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_hCurrentMessage, LTNULL, (nScreenWidth - m_cxCurrentMessage) >> 1, nScreenHeight - m_cyCurrentMessage - 10, LTNULL);

		// see if we should remove it now

		m_fDisplayTimeLeft -= m_pClientDE->GetFrameTime();
		if (m_fDisplayTimeLeft < 0.0f)
		{
			m_pClientDE->DeleteSurface (m_hCurrentMessage);
			m_pClientShell->AddToClearScreenCount();
			m_hCurrentMessage = LTNULL;
			m_fDisplayTimeLeft = 0.0f;
		}
	}

	// see if we need to draw the ordinance display

	if (bDrawOrdinance)
	{
		uint32 nOrdWidth, nOrdHeight;
		m_pClientDE->GetSurfaceDims (m_hOrdinance, &nOrdWidth, &nOrdHeight);

		int nOriginX = (int) ((LTFLOAT)ORDINANCE_X * (((LTFLOAT)nScreenWidth - (LTFLOAT)nOrdWidth) / (640.0f - (LTFLOAT)nOrdWidth)));
		int nOriginY = (int) ((LTFLOAT)ORDINANCE_Y * (((LTFLOAT)nScreenHeight - (LTFLOAT)nOrdHeight) / (480.0f - (LTFLOAT)nOrdHeight)));;

		m_pClientDE->DrawSurfaceToSurfaceTransparent (hScreen, m_hOrdinance, LTNULL, nOriginX, nOriginY, LTNULL);

		for (uint32 i = 0; i < 5; i++)
		{
			if (m_bHaveShogoLetter[i])
			{
				int x = 0;
				char* strFilename = LTNULL;

				switch (i)
				{
					case 0:		x = 9;		break;
					case 1:		x = 63;		break;
					case 2:		x = 127;	break;
					case 3:		x = 192;	break;
					case 4:		x = 251;	break;
				}

				m_pClientDE->DrawSurfaceToSurface (hScreen, m_hShogoLetter[i], LTNULL, nOriginX + x, nOriginY + 116);
			}
		}
		
		HLTCOLOR hOrangeColor = m_pClientDE->SetupColor2(0.98f, 0.317647f, 0.0f, LTFALSE);
		//HLTCOLOR hGreyColor  = m_pClientDE->SetupColor1 (0.2549f, 0.28235f, 0.2941f, LTFALSE);
		HLTCOLOR hWhiteColor = m_pClientDE->SetupColor2 (1.0f, 1.0f, 1.0f, LTFALSE);
		
		uint32 min = GUN_NONE;
		uint32 max = GUN_NONE;

		if (m_pClientShell->IsOnFoot())
		{
			min = GUN_FIRSTONFOOT_ID;
			max = GUN_LASTONFOOT_ID;
		}
		else
		{
			min = GUN_FIRSTMECH_ID;
			max = GUN_LASTMECH_ID;
		}
		
		for (int i = min; i <= max; i++)
		{
			if (!m_hGunName[i]) continue;

			int y = 0;

			switch (i)
			{
				case GUN_TANTO_ID :					y = 56;			break;
				case GUN_COLT45_ID :				y = 69;			break;
				case GUN_SHOTGUN_ID	:				y = 82;			break;
				case GUN_MAC10_ID :					y = 95;			break;
				case GUN_ASSAULTRIFLE_ID :			y = 108;		break;
				case GUN_ENERGYGRENADE_ID :			y = 121;		break;
				case GUN_KATOGRENADE_ID :			y = 134;		break;
				case GUN_TOW_ID	:					y = 147;		break;
				case GUN_SQUEAKYTOY_ID :			y = 160;		break;

				case GUN_ENERGYBATON_ID:
				case GUN_ENERGYBLADE_ID:
				case GUN_KATANA_ID:
				case GUN_MONOKNIFE_ID:				y = 56;			break;
				case GUN_PULSERIFLE_ID :			y = 69;			break;
				case GUN_LASERCANNON_ID	:			y = 82;			break;
				case GUN_SPIDER_ID :				y = 95;			break;
				case GUN_BULLGUT_ID :				y = 108;		break;
				case GUN_SNIPERRIFLE_ID :			y = 121;		break;
				case GUN_JUGGERNAUT_ID :			y = 134;		break;
				case GUN_SHREDDER_ID :				y = 147;		break;
				case GUN_REDRIOT_ID :				y = 160;		break;
			}

			uint32 nCountWidth = 0;
			uint32 nCountHeight = 0;

			// Only display info if we have it...

			if (CanDrawGun((uint8)i))
			{
				uint8 nCurWeapon = m_pClientShell->GetPlayerStats()->GetCurWeapon();
				LTBOOL bCurGun    = (nCurWeapon == i);

				m_pClientDE->GetSurfaceDims(m_hAmmoCount[i], &nCountWidth, &nCountHeight);

				int nAmmoX = m_pClientShell->IsOnFoot() ? 188 : 128;

				m_pClientDE->DrawSurfaceSolidColor(hScreen, m_hGunName[i], LTNULL, 10 + nOriginX, y + 1 + nOriginY, LTNULL, bCurGun ? hWhiteColor : hOrangeColor);
				m_pClientDE->DrawSurfaceSolidColor(hScreen, m_hAmmoCount[i], LTNULL, 310 - nCountWidth + nOriginX, y + nOriginY, LTNULL, bCurGun ? hWhiteColor : hOrangeColor);
			}
		}
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::CanDrawGun
//
//	PURPOSE:	Determine if we should draw the current weapon info
//
// --------------------------------------------------------------------------- //

LTBOOL CPlayerInventory::CanDrawGun(uint8 nWeaponId)
{
	if (nWeaponId < GUN_FIRST_ID || 
		nWeaponId >= GUN_MAX_NUMBER) return LTFALSE;

	if (!m_bHaveGun[nWeaponId]) return LTFALSE;

	// Special case for melee weapons.  In mecha mode we have all
	// four mecha weapons, but we can only display the one associated
	// with our mecha...

	switch (nWeaponId)
	{
		case GUN_TANTO_ID:
		case GUN_ENERGYBATON_ID:
		case GUN_ENERGYBLADE_ID:
		case GUN_MONOKNIFE_ID:
		case GUN_KATANA_ID:
		{
			uint8 nId = GetWeaponId(COMMAND_ID_WEAPON_10, 
									m_pClientShell->GetPlayerMode());

			return (nId == nWeaponId);
		}
		break;

		default : break;
	}

	return LTTRUE;
}

LTBOOL CPlayerInventory::InitSurfaces()
{
	if (!m_pClientDE) return LTFALSE;

	// initialize the ordinance screen

	m_hOrdinance = m_pClientDE->CreateSurfaceFromBitmap ("interface\\ordinance.pcx");
	if (!m_hOrdinance) return LTFALSE;

	HSTRING hstrFont = m_pClientDE->FormatString (IDS_INGAMEFONT);
	FONT fontdef (const_cast<char *>(m_pClientDE->GetStringData(hstrFont)), 
				  TextHelperGetIntValFromStringID(m_pClientDE, IDS_ORDINANCETITLEWIDTH, 15),
				  TextHelperGetIntValFromStringID(m_pClientDE, IDS_ORDINANCETITLEHEIGHT, 30),
				  LTFALSE, LTFALSE, LTTRUE);
	m_pClientDE->FreeString (hstrFont);

	HLTCOLOR hOrangeColor = m_pClientDE->SetupColor1 (0.98f, 0.317647f, 0.0f, LTFALSE);
	HSURFACE hString = CTextHelper::CreateSurfaceFromString (m_pClientDE, &fontdef, IDS_ORDINANCE, hOrangeColor);
	if (!hString) return LTFALSE;

	uint32 nWidth = 0;
	uint32 nHeight = 0;
	m_pClientDE->GetSurfaceDims (hString, &nWidth, &nHeight);
	m_pClientDE->DrawSurfaceToSurfaceTransparent (m_hOrdinance, hString, LTNULL, 32 + (((286 - 32) - nWidth) >> 1), 9 + (((43 - 9) - nHeight) >> 1) - 5, LTNULL);
	m_pClientDE->DeleteSurface (hString);
	hString = LTNULL;

	fontdef.nWidth = 6;
	fontdef.nHeight = 13;
	fontdef.bBold = LTFALSE;

	hString = CTextHelper::CreateSurfaceFromString (m_pClientDE, &fontdef, IDS_WEAPONS, hOrangeColor);
	if (!hString) return LTFALSE;

	nWidth = nHeight = 0;
	m_pClientDE->GetSurfaceDims (hString, &nWidth, &nHeight);
	m_pClientDE->DrawSurfaceToSurfaceTransparent (m_hOrdinance, hString, LTNULL, 12, 38 + (((53 - 38) - nHeight) >> 1), LTNULL);
	m_pClientDE->DeleteSurface (hString);
	hString = LTNULL;

/*
	hString = CTextHelper::CreateSurfaceFromString (m_pClientDE, &fontdef, IDS_AMMUNITION, hOrangeColor);
	if (!hString) return LTFALSE;

	nWidth = nHeight = 0;
	m_pClientDE->GetSurfaceDims (hString, &nWidth, &nHeight);
	m_pClientDE->DrawSurfaceToSurfaceTransparent (m_hOrdinance, hString, LTNULL, 8 + (((312 - 8) - nWidth) >> 1), 38 + (((53 - 38) - nHeight) >> 1), LTNULL);
	m_pClientDE->DeleteSurface (hString);
	hString = LTNULL;
*/
	hString = CTextHelper::CreateSurfaceFromString (m_pClientDE, &fontdef, IDS_COUNT, hOrangeColor);
	if (!hString) return LTFALSE;

	nWidth = nHeight = 0;
	m_pClientDE->GetSurfaceDims (hString, &nWidth, &nHeight);
	m_pClientDE->DrawSurfaceToSurfaceTransparent (m_hOrdinance, hString, LTNULL, 308 - nWidth, 38 + (((53 - 38) - nHeight) >> 1), LTNULL);
	m_pClientDE->DeleteSurface (hString);
	hString = LTNULL;

	HLTCOLOR hTransColor = m_pClientDE->SetupColor1(0.0f, 0.0f, 0.0f, LTTRUE);
	m_pClientDE->OptimizeSurface (m_hOrdinance, hTransColor);

	// init the weapon name surfaces

	HLTCOLOR hWhiteColor = m_pClientDE->SetupColor2(1.0f, 1.0f, 1.0f, LTFALSE);

	for (uint32 i = GUN_FIRST_ID; i < GUN_MAX_NUMBER; i++)
	{
		if (i == GUN_UNUSED1_ID || i == GUN_UNUSED2_ID) continue;

		m_hGunName[i] = CTextHelper::CreateSurfaceFromString (m_pClientDE, &fontdef, GetWeaponString ((RiotWeaponId) i), hWhiteColor);
		if (!m_hGunName[i]) return LTFALSE;

		m_pClientDE->OptimizeSurface (m_hGunName[i], hTransColor);
	}

	// init the ammo count surfaces

	hOrangeColor = m_pClientDE->SetupColor2(0.98f, 0.317647f, 0.0f, LTFALSE);

	for (int i = GUN_FIRST_ID; i < GUN_MAX_NUMBER; i++)
	{
		if (!UsesAmmo((RiotWeaponId) i))
		{
			m_hAmmoCount[i] = m_pClientDE->CreateSurfaceFromBitmap ("Interface/Infinity.pcx");
			if (!m_hAmmoCount[i]) return LTFALSE;
			m_pClientDE->OptimizeSurface (m_hAmmoCount[i], hTransColor);
		}
		else
		{
			m_hAmmoCount[i] = CTextHelper::CreateSurfaceFromString (m_pClientDE, m_hAmmoCountFont, IDS_ZEROCOUNT, hOrangeColor);
			if (!m_hAmmoCount[i]) return LTFALSE;
			m_pClientDE->OptimizeSurface (m_hAmmoCount[i], hTransColor);
		}
	}

	return LTTRUE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::Save
//
//	PURPOSE:	Save the player inventory info
//
// --------------------------------------------------------------------------- //

void CPlayerInventory::Save(ILTMessage_Write* hWrite)
{
	if (!m_pClientDE) return;

	hWrite->Writeuint32(m_nNewAmmoType);
	hWrite->Writeuint32(m_nNewAmmoAmount);
	hWrite->Writeuint8(m_bAmmoChanged);

	for (int i=0; i < GUN_MAX_NUMBER; i++)
	{
		hWrite->Writeuint8(m_bHaveGun[i]);
	}
	
	for (int i=0; i < 5; i++)
	{
		hWrite->Writeuint8(m_bHaveShogoLetter[i]);
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::Load
//
//	PURPOSE:	Load the player inventory info
//
// --------------------------------------------------------------------------- //

void CPlayerInventory::Load(ILTMessage_Read* hRead)
{
	if (!m_pClientDE || !m_pClientShell) return;

	m_nNewAmmoType		= hRead->Readuint32();
	m_nNewAmmoAmount	= hRead->Readuint32();
	m_bAmmoChanged		= (LTBOOL) hRead->Readuint8();

	for (int i=0; i < GUN_MAX_NUMBER; i++)
	{
		m_bHaveGun[i] = hRead->Readuint8();
	}
	
	for (int i=0; i < 5; i++)
	{
		m_bHaveShogoLetter[i] = (LTBOOL)hRead->Readuint8();
	}

	// update the ammo counts

	for (int i=0; i < GUN_MAX_NUMBER; i++)
	{
		if (m_bHaveGun[i])
		{
			UpdateAmmo (i, m_pClientShell->GetPlayerStats()->GetAmmoCount(i));
		}
	}
}
