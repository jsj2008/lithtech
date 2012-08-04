// ----------------------------------------------------------------------- //
//
// MODULE  : PickupItemFX.cpp
//
// PURPOSE : PickupItem - Implementation
//
// CREATED : 8/20/98
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "PickupItemFX.h"
#include "NavMarkerFX.h"
#include "iltclient.h"
#include "ClientUtilities.h"
#include "ClientServerShared.h"
#include "GameClientShell.h"
#include "SFXMsgIds.h"

extern CGameClientShell* g_pGameClientShell;

CPickupItemFX::CPickupItemFX()
{
	m_nTeamId = INVALID_TEAM;
	m_ePickupItemType = kPickupItemType_Unknown;
	m_hRecord = NULL;
	m_bTakesInventorySlot = false;
	m_bLocked = false;
	m_hNavMarker = NULL;
}

CPickupItemFX::~CPickupItemFX()
{
	if( m_linkClientFX.IsValid() )
	{
		g_pGameClientShell->GetSimulationTimeClientFXMgr().ShutdownClientFX( &m_linkClientFX );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPickupItemFX::Init
//
//	PURPOSE:	Init the object from a message.
//
// ----------------------------------------------------------------------- //

bool CPickupItemFX::Init(HLOCALOBJ hServObj, ILTMessage_Read *pMsg)
{
	if (!CSpecialFX::Init(hServObj, pMsg)) return false;
	if (!pMsg) return false;

	PICKUPITEMCREATESTRUCT pickupitem;
	pickupitem.hServerObj = hServObj;
	pickupitem.Read( pMsg );

	return Init(&pickupitem);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPickupItemFX::Init
//
//	PURPOSE:	Init the fx from struct
//
// ----------------------------------------------------------------------- //

bool CPickupItemFX::Init(SFXCREATESTRUCT* psfxCreateStruct)
{
	if (!CSpecialFX::Init(psfxCreateStruct)) return false;

	PICKUPITEMCREATESTRUCT* pPICS = (PICKUPITEMCREATESTRUCT*)psfxCreateStruct;

	m_nTeamId			= pPICS->m_nTeamId;
	m_ePickupItemType	= pPICS->m_ePickupItemType;
	m_hRecord			= pPICS->m_hRecord;
	m_hNavMarker		= pPICS->m_hNavMarker;

	m_bTakesInventorySlot = false;
	//if it's a weapon, see if it takes an inventory slot...
	if (m_ePickupItemType == kPickupItemType_Weapon )
	{
		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData( m_hRecord, !USE_AI_DATA );
		if( hWpnData )
		{
			m_bTakesInventorySlot = g_pWeaponDB->GetBool(hWpnData,WDB_WEAPON_bTakesInventorySlot);
		}

	}

	m_bLocked = pPICS->m_bLocked;


	// Shutdown any currently playing FX...

	if( m_linkClientFX.IsValid() )
	{
		g_pGameClientShell->GetSimulationTimeClientFXMgr().ShutdownClientFX( &m_linkClientFX );
	}


	if( !pPICS->m_sClientFX.empty() )
	{
		CLIENTFX_CREATESTRUCT fxInit( pPICS->m_sClientFX.c_str(), FXFLAG_LOOP, m_hServerObject );
		g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( &m_linkClientFX, fxInit, true );
	}

	if( !pPICS->m_sDroppedClientFX.empty() )
	{
		CLIENTFX_CREATESTRUCT fxInit( pPICS->m_sDroppedClientFX.c_str(), 0, m_hServerObject );
		g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( NULL, fxInit, true );
	}

	if (m_hNavMarker)
	{
		CNavMarkerFX* const pFX = (CNavMarkerFX*)g_pGameClientShell->GetSFXMgr()->FindSpecialFX(SFX_NAVMARKER_ID, m_hNavMarker);
		pFX->SetTarget(m_hServerObject);
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPickupItemFX::CreateObject
//
//	PURPOSE:	Create object associated the fx
//
// ----------------------------------------------------------------------- //

bool CPickupItemFX::CreateObject(ILTClient *pClientDE)
{
	bool bRet = CSpecialFX::CreateObject(pClientDE);
	if (!bRet) return bRet;

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPickupItemFX::Update
//
//	PURPOSE:	Update the pickupitem
//
// ----------------------------------------------------------------------- //

bool CPickupItemFX::Update()
{
	if (!m_pClientDE || m_bWantRemove || !m_hServerObject) return false;

	// If we have a ClientFX that is playing hide or show it based on the serverobject...

	uint32 dwFlags;
	g_pCommonLT->GetObjectFlags( m_hServerObject, OFT_Flags, dwFlags );

	if( m_linkClientFX.IsValid() )
	{
		if( dwFlags & FLAG_VISIBLE )
		{
			m_linkClientFX.GetInstance()->Show();
		}
		else
		{
			m_linkClientFX.GetInstance()->Hide();
		}
	}

	// If item is not rayhit, then don't tell object detector about it, since it's waiting to respawn.
	if( !( dwFlags & FLAG_RAYHIT ) || !( dwFlags & FLAG_VISIBLE ))
	{
		if( m_iObjectDetectorLink.IsRegistered( ))
		{
			// Unregister with objectdetector.
			g_pPlayerMgr->GetPickupObjectDetector( ).ReleaseLink( m_iObjectDetectorLink );
		}
	}
	// Make sure objectdetector knows about this item.
	else
	{
		if( !m_iObjectDetectorLink.IsRegistered( ))
		{
			// Register with objectdetector.
			g_pPlayerMgr->GetPickupObjectDetector( ).RegisterObject( m_iObjectDetectorLink, m_hServerObject, this );
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPickupItemFX::OnServerMessage
//
//	PURPOSE:	Handle recieving a message from the server...
//
// ----------------------------------------------------------------------- //

bool CPickupItemFX::OnServerMessage( ILTMessage_Read *pMsg )
{
	if( !CSpecialFX::OnServerMessage( pMsg ))
		return false;

	uint8 nMsgId = pMsg->Readuint8();

	switch( nMsgId )
	{
	case PUFX_CLIENTFX:
		{
			char szClientFX[256] = {0};

			pMsg->ReadString( szClientFX, ARRAY_LEN( szClientFX ));

			// Shutdown any currently playing FX...

			if( m_linkClientFX.IsValid() )
			{
				g_pGameClientShell->GetSimulationTimeClientFXMgr().ShutdownClientFX( &m_linkClientFX );
			}


			if( szClientFX[0] )
			{
				CLIENTFX_CREATESTRUCT fxInit( szClientFX, FXFLAG_LOOP, m_hServerObject );
				g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX( &m_linkClientFX, fxInit, true );
			}
		}
		break;
	case PUFX_ENDFX:
		{
			char szClientFX[256] = {0};

			pMsg->ReadString( szClientFX, ARRAY_LEN( szClientFX ));

			// Shutdown any currently playing FX...

			if( m_linkClientFX.IsValid() )
			{
				g_pGameClientShell->GetSimulationTimeClientFXMgr().ShutdownClientFX( &m_linkClientFX );
			}


			if( szClientFX[0] )
			{
				CLIENTFX_CREATESTRUCT fxInit( szClientFX, 0, m_hServerObject );
				g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX(NULL, fxInit, true );
			}
		}
		break;

	case PUFX_DROPPEDCLIENTFX:
		{
			char szClientFX[256] = {0};

			pMsg->ReadString( szClientFX, ARRAY_LEN( szClientFX ));
			if( szClientFX[0] )
			{
				CLIENTFX_CREATESTRUCT fxInit( szClientFX, 0, m_hServerObject );
				g_pGameClientShell->GetSimulationTimeClientFXMgr().CreateClientFX(NULL, fxInit, true );
			}
		}
		break;

	case PUFX_TEAMID:
		{
			m_nTeamId = pMsg->Readuint8( );
		}
		break;

	case PUFX_LOCKED:
		{
			m_bLocked = pMsg->Readbool();
		}
		break;

	case PUFX_RESPAWN:
		{
			HRECORD hSound = NULL;
			switch( m_ePickupItemType )
			{
			case kPickupItemType_AmmoBox:
				hSound		= g_pSoundDB->GetSoundDBRecord("AmmoBoxRespawn");
				break;
			case kPickupItemType_Gear:
				hSound		= g_pWeaponDB->GetRecordLink( m_hRecord, WDB_GEAR_rRespawnSnd );
				break;
			case kPickupItemType_Weapon:
				hSound		= g_pSoundDB->GetSoundDBRecord("WeaponItemRespawn");
				break;
			}

			if( hSound )
			{
				LTVector vPos;
				g_pLTClient->GetObjectPos( GetServerObj( ), &vPos );
				g_pClientSoundMgr->PlayDBSoundFromPos( vPos, hSound );
			}
		}
		break;


		case PUFX_PICKEDUP:
		{
			HRECORD hSound = NULL;
			switch( m_ePickupItemType )
			{
				case kPickupItemType_AmmoBox:
					hSound		= g_pSoundDB->GetSoundDBRecord("AmmoBoxPickup");
					break;
				case kPickupItemType_Gear:
					hSound		= g_pWeaponDB->GetRecordLink( m_hRecord, WDB_GEAR_rPickupSnd );
					break;
				case kPickupItemType_Weapon:
					hSound		= g_pSoundDB->GetSoundDBRecord("WeaponItemPickup");
					break;
			}

			if( hSound )
			{
				LTVector vPos;
				g_pLTClient->GetObjectPos( GetServerObj( ), &vPos );
				g_pClientSoundMgr->PlayDBSoundFromPos( vPos, hSound );
			}
		}
		break;

		default:
		break;
		
	}

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::GetName
//
//	PURPOSE:	Gets the name of the pickup
//
// ----------------------------------------------------------------------- //

bool CPickupItemFX::GetName( wchar_t* pszName, uint32 nNameLen ) const
{
	// Initialize out variables.
	if( pszName && nNameLen > 0 )
		pszName[0] = '\0';

	// Only applicable to weapons.
	if( !m_hRecord )
		return false;

	if( GetPickupItemType( ) == kPickupItemType_Weapon )
	{
		// Get the name.
		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData( m_hRecord, !USE_AI_DATA );
		if( !hWpnData )
			return false;

		const wchar_t* wszValue = g_pWeaponDB->GetWStringFromId( hWpnData, WDB_WEAPON_nShortNameId );
		LTStrCpy( pszName, wszValue, nNameLen );
	}
	else if( GetPickupItemType( ) == kPickupItemType_Gear )
	{
		const wchar_t* wszValue = g_pWeaponDB->GetWStringFromId( m_hRecord, WDB_GEAR_nNameId );
		LTStrCpy( pszName, wszValue, nNameLen );
	}
	else
	{
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerMgr::GetIcon
//
//	PURPOSE:	Gets the icon of the pickup
//
// ----------------------------------------------------------------------- //

char const* CPickupItemFX::GetIcon( ) const
{
	// Only applicable to weapons.
	if( !m_hRecord || GetPickupItemType( ) != kPickupItemType_Weapon )
		return NULL;

	// Get the name.
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData( m_hRecord, !USE_AI_DATA );
	if( !hWpnData )
		return NULL;

	return g_pWeaponDB->GetString( hWpnData, WDB_WEAPON_sSilhouetteIcon );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPickupItemFX::IsMustSwap
//
//	PURPOSE:	Does the player have to swap to pick the weapon up
//
// ----------------------------------------------------------------------- //

bool CPickupItemFX::IsMustSwap( ) const
{
	if (IsLocked()) return false;
	if (GetPickupItemType( ) != kPickupItemType_Weapon ) return false;
#ifdef PROJECT_DARK
	//!!ARL: We only have one inventory slot so we always have to swap,
	// but if we're holding a pipe and want to pick up a different one,
	// we should be able to do that.  There is no concept of running
	// over it to pick up its ammo.
	return ( m_bTakesInventorySlot && !g_pPlayerStats->HasEmptySlot());
#else

	if (m_bTakesInventorySlot && !g_pPlayerStats->HasEmptySlot())
	{
		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData( m_hRecord, !USE_AI_DATA );
		if( !hWpnData )
			return false;

		HWEAPON hDualWeapon = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rDualWeapon );
		if( hDualWeapon )
		{
			return (!g_pPlayerStats->HaveWeapon(hDualWeapon ) && !g_pPlayerStats->HaveWeapon( m_hRecord ));
		}
		else
		{
			return (!g_pPlayerStats->HaveWeapon( m_hRecord ) );
		}
	}

	return false;
	
#endif
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPickupItemFX::CanPickup
//
//	PURPOSE:	Can the item be picked up
//
// ----------------------------------------------------------------------- //

bool CPickupItemFX::CanPickup( ) const
{
	if (IsLocked())
		return false;

	if (GetPickupItemType( ) != kPickupItemType_Weapon ) 
		return true;

	if (m_hRecord == g_pWeaponDB->GetUnarmedRecord())
		return false;

	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData( m_hRecord, !USE_AI_DATA );
	if( !hWpnData )
		return false;

#if !defined(PROJECT_DARK)
	if (g_pWeaponDB->GetBool( hWpnData, WDB_WEAPON_bInfiniteAmmo ))
		return false;

	HAMMO hDefaultAmmo = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rAmmoName );
	uint32 nCount = g_pPlayerStats->GetAmmoCount(hDefaultAmmo);
	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(hDefaultAmmo,!USE_AI_DATA);
	uint32 maxAmmo = g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nMaxAmount );

	return nCount < maxAmmo;
#else

	// For the DARK, the player needs to be able to pick up weapons
	// regardless of the whether the player actually benefits by
	// picking it up.
	return true;

#endif
	
}
