
// ----------------------------------------------------------------------- //
//
// MODULE  :	PlayerInventory.cpp
//
// PURPOSE :	Provides a layer to manage the player's limited weapon capacity
//
// CREATED :	11/21/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //


#include "Stdafx.h"
#include "PlayerInventory.h"
#include "PickupItem.h"
#include "PlayerObj.h"
#include "ObjectMsgs.h"
#include "WeaponItems.h"
#include "AmmoBox.h"
#include "WeaponDB.h"
#include "ServerMissionMgr.h"
#include "Weapon.h"
#include "Spawner.h"
#include "SkillDefs.h"
#include <vector>
#include "GameModeMgr.h"
#include "ServerDB.h"
#include "SlowMoDB.h"
#include "NavMarker.h"
#include "NavMarkerTypeDB.h"

static VarTrack s_vtWeaponDropImpulse;
static VarTrack s_vtWeaponDropVel;
static VarTrack s_vtWeaponDropLockout;

CPlayerInventory::CPlayerInventory() :
	m_pPlayer(NULL),
	m_pArsenal(NULL),
	m_hUnarmed(NULL),
	m_nWeaponCapacity(0),
	m_fSlowMoCharge(0.0f),
	m_fSlowMoMaxCharge(0.0f),
	m_bSlowMoPlayerControlled(false),
	m_fSlowMoRechargeRate(0.0f),
	m_hSlowMoRecord( NULL ),
	m_hSlowMoGearObject( NULL ),
	m_hSlowMoGearRecord( NULL ),
	m_bUpdateCharge( true ),
	m_hSlowMoNavMarker( NULL ),
	m_hEnemySlowMoNavMarker( NULL ),
	m_hFlagNavMarker( NULL )
{
}

CPlayerInventory::~CPlayerInventory()
{
	RemoveSlowMoNavMarker( );

	//remove any old nav marker
	if( m_hFlagNavMarker )
	{
		g_pLTServer->RemoveObject( m_hFlagNavMarker );
		m_hFlagNavMarker = NULL;
	}



	// Respawn the slowmo gearitem if we cached one...
	if( m_hSlowMoGearObject )
	{
		g_pCmdMgr->QueueMessage( NULL, m_hSlowMoGearObject, "RESPAWN" );
		m_hSlowMoGearObject = NULL;
	}
}

void CPlayerInventory::Init(CPlayerObj* pPlayer)
{
	LTASSERT(pPlayer,"Player inventory initialized without a player.");
	m_pPlayer = pPlayer;
	if (pPlayer)
		m_pArsenal = pPlayer->GetArsenal();

	//set up our gear array
	uint8 nNumGear = g_pWeaponDB->GetNumGear();
	m_vecGearCount.clear();
	m_vecGearCount.insert(m_vecGearCount.begin(),nNumGear,0);

	m_vecPriorities.clear();
	m_vecPriorities.reserve(g_pWeaponDB->GetNumDefaultWeaponPriorities());
	for (uint8 nWpn = 0; nWpn < g_pWeaponDB->GetNumDefaultWeaponPriorities(); ++nWpn)
	{
		HWEAPON hWpn = g_pWeaponDB->GetWeaponFromDefaultPriority(nWpn);
		m_vecPriorities.push_back(hWpn);
	}

	if( IsMultiplayerGameServer( ))
	{
		m_hSlowMoRecord = g_pServerDB->GetPlayerRecordLink( SrvDB_rMPSlowMo );
	}
	else
	{
		m_hSlowMoRecord = g_pServerDB->GetPlayerRecordLink(SrvDB_rSlowMo);
	}

	m_fSlowMoCharge = 0.0f;
	m_fSlowMoMaxCharge = GETCATRECORDATTRIB( SlowMo, m_hSlowMoRecord, Period );
	m_bSlowMoPlayerControlled = false;
	m_fSlowMoRechargeRate = GETCATRECORDATTRIB( SlowMo, m_hSlowMoRecord, RechargeRate );
	m_bUpdateCharge = true;

	SendSlowMoValuesToClient( );

	if(!s_vtWeaponDropImpulse.IsInitted())
		s_vtWeaponDropImpulse.Init(g_pLTServer, "WeaponDropImpulse", NULL, g_pServerDB->GetPlayerFloat("WeaponDropImpulse"));
	if(!s_vtWeaponDropVel.IsInitted())
		s_vtWeaponDropVel.Init(g_pLTServer, "WeaponDropVelocity", NULL, g_pServerDB->GetPlayerFloat("WeaponDropVelocity"));
	if(!s_vtWeaponDropLockout.IsInitted())
		s_vtWeaponDropLockout.Init(g_pLTServer, "WeaponDropLockoutTime", NULL, g_pServerDB->GetPlayerFloat("WeaponDropLockoutTime"));

}

void CPlayerInventory::Reset()
{
	m_hUnarmed = NULL;
	ClearWeaponSlots();
	m_pArsenal->Reset();

	SendSlowMoValuesToClient( );
}

void CPlayerInventory::Update()
{
	if (g_pGameServerShell->IsInSlowMo() && IsSlowMoPlayerControlled() && m_bUpdateCharge )
	{
		float fDischarge = GameTimeTimer::Instance().GetTimerElapsedS();
		m_fSlowMoCharge = LTMAX(0.0f,m_fSlowMoCharge - fDischarge);
	}
	else
	{
		float fRecharge = GameTimeTimer::Instance().GetTimerElapsedS() * m_fSlowMoRechargeRate;
		m_fSlowMoCharge = LTMIN(m_fSlowMoMaxCharge,m_fSlowMoCharge + fRecharge);
	}
}

void CPlayerInventory::RemoveWeapons()
{
	m_hUnarmed = NULL;
	ClearWeaponSlots();
	m_pArsenal->RemoveAllActiveWeapons( );
}

void CPlayerInventory::SetCapacity(uint8 nCap)
{
	//TODO: deal with setting weapon cpacity to less than current number held
	LTASSERT(nCap >= m_vecWeapons.size(),"Inventory Capacity set to less than current weapon count.");
	if (nCap < m_vecWeapons.size())
		return;

	if (nCap > m_nWeaponCapacity)
	{
		m_nWeaponCapacity = nCap;
		m_vecWeapons.resize(m_nWeaponCapacity,(HWEAPON)NULL);
	}

	SendWeaponCapacityToClient( );
}


void CPlayerInventory::SendWeaponCapacityToClient( )
{
	//notify client
	HCLIENT hClient = m_pPlayer->GetClient();
	if (hClient)
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
		cMsg.Writeuint8(IC_WEAPONCAP_ID);
		cMsg.Writeuint8(m_nWeaponCapacity);
		g_pLTServer->SendToClient(cMsg.Read(), hClient, MESSAGE_GUARANTEED);
	}
}

void CPlayerInventory::OnObtainClient( )
{
	SendWeaponCapacityToClient( );
	SendSlowMoValuesToClient( );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::HandlePickupMsg
//
//	PURPOSE:	Handle messages from pickup items
//
// ----------------------------------------------------------------------- //

uint32 CPlayerInventory::HandlePickupMsg(HOBJECT hSender, ILTMessage_Read *pMsg)
{
	bool bPickedUp = false;
	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();
	switch(messageID)
	{
	case MID_ADDWEAPON:
		{
			bPickedUp = PickupWeapon( hSender, pMsg );
		}
		break;

	case MID_AMMOBOX:
		{
			bPickedUp = PickupAmmoBox( hSender, pMsg );
		}
		break;

	case MID_ADDMOD:
		{
			bPickedUp = PickupMod( hSender, pMsg );
		}
		break;

	case MID_ADDGEAR:
		{
			bPickedUp = PickupGear( hSender, pMsg );
		}
		break;


	default : break;
	}

	if (!bPickedUp)
	{
		CAutoMessage cMsg;
		cMsg.Writeuint32(MID_PICKEDUP);
		cMsg.Writebool( false );
		cMsg.Writebool( false );
		g_pLTServer->SendToObject(cMsg.Read(), m_pPlayer->m_hObject, hSender, MESSAGE_GUARANTEED);

		return 0;
		
	}

	return 1;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::PickupWeapon
//
//	PURPOSE:	Try to pickup a weapon item
//
// ----------------------------------------------------------------------- //

bool CPlayerInventory::PickupWeapon( HOBJECT hSender, ILTMessage_Read *pMsg )
{
	HWEAPON	hWeapon	= pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetWeaponsCategory() );
	HAMMO	hAmmo	= pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetAmmoCategory() );
	int		nAmmo	= pMsg->Readint32();
	bool	bWillRespawn = pMsg->Readbool();
	uint32	nHealth = pMsg->Readuint32();
	bool	bWasPlaced = pMsg->Readbool();
	HOBJECT hDroppedBy = pMsg->ReadObject();

	if( !hWeapon || !hAmmo )
		return false;

	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);
	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData( hAmmo, !USE_AI_DATA );

	bool bPickedUp = false;
	bool bTakesInventorySlot = g_pWeaponDB->GetBool(hWpnData,WDB_WEAPON_bTakesInventorySlot);
	bool bIsAmmo = g_pWeaponDB->GetBool(hWpnData,WDB_WEAPON_bIsAmmo);
	bool bHasWeapon = HaveWeapon(hWeapon);
	bool bWeaponsStay = IsMultiplayerGameServer( ) && GameModeMgr::Instance( ).m_grbWeaponsStay;

	if (hDroppedBy == m_pPlayer->m_hObject)
	{
		PickupItem *pPickupItem = dynamic_cast<PickupItem*>(g_pLTServer->HandleToObject( hSender ));
		if (pPickupItem)
		{
			double fElapsedTime = SimulationTimer::Instance().GetTimerAccumulatedS() - pPickupItem->DropTime();
			if (fElapsedTime < s_vtWeaponDropLockout.GetFloat())
			{
				return false;
			}
			
		}

		
	}

	// In multiplayer the weapon should already have the correct amount...
	// If it was dropped by this player, the amount will also be set correctly already
	if( !IsMultiplayerGameServer( )  && (hDroppedBy != m_pPlayer->m_hObject))
	{
		// In singleplayer use the initial amount if they do not have the weapon already and the weapon was placed
		//  or dropped by someone else
		if (!bHasWeapon)
		{
			nAmmo = g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nPickupInitialAmount );
		}
		else if (!bWasPlaced) 
		{
			//	use	supplemental ammo amount if the player already has the weapon and the weapon was dropped (i.e. not placed by LD)
			nAmmo = g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nPickupSupplementalAmount );
		}
		
	}

	// Does this weapon support dual weapons...
	HWEAPON hDualWeapon = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rDualWeapon );
	if( hDualWeapon )
	{
		bool bHasDualVersion = HaveWeapon(hDualWeapon);
		if( bHasWeapon )
		{
			if( bHasDualVersion )
			{
				LTERROR( "Dual can only be aquired from picking up two single weaopns" );
				return false;
			}
			
			// Keep the current ammo amount...
			nAmmo += m_pArsenal->GetAmmoCount( hAmmo );

			// Remove single version of the weapon...
			// NOTE: This will free a weapon slot so there is room to acquire the new dual version below.
			RemoveWeapon( hWeapon, false );
			
			// Acquire the dual weapon...
			hWeapon = hDualWeapon;
			bHasWeapon = false;
		}
		else if( bHasDualVersion )
		{
			// If we don't have the weapon, but we do have it's dual version, just add the ammo
			hWeapon = hDualWeapon;
			bHasWeapon = true;
		}
	}

	if (bHasWeapon && bWeaponsStay && bWillRespawn)
		return false;

	//if we have too many weapons, or it doesn't take a slot
	uint8 nSlot = GetWeaponSlot(hWeapon);
	if (nSlot == WDB_INVALID_WEAPON_INDEX)
		nSlot = FindFirstEmptySlot();
	if ( bHasWeapon || nSlot != WDB_INVALID_WEAPON_INDEX || !bTakesInventorySlot)
	{
		bPickedUp = m_pArsenal->AddWeapon( hSender, hWeapon, hAmmo, nAmmo, nHealth, nSlot );

		//if this is a newly acquired weapon...
		if (bPickedUp && bTakesInventorySlot && !bHasWeapon)
		{
			m_vecWeapons[nSlot] = hWeapon;			
		}
			
	}

	if( bPickedUp)
	{
		// Tell weapon powerup it was picked up...
		CAutoMessage cMsg;
		cMsg.Writeuint32(MID_PICKEDUP);
		cMsg.Writebool( true );
		cMsg.Writebool( bWeaponsStay && bWillRespawn && !bIsAmmo );
		g_pLTServer->SendToObject(cMsg.Read(), m_pPlayer->m_hObject, hSender, MESSAGE_GUARANTEED);

		// Tell the client whether this weapon is weak (eg: hud indicator)
		uint32 nWarnHealth = g_pWeaponDB->GetInt32(hWpnData, WDB_WEAPON_nWarnHealth);
		if( nWarnHealth > 0 )
		{
			HCLIENT hClient = m_pPlayer->GetClient();
			if( hClient )
			{
				CAutoMessage cMsg;
				cMsg.Writeuint8( MID_WEAPON_BREAK_WARN );
				cMsg.WriteDatabaseRecord( g_pLTDatabase, hWeapon );
				cMsg.Writebool(nHealth <= nWarnHealth);
				g_pLTServer->SendToClient( cMsg.Read(), hClient, MESSAGE_GUARANTEED );
			}
		}

		//see if there's another weapon we're supposed to get at the same time...
		HWEAPON hLinkedWeapon = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rLinkedWeapon );
		if( hLinkedWeapon && !m_pArsenal->HasWeapon(hLinkedWeapon) )
		{
			AcquireWeapon(hLinkedWeapon,NULL,-1,true);
		}		
	}

	return bPickedUp;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::PickupAmmoBox
//
//	PURPOSE:	Collect ammo from a box
//
// ----------------------------------------------------------------------- //

bool CPlayerInventory::PickupAmmoBox( HOBJECT hSender, ILTMessage_Read *pMsg )
{
	uint8	nAmmoIdsLeft = 0;

	uint8	nNumUniqueAmmo   = pMsg->Readuint8();
	HAMMO	hAmmo[AB_MAX_TYPES];
	int32	nAmmo[AB_MAX_TYPES];
	int32	nTaken[AB_MAX_TYPES];

	bool bTookAmmo = false;

	uint8 i;
	for( i = 0; i < nNumUniqueAmmo; ++i )
	{
		hAmmo[i]	= pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetAmmoCategory() );
		nAmmo[i]	= pMsg->Readuint32();
		nTaken[i]	= 0;

		
		if( hAmmo[i] )
		{
			//verify that we have a weapon that uses this ammo
			if (m_pArsenal->CanUseAmmo(hAmmo[i]))
			{
				nTaken[i] = m_pArsenal->AddAmmo( hAmmo[i], nAmmo[i] );
				HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(hAmmo[i],false);
				if( g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nSelectionAmount ) >= 1000 )
					nTaken[i] = nAmmo[i];
			}

			if( nTaken[i] > 0 )
			{
				bTookAmmo = true;
			}

			if( nTaken[i] < nAmmo[i] )
			{
				++nAmmoIdsLeft;
			}

			nAmmo[i] -= nTaken[i];
		}
	}

	if( nAmmoIdsLeft )
	{
		// Tell powerup what is left in the box...If we actually
		// took something...
		if( bTookAmmo )
		{
			CAutoMessage cMsg;
			cMsg.Writeuint32(MID_AMMOBOX);
			cMsg.Writeuint8(nNumUniqueAmmo);
			for( i = 0; i < nNumUniqueAmmo; ++i )
			{
				cMsg.WriteDatabaseRecord( g_pLTDatabase, hAmmo[i] );
				cMsg.Writeint32( nAmmo[i] );
			}
			g_pLTServer->SendToObject( cMsg.Read(), m_pPlayer->m_hObject, hSender, MESSAGE_GUARANTEED );
		}
	}  
	else
	{
		// Tell ammo powerup it was picked up...
		CAutoMessage cMsg;
		cMsg.Writeuint32(MID_PICKEDUP);
		cMsg.Writebool( true );
		cMsg.Writebool( false );
		g_pLTServer->SendToObject(cMsg.Read(), m_pPlayer->m_hObject, hSender, MESSAGE_GUARANTEED);
	}



	// Send the appropriate message to the client...
	if( !m_pPlayer )
		return false;

	HCLIENT hClient = m_pPlayer->GetClient();
	if( hClient )
	{
		for( i = 0; i < nNumUniqueAmmo; ++i )
		{
			if( hAmmo[i] )
			{
				if( !AddIsAmmoWeapon( hAmmo[i], nTaken[i] ))
				{
					// Normal method of picking up ammo...
					if (nTaken[i])
					{
						CAutoMessage cMsg;
						cMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
						cMsg.Writeuint8(IC_WEAPON_PICKUP_ID);
						cMsg.WriteDatabaseRecord( g_pLTDatabase, NULL );
						cMsg.WriteDatabaseRecord( g_pLTDatabase, hAmmo[i] );
						cMsg.Writeuint32(m_pArsenal->GetAmmoCount( hAmmo[i] ));
						cMsg.Writeuint8( WDB_INVALID_WEAPON_INDEX );
						g_pLTServer->SendToClient(cMsg.Read(), hClient, MESSAGE_GUARANTEED);
					}
				}
			}
		}
	}

	return true;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::AddIsAmmoWeapon
//
//	PURPOSE:	Add an IsAmmo type weapon if necessary...
//
// ----------------------------------------------------------------------- //

bool CPlayerInventory::AddIsAmmoWeapon(HAMMO hAmmo, uint32 nTaken )
{
	if( !hAmmo )
		return false;

	// If this ammo type is used by an IsAmmo weapon give us the weapon
	
	HWEAPON hWeapon = g_pWeaponDB->GetWeaponFromAmmo( hAmmo, !USE_AI_DATA );
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);

	if( hWeapon && g_pWeaponDB->GetBool( hWpnData, WDB_WEAPON_bIsAmmo ))
	{
		bool bAcquired = !m_pArsenal->HasWeapon(hWeapon);
		if (bAcquired)
		{
			m_pArsenal->ObtainWeapon(hWeapon, hAmmo, nTaken, true  );

			//see if there's another weapon we're supposed to get at the same time...
			HWEAPON hLinkedWeapon = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rLinkedWeapon );
			if( hLinkedWeapon && !m_pArsenal->HasWeapon(hLinkedWeapon) )
			{
				AcquireWeapon(hLinkedWeapon, NULL, -1, true);
			}		

			//notify client
			HCLIENT hClient = m_pPlayer->GetClient();
			if (hClient)
			{
				CAutoMessage cMsg;
				cMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
				cMsg.Writeuint8(IC_WEAPON_PICKUP_ID);
				cMsg.WriteDatabaseRecord( g_pLTDatabase, hWeapon );
				cMsg.WriteDatabaseRecord( g_pLTDatabase, hAmmo );
				cMsg.Writeuint32(m_pArsenal->GetAmmoCount( hAmmo ));
				cMsg.Writeuint8(GetWeaponSlot(hWeapon));
				g_pLTServer->SendToClient(cMsg.Read(), hClient, MESSAGE_GUARANTEED);
			}

		}
		return bAcquired;
	}

	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::PickupMod
//
//	PURPOSE:	Try to pickup a weapon mod
//
// ----------------------------------------------------------------------- //

bool CPlayerInventory::PickupMod( HOBJECT hSender, ILTMessage_Read *pMsg )
{
	bool bRet = false;

	HMOD hMod = pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetModsCategory() );

	if( hMod )
	{
		HWEAPON hWeapon = g_pWeaponDB->GetWeaponFromMod( hMod, !USE_AI_DATA );
	
		bool bPickedUp = false;
		if (m_pArsenal->HasWeapon(hWeapon))
		{
			// Check to see if we already have the mod...

			if( !m_pArsenal->HasMod(hWeapon, hMod ))
			{
				m_pArsenal->ObtainMod( hWeapon, hMod );
				bPickedUp = true;
			}

			bRet = true;
		}

		if (bPickedUp)
		{
			// Tell mod powerup if it was picked up...
			CAutoMessage cMsg;
			cMsg.Writeuint32(MID_PICKEDUP);
			cMsg.Writebool(true);
			cMsg.Writebool( false );
			g_pLTServer->SendToObject(cMsg.Read(), m_pPlayer->m_hObject, hSender, MESSAGE_GUARANTEED);

		}
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::HandleCheatWeapon
//
//	PURPOSE:	Do the single weapon cheat for the player
//
// ----------------------------------------------------------------------- //

bool CPlayerInventory::HandleCheatWeapon( HWEAPON hWeapon )
{
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);
	bool bTakesSlot = g_pWeaponDB->GetBool(hWpnData,WDB_WEAPON_bTakesInventorySlot);
	bool bHas = HaveWeapon(hWeapon);

	//if the weapon takes a slot and we don't have one...
	uint8 nSlot = FindFirstEmptySlot();
	if (bTakesSlot && !bHas && nSlot != WDB_INVALID_WEAPON_INDEX )
	{
		//pick a weapon to drop
		HWEAPON hWeaponToDrop = m_pArsenal->GetCurWeaponRecord();
		//we can't drop our fists...
		if (hWeaponToDrop == m_hUnarmed)
		{
			hWeaponToDrop = GetLowestPriorityWeapon();
		}

		LTVector vPos;
		LTRotation rRot;
		g_pLTServer->GetObjectPos(m_pPlayer->m_hObject,&vPos);
		g_pLTServer->GetObjectRotation(m_pPlayer->m_hObject,&rRot);	


		rRot.Rotate(rRot.Up( ),GetRandom(-1.0f,1.0f));
		vPos += (rRot.Up() * 50.0f); 

		DropWeapon( hWeaponToDrop, vPos, rRot, m_pPlayer->GetLastVelocity() );
	}

	return AcquireWeapon(hWeapon,NULL,10000,true);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::HandleCheatFullWeapon
//
//	PURPOSE:	Do the full weapon cheat for the player
//
// ----------------------------------------------------------------------- //

void CPlayerInventory::HandleCheatFullWeapon()
{
	uint8 nNumWeapons = g_pWeaponDB->GetNumPlayerWeapons();
	SetCapacity(nNumWeapons);
	for( uint8 nWeapon = 0; nWeapon < nNumWeapons; ++nWeapon )
	{
		HWEAPON hWeapon = g_pWeaponDB->GetPlayerWeapon( nWeapon );
		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);
		HWEAPON hDualWeapon = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rDualWeapon );
		if( hDualWeapon )
		{
			//if there's a dual version, skip the single version weapon
			continue;
		}
		AcquireWeapon(hWeapon,NULL,10000,true);
	}


	HandleCheatFullAmmo();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::HandleCheatFullAmmo
//
//	PURPOSE:	Do the full ammo cheat for the player
//
// ----------------------------------------------------------------------- //

void CPlayerInventory::HandleCheatFullAmmo()
{
	uint8 nNumAmmoTypes = g_pWeaponDB->GetNumAmmo();
	for( uint8 nAmmo = 0; nAmmo < nNumAmmoTypes; ++nAmmo )
	{
		HAMMO hAmmo = g_pWeaponDB->GetAmmoRecord( nAmmo );
		AcquireAmmo( hAmmo );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::HandleCheatFullMods
//
//	PURPOSE:	Do the full mod cheat for the player
//
// ----------------------------------------------------------------------- //

void CPlayerInventory::HandleCheatFullMods()
{
	uint8 nNumWeapons = g_pWeaponDB->GetNumWeapons();
	for( uint8 nWeapon = 0 ; nWeapon < nNumWeapons ; ++nWeapon )
	{
		HWEAPON hWeapon = g_pWeaponDB->GetWeaponRecord( nWeapon );
		if( hWeapon )
		{
			HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);
			uint8 nNumMods = ( uint8 )LTMIN( g_pWeaponDB->GetNumValues( hWpnData, WDB_WEAPON_rModName ), 255 );
			for( uint8 nMod = 0; nMod < nNumMods; ++nMod )
			{
				HMOD hMod = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rModName, nMod );
				if( hMod )
				{
					m_pArsenal->ObtainMod( hWeapon, hMod, true );
				}
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::HandleCheatFullGear
//
//	PURPOSE:	Do the full gear cheat for the player
//
// ----------------------------------------------------------------------- //

void CPlayerInventory::HandleCheatFullGear()
{
	uint8 nNumGear = g_pWeaponDB->GetNumGear();
	for( uint8 nGear = 0 ; nGear < nNumGear ; ++nGear )
	{
		HGEAR hGear = g_pWeaponDB->GetGearRecord( nGear );
		if( hGear )
		{
			AcquireGear(hGear);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::AcquireDefaultWeapon
//
//	PURPOSE:	Obtain the default weapon for the player
//
// ----------------------------------------------------------------------- //

bool CPlayerInventory::AcquireDefaultWeapon(HWEAPON hWeapon)
{
	if( !hWeapon || !g_pWeaponDB->IsPlayerWeapon( hWeapon ))
		return false;

	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);

	HAMMO hAmmo = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rAmmoName ) ;
	if( !hAmmo )
		return false;

	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(hAmmo,false);
	uint32 nAmount = g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nSelectionAmount );

	if( hWeapon == g_pWeaponDB->GetUnarmedRecord() )
	{
		m_hUnarmed = hWeapon;
	}

	return AcquireWeapon(hWeapon,hAmmo,nAmount,true);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::AcquireWeapon
//
//	PURPOSE:	Obtain the specified weapon for the player
//
// ----------------------------------------------------------------------- //

bool CPlayerInventory::AcquireWeapon(HWEAPON hWeapon, HAMMO hAmmo /* = NULL */, int32 nAmmo /* = -1 */, bool bNotifyClient /* = false */)
{
	if( !hWeapon || !g_pWeaponDB->IsPlayerWeapon( hWeapon ))
	{
		LTERROR( "Invalid weapon for player." );
		return false;
	}

//	DebugCPrint(0,"CPlayerInventory::AcquireWeapon - %s",g_pWeaponDB->GetRecordName(hWeapon));
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);

	//no ammo specified, try the default
	if (!hAmmo)
		hAmmo = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rAmmoName ) ;

	//still no ammo? abort
	if( !hAmmo )
	{
		LTERROR( "Invalid ammo." );
		return false;
	}

	bool bAcquired = false;
	bool bTakesSlot = g_pWeaponDB->GetBool(hWpnData,WDB_WEAPON_bTakesInventorySlot);
	bool bHas = HaveWeapon(hWeapon);

	//if we have a free slot, or we don't need one for this weapon...
	uint8 nSlot = FindFirstEmptySlot();
	if (nSlot != WDB_INVALID_WEAPON_INDEX || !bTakesSlot)
	{
		bAcquired = true;
		//if this is a newly acquired weapon...
		if (bTakesSlot && !bHas)
			m_vecWeapons[nSlot] = hWeapon;
		m_pArsenal->ObtainWeapon(hWeapon, hAmmo, nAmmo, bNotifyClient  );

		//see if there's another weapon we're supposed to get at the same time...
		HWEAPON hLinkedWeapon = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rLinkedWeapon );
		if( hLinkedWeapon && !m_pArsenal->HasWeapon(hLinkedWeapon) )
		{
			AcquireWeapon(hLinkedWeapon, NULL, -1, bNotifyClient);
		}		

	}

	return bAcquired;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::AcquireMod
//
//	PURPOSE:	Give the player the specified mod, fail if weapon not owned
//
// ----------------------------------------------------------------------- //

bool CPlayerInventory::AcquireMod( HMOD hMod , bool bDisplayMsg/* = true*/)
{
	bool bRet = false;

	if( hMod )
	{
		HWEAPON hWeapon = g_pWeaponDB->GetWeaponFromMod( hMod, !USE_AI_DATA );
		if( hWeapon )
		{
			bool bHas = HaveWeapon(hWeapon);
			if (bHas)
			{
				m_pArsenal->ObtainMod( hWeapon, hMod, true, bDisplayMsg );
				bRet = true;
			}
		}
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::AcquireAmmo
//
//	PURPOSE:	Do the specific ammo cheat for the player
//
// ----------------------------------------------------------------------- //

bool CPlayerInventory::AcquireAmmo( HAMMO hAmmo )
{
	//verify that we have a weapon that uses this ammo
	if( hAmmo && m_pArsenal->CanUseAmmo(hAmmo) )
	{
		m_pArsenal->SetAmmo( hAmmo );
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::AcquireGear
//
//	PURPOSE:	Give the player the specified Gear
//
// ----------------------------------------------------------------------- //

bool CPlayerInventory::AcquireGear(HGEAR hGear, uint8 nNum /* = -1  */)
{
	bool bAdded = false;

	if( hGear )
	{
		uint32 nIndex = g_pWeaponDB->GetRecordIndex(hGear);
		LTASSERT(nIndex < m_vecGearCount.size(),"Invalid gear index.");
		if (nIndex >= m_vecGearCount.size())
			return false;

		//check to see if we can carry more of this type of item
		//	if the limit is 0, then there is no limit
		uint8 nMax = g_pWeaponDB->GetInt32(hGear,WDB_GEAR_nMaxAmount);
		uint8 nAmount = ( nMax - m_vecGearCount[nIndex] );

		//figure out how many to get...
		nAmount = LTMIN(nNum,nAmount);
		
		m_vecGearCount[nIndex] += nAmount;

		//automatically activate items that are not counted
		if (nMax == 0)
		{
			bAdded = UseGear(hGear,false);
		}
		else
		{
			bAdded = (nAmount > 0);
		}

		if (bAdded)
		{
			HCLIENT hClient = m_pPlayer->GetClient();
			if (hClient)
			{
				CAutoMessage cMsg;
				cMsg.Writeuint8( MID_SFX_MESSAGE );
				cMsg.Writeuint8( SFX_CHARACTER_ID );
				cMsg.WriteObject( m_pPlayer->m_hObject );
				cMsg.WriteBits(CFX_USE_GEAR, FNumBitsExclusive<CFX_COUNT>::k_nValue );
				cMsg.WriteDatabaseRecord( g_pLTDatabase, hGear );
				cMsg.Writeuint8(kGearAdd);
				cMsg.Writeuint8( nAmount );
				g_pLTServer->SendToClient( cMsg.Read( ), NULL, MESSAGE_GUARANTEED );
			}
		}
	}

	return bAdded;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::DropWeapon
//
//	PURPOSE:	Remove the specified weapon from the players inventory and spawn a pickup item
//
// ----------------------------------------------------------------------- //

void CPlayerInventory::DropWeapon(HWEAPON hWeapon, LTVector const& vPos, LTRotation const& rRot, LTVector const& vVel, bool bChangeToUnarmed /* = true  */)
{
	if (!hWeapon || !m_pArsenal->IsValidWeaponRecord(hWeapon) || hWeapon == m_hUnarmed)
		return;
	
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);

	bool bTakesSlot = g_pWeaponDB->GetBool(hWpnData,WDB_WEAPON_bTakesInventorySlot);
	bool bHas = HaveWeapon(hWeapon);

	//if we don't have it... bail out early
	if (!bHas)
		return;

	CWeapon* pWeapon = m_pArsenal->GetWeapon(hWeapon);
	if( !pWeapon )
		return;

	HAMMO hAmmo = pWeapon->GetAmmoRecord();
	
	int32 nAmount = pWeapon->GetAmmoInClip( );
	if( !g_pWeaponDB->GetBool( hWpnData, WDB_WEAPON_bWhenDroppedGiveAmmoInClip ))
	{
		// Keep the total amount of ammo with the weapon...
		nAmount = m_pArsenal->GetAmmoCount( hAmmo );
	}

	//if we didn't remove the weapon, fail out...
	if (!RemoveWeapon( hWeapon, bChangeToUnarmed ))
		return;

	//if this is a ammo weapon, and we're out of ammo for it, do not spawn a pickup item...
	if (nAmount == 0 && g_pWeaponDB->GetBool( hWpnData, WDB_WEAPON_bIsAmmo ))
		return;


	float fLifetime = -1.0f;
	if (IsMultiplayerGameServer()) 
	{
		HRECORD hGlobalRec = g_pWeaponDB->GetGlobalRecord();
		fLifetime = float(g_pWeaponDB->GetInt32(hGlobalRec,WDB_GLOBAL_tDroppedWeaponLifeTime)) / 1000.0f;
	}

	//otherwise spawn the item...
	
	uint8 nNumSpawns = 1;
	HWEAPON hSingleWeapon = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rSingleWeapon );
	if( hSingleWeapon )
	{
		// Dropping a Dual weapon so two single weapons need to be respawned...
		hWeapon = hSingleWeapon;
		nNumSpawns = 2;
		nAmount /= 2;
	}

	// [BJL 11/04/04] - Optionally disable movement to the floor based on
	// the weapon properties.

	bool bMoveToFloor = g_pWeaponDB->GetBool( hWpnData, WDB_WEAPON_bDroppedMoveToFloor );

	char szSpawn[1024] = "";
	LTSNPrintF( szSpawn, ARRAY_LEN(szSpawn), "WeaponItem Gravity 0;MoveToFloor %d;AmmoAmount %d;WeaponType (%s);AmmoType (%s);MPRespawn 0; DMTouchPickup 1;LifeTime %0.2f;Health %d;Placed 0",
		bMoveToFloor, nAmount, g_pWeaponDB->GetRecordName( hWeapon ), g_pWeaponDB->GetRecordName( hAmmo ), fLifetime, pWeapon->GetHealth());

	LTVector vSpawnPos = vPos;
	uint8 nSpawn = 0;
	while( nNumSpawns > 0 )
	{ 

		BaseClass* pObj = SpawnObject(szSpawn, vSpawnPos, rRot);
		if (!pObj)
		{
			LTASSERT_PARAM1(0, "CPlayerInventory::DropWeapon : Failed to Spawn: %s", szSpawn);
			return;
		}

		WeaponItem* pWeaponItem = dynamic_cast< WeaponItem* >( pObj );
		if ( pWeaponItem )
		{
			LTVector vToss;
			//toss the first to the right, and the second to the left
			if (nSpawn == 0)
			{
				vToss = rRot.Right();
			}
			else
			{
				vToss = -rRot.Right();
			}
			LTVector vImpulse = rRot.Forward() + rRot.Up() + vToss;
			vImpulse *= s_vtWeaponDropImpulse.GetFloat();
			if (m_pPlayer->GetDestructible()->IsDead() && m_pPlayer->GetDestructible()->GetDeathType() == DT_EXPLODE)
			{
				vImpulse += m_pPlayer->GetDestructible()->GetDeathDir() * m_pPlayer->GetDestructible()->GetDeathImpulseForce();
			}

			vToss *= 0.5f;
			LTVector vAdjVel = vVel + ( (rRot.Forward() + vToss) * s_vtWeaponDropVel.GetFloat());
			

			LTVector vAng( GetRandom(-10.0f,10.0f),GetRandom(-10.0f,10.0f),GetRandom(-20.0f,20.0f));
			pWeaponItem->DropItem( vImpulse, vAdjVel, vAng, m_pPlayer->m_hObject );

			//in the case of dropping an empty weapon, we need to force the ammo count to 0
			if (nAmount == 0)
			{
				pWeaponItem->SetAmmoAmount(0);
			}
			
		}

		// Randomize the position for the next spawn...
		LTVector vDiff =  GetRandom( -50.0f, 0.0f ) * rRot.Right() + GetRandom( 0.0f, 50.0f ) * rRot.Forward();
		vSpawnPos = (vPos + vDiff);

		--nNumSpawns;
		++nSpawn;
	}


}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::DropAllWeapons
//
//	PURPOSE:	Drop and spawn all carried weapons
//
// ----------------------------------------------------------------------- //
void CPlayerInventory::DropAllWeapons()
{
	LTVector vPlayerPos, vUp, vVel;
	LTRotation rPlayerRot;
	g_pLTServer->GetObjectPos(m_pPlayer->m_hObject,&vPlayerPos);
	g_pLTServer->GetObjectRotation(m_pPlayer->m_hObject,&rPlayerRot);
	vUp = rPlayerRot.Up();

	uint8 nNumWeapons = g_pWeaponDB->GetNumPlayerWeapons();
	for( uint8 nWeapon = 0; nWeapon < nNumWeapons; ++nWeapon )
	{
		HWEAPON hWeapon = g_pWeaponDB->GetPlayerWeapon( nWeapon );

		LTRotation rRot = rPlayerRot;
		rRot.Rotate(vUp,GetRandom(-1.0f,1.0f));
		LTVector vPos = vPlayerPos + (vUp * 50.0f);

		DropWeapon(hWeapon, vPos, rRot, m_pPlayer->GetLastVelocity() );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::RemoveWeapon
//
//	PURPOSE:	Remove the specified weapon from the players inventory
//
// ----------------------------------------------------------------------- //

bool CPlayerInventory::RemoveWeapon( HWEAPON hWeapon, bool bChangeToUnarmed /*=true*/)
{
	if( !hWeapon || !g_pWeaponDB->IsPlayerWeapon( hWeapon ))
		return false;
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, !USE_AI_DATA);

	// if the weapon takes a slot, clear it...
	if (g_pWeaponDB->GetBool(hWpnData,WDB_WEAPON_bTakesInventorySlot))
	{
		//we don't actually have it...
		uint8 nSlot = GetWeaponSlot(hWeapon);
		if (nSlot == WDB_INVALID_WEAPON_INDEX)
			return false;
		m_vecWeapons[nSlot] = (HWEAPON)NULL;

	}

	if( (m_pArsenal->GetCurWeaponRecord() == hWeapon) && bChangeToUnarmed )
		m_pPlayer->ChangeWeapon( m_hUnarmed, true, NULL, false, false );

	m_pArsenal->RemoveWeapon(hWeapon);

	return true;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::WeaponSwap
//
//	PURPOSE:	drop our current weapon, and pickup one on the ground
//
// ----------------------------------------------------------------------- //

bool CPlayerInventory::WeaponSwap( HOBJECT hTarget, const LTRigidTransform& tPickup )
{
	//can't drop fists
	HWEAPON hWeapon = m_pArsenal->GetCurWeaponRecord();
	if (hWeapon == m_hUnarmed)
		hWeapon = GetLowestPriorityWeapon();

	if (!hWeapon)
		return false;

	WeaponItem *pWpnItem = dynamic_cast<WeaponItem*>(g_pLTServer->HandleToObject( hTarget ));
	if (!pWpnItem)
		return false;

	HWEAPONDATA hTgtWpnData = g_pWeaponDB->GetWeaponData( pWpnItem->GetWeaponRecord( ), !USE_AI_DATA );
	if (!g_pWeaponDB->GetBool(hTgtWpnData,WDB_WEAPON_bTakesInventorySlot))
	{
		//not an weapon that takes a slot we shouldn't swap, we should just get it
		return false;
	}

	// Drop the weapon in hand but don't switch to the unarmed weapon...
	DropWeapon( hWeapon, tPickup.m_vPos, tPickup.m_rRot, LTVector::GetIdentity(), false );

	//touch the item on the ground so that we pick it up
	pWpnItem->ObjectTouch(m_pPlayer->m_hObject);

	// Force a change to the weapon since we dropped the on in hand...
	m_pPlayer->ChangeWeapon( pWpnItem->GetWeaponRecord( ), true, NULL, true, true );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::SwapWeapon
//
//	PURPOSE:	Replaces one weapon with another one...
//
// ----------------------------------------------------------------------- //

bool CPlayerInventory::SwapWeapon( HWEAPON hFromWeapon, HWEAPON hToWeapon, bool bNotifyClient )
{
	// Both weapons must be usable by the player...
	if( !hFromWeapon || !g_pWeaponDB->IsPlayerWeapon( hFromWeapon ) ||
		!hToWeapon || !g_pWeaponDB->IsPlayerWeapon( hToWeapon ) )
	{
		return false;
	}

	// Get at the weapon data...
	HWEAPONDATA hFromWeaponData = g_pWeaponDB->GetWeaponData( hFromWeapon, false );
	HWEAPONDATA hToWeaponData = g_pWeaponDB->GetWeaponData( hToWeapon, false );

	// Only weapons that take up slots will be valid for swapping...
	if( !g_pWeaponDB->GetBool( hFromWeaponData, WDB_WEAPON_bTakesInventorySlot ) ||
		!g_pWeaponDB->GetBool( hToWeaponData, WDB_WEAPON_bTakesInventorySlot ) )
	{
		return false;
	}

	// Get at the ammo for the new weapon
	HAMMO hToAmmo = g_pWeaponDB->GetRecordLink( hToWeaponData, WDB_WEAPON_rAmmoName );

	if( !hToAmmo )
	{
		// Must have a valid ammo...
		return false;
	}

	// If our from weapon is taking up a slot... clear that slot out
	uint8 nWeaponSlot = GetWeaponSlot( hFromWeapon );

	if( nWeaponSlot == WDB_INVALID_WEAPON_INDEX )
	{
		// We didn't actually have the old weapon... so just bail!
		return false;
	}

	// Change it to the new weapon...
	m_vecWeapons[ nWeaponSlot ] = hToWeapon;

	if ( m_pArsenal->GetCurWeapon() )
	{
		m_pArsenal->GetCurWeapon()->Drop();
	}

	// Obtain the new weapon, and ditch the old one...
	m_pArsenal->DeselectCurWeapon();
	//!!ARL:	Removing the weapon makes it disappear immediately (which looks bad).
	//			We should probably still get rid of it when the complimentary weapon
	//			is dropped or something.  But since there's no way to switch to it 
	//			in the Dark, we won't worry about it for now.
	//	m_pArsenal->RemoveWeapon( hFromWeapon );
	m_pArsenal->ObtainWeapon( hToWeapon, hToAmmo, -1, bNotifyClient );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::HandleWeaponBroke
//
//	PURPOSE:	Replaces current weapon with a broken version
//
// ----------------------------------------------------------------------- //

bool CPlayerInventory::HandleWeaponBroke( HWEAPON hBrokenWeapon, HWEAPON hReplacementWeapon )
{
	// new weapon must be usable by the player...
	if( !hReplacementWeapon || !g_pWeaponDB->IsPlayerWeapon( hReplacementWeapon ) )
	{
		return false;
	}

	// Get at the weapon data...
	HWEAPONDATA hReplacementWeaponData = g_pWeaponDB->GetWeaponData( hReplacementWeapon, false );

	// Get at the ammo for the new weapon
	HAMMO hReplacementAmmo = g_pWeaponDB->GetRecordLink( hReplacementWeaponData, WDB_WEAPON_rAmmoName );

	if( !hReplacementAmmo )
	{
		// Must have a valid ammo...
		return false;
	}

	// If our from weapon is taking up a slot... clear that slot out
	uint8 nWeaponSlot = GetWeaponSlot( hBrokenWeapon );

	if( nWeaponSlot == WDB_INVALID_WEAPON_INDEX )
	{
		// We didn't actually have the old weapon... so just bail!
		return false;
	}

	// Change it to the new weapon...
	m_vecWeapons[ nWeaponSlot ] = hReplacementWeapon;

	// remove broken weapon from arsenal
	m_pArsenal->RemoveWeapon( hBrokenWeapon );

	// add a replacement weapon - don't play any weapon change anims
	m_pArsenal->ObtainWeapon(hReplacementWeapon, hReplacementAmmo, -1, true, -1, true);

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::GetLowestPriorityWeapon
//
//	PURPOSE:	find the weapon we least care about
//
// ----------------------------------------------------------------------- //

HWEAPON CPlayerInventory::GetLowestPriorityWeapon()
{
	for( uint8 nWeapon = 0; nWeapon < m_vecPriorities.size(); ++nWeapon )
	{
		HWEAPON hWeapon = m_vecPriorities[nWeapon];
		//skip our fists...
		if( hWeapon != m_hUnarmed && HaveWeapon(hWeapon) )
		{
			return hWeapon;
		}
	}
	return NULL;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::PickupGear
//
//	PURPOSE:	Try to pickup a gear item
//
// ----------------------------------------------------------------------- //

bool CPlayerInventory::PickupGear(HOBJECT hSender, ILTMessage_Read *pMsg)
{
	if (!m_pPlayer || !m_pPlayer->IsAlive()) return false;
	

	HGEAR hGear = pMsg->ReadDatabaseRecord(g_pLTDatabase, g_pWeaponDB->GetGearCategory() );
	if (!hGear)
		return false;

	uint32 nIndex = g_pWeaponDB->GetRecordIndex(hGear);
	LTASSERT(nIndex < m_vecGearCount.size(),"Invalid gear index.");
	if (nIndex >= m_vecGearCount.size())
		return false;

	/* hacked out stealth support 
	// This may change after the new item is added...
	float fOldStealth = GetStealthModifier();
*/

	//check to see if we can carry more of this type of item
	//	if the limit is 0, then there is no limit
	uint16 nMax = g_pWeaponDB->GetInt32(hGear,WDB_GEAR_nMaxAmount);
	if (nMax && m_vecGearCount[nIndex] >= nMax)
		return false;


	bool bAdded = true;

	if (nMax == 0)
	{
		bAdded = UseGear(hGear,false);
	}
	else
	{
		//if we're counting pickups, and we added this one increment our count
		++m_vecGearCount[nIndex];

		// Update SlowMo recharge rates if the gear item is supposed to recharge it...
		if( g_pWeaponDB->GetFloat( hGear, WDB_GEAR_fSlowMoTime ) > 0.0f )
		{
			m_fSlowMoRechargeRate = g_pWeaponDB->GetFloat( hGear, WDB_GEAR_fSlowMoTime );
			
			PickupItem *pPickupItem = dynamic_cast<PickupItem*>(g_pLTServer->HandleToObject( hSender ));
			if( pPickupItem )
			{
				// Cache the original slow-mo gear object...
				m_hSlowMoGearObject = pPickupItem->GetOriginalPickupObject( );
				pPickupItem->SetOriginalPickupObject( NULL );
				if( !m_hSlowMoGearObject )
					m_hSlowMoGearObject = hSender;
			}

			NavMarkerCreator nmc;
			//see what kind of marker we're supposed to use
			nmc.m_hType = g_pNavMarkerTypeDB->GetRecord( "SlowMo_Enemy" );

			//create the marker, if a type was specified
			if (nmc.m_hType && GameModeMgr::Instance( ).m_grbSlowMoNavMarker)
			{
				if( GameModeMgr::Instance( ).m_grbUseTeams )
				{
					nmc.m_nTeamId = 1 - m_pPlayer->GetTeamID();
				}
				else
				{
					nmc.m_nTeamId = 255;
				}
				
				nmc.m_hTarget = m_pPlayer->m_hObject;
				g_pLTServer->GetObjectPos( m_pPlayer->m_hObject, &nmc.m_vPos );

				NavMarker* pNM = nmc.SpawnMarker();
				if (pNM)
				{
					//keep track incase we need to remove it later
					m_hEnemySlowMoNavMarker = pNM->m_hObject;
				}
			}

			if( GameModeMgr::Instance( ).m_grbUseTeams )
			{
				//see what kind of marker we're supposed to use for my friends
				nmc.m_hType = g_pNavMarkerTypeDB->GetRecord( "SlowMo_Team" );

				//create the marker, if a type was specified
				if (nmc.m_hType)
				{
					nmc.m_nTeamId = m_pPlayer->GetTeamID();
					nmc.m_hTarget = m_pPlayer->m_hObject;
					g_pLTServer->GetObjectPos( m_pPlayer->m_hObject, &nmc.m_vPos );

					NavMarker* pNM = nmc.SpawnMarker();
					if (pNM)
					{
						//keep track in case we need to remove it later
						m_hSlowMoNavMarker = pNM->m_hObject;
					}
				}

			}

			m_pPlayer->SetHasSlowMoRecharge(true);

			m_hSlowMoGearRecord = hGear;

			SendSlowMoValuesToClient( );
		}
	}

/* hacked out stealth support 
	// If our stealth modifier has changed, notify the clients...
	if (fOldStealth != GetStealthModifier())
	{
		if (IsCharacter(m_hObject))
		{
			CCharacter* pChar = (CCharacter*)g_pLTServer->HandleToObject(m_hObject);
			if (pChar)
			{
				pChar->SendStealthToClients();
			}
		}
	}
*/

	if (bAdded)
	{
		CAutoMessage cMsg;
		cMsg.Writeuint32(MID_PICKEDUP);
		cMsg.Writebool( true );
		cMsg.Writebool( false ); // bWeaponsStay
		g_pLTServer->SendToObject(cMsg.Read(), m_pPlayer->m_hObject, hSender, MESSAGE_GUARANTEED);

	}

	return bAdded;
}

uint16 CPlayerInventory::GetGearCount(HGEAR hGear) const
{
	if (!hGear)
		return 0;
	uint32 nIndex = g_pWeaponDB->GetRecordIndex(hGear);
	LTASSERT(nIndex < m_vecGearCount.size(),"Invalid gear index.");
	if (nIndex >= m_vecGearCount.size())
		return 0;

	return m_vecGearCount[nIndex];

}

bool CPlayerInventory::UseGear(HGEAR hGear, bool bNotifyClient /* = true */)
{
	if (!m_pPlayer->IsAlive())
		return false;

	if (!hGear)
		return false;
	
	if( !m_pPlayer->IsAlive() )
		return false;

	uint32 nIndex = g_pWeaponDB->GetRecordIndex(hGear);
	uint8 nMax = g_pWeaponDB->GetInt32(hGear,WDB_GEAR_nMaxAmount);
	LTASSERT(nIndex < m_vecGearCount.size(),"Invalid gear index.");
	if (nIndex >= m_vecGearCount.size() ||			//valid index
		(nMax > 0 && m_vecGearCount[nIndex] == 0))	//is uncounted type, or has at least one
		return false;

	bool bUsed = false;
	float fRepair = g_pWeaponDB->GetFloat(hGear,WDB_GEAR_fArmor);
	if (fRepair > 0.0f)
	{
		fRepair *= GetSkillValue(eArmorPickup);
		bUsed = m_pPlayer->GetDestructible()->Repair(fRepair);
		if (bUsed)
		{
			m_pPlayer->UpdateSurfaceFlags();
		}
	}

	float fHealth = g_pWeaponDB->GetFloat(hGear,WDB_GEAR_fHealth);
	if (fHealth > 0.0f)
	{
		fHealth *= GetSkillValue(eHealthPickup);
		bUsed =  m_pPlayer->GetDestructible()->Heal(fHealth);
	}

	float fSlowMoMax = g_pWeaponDB->GetFloat(hGear,WDB_GEAR_fSlowMoMaxBonus);
	if (fSlowMoMax > 0.0f) 
	{
		m_fSlowMoMaxCharge += fSlowMoMax;
		bUsed = true;
	}

	float fSlowMo = g_pWeaponDB->GetFloat(hGear,WDB_GEAR_fSlowMoTime);
	if (fSlowMo > 0.0f) 
	{
		bUsed = (m_fSlowMoCharge < m_fSlowMoMaxCharge);
		m_fSlowMoCharge = LTMIN(m_fSlowMoMaxCharge,m_fSlowMoCharge+fSlowMo);
	}

	float fMaxBonus = g_pWeaponDB->GetFloat(hGear,WDB_GEAR_fHealthMax);
	float fCurrentMax = m_pPlayer->GetDestructible()->GetMaxHitPoints();
	float fAbsoluteMax = (float)g_pServerDB->GetMaxPlayerHealth();
	if (fMaxBonus > 0.0f && m_pPlayer->GetClient( ) && fCurrentMax < fAbsoluteMax)
	{
		fMaxBonus *= GetSkillValue(eHealthPickup);
		float fMax =  fCurrentMax + fMaxBonus;
		fMax = LTMIN( fAbsoluteMax, fMax);
		m_pPlayer->GetDestructible()->SetMaxHitPoints(fMax);

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
		cMsg.Writeuint8(IC_MAX_HEALTH_ID);
		cMsg.Writeuint8(0);
		cMsg.Writeuint8(0);
		cMsg.Writefloat(fMax);
		g_pLTServer->SendToClient(cMsg.Read( ), m_pPlayer->GetClient( ), MESSAGE_GUARANTEED);

		m_pPlayer->GetDestructible()->Heal(fMax);
		bUsed = true;

	}

	fMaxBonus = g_pWeaponDB->GetFloat(hGear,WDB_GEAR_fArmorMax);
	fCurrentMax = m_pPlayer->GetDestructible()->GetMaxArmorPoints();
	fAbsoluteMax = (float)g_pServerDB->GetMaxPlayerArmor();
	if (fMaxBonus > 0.0f && m_pPlayer->GetClient( ) && fCurrentMax < fAbsoluteMax)
	{
		fMaxBonus *= GetSkillValue(eArmorPickup);
		float fMax = fCurrentMax + fMaxBonus;
		fMax = LTMIN( fAbsoluteMax, fMax);

		m_pPlayer->GetDestructible()->SetMaxArmorPoints(fMax);

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
		cMsg.Writeuint8(IC_MAX_ARMOR_ID);
		cMsg.Writeuint8(0);
		cMsg.Writeuint8(0);
		cMsg.Writefloat(fMax);
		g_pLTServer->SendToClient(cMsg.Read( ), m_pPlayer->GetClient( ), MESSAGE_GUARANTEED);

		m_pPlayer->GetDestructible()->Repair(fMax);
		bUsed = true;
	}


	if (bUsed)
	{
		if (fSlowMo > 0.0f || fSlowMoMax > 0.0f)
		{
			SendSlowMoValuesToClient( );
		}

		//this might be 0 for uncounted items
		if (m_vecGearCount[nIndex] > 0)
			--m_vecGearCount[nIndex];

		HCLIENT hClient = m_pPlayer->GetClient();
		if (hClient && bNotifyClient)
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8( MID_SFX_MESSAGE );
			cMsg.Writeuint8( SFX_CHARACTER_ID );
			cMsg.WriteObject( m_pPlayer->m_hObject );
			cMsg.WriteBits(CFX_USE_GEAR, FNumBitsExclusive<CFX_COUNT>::k_nValue );
			cMsg.WriteDatabaseRecord( g_pLTDatabase, hGear );
			cMsg.Writeuint8(kGearUse);
			cMsg.Writeuint8( 1 );
			g_pLTServer->SendToClient( cMsg.Read( ), NULL, MESSAGE_GUARANTEED );
		}
	}
	return bUsed;
}

bool CPlayerInventory::RemoveGear(HGEAR hGear, uint8 nAmount /* = 1 */)
{
	if (!hGear)
		return false;
	uint32 nIndex = g_pWeaponDB->GetRecordIndex(hGear);
	LTASSERT(nIndex < m_vecGearCount.size(),"Invalid gear index.");
	if (nIndex >= m_vecGearCount.size() || m_vecGearCount[nIndex] == 0)
		return false;

	if (nAmount > m_vecGearCount[nIndex])
	{
		nAmount = m_vecGearCount[nIndex];
	}

	m_vecGearCount[nIndex] -= nAmount;

	HCLIENT hClient = m_pPlayer->GetClient();
	if (hClient)
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_SFX_MESSAGE );
		cMsg.Writeuint8( SFX_CHARACTER_ID );
		cMsg.WriteObject( m_pPlayer->m_hObject );
		cMsg.WriteBits(CFX_USE_GEAR, FNumBitsExclusive<CFX_COUNT>::k_nValue );
		cMsg.WriteDatabaseRecord( g_pLTDatabase, hGear );
		cMsg.Writeuint8(kGearRemove);
		cMsg.Writeuint8( nAmount );
		g_pLTServer->SendToClient( cMsg.Read( ), NULL, MESSAGE_GUARANTEED );
	}

	return true;
}
void CPlayerInventory::RemoveAllGear()
{
	GearArray::iterator itr = m_vecGearCount.begin();
	while (itr != m_vecGearCount.end())
	{
		(*itr) = 0;
		++itr;
	}
}


float CPlayerInventory::GetGearProtection(DamageType DT) const
{
	float fProtection = 0.0f;

	// See if we have protection from this kind of damage...
	for (uint8 nGearId=0; nGearId < g_pWeaponDB->GetNumGear(); nGearId++)
	{
		//if we have this gear type...
		if (m_vecGearCount[nGearId] > 0)
		{
			HGEAR hGear = g_pWeaponDB->GetGearRecord(nGearId);
			//does it protect against this damage type?
			if (StringToDamageType(g_pWeaponDB->GetString(hGear,WDB_GEAR_sProtectionType)) == DT)
			{
				fProtection += g_pWeaponDB->GetFloat(hGear,WDB_GEAR_fProtection);

				//fully protected stop looking
				if (fProtection >= 1.0f)
				{
					return 1.0f;
				}
			}
		}
	}

	return fProtection;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::GetStealthModifier
//
//	PURPOSE:	Returns footstep sound volume multiplier for the current gear
//					and skills
//
// ----------------------------------------------------------------------- //

float CPlayerInventory::GetStealthModifier()
{
	float fMult = 1.0f;

	for (uint8 nGearId=0; nGearId < g_pWeaponDB->GetNumGear(); nGearId++)
	{
		HGEAR hGear = g_pWeaponDB->GetGearRecord(nGearId);
		if (hGear && GetGearCount(hGear) > 0)
		{
			float fStealth = g_pWeaponDB->GetFloat(hGear,WDB_GEAR_fStealth);
			fMult *= (1.0f - fStealth);
		}
	}

	fMult *= GetSkillValue(eStealthRadius);		

	return fMult;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::HasAirSupply
//
//	PURPOSE:	Determine whether player can survive underwater
//
// ----------------------------------------------------------------------- //

bool CPlayerInventory::HasAirSupply() 
{ 
	return true; 
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::HandleGearMsg
//
//	PURPOSE:	handle gear message from client
//
// ----------------------------------------------------------------------- //

void CPlayerInventory::HandleGearMsg(ILTMessage_Read *pMsg)
{
	HGEAR hGear	= pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetGearCategory() );
	GearMsgType eMsgType = static_cast<GearMsgType>(pMsg->Readuint8());

	switch(eMsgType)
	{
	case kGearUse:
		UseGear(hGear);
		break;
	case kGearAdd:
	case kGearRemove:
	default:
		break;
		//unhandled messages
	};

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::HandlePriorityMsg
//
//	PURPOSE:	handle weapon priority message from client
//
// ----------------------------------------------------------------------- //

void CPlayerInventory::HandlePriorityMsg(ILTMessage_Read *pMsg)
{
	m_vecPriorities.clear();

	uint8 nNum = pMsg->Readuint8();
	m_vecPriorities.reserve(nNum);

	for (uint8 n = 0; n < nNum; ++n)
	{
		HWEAPON	hWeapon = pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pWeaponDB->GetWeaponsCategory() );
		m_vecPriorities.push_back(hWeapon);
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CPlayerInventory::Save(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

	SAVE_BYTE( m_nWeaponCapacity );

	//save set of weapons we are carrying
	WeaponArray::iterator iter = m_vecWeapons.begin();
	while (iter != m_vecWeapons.end())
	{
		SAVE_HRECORD( (*iter));
		++iter;
	};


	//save vector tracking gear items that have been picked up
	SAVE_BYTE(g_pWeaponDB->GetNumGear());
	for( uint8 nGear = 0; nGear < g_pWeaponDB->GetNumGear(); ++nGear )
	{
		// Save the name of the gear record so the correct index can be properly resolved when loading...
		// This will help minimize the possibility of thrashing saved games in future updates...
		HGEAR hGear = g_pLTDatabase->GetRecordByIndex( g_pWeaponDB->GetGearCategory( ), nGear );
		const char *pszGearName = g_pWeaponDB->GetRecordName( hGear );
		SAVE_CHARSTRING( pszGearName );
		SAVE_BYTE( m_vecGearCount[nGear] );
	}

	SAVE_HRECORD( m_hSlowMoRecord );

	SAVE_FLOAT(m_fSlowMoCharge);
	SAVE_bool(m_bSlowMoPlayerControlled);
	SAVE_FLOAT(m_fSlowMoMaxCharge);
	SAVE_FLOAT(m_fSlowMoRechargeRate);
	SAVE_bool( m_bUpdateCharge );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CPlayerInventory::Load(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

	LOAD_BYTE( m_nWeaponCapacity );

	m_vecWeapons.clear();
	m_vecWeapons.resize(m_nWeaponCapacity,(HWEAPON)NULL);
	for (uint8 i = 0; i < m_nWeaponCapacity; ++i)
	{
		HWEAPON hWeapon;
		LOAD_HRECORD(hWeapon, g_pWeaponDB->GetWeaponsCategory() );
		m_vecWeapons[i] = hWeapon;
	}


	uint8 numGearSaved;
	LOAD_BYTE(numGearSaved);

	//set up our gear array
	uint8 nNumGear = g_pWeaponDB->GetNumGear();
	m_vecGearCount.clear();
	m_vecGearCount.insert(m_vecGearCount.begin(),nNumGear,0);
	LTASSERT(numGearSaved == nNumGear,"Number of gear in saved game doesn't match current DB.");

	char szGearName[256] = {0};
	for( uint8 nGear = 0; nGear < numGearSaved; ++nGear)
	{
		// Resolve the record name to the proper index...
		LOAD_CHARSTRING( szGearName, LTARRAYSIZE( szGearName ));
		HGEAR hGear = g_pWeaponDB->GetGearRecord( szGearName );
		uint32 nGearIndex = g_pWeaponDB->GetRecordIndex( hGear );

		uint8 nCount;
		LOAD_BYTE(nCount);
		m_vecGearCount[nGearIndex] = nCount;
	}

	m_hUnarmed = g_pWeaponDB->GetUnarmedRecord();

	LOAD_HRECORD( m_hSlowMoRecord, DATABASE_CATEGORY( SlowMo ).GetCategory( ));
	LOAD_FLOAT(m_fSlowMoCharge);
	LOAD_bool(m_bSlowMoPlayerControlled);
	LOAD_FLOAT(m_fSlowMoMaxCharge);
	LOAD_FLOAT(m_fSlowMoRechargeRate);
	LOAD_bool( m_bUpdateCharge );

	SendSlowMoValuesToClient( );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::IsPreferredWeapon( )
//
//	PURPOSE:	Checks if WeaponA is preferred to WeaponB.
//
// ----------------------------------------------------------------------- //

bool CPlayerInventory::IsPreferredWeapon( HWEAPON hWeaponA, HWEAPON hWeaponB ) const
{
	if (hWeaponA == hWeaponB)
		return false;

	const char *szA = g_pWeaponDB->GetRecordName(hWeaponA);
	const char *szB = g_pWeaponDB->GetRecordName(hWeaponB);

	//using indices rather than iterators because this needs to be a const function
	for (uint32 n = m_vecPriorities.size()-1; n < m_vecPriorities.size(); --n)
	{
		//found A first, its the preferred one...
		if (hWeaponA == m_vecPriorities[n]) 
		{
			return true;
		}

		//found B first, so A is not preferred...
		if (hWeaponB == m_vecPriorities[n]) 
		{
			return false;
		}

	}

	//didn't find either one, no preference...
	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::HaveWeapon( )
//
//	PURPOSE:	Do we have the specified?
//
// ----------------------------------------------------------------------- //

bool CPlayerInventory::HaveWeapon( HWEAPON hWpn )
{
	return (GetWeaponSlot(hWpn) != WDB_INVALID_WEAPON_INDEX);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::GetWeaponSlot( )
//
//	PURPOSE:	Find the specified weapon...
//
// ----------------------------------------------------------------------- //

uint8 CPlayerInventory::GetWeaponSlot( HWEAPON hWpn )
{
	uint8 n = 0;
	while (n < m_nWeaponCapacity && hWpn != m_vecWeapons[n])
	{
		n++;
	}

	if (n >= m_nWeaponCapacity )
	{
		return WDB_INVALID_WEAPON_INDEX;
	}
	else
	{
		return n;
	}
	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::FindFirstEmptySlot( )
//
//	PURPOSE:	Find the first open slot
//
// ----------------------------------------------------------------------- //

uint8 CPlayerInventory::FindFirstEmptySlot( )
{
	uint8 n = 0;
	while (n < m_nWeaponCapacity && (NULL != m_vecWeapons[n]))
	{
		n++;
	}
	if (n >= m_nWeaponCapacity )
	{
		return WDB_INVALID_WEAPON_INDEX;
	}
	else
	{
		return n;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::ClearWeaponSlots( )
//
//	PURPOSE:	Clear out our slots
//
// ----------------------------------------------------------------------- //

void CPlayerInventory::ClearWeaponSlots()
{
	if (m_nWeaponCapacity != m_vecWeapons.size())
	{
		m_vecWeapons.clear();
		SetCapacity(m_nWeaponCapacity);
		return;
	}

	for (uint8 n =0;n < m_nWeaponCapacity;++n )
	{
		m_vecWeapons[n] = NULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::GetWeaponInSlot( )
//
//	PURPOSE:	Get the weapon in the specified slot...
//
// ----------------------------------------------------------------------- //

HWEAPON CPlayerInventory::GetWeaponInSlot( uint8 nSlot )
{
	if (nSlot < m_vecWeapons.size())
		return m_vecWeapons[nSlot];

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::AddHealth
//
//	PURPOSE:	Add the specified ammount of health to the player...
//
// ----------------------------------------------------------------------- //

bool CPlayerInventory::AddHealth( uint32 nAmmount )
{
	return m_pPlayer->GetDestructible( )->Heal( (float)nAmmount );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::AddArmor
//
//	PURPOSE:	Add the specified amount of armor to the player...
//
// ----------------------------------------------------------------------- //

bool CPlayerInventory::AddArmor( uint32 nAmmount )
{
	return m_pPlayer->GetDestructible( )->Repair( (float)nAmmount );	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::AddAmmo
//
//	PURPOSE:	Add the specified amount of ammo of the specified ammo type to the player...
//
// ----------------------------------------------------------------------- //

bool CPlayerInventory::AddAmmo( HAMMO hAmmo, uint32 nAmmount )
{
	if (!m_pArsenal->CanUseAmmo(hAmmo))
		return false;

	return (m_pArsenal->AddAmmo( hAmmo, nAmmount ) > 0);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::AddSlowMo
//
//	PURPOSE:	Add the specified amount of slow-mo to the total slow-mo charge for the player...
//
// ----------------------------------------------------------------------- //

bool CPlayerInventory::AddSlowMo( uint32 nAmount )
{
	if (!nAmount) return false;

	// Don't give more than the max limit...
	if( m_fSlowMoCharge >= m_fSlowMoMaxCharge )
		return false;

	// Add in the extra amount...
	float fAmount = nAmount / 1000.0f;
	m_fSlowMoCharge = LTMIN( m_fSlowMoCharge + fAmount, m_fSlowMoMaxCharge );

	// Let the client know of the changed value...
	SendSlowMoValuesToClient( );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::IsSlowMoPlayerControlled
//
//	PURPOSE:	Is the player controlling a slow mo
//
// ----------------------------------------------------------------------- //

bool CPlayerInventory::IsSlowMoPlayerControlled() const
{
	return (m_bSlowMoPlayerControlled);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::SetSlowMoPlayerControl
//
//	PURPOSE:	set/clear the player controlled slow mor flag
//
// ----------------------------------------------------------------------- //

void CPlayerInventory::SetSlowMoPlayerControl(bool bPlayer )
{
	m_bSlowMoPlayerControlled = bPlayer;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::SendSlowMoValues
//
//	PURPOSE:	notify client of current values
//
// ----------------------------------------------------------------------- //

void CPlayerInventory::SendSlowMoValuesToClient( )
{
	HCLIENT hClient = m_pPlayer->GetClient( );
	if( hClient )
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_SLOWMO );
		// Leave slowmo.
		cMsg.Writeuint8( kSlowMoInit );
		cMsg.Writefloat( m_fSlowMoCharge );
		cMsg.Writefloat( m_fSlowMoMaxCharge );
		cMsg.Writefloat( GETCATRECORDATTRIB( SlowMo, m_hSlowMoRecord, MinimumPeriod ) );
		cMsg.Writefloat( m_fSlowMoRechargeRate );
		g_pLTServer->SendToClient( cMsg.Read(), hClient, MESSAGE_GUARANTEED );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::TransferPersistentInventory
//
//	PURPOSE:	Transfer all persistent inventory items from specified inventory to this one...
//
// ----------------------------------------------------------------------- //

void CPlayerInventory::TransferPersistentInventory( const CPlayerInventory *pFromInventory )
{
	if( !pFromInventory )
		return;

	if( GameModeMgr::Instance( ).m_grbSlowMoPersistsAcrossDeath )
		m_fSlowMoCharge = pFromInventory->m_fSlowMoCharge;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::ExitSlowMo
//
//	PURPOSE:	Handle special functionality when a player exits slow-mo...
//
// ----------------------------------------------------------------------- //

void CPlayerInventory::ExitSlowMo( )
{
	m_bSlowMoPlayerControlled = false;

	if( GameModeMgr::Instance( ).m_grbSlowMoRespawnAfterUse )
	{
		// Respawn the slowmo gearitem if we cached one...
		if( m_hSlowMoGearObject )
		{
			g_pCmdMgr->QueueMessage( m_pPlayer->m_hObject, m_hSlowMoGearObject, "RESPAWN" );
		}

		m_hSlowMoGearObject = NULL;

		RemoveGear( m_hSlowMoGearRecord );
		RemoveSlowMoNavMarker();

		m_hSlowMoGearRecord = NULL;

		// Reset slow-mo values...
		m_fSlowMoCharge = 0.0f;
		
		m_fSlowMoRechargeRate = GETCATRECORDATTRIB( SlowMo, m_hSlowMoRecord, RechargeRate );

		m_bUpdateCharge = true;

		SendSlowMoValuesToClient( );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::RemoveSlowMoNavMarker
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CPlayerInventory::RemoveSlowMoNavMarker( )
{
	if( m_hSlowMoNavMarker )
	{
		g_pLTServer->RemoveObject( m_hSlowMoNavMarker );
		m_hSlowMoNavMarker = NULL;
	}
	if( m_hEnemySlowMoNavMarker )
	{
		g_pLTServer->RemoveObject( m_hEnemySlowMoNavMarker );
		m_hEnemySlowMoNavMarker = NULL;
	}

	m_pPlayer->SetHasSlowMoRecharge(false);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerInventory::SetCTFFlag
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CPlayerInventory::SetCTFFlag( HOBJECT hCTFFlag )
{ 
	m_hCTFFlag = hCTFFlag; 
	if (m_hCTFFlag)
	{
		//remove any old marker
		if( m_hFlagNavMarker )
		{
			g_pLTServer->RemoveObject( m_hFlagNavMarker );
			m_hFlagNavMarker = NULL;
		}

		NavMarkerCreator nmc;
		//see what kind of marker we're supposed to use
		nmc.m_hType = g_pNavMarkerTypeDB->GetRecord( "Flag_Team" );
		//create the marker, if a type was specified
		if (nmc.m_hType)
		{
			nmc.m_nTeamId = m_pPlayer->GetTeamID();
			nmc.m_hTarget = m_pPlayer->m_hObject;
			g_pLTServer->GetObjectPos( m_pPlayer->m_hObject, &nmc.m_vPos );

			NavMarker* pNM = nmc.SpawnMarker();
			if (pNM)
			{
				//keep track in case we need to remove it later
				m_hFlagNavMarker = pNM->m_hObject;
			}
		}

	}
	else
	{
		if( m_hFlagNavMarker )
		{
			g_pLTServer->RemoveObject( m_hFlagNavMarker );
			m_hFlagNavMarker = NULL;
		}
	}
}


// EOF
