// ----------------------------------------------------------------------- //
//
// MODULE  : Arsenal.cpp
//
// PURPOSE : Arsenal aggregate object - Implementation
//
// CREATED : 9/25/97
//			 5/01/03 - Renamed to Arsenal from Weapons.
//
// (c) 1997-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

	#include "Stdafx.h"
	#include "Character.h"
	#include "Attachments.h"
	#include "ObjectMsgs.h"
	#include "PlayerObj.h"
	#include "Weapon.h"
	#include "AmmoBox.h"
	#include "HHWeaponModel.h"
	#include "Projectile.h"
	#include "PlayerButes.h"
	#include "WeaponDB.h"
	#include "ServerDB.h"
	#include "Arsenal.h"
	#include "ServerMissionMgr.h"
	#include "AIUtils.h"


CBankedList<CWeapon> s_bankCWeapon;

CMDMGR_BEGIN_REGISTER_CLASS( CArsenal )
CMDMGR_END_REGISTER_CLASS( CArsenal, IAggregate )


void DeleteActiveWeapon(HOBJECT hParent, CActiveWeapon* pWeapon)
{
	if (!pWeapon)
	{
		return;
	}

	if (g_pLTServer)
	{
		// Remove the attachment.to the parent, if one exists.

		HOBJECT hModel = pWeapon->GetModelObject();
		HATTACHMENT hAttachment;
		if( LT_OK == g_pLTServer->FindAttachment( hParent, hModel, &hAttachment) )
		{
			g_pLTServer->RemoveAttachment( hAttachment );
		}

		hModel = pWeapon->GetDualModelObject();
		if( hModel )
		{
			if( g_pLTServer->FindAttachment( hParent, hModel, &hAttachment ) == LT_OK )
			{
				g_pLTServer->RemoveAttachment( hAttachment );
			}
		}
	}

	// Detach the weapon from its owner
	pWeapon->Detach();
	
	// Delete the ActiveWeapon itself.
	debug_delete( pWeapon );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::CArsenal()
//
//	PURPOSE:	Initialization
//
// ----------------------------------------------------------------------- //

CArsenal::CArsenal()
:	IAggregate		( "CArsenal" ),
	m_hObject		( NULL ),
	m_hCurWeapon	( NULL ),
	m_pWeapons		( NULL ),
	m_pAmmo			( NULL ),
	m_pPlayer		( NULL )
{
	m_pVecProjectile = debug_new( CProjectile );
	m_vecpActiveWeapons.reserve( 3 );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::~CArsenal()
//
//	PURPOSE:	Destructor - deallocate weapons
//
// ----------------------------------------------------------------------- //

CArsenal::~CArsenal()
{
	DeleteWeapons();

	if (m_pAmmo)
	{
		debug_deletea(m_pAmmo);
		m_pAmmo = NULL;
	}

	if ( m_pVecProjectile )
	{
		debug_delete( m_pVecProjectile );
		m_pVecProjectile = 0;
	}

	// Delete the active weapon pointers...

	ActiveWeaponPtrArray::iterator iter;
	for( iter = m_vecpActiveWeapons.begin(); iter != m_vecpActiveWeapons.end(); ++iter )
	{
		DeleteActiveWeapon( m_hObject, (*iter) );
	}

	m_vecpActiveWeapons.clear();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::Init()
//
//	PURPOSE:	Initialize weapons
//
// ----------------------------------------------------------------------- //

bool CArsenal::Init( HOBJECT hObject )
{
	if( !hObject )
		return false;

	LTASSERT( m_hObject == NULL, "Arsenal already an aggraget of an object." );
	m_hObject = hObject;

	if (IsPlayer(m_hObject))
	{
		m_pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject( m_hObject ));
		if (!m_pPlayer)
			return false;
	}
	else
	{
		m_pPlayer = NULL;
	}

	// Delete and reallocate the weapons..

	DeleteWeapons();

	uint8 nNumWeapons = g_pWeaponDB->GetNumWeapons( );
	if( nNumWeapons > 0 )
	{
		ASSERT( NULL == m_pWeapons );
		m_pWeapons = debug_newa( CWeapon*, nNumWeapons );
		if( m_pWeapons )
		{
			memset( m_pWeapons, 0, sizeof(CWeapon*) * nNumWeapons );
		}
		else
		{
			return false;
		}
	}

	// Delete and reallocate the ammo...

	if( m_pAmmo )
	{
		debug_deletea(m_pAmmo);
		m_pAmmo = NULL;
	}

	uint8 nNumAmmoIds = g_pWeaponDB->GetNumAmmo( );
	if (nNumAmmoIds > 0)
	{
		ASSERT( NULL == m_pAmmo );
		m_pAmmo = debug_newa( int, nNumAmmoIds );
		if (m_pAmmo)
		{
			memset( m_pAmmo, 0, sizeof(int) * nNumAmmoIds );
		}
		else
		{
			return false;
		}
	}

	CreateAllWeapons();

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::CreateWeapon()
//
//	PURPOSE:	Create the specified weapon
//
// ----------------------------------------------------------------------- //

void CArsenal::CreateWeapon( uint32 nWeaponIndex )
{
	if( !IsValidWeaponIndex(nWeaponIndex) || !m_pWeapons || m_pWeapons[nWeaponIndex] )
		return;

	// Get the weapon and ammo records.  Use the first ammo value as the default...

	HWEAPON	hWeapon	= g_pWeaponDB->GetWeaponRecord( nWeaponIndex );
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, IsAI(m_hObject));
	HAMMO	hAmmo	= g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rAmmoName, 0 ); 

	CWeapon* pWeapon = s_bankCWeapon.New();
	if( pWeapon )
	{
		pWeapon->Init( this, m_hObject, hWeapon, hAmmo );
		m_pWeapons[nWeaponIndex] = pWeapon;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::DeleteWeapons()
//
//	PURPOSE:	free the weapon arsenal
//
// ----------------------------------------------------------------------- //

void CArsenal::DeleteWeapons()
{
	if( m_pWeapons && g_pWeaponDB )
	{
		uint32 nNumWeapons = g_pWeaponDB->GetNumWeapons();
		for( uint32 i = 0; i < nNumWeapons; ++i )
		{
			if( m_pWeapons[i] )
			{
				s_bankCWeapon.Delete(m_pWeapons[i]);
			}
		}

		debug_deletea(m_pWeapons);
		m_pWeapons = NULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::CreateAllWeapons()
//
//	PURPOSE:	Set up the weapon arsenal
//
// ----------------------------------------------------------------------- //

void CArsenal::CreateAllWeapons()
{
	// Make sure we don't already have this arsenal...

	if( !m_pWeapons || m_pWeapons[0] )
		return;

	uint8 nNumWeapons = g_pWeaponDB->GetNumWeapons();
	for( uint8 i = 0; i <= nNumWeapons; ++i )
	{
		CreateWeapon(i);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::EngineMessageFn
//
//	PURPOSE:	Handle message from the engine
//
// ----------------------------------------------------------------------- //

uint32 CArsenal::EngineMessageFn(LPBASECLASS pObject, uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_PRECREATE :
		{
			int nInfo = (int)fData;
			if( nInfo == PRECREATE_WORLDFILE || nInfo == PRECREATE_STRINGPROP )
			{
				ReadProp( pObject, &((ObjectCreateStruct*)pData)->m_cProperties );
			}
		}
		break;

		case MID_OBJECTCREATED :
		{
			if( !pObject || !pObject->m_hObject )
				break;
			
			Init( pObject->m_hObject );

			if( fData != OBJECTCREATED_SAVEGAME )
			{
				ActivateWeapons();
			}
		}
		break;

		case MID_SAVEOBJECT :
		{
			Save( (ILTMessage_Write*)pData, (uint8)fData );
		}
		break;

		case MID_LOADOBJECT :
		{
			Load( (ILTMessage_Read*)pData, (uint8)fData );
		}
		break;
	}

	return 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

bool CArsenal::ReadProp(LPBASECLASS pObject, const GenericPropList *pProps)
{
	LTASSERT( pObject && pProps, "Arsenal ReadProp encountered without either object or properties");
	if( !pObject || !pProps )
		return false;

	char	szPropName[32] = {0};
	HWEAPON	hWeapon;
	HAMMO	hAmmo;

	m_vecpActiveWeapons.reserve( DEFAULT_ACTIVE_WEAPONS );

	// Read the Weapon properties and fill our active weapon array 

	for( uint32 dwWeapon = 0; dwWeapon < DEFAULT_ACTIVE_WEAPONS; ++dwWeapon )
	{
		LTSNPrintF( szPropName, ARRAY_LEN( szPropName ), "Weapon%i", dwWeapon );
		const char *pszWeapon = pProps->GetString( szPropName, SELECTION_NONE );

		// Dont' read a weapon that isn't a valid selection...

		if( pszWeapon[0] && !LTStrIEquals( SELECTION_NONE, pszWeapon ))
		{
			// Find the handles from the weapon string...

			// Currently, ONLY AI use this functionality.  If this changes, the 
			// define below will need to be re-evaluated.  We don't use the IsAI()
			// technique we use elsewhere because the HOBJECT is not yet 
			// initialized/created.

			g_pWeaponDB->ReadWeapon( pszWeapon, hWeapon , hAmmo, USE_AI_DATA );

			if( hWeapon && hAmmo )
			{
				LTSNPrintF( szPropName, ARRAY_LEN( szPropName ), "Socket%i", dwWeapon );
				const char *pszSocket = pProps->GetString( szPropName, SELECTION_NONE );

				if( pszSocket[0] && !LTStrIEquals( SELECTION_NONE, pszSocket ))
				{
					// Store a properties of each ReadProp added weapon.  This allows
					// another object to query for these later and add them to the AI.
					WeaponAttachmentProperties ReadPropWeaponProperties;
					ReadPropWeaponProperties.m_hAmmo = hAmmo;
					ReadPropWeaponProperties.m_hWeapon = hWeapon;
					ReadPropWeaponProperties.m_sSocket = pszSocket;
					m_ReadPropWeaponList.push_back( ReadPropWeaponProperties );
				}
			}
		}
	}

	// Shrink-to-fit...

	ActiveWeaponPtrArray( m_vecpActiveWeapons ).swap( m_vecpActiveWeapons );
	
	return true;

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::AddWeapon
//
//	PURPOSE:	Add a new weapon from a pickup item
//
// ----------------------------------------------------------------------- //

bool CArsenal::AddWeapon( HOBJECT hSender, HWEAPON hWeapon,
						   HAMMO hAmmo, int32 nAmmo, uint32 nHealth, uint8 nInventorySlot)
{
	//only players can pick up items
	if( !m_pPlayer) 
		return false;

	HCLIENT hClient = m_pPlayer->GetClient();

	// Convert the handles into indicies for access to our arrays...
	uint32 nWeaponIndex	= g_pWeaponDB->GetRecordIndex( hWeapon );
	uint32 nAmmoIndex	= g_pWeaponDB->GetRecordIndex( hAmmo );

	if( (nWeaponIndex == INVALID_GAME_DATABASE_INDEX) || (nAmmoIndex == INVALID_GAME_DATABASE_INDEX) )
		return false;

	// Get the default ammo for the weapon...
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, IsAI(m_hObject));
	HAMMO	hDefaultAmmo = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rAmmoName ) ;

	if( hWeapon && hAmmo && hDefaultAmmo && (hDefaultAmmo != hAmmo) )
	{
		uint32 nDefaultAmmoIndex = g_pWeaponDB->GetRecordIndex( hDefaultAmmo );

		// Only add the ammo if we don't have any of it...

		if( (nDefaultAmmoIndex != INVALID_GAME_DATABASE_INDEX) && m_pAmmo[nDefaultAmmoIndex] == 0 )
		{
			// Give the default ammo, and make sure the client is updated
			// if this is a player (and we actually added some)...

			HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(hDefaultAmmo,IsAI(m_hObject));
			int32 nSelectionAmount = g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nSelectionAmount );

			if( AddAmmo( hDefaultAmmo, nSelectionAmount ))
			{
				if( hClient )
				{
					CAutoMessage cMsg;
					cMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
					cMsg.Writeuint8(IC_WEAPON_PICKUP_ID);
					cMsg.WriteDatabaseRecord( g_pLTDatabase, NULL );
					cMsg.WriteDatabaseRecord( g_pLTDatabase, hDefaultAmmo );
					cMsg.Writeuint32(GetAmmoCount( hDefaultAmmo ));
					cMsg.Writeuint8(nInventorySlot);
					g_pLTServer->SendToClient(cMsg.Read(), hClient, MESSAGE_GUARANTEED);
				}
			}
		}
	}

	bool bHaveIt	= true;
	bool bPickedUp	= false;

	if( m_pWeapons && m_pWeapons[nWeaponIndex] && m_pWeapons[nWeaponIndex]->Have() )
	{
		bPickedUp = ( 0 != AddAmmo( hAmmo, nAmmo ) ) ;
	}
	else
	{
		bHaveIt		= false;
		bPickedUp	= true;
		ObtainWeapon( hWeapon, hAmmo, nAmmo, false, nHealth );
	}


	if( bPickedUp )
	{

		// Send the appropriate message to the client...

		if( hClient )
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
			cMsg.Writeuint8(IC_WEAPON_PICKUP_ID);
			cMsg.WriteDatabaseRecord( g_pLTDatabase, hWeapon );
			cMsg.WriteDatabaseRecord( g_pLTDatabase, hAmmo );
			cMsg.Writeuint32(GetAmmoCount(hAmmo));
			cMsg.Writeuint8(nInventorySlot);
			g_pLTServer->SendToClient(cMsg.Read(), hClient, MESSAGE_GUARANTEED);
		}

		if( bPickedUp )
		{
			HandlePotentialWeaponChange( m_pPlayer, hWeapon, hAmmo, bHaveIt );
		}
	}
	// 08/05/03 - KEF - Don't send empty pickup messages.  (Unless you're in single player,
	// so we don't break anything.)
	else if (!IsMultiplayerGameServer())
	{
		if( hClient )
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
			cMsg.Writeuint8(IC_WEAPON_PICKUP_ID);
			cMsg.WriteDatabaseRecord( g_pLTDatabase, hWeapon );
			cMsg.WriteDatabaseRecord( g_pLTDatabase, NULL );
			cMsg.Writeuint32( 0 );
			cMsg.Writeuint8(nInventorySlot);
			g_pLTServer->SendToClient(cMsg.Read(), hClient, MESSAGE_GUARANTEED);
		}
	}

	return bPickedUp;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::HasWeapon
//
//	PURPOSE:	Determine if the specified weapon is active
//
// ----------------------------------------------------------------------- //
bool CArsenal::HasWeapon(HWEAPON hWeapon)
	{
	if (!hWeapon  || !IsValidWeaponRecord( hWeapon ))
			return false;
	uint32 nWeaponIndex	= g_pWeaponDB->GetRecordIndex( hWeapon );
	if( m_pWeapons && m_pWeapons[nWeaponIndex] && m_pWeapons[nWeaponIndex]->Have( ))
		return true;
	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::HasMod
//
//	PURPOSE:	Determine if the specified weapon is active
//
// ----------------------------------------------------------------------- //
bool CArsenal::HasMod(HWEAPON hWeapon, HMOD hMod)
{
	if (!hWeapon  || !IsValidWeaponRecord( hWeapon ))
		return false;
	if (!hMod)
		return false;
	uint32 nWeaponIndex	= g_pWeaponDB->GetRecordIndex( hWeapon );
	if( m_pWeapons && m_pWeapons[nWeaponIndex] && m_pWeapons[nWeaponIndex]->HaveMod( hMod ))
		return true;
	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::CanUseAmmo
//
//	PURPOSE:	Determine if there is any active weapon that can use the specified ammo type?
//
// ----------------------------------------------------------------------- //
bool CArsenal::CanUseAmmo(HAMMO hAmmo)
{
	if (!m_pWeapons || !hAmmo)
		return false;
	uint8 nNumWeapons = g_pWeaponDB->GetNumWeapons();

	for( uint8 i = 0; i < nNumWeapons; ++i )
	{
		if (!m_pWeapons[i]->Have())
			continue;	
		if (g_pWeaponDB->CanWeaponUseAmmo(m_pWeapons[i]->GetWeaponRecord(),hAmmo, IsAI(m_hObject)))
			return true;
			}
	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::AddAmmo
//
//	PURPOSE:	Add ammo to a specific ammo type
//
// ----------------------------------------------------------------------- //

int32 CArsenal::AddAmmo( HAMMO hAmmo, int32 nAmount )
{
	uint32	nAmmoIndex	= g_pWeaponDB->GetRecordIndex( hAmmo );
	
	if( nAmmoIndex == INVALID_GAME_DATABASE_INDEX )
		return 0;
    
	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(hAmmo,IsAI(m_hObject));
	int32	nMaxAmmo	= g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nMaxAmount );
	int32	nMaxTaken	= nMaxAmmo - m_pAmmo[nAmmoIndex];
	int32	taken		= LTMIN( nAmount, nMaxTaken );

	m_pAmmo[nAmmoIndex] += taken;

	if( m_pAmmo[nAmmoIndex] > nMaxAmmo )
	{
		m_pAmmo[nAmmoIndex] = nMaxAmmo;
	}
	else if( m_pAmmo[nAmmoIndex] < 0 )
	{
		m_pAmmo[nAmmoIndex] = 0;
	}

	return taken;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::DecrementAmmo
//
//	PURPOSE:	Decrement the specified ammo count
//
// ----------------------------------------------------------------------- //

void CArsenal::DecrementAmmo( HAMMO hAmmo )
{
	if( !hAmmo)
		return;
	uint32	nAmmoIndex	= g_pWeaponDB->GetRecordIndex( hAmmo );

	--m_pAmmo[nAmmoIndex];

	if( m_pAmmo[nAmmoIndex] < 0 )
	{
		m_pAmmo[nAmmoIndex] = 0;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::SetAmmo
//
//	PURPOSE:	Set the ammount of ammo for a specific ammo type
//
// ----------------------------------------------------------------------- //

bool CArsenal::SetAmmo( HAMMO hAmmo, int32 nAmount /* = -1 */)
{
	if( !hAmmo)
		return false;

	uint32	nAmmoIndex	= g_pWeaponDB->GetRecordIndex( hAmmo );
	HAMMODATA hAmmoData = g_pWeaponDB->GetAmmoData(hAmmo,IsAI(m_hObject));
	int32 nMaxAmmo = g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nMaxAmount );

	// Set to max if less than 0...

	if( nAmount < 0 )
	{
		nAmount = nMaxAmmo;
	}

	m_pAmmo[nAmmoIndex] = nAmount;

	if( m_pAmmo[nAmmoIndex] > nMaxAmmo )
	{
		m_pAmmo[nAmmoIndex] = nMaxAmmo;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::ObtainWeapon()
//
//	PURPOSE:	Mark a specific weapon as aquired
//
// ----------------------------------------------------------------------- //

void CArsenal::ObtainWeapon( HWEAPON hWeapon, HAMMO hAmmo/*= NULL*/, int32 nAmmo/*= -1*/, bool bNotifyClient/*= false*/, int32 nHealth/*= -1*/, bool bUseWeaponChangeAnims/*= false*/ )
{
	if( !m_pWeapons || !hWeapon )
		return;
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, IsAI(m_hObject));

	// If necessary set the ammo type based on the weapon's default
	// ammo...

	if( hAmmo == NULL )
	{

		hAmmo = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rAmmoName ) ;
	}

	uint32 nWeaponIndex	= g_pWeaponDB->GetRecordIndex( hWeapon );

	// Make sure we actually have the thing...

	if( !m_pWeapons[nWeaponIndex] )
		return;


	// Give us the weapon!

	m_pWeapons[nWeaponIndex]->Aquire();


	// Set the weapon's default ammo if appropriate...

	if( nAmmo >= 0 )
	{
		SetAmmo( hAmmo, nAmmo + GetAmmoCount( hAmmo ));
	}

	// Set weapon durability for partially damaged weapons

	if( nHealth >= 0 )
	{
		m_pWeapons[nWeaponIndex]->SetHealth(nHealth);
	}

	uint32 nNumMods = g_pWeaponDB->GetNumValues( hWpnData, WDB_WEAPON_rModName );

	for( uint32 m = 0; m < nNumMods; ++m )
	{
		HMOD hMod = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rModName, m );

		if( hMod )
		{
			if( g_pWeaponDB->GetBool( hMod, WDB_MOD_bIntegrated ))
			{
				m_pWeapons[nWeaponIndex]->AddMod( hMod );
			}
		}
	}



	// Notify the client if this is a player's weapon, and the flag
	// is set...

	if( bNotifyClient && m_pPlayer)
	{
		// Send the appropriate message to the client...

		HCLIENT hClient = m_pPlayer->GetClient();
		if (hClient)
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
			cMsg.Writeuint8(IC_WEAPON_OBTAIN_ID);
			cMsg.WriteDatabaseRecord( g_pLTDatabase, hWeapon );
			cMsg.WriteDatabaseRecord( g_pLTDatabase, hAmmo );
			cMsg.Writeuint32(GetAmmoCount( hAmmo ));
			cMsg.Writeuint8(m_pPlayer->GetInventory()->GetWeaponSlot(hWeapon));
			g_pLTServer->SendToClient(cMsg.Read(), hClient, MESSAGE_GUARANTEED);
		}

		// If there isn't currently a weapon selected, select the new weapon..
		if ( m_hCurWeapon == NULL )
		{
			m_pPlayer->ChangeWeapon( hWeapon, true, NULL, !bUseWeaponChangeAnims, !bUseWeaponChangeAnims );
		}

	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::ObtainMod()
//
//	PURPOSE:	Mark a specific weapon as aquired
//
// ----------------------------------------------------------------------- //

void CArsenal::ObtainMod( HWEAPON hWeapon, HMOD hMod, bool bNotifyClient, bool bDisplayMsg /*=true*/)
{
	if( !m_pWeapons || hWeapon || hMod )
		return;

	uint32 nWeaponIndex = g_pWeaponDB->GetRecordIndex( hWeapon );

	// Make sure we actually have the thing...

	if( !IsValidWeaponIndex( nWeaponIndex ) || !m_pWeapons[nWeaponIndex] )
		return;

	m_pWeapons[nWeaponIndex]->AddMod( hMod );


	// Notify the client if this is a player's weapon, and the flag
	// is set...

	if( bNotifyClient && m_pPlayer)
	{
		HCLIENT hClient = m_pPlayer->GetClient();
		if( hClient )
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
			cMsg.Writeuint8(IC_MOD_PICKUP_ID);
			cMsg.Writebool( true );
			cMsg.WriteDatabaseRecord( g_pLTDatabase, hMod );
			cMsg.Writebool( bDisplayMsg );
			g_pLTServer->SendToClient(cMsg.Read(), hClient, MESSAGE_GUARANTEED);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::ChangeWeapon()
//
//	PURPOSE:	Set the given active weapon as the current weapon...
//
// ----------------------------------------------------------------------- //

bool CArsenal::ChangeWeapon( CActiveWeapon *pActiveWeapon )
{
	if( !pActiveWeapon )
		return false;

	return ChangeWeapon( pActiveWeapon->m_hWeapon, pActiveWeapon->m_hAmmo, pActiveWeapon );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::ChangeWeapon()
//
//	PURPOSE:	Change to a new weapon...
//
// ----------------------------------------------------------------------- //

bool CArsenal::ChangeWeapon( HWEAPON hNewWeapon, HAMMO hAmmo/*=NULL*/, CActiveWeapon *pWeaponToChange/*=NULL*/ )
{
	if( hNewWeapon == m_hCurWeapon )
		return true;

	if( !m_pWeapons || !IsValidWeaponRecord( hNewWeapon ))
		return false;

	// Set this as our current weapon...

	m_hCurWeapon = hNewWeapon;

	uint32 nCurWeaponIndex = g_pWeaponDB->GetRecordIndex( m_hCurWeapon );
	if( !IsValidWeaponIndex( nCurWeaponIndex ))
		return false;

	// Change our weapon model to the newly selected weapon

	if( !m_vecpActiveWeapons.empty() )
	{

		// Use the passed in active weapon if their was one, 
		// otherwise just use the default primary weapon...

		CActiveWeapon *pActiveWeapon = (pWeaponToChange ? pWeaponToChange : m_vecpActiveWeapons.front());
		if( pActiveWeapon )
		{
			if (pActiveWeapon->m_hHHWeaponModel )
			{
				CHHWeaponModel *pHHModel = dynamic_cast<CHHWeaponModel*>(g_pLTServer->HandleToObject( pActiveWeapon->m_hHHWeaponModel ));
				if( !pHHModel )
					return false;

				pHHModel->Setup( m_pWeapons[nCurWeaponIndex], m_hCurWeapon, WDB_WEAPON_RightHandWeapon );
				HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(m_hCurWeapon, IsAI(m_hObject));
				hAmmo = (hAmmo != NULL ? hAmmo : g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rAmmoName ));

				// If the weapon is a dual weapon then create the dual weapon model...
				CHHWeaponModel *pDualWeaponModel = dynamic_cast<CHHWeaponModel*>(g_pLTServer->HandleToObject( pActiveWeapon->m_hDualWeaponModel ));
				if( pDualWeaponModel && pDualWeaponModel->m_hObject )
				{
					bool bDualWeapon = !!g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rSingleWeapon );
					EEngineLOD eShadowLOD = eEngineLOD_Never;
					if( bDualWeapon )
					{
						const char* szFoo = g_pWeaponDB->GetRecordName(m_hCurWeapon);
						pDualWeaponModel->SetupDualWeaponModel( m_pWeapons[nCurWeaponIndex], m_hCurWeapon, WDB_WEAPON_LeftHandWeapon );

						g_pLTServer->GetObjectShadowLOD( pActiveWeapon->m_hHHWeaponModel, eShadowLOD );
						g_pCommonLT->SetObjectFlags( pDualWeaponModel->m_hObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE );
					}
					else
					{
						g_pCommonLT->SetObjectFlags( pDualWeaponModel->m_hObject, OFT_Flags, 0, FLAG_VISIBLE );
					}

					g_pLTServer->SetObjectShadowLOD( pDualWeaponModel->m_hObject, eShadowLOD );
				}
			}

			// Clear the old activeweapon on the weapon.
			if( pActiveWeapon->m_pWeapon )
			{
				pActiveWeapon->m_pWeapon->SetActiveWeapon( NULL );
			}

			pActiveWeapon->m_pWeapon	= m_pWeapons[nCurWeaponIndex];
			pActiveWeapon->m_pWeapon->SetActiveWeapon( pActiveWeapon );

			pActiveWeapon->m_hWeapon	= m_hCurWeapon;
			pActiveWeapon->m_hAmmo		= hAmmo;
		}
	}
		
	// Select the weapon...

	m_pWeapons[nCurWeaponIndex]->Select();
	m_pWeapons[nCurWeaponIndex]->SetAmmo( hAmmo );

	// Notify all clients of the character's weapon change...
	CCharacter *pChar = CCharacter::DynamicCast( m_hObject );
	if( pChar )
		pChar->SendWeaponRecordToClients( );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::DeselectCurWeapon()
//
//	PURPOSE:	Deselect the current weapon
//
// ----------------------------------------------------------------------- //

void CArsenal::DeselectCurWeapon()
{
	if( IsValidWeaponRecord( m_hCurWeapon ))
	{
		uint32 nWeaponIndex = g_pWeaponDB->GetRecordIndex( m_hCurWeapon );
		m_pWeapons[nWeaponIndex]->Deselect();
	}

	m_hCurWeapon = NULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::GetWeaponAmmoCount()
//
//	PURPOSE:	Return the ammo amount of the specified selected weapon
//
// ----------------------------------------------------------------------- //

int32 CArsenal::GetWeaponAmmoCount( HWEAPON hWeapon )
{
	if( !m_pWeapons || !m_pAmmo )
		return 0;

	if( IsValidWeaponRecord( hWeapon ))
	{
		uint32 nWeaponIndex = g_pWeaponDB->GetRecordIndex( hWeapon );
		return GetAmmoCount( m_pWeapons[nWeaponIndex]->GetAmmoRecord() );
	}

	return 0;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::GetAmmoCount()
//
//	PURPOSE:	Return the amount of the specified type of ammo
//
// ----------------------------------------------------------------------- //

int32 CArsenal::GetAmmoCount( HAMMO hAmmo )
{
	if( !hAmmo)
		return 0;
	
	uint32	nAmmoIndex	= g_pWeaponDB->GetRecordIndex( hAmmo );
	return m_pAmmo[nAmmoIndex];
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CArsenal::Save( ILTMessage_Write *pMsg, uint8 nType )
{
	SAVE_HRECORD( m_hCurWeapon );

	if( m_pWeapons )
	{
		uint8 nNumWeapons = g_pWeaponDB->GetNumWeapons();
		SAVE_BYTE( nNumWeapons );

		for( uint8 i = 0; i < nNumWeapons; ++i )
		{
			if( m_pWeapons[i] )
			{
                SAVE_BOOL(true);

				// Save the name of the weapon record so the correct index can be properly resolved when loading...
				// This will help minimize the possibility of thrashing saved games in future updates...
				const char *pszWeaponName = g_pWeaponDB->GetRecordName( m_pWeapons[i]->GetWeaponRecord( ));
				SAVE_CHARSTRING( pszWeaponName );
				m_pWeapons[i]->Save(pMsg, nType);
			}
			else
			{
				SAVE_BOOL(false);
			}
		}
	}

	uint8 nNumAmmoIds = g_pWeaponDB->GetNumAmmo();

	if( !m_pAmmo )
	{
		nNumAmmoIds = 0;
	}

	SAVE_BYTE( nNumAmmoIds );

	for( uint8 nAmmo = 0; nAmmo < nNumAmmoIds; ++nAmmo )
	{
		// Save the name of the ammo record so the correct index can be properly resolved when loading...
		// This will help minimize the possibility of thrashing saved games in future updates...
		
		HAMMO hAmmo = g_pLTDatabase->GetRecordByIndex( g_pWeaponDB->GetAmmoCategory( ), nAmmo );
		const char *pszAmmoName = g_pWeaponDB->GetRecordName( hAmmo );
		SAVE_CHARSTRING( pszAmmoName );
		SAVE_INT( m_pAmmo[nAmmo] );
	}

	// Save the active weapons...

	SAVE_DWORD( m_vecpActiveWeapons.size() );
	
	ActiveWeaponPtrArray::iterator iter;
	for( iter = m_vecpActiveWeapons.begin(); iter != m_vecpActiveWeapons.end(); ++iter )
	{
		(*iter)->Save( pMsg );
	}

	// Save the WeaponProperties

	SAVE_INT( m_ReadPropWeaponList.size() );
	for ( WeaponAttachmentPropertiesListType::iterator itEach = m_ReadPropWeaponList.begin(); itEach != m_ReadPropWeaponList.end(); ++itEach )
	{
		SAVE_HRECORD( itEach->m_hWeapon );
		SAVE_HRECORD( itEach->m_hAmmo );
		SAVE_STDSTRING( itEach->m_sSocket );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CArsenal::Load( ILTMessage_Read *pMsg, uint8 nType )
{
	LOAD_HRECORD( m_hCurWeapon, g_pWeaponDB->GetWeaponsCategory() );

	uint8 nNumWeapons = 0;
	LOAD_BYTE( nNumWeapons );

	char szWeaponName[256] = {0};
	for( uint8 nWeapon = 0; nWeapon < nNumWeapons; ++nWeapon )
	{
		bool bLoad;
		LOAD_BOOL(bLoad);

		if( bLoad )
		{
            // Resolve the record name to the proper index...
			LOAD_CHARSTRING( szWeaponName, LTARRAYSIZE( szWeaponName ));
			HWEAPON hWeaon = g_pWeaponDB->GetWeaponRecord( szWeaponName );
			uint32 nWeaponIndex = g_pWeaponDB->GetRecordIndex( hWeaon );
			
			CreateWeapon( nWeaponIndex );

			if( m_pWeapons && m_pWeapons[nWeaponIndex] )
			{
				m_pWeapons[nWeaponIndex]->SetArsenal( this );
				m_pWeapons[nWeaponIndex]->Load(pMsg, nType);
			}
		}
	}

	uint8 nNumAmmo;
	LOAD_BYTE( nNumAmmo );

	char szAmmoName[256] = {0};
	for( uint nAmmo = 0; nAmmo < nNumAmmo; ++nAmmo )
	{
		if( m_pAmmo )
		{
			// Resolve the record name to the proper index...
			LOAD_CHARSTRING( szAmmoName, LTARRAYSIZE( szAmmoName ));
			HAMMO hAmmo = g_pWeaponDB->GetAmmoRecord( szAmmoName );
			uint32 nAmmoIndex = g_pWeaponDB->GetRecordIndex( hAmmo );
			
			LOAD_INT( m_pAmmo[nAmmoIndex] );
		}
	}

	// Delete any current active weapon...

	ActiveWeaponPtrArray::iterator iter;
	for( iter = m_vecpActiveWeapons.begin(); iter != m_vecpActiveWeapons.end(); ++iter )
	{
		DeleteActiveWeapon( m_hObject, (*iter) );
	}
	m_vecpActiveWeapons.clear( );

	// Load the active weapons...

	uint32 nSize = 0;
	LOAD_DWORD( nSize );

	for( uint32 i = 0; i < nSize; ++i )
	{
		CActiveWeapon *pActiveWeapon = debug_new( CActiveWeapon );
		if( pActiveWeapon )
		{
			pActiveWeapon->m_hHHWeaponModel.SetReceiver( *this );
			pActiveWeapon->m_hDualWeaponModel.SetReceiver( *this );
			pActiveWeapon->Load( pMsg );

			if( pActiveWeapon->m_hHHWeaponModel )
			{
				CHHWeaponModel *pHHModel = dynamic_cast<CHHWeaponModel*>(g_pLTServer->HandleToObject( pActiveWeapon->m_hHHWeaponModel ));
				if( pHHModel )
				{
					uint32 nWeaponIndex = g_pWeaponDB->GetRecordIndex( pActiveWeapon->m_hWeapon );
					if( IsValidWeaponIndex( nWeaponIndex ))
					{
						pHHModel->Setup( m_pWeapons[nWeaponIndex], pActiveWeapon->m_hWeapon, WDB_WEAPON_RightHandWeapon );
					}
				}
			}

			pActiveWeapon->m_pWeapon = GetWeapon( pActiveWeapon->m_hWeapon );
			if( !pActiveWeapon->m_pWeapon )
			{
				LTERROR( "Invalid active weapon found." );
				DeleteActiveWeapon( m_hObject, pActiveWeapon );
				continue;
			}

			pActiveWeapon->m_pWeapon->SetActiveWeapon( pActiveWeapon );
			m_vecpActiveWeapons.push_back( pActiveWeapon );
		}
	}

	// Restore the WeaponProperties

	int nProperties = 0;
	LOAD_INT( nProperties );
	m_ReadPropWeaponList.reserve( nProperties );
	for ( int i = 0; i < nProperties; ++i )
	{
		WeaponAttachmentProperties TempWeaponAttachmentProperties;
		LOAD_HRECORD( TempWeaponAttachmentProperties.m_hWeapon, g_pWeaponDB->GetWeaponsCategory() );
		LOAD_HRECORD( TempWeaponAttachmentProperties.m_hAmmo, g_pWeaponDB->GetAmmoCategory() );
		LOAD_STDSTRING( TempWeaponAttachmentProperties.m_sSocket );
		m_ReadPropWeaponList.push_back( TempWeaponAttachmentProperties );
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::IsValidWeaponRecord()
//
//	PURPOSE:	Check if the current id is valid and we have it...
//
// ----------------------------------------------------------------------- //

bool CArsenal::IsValidWeaponRecord( HWEAPON hWeapon )
{
	if( !hWeapon )
		return false;
	
	uint32 nWeaponIndex = g_pWeaponDB->GetRecordIndex( hWeapon );

	if( m_pWeapons && IsValidWeaponIndex( nWeaponIndex ) && m_pWeapons[nWeaponIndex] &&
		m_pWeapons[nWeaponIndex]->Have()) return true;

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::Reset()
//
//	PURPOSE:	Reset all the weapons (i.e., we don't have any of them)
//
// ----------------------------------------------------------------------- //

void CArsenal::Reset()
{
	// Toss our current weapon since we won't have any after this function.
	DeselectCurWeapon( );

	uint8 nNumWeapons = g_pWeaponDB->GetNumWeapons();
	for( uint8 nWeapon = 0; nWeapon < nNumWeapons; ++nWeapon )
	{
		if( m_pWeapons && m_pWeapons[nWeapon] )
		{
			m_pWeapons[nWeapon]->Drop();
			m_pWeapons[nWeapon]->Reset();
		}
	}

	uint8 nNumAmmo = g_pWeaponDB->GetNumAmmo();
	for( uint8 nAmmo = 0; nAmmo < nNumAmmo; ++nAmmo )
	{
		if( m_pAmmo )
		{
			m_pAmmo[nAmmo] = 0;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::HandlePotentialWeaponChange()
//
//	PURPOSE:	Handle potentially changing weapons
//
// ----------------------------------------------------------------------- //

void CArsenal::HandlePotentialWeaponChange( CPlayerObj *pPlayer, HWEAPON hWeapon, HAMMO hAmmo, bool bHaveIt )
{
	if( !hWeapon )
		return;

	bool bChangeWeapon   = false;
	bool bIsBetterWeapon = IsBetterWeapon( hWeapon );

	// If there isn't currently a weapon selected, or this weapon is
	// better than our currently selected weapon (and we didn't already
	// have it) select the new weapon...

	if( (m_hCurWeapon == NULL) || (!bHaveIt && bIsBetterWeapon) )
	{
		bChangeWeapon = true;
	}

	if( bChangeWeapon )
	{
		// Don't force the weapon change, let the client decide if they want to change (autoswitch)...
		pPlayer->ChangeWeapon( hWeapon, false, hAmmo, true, true );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::IsBetterWeapon()
//
//	PURPOSE:	Determine if the passed in weapon type is "better" than the
//				weapon we are currently using
//
// ----------------------------------------------------------------------- //

bool CArsenal::IsBetterWeapon( HWEAPON hWeapon )
{

	if (m_pPlayer)
	{
		return m_pPlayer->IsPreferredWeapon( hWeapon, m_hCurWeapon );
	}
	else
	{
		CCharacter* pChar = dynamic_cast<CCharacter*>(g_pLTServer->HandleToObject(m_hObject));
		if( !pChar )
			return false;

		return pChar->IsPreferredWeapon( hWeapon, m_hCurWeapon );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::ActivateWeapon
//
//	PURPOSE:	Creates a weapon attachment
//
// ----------------------------------------------------------------------- //

// Helper function used by ActivateWeapon to perform ObjectCreateStruct 
// setup on both the standard weapon and the dual weapon.
static void CommonWeaponSetup( HOBJECT hObject, const char* pszSocketName, ObjectCreateStruct& theStruct )
{
	// Get the initial position/rotation of the weapon.

	LTVector vAttachPos( LTVector::GetIdentity() );
	LTRotation vAttachRot( LTRotation::GetIdentity() );
	HMODELSOCKET hSocket =  INVALID_MODEL_SOCKET;
	if ( LT_OK == g_pModelLT->GetSocket( hObject, pszSocketName, hSocket ) )
	{
		LTTransform tAttachmentSocket;
		if ( LT_OK == g_pModelLT->GetSocketTransform( hObject, hSocket, tAttachmentSocket, true ) )
		{
			vAttachPos = tAttachmentSocket.m_vPos;
			vAttachRot = tAttachmentSocket.m_rRot;
		}
	}

	theStruct.m_Rotation = vAttachRot;
	theStruct.m_Pos = vAttachPos;
}

bool CArsenal::ActivateWeapon( CActiveWeapon *pActiveWeapon )
{
	// Remove any previous weapons...

	pActiveWeapon->Detach();

	pActiveWeapon->m_hHHWeaponModel.SetReceiver( *this );
	pActiveWeapon->m_hDualWeaponModel.SetReceiver( *this );

	// Prepare the object create struct

	ObjectCreateStruct theStruct;

	theStruct.m_ObjectType = OT_MODEL;
	theStruct.m_Flags = FLAG_VISIBLE | FLAG_GOTHRUWORLD;
	theStruct.m_eGroup = ePhysicsGroup_NonSolid;
	theStruct.m_NextUpdate = 1.0f;

	// Create the attachment

	char szClass[128];
	LTStrCpy( szClass, "CHHWeaponModel", ARRAY_LEN(szClass) );

	CommonWeaponSetup( m_hObject, pActiveWeapon->m_sSocket.c_str(), theStruct );

	HCLASS hClass = g_pLTServer->GetClass( szClass );
	CHHWeaponModel *pHHModel = dynamic_cast<CHHWeaponModel*>(g_pLTServer->CreateObject( hClass, &theStruct ));
	if( !pHHModel || !pHHModel->m_hObject )
		return false;
	
	uint32 nWeaponIndex = g_pWeaponDB->GetRecordIndex( pActiveWeapon->m_hWeapon );
	
	if( !IsValidWeaponIndex( nWeaponIndex ))
		return false;

	pHHModel->Setup( m_pWeapons[nWeaponIndex], pActiveWeapon->m_hWeapon, WDB_WEAPON_RightHandWeapon );

	pActiveWeapon->m_hHHWeaponModel = pHHModel->m_hObject;

	// Attach it

	HATTACHMENT hAttachment;
	LTVector   cEmptyVector(0,0,0);
	LTRotation cEmptyRotation;
	g_pLTServer->CreateAttachment(	m_hObject, 
									pActiveWeapon->m_hHHWeaponModel,
									(!pActiveWeapon->m_sSocket.empty( ) ? pActiveWeapon->m_sSocket.c_str() : NULL),	
									&cEmptyVector,
									&cEmptyRotation,
									&hAttachment );

	g_pCommonLT->SetObjectFlags( pActiveWeapon->m_hHHWeaponModel, OFT_User, USRFLG_ATTACH_HIDE1SHOW3, USRFLG_ATTACH_HIDE1SHOW3 );

	// If the weapon is a dual weapon then create the dual weapon model...
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData( pActiveWeapon->m_hWeapon, !USE_AI_DATA );
	bool bDualWeapon = !!g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rSingleWeapon );

	//if this is a player and is their default weapon, create the second weapon so that weapons that are actually dual
	// weapons have a model
	if (m_pPlayer && g_pServerDB->GetPlayerDefaultWeapon() == pActiveWeapon->m_hWeapon)
	{
		bDualWeapon = true;
	}
	if( bDualWeapon )
	{
		CommonWeaponSetup( m_hObject, pActiveWeapon->m_sDualWeaponSocket.c_str(), theStruct );

		// Create the dual weapon model...
		CHHWeaponModel *pDualWeaponModel = dynamic_cast<CHHWeaponModel*>(g_pLTServer->CreateObject( hClass, &theStruct ));
		if( pDualWeaponModel && pDualWeaponModel->m_hObject )
		{
			pDualWeaponModel->SetupDualWeaponModel( m_pWeapons[nWeaponIndex], pActiveWeapon->m_hWeapon, WDB_WEAPON_LeftHandWeapon );
			pActiveWeapon->m_hDualWeaponModel = pDualWeaponModel->m_hObject;

			// Attach it...
			g_pLTServer->CreateAttachment(	m_hObject, 
											pActiveWeapon->m_hDualWeaponModel,
											(!pActiveWeapon->m_sDualWeaponSocket.empty( ) ? pActiveWeapon->m_sDualWeaponSocket.c_str() : NULL),	
											&cEmptyVector,
											&cEmptyRotation,
											&hAttachment );
			
			g_pCommonLT->SetObjectFlags( pActiveWeapon->m_hDualWeaponModel, OFT_User, USRFLG_ATTACH_HIDE1SHOW3, USRFLG_ATTACH_HIDE1SHOW3 );
		}
	}

	// Clear the old activeweapon on the weapon.
	if( pActiveWeapon->m_pWeapon )
	{
		pActiveWeapon->m_pWeapon->SetActiveWeapon( NULL );
	}

	pActiveWeapon->m_pWeapon = m_pWeapons[nWeaponIndex];
	pActiveWeapon->m_pWeapon->SetActiveWeapon( pActiveWeapon );
	ObtainWeapon( pActiveWeapon->m_hWeapon );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::ActivateWeapon
//
//	PURPOSE:	Creates a weapon attachment from the given weapon name at the given socket position...
//
// ----------------------------------------------------------------------- //

CActiveWeapon* CArsenal::ActivateWeapon( const char *szWeapon, const char *szSocket )
{
	if( !szWeapon || !szWeapon[0] )
		return NULL;

	HWEAPON hWeapon = NULL;
	HAMMO	hAmmo	= NULL;
	 
	g_pWeaponDB->ReadWeapon( szWeapon, hWeapon, hAmmo, IsAI(m_hObject) );
	if( !hWeapon || !hAmmo )
		return NULL;

	// Create a new active weapon....

	CActiveWeapon *pActiveWeapon = debug_new( CActiveWeapon );
	if( !pActiveWeapon )
		return NULL;

	pActiveWeapon->m_hWeapon	= hWeapon;
	pActiveWeapon->m_hAmmo		= hAmmo;
	pActiveWeapon->m_sSocket	= ( szSocket ? szSocket : "" );

	// Activate it...

	if( !ActivateWeapon( pActiveWeapon  ))
	{
		debug_delete( pActiveWeapon );
		return NULL;
	}

	// Add the weapon to the list after we potentially delete it and fail.

	m_vecpActiveWeapons.push_back( pActiveWeapon );

	return pActiveWeapon;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::ActivateWeapons
//
//	PURPOSE:	Activates all the weapons in the active weapons array...
//
// ----------------------------------------------------------------------- //

void CArsenal::ActivateWeapons()
{
	if( m_pPlayer )
	{
		// Make sure we have no currently active weapons...

		RemoveAllActiveWeapons();

		// Read the weapon information for the player...
		HWEAPON hWeapon = g_pServerDB->GetPlayerDefaultWeapon();
		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon, IsAI(m_hObject));
		HAMMO hAmmo = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rAmmoName );
		if( !hWeapon || !hAmmo )
			return;

		// Create a new active weapon....

		CActiveWeapon *pActiveWeapon = debug_new( CActiveWeapon );
		if( !pActiveWeapon )
			return;

		pActiveWeapon->m_hWeapon	= hWeapon;
		pActiveWeapon->m_hAmmo		= hAmmo;
		pActiveWeapon->m_sSocket	= "RightHand";
		
		pActiveWeapon->m_hHHWeaponModel.SetReceiver( *this );

		pActiveWeapon->m_sDualWeaponSocket	= "LeftHand";
		pActiveWeapon->m_hDualWeaponModel.SetReceiver( *this );

		// Add it to the list, so it will be activated below...
		m_vecpActiveWeapons.push_back( pActiveWeapon );
	}

	ActiveWeaponPtrArray::iterator iter;
	for( iter = m_vecpActiveWeapons.begin(); iter != m_vecpActiveWeapons.end(); ++iter )
	{
		ActivateWeapon( (*iter) );
	}

	if( !m_vecpActiveWeapons.empty() )
	{
		// Change to the first active weapon which we consider the primary...

		ChangeWeapon( m_vecpActiveWeapons[0] );
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::EnumerateWeapons
//
//	PURPOSE:	Fills out an array with a list of all weapon attachments
//
// ----------------------------------------------------------------------- //

int CArsenal::EnumerateActiveWeapons( CActiveWeapon **apActiveWeapons, const int cMaxWeapons )
{
	uint32 nWeapons = 0;
	ActiveWeaponPtrArray::iterator iter;
	for( iter = m_vecpActiveWeapons.begin(); iter != m_vecpActiveWeapons.end(); ++iter )
	{
		if( apActiveWeapons )
		{
			apActiveWeapons[nWeapons]	= m_vecpActiveWeapons[nWeapons];
			LTASSERT( apActiveWeapons[nWeapons] != NULL, "EnumerateActiveWeapons - Invalid ActiveWeapon!" );
		}
		
		if( apActiveWeapons && apActiveWeapons[nWeapons] )
		{
			++nWeapons;
			if( nWeapons == cMaxWeapons )
			{
				break;
			}
		}
	}

	return nWeapons;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::GetInfiniteAmmo
//
//	PURPOSE:	Used by the AIs to get a big mess of ammunition
//
// ----------------------------------------------------------------------- //

void CArsenal::GetInfiniteAmmo()
{
	uint8 nNumAmmoTypes = g_pWeaponDB->GetNumAmmo();
	for( uint8 nAmmo = 0; nAmmo < nNumAmmoTypes; ++nAmmo )
	{
		HAMMO hAmmo = g_pWeaponDB->GetAmmoRecord( nAmmo );
		AddAmmo( hAmmo, 1000 );
	}

	CWeapon *pWeapon = GetCurWeapon();
	if( pWeapon )
		pWeapon->ReloadClip( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::RemoveAllActiveWeapons
//
//	PURPOSE:	Remove all active weapons from this arsenal...
//
// ----------------------------------------------------------------------- //

void CArsenal::RemoveAllActiveWeapons()
{
	ActiveWeaponPtrArray::iterator iter;
	for( iter = m_vecpActiveWeapons.begin(); iter != m_vecpActiveWeapons.end(); ++iter )
	{
		// Let the ActiveWeapon remove itself...

		DeleteActiveWeapon( m_hObject, (*iter) );
	}

	m_vecpActiveWeapons.clear( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::RemoveActiveWeapon
//
//	PURPOSE:	Remove the specified active weapon from this arsenal...
//
// ----------------------------------------------------------------------- //

void CArsenal::RemoveActiveWeapon( const CActiveWeapon *pActiveWeapon )
{
	// Find the active weapon...

	ActiveWeaponPtrArray::iterator iter;
	for( iter = m_vecpActiveWeapons.begin(); iter != m_vecpActiveWeapons.end(); ++iter )
	{
		if( pActiveWeapon == (*iter) )
			break;
	}

	// Only remove if it's actually apart of our arsenal...

	if( iter != m_vecpActiveWeapons.end() )
	{
		if( m_hCurWeapon == (*iter)->m_hWeapon )
		{
			// We are removing the current active weapon so be sure to deselect it...

			DeselectCurWeapon();
		}

		DeleteActiveWeapon(m_hObject, (*iter));
		m_vecpActiveWeapons.erase( iter );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::GetActiveWeapon
//
//	PURPOSE:	Retrieve an active weapon attached to the specified socket...
//
// ----------------------------------------------------------------------- //

CActiveWeapon* CArsenal::GetActiveWeapon( const char *pSocketName )
{
	if( !pSocketName )
		return NULL;

	ActiveWeaponPtrArray::iterator iter;
	for( iter = m_vecpActiveWeapons.begin(); iter != m_vecpActiveWeapons.end(); ++iter )
	{
		if( LTStrIEquals( pSocketName, (*iter)->m_sSocket.c_str() ))
		{
			return (*iter);
		}
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::OnLinkBroken
//
//	PURPOSE:	Handle attached model getting removed.
//
// ----------------------------------------------------------------------- //

void CArsenal::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	ActiveWeaponPtrArray::iterator iter;
	for( iter = m_vecpActiveWeapons.begin(); iter != m_vecpActiveWeapons.end(); ++iter )
	{
		if( &(*iter)->m_hHHWeaponModel == pRef )
		{
			HATTACHMENT hAttachment;
			if( LT_OK == g_pLTServer->FindAttachment( m_hObject, hObj, &hAttachment) )
			{
				g_pLTServer->RemoveAttachment( hAttachment );
				return;
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::HideWeapons
//
//	PURPOSE:	Hide/Show weapons...
//
// ----------------------------------------------------------------------- //

void CArsenal::HideWeapons( bool bHide )
{
	EEngineLOD eShadowLOD = eEngineLOD_Never;
	g_pLTServer->GetObjectShadowLOD( m_hObject, eShadowLOD );

	ActiveWeaponPtrArray::iterator iter;
	for( iter = m_vecpActiveWeapons.begin(); iter != m_vecpActiveWeapons.end(); ++iter )
	{
		g_pCommonLT->SetObjectFlags( (*iter)->GetModelObject(), OFT_Flags, (bHide ? FLAG_FORCECLIENTUPDATE : FLAG_VISIBLE), FLAG_VISIBLE | FLAG_FORCECLIENTUPDATE );
		g_pLTServer->SetObjectShadowLOD( (*iter)->GetModelObject( ), bHide ? eEngineLOD_Never : eShadowLOD );
	}
}

// Plugin statics

bool CArsenalPlugin::sm_bInitted = false;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenalPlugin::PreHook_EditStringList
//
//	PURPOSE:	Callback handler for EditStringList plugin
//
// ----------------------------------------------------------------------- //

LTRESULT CArsenalPlugin::PreHook_EditStringList(const char* szRezPath,
												const char* szPropName,
												char** aszStrings,
												uint32* pcStrings,
												const uint32 cMaxStrings,
												const uint32 cMaxStringLength)
{
	if ( !sm_bInitted )
	{
		sm_bInitted = true;
		g_pWeaponDBPlugin->PreHook_EditStringList(szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength);
	}

	std::string sProperty( szPropName );

	if( sProperty.find( "Weapon" ) != sProperty.npos )
	{
		// Fill the first string in the list with a <none> selection...

		LTStrCpy( aszStrings[(*pcStrings)++], SELECTION_NONE, cMaxStringLength );
		
		// Just let the weapon mgr plugin fill in the rest of the list...

		g_pWeaponDBPlugin->PopulateStringList(aszStrings, pcStrings, cMaxStrings, cMaxStringLength, false);

		qsort( aszStrings, *pcStrings, sizeof( char * ), CaseInsensitiveCompare );

		return LT_OK;
	}
	else if( sProperty.find( "Socket" ) != sProperty.npos )
	{
        // Fill the first string in the list with a <none> selection...
		
		LTStrCpy( aszStrings[(*pcStrings)++], SELECTION_NONE, cMaxStringLength );

		// Unfortunately we don't have access to the property values and the list is per class
		// rather than per object.  So we will just have to fill the list with *every*
		// possible socket :(

		StringSet ssSockets;
		uint32 nNumSets = g_pModelsDB->GetNumSocketSets();

		// Initially add the socket names to a set to avoid duplicates...

		for( uint32 iSet = 0; iSet < nNumSets; ++iSet )
		{
			ModelsDB::HSOCKETSET hSocketSet = g_pModelsDB->GetSocketSet( iSet );
			int nNumSockets = g_pModelsDB->GetSocketSetNumSockets( hSocketSet );
			for( int iSocket = 0; iSocket < nNumSockets; ++iSocket )
			{
				const char *pSocket = g_pModelsDB->GetSocketSetSocket( hSocketSet, iSocket );
				if( pSocket )
				{
					ssSockets.insert( pSocket );
				}
			}
		}

		if( ssSockets.size() >= cMaxStrings )
			return LT_UNSUPPORTED;

		// Copy the strings in the set to the strings in the dropdown list...

		StringSet::iterator iter;
		for( iter = ssSockets.begin(); iter != ssSockets.end(); ++iter )
		{
			LTStrCpy( aszStrings[(*pcStrings)++], (*iter).c_str(), cMaxStringLength );
		}

		return LT_OK;
	}
	else
	{
		return LT_UNSUPPORTED;
	}
};


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CActiveWeapon::Term
//
//	PURPOSE:	Remove the hand held model if it has one...
//
// ----------------------------------------------------------------------- //

void CActiveWeapon::Term() 
{
	Detach();

	m_hWeapon	= NULL;
	m_hAmmo		= NULL;
	
	m_sSocket.clear();
	m_sDualWeaponSocket.clear();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CActiveWeapon::Detach
//
//	PURPOSE:	Remove the hand held model if it has one...
//
// ----------------------------------------------------------------------- //

void CActiveWeapon::Detach()
{
	if( m_hHHWeaponModel )
	{
		g_pLTServer->RemoveObject( m_hHHWeaponModel );
		m_hHHWeaponModel = NULL;
	}

	if( m_hDualWeaponModel )
	{
		g_pLTServer->RemoveObject( m_hDualWeaponModel );
		m_hDualWeaponModel = NULL;
	}

	// Clear the old activeweapon on the weapon.
	if( m_pWeapon )
	{
		m_pWeapon->SetActiveWeapon( NULL );
		m_pWeapon = NULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CActiveWeapon::CreateSpawnString
//
//	PURPOSE:	Creates a spawn string for when we are dropped...
//
// ----------------------------------------------------------------------- //

void CActiveWeapon::CreateSpawnString( char* szSpawn, uint32 dwBufLen )
{
	szSpawn[0] = '\0';

	if( m_hWeapon && m_hAmmo )
	{
		// [BJL 7/19/04] - Preserve the old behavior, described in KLS 
		// comment, but also allow giving the weapon straight based on a 
		// data flag.
		// [KLS 7/15/02] - Pick a random value between 1 and clip-size for the
		// ammo...
		int32 nAmmoCount = 0;
		HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(m_hWeapon, IsAI(m_pWeapon->GetObject()));

		if ( g_pWeaponDB->GetBool( hWpnData, WDB_WEAPON_bWhenDroppedGiveAmmoInClip ) )
		{
			nAmmoCount = m_pWeapon->GetAmmoInClip();
		}
		else
		{
			if( IsPlayer( m_pWeapon->GetObject( )))
			{
				// Player dropped the weapon so keep the total amount of ammo with the weapon...
				CPlayerObj *pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject( m_pWeapon->GetObject( )));
				if( pPlayer )
				{
					nAmmoCount = pPlayer->GetArsenal( )->GetAmmoCount( m_hAmmo );
				}
			}
			else
			{
				// Supply the initial amount, the player may modify this when they actually pick it up...
				HWEAPONDATA hAmmoData = g_pWeaponDB->GetAmmoData( m_hAmmo );
				nAmmoCount = g_pWeaponDB->GetInt32( hAmmoData, WDB_AMMO_nPickupInitialAmount );
			}
			
		}

		// [BJL 11/04/04] - Optionally disable movement to the floor based on
		// the weapon properties.

		bool bMoveToFloor = g_pWeaponDB->GetBool( hWpnData, WDB_WEAPON_bDroppedMoveToFloor );

		LTSNPrintF( szSpawn, dwBufLen, "WeaponItem Gravity 0;AmmoAmount %d;WeaponType (%s);AmmoType (%s);MPRespawn 0;MoveToFloor %d;Health %d;Placed 0",
			nAmmoCount, g_pWeaponDB->GetRecordName( m_hWeapon ), g_pWeaponDB->GetRecordName( m_hAmmo ), bMoveToFloor, m_pWeapon->GetHealth() );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CActiveWeapon::CreateAttachString
//
//	PURPOSE:	Creates an attach string for when we need to be attached...
//
// ----------------------------------------------------------------------- //

void CActiveWeapon::CreateAttachString( char* szAttach, uint32 dwBufLen )
{
	szAttach[0] = '\0';

	if( m_hWeapon && m_hAmmo && !m_sSocket.empty() )
	{
		const char *pszWeapon = g_pWeaponDB->GetRecordName( m_hWeapon );
		if( !pszWeapon )	
			return;

		const char *pszAmmo = g_pWeaponDB->GetRecordName( m_hAmmo );
		if( !pszAmmo )
			return;

		LTSNPrintF( szAttach, dwBufLen, "ATTACH %s (%s,%s)", m_sSocket.c_str(), pszWeapon, pszAmmo );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CActiveWeapon::Save
//
//	PURPOSE:	Save the active weapon...
//
// ----------------------------------------------------------------------- //

void CActiveWeapon::Save( ILTMessage_Write *pMsg )
{
	if( !pMsg )
		return;

	SAVE_HRECORD( m_hWeapon );
	SAVE_HRECORD( m_hAmmo );
	SAVE_STDSTRING( m_sSocket );
	SAVE_HOBJECT( m_hHHWeaponModel );
	SAVE_STDSTRING( m_sDualWeaponSocket );
	SAVE_HOBJECT( m_hDualWeaponModel );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CActiveWeapon::Load
//
//	PURPOSE:	Load the active weapon..
//
// ----------------------------------------------------------------------- //

void CActiveWeapon::Load( ILTMessage_Read *pMsg )
{
	if( !pMsg )
		return;

	LOAD_HRECORD( m_hWeapon, g_pWeaponDB->GetWeaponsCategory() );
	LOAD_HRECORD( m_hAmmo, g_pWeaponDB->GetAmmoCategory() );

	LOAD_STDSTRING( m_sSocket );
	LOAD_HOBJECT( m_hHHWeaponModel );

	LOAD_STDSTRING( m_sDualWeaponSocket );
	LOAD_HOBJECT( m_hDualWeaponModel );
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::RemoveWeapon()
//
//	PURPOSE:	Mark a specific weapon as removed
//
// ----------------------------------------------------------------------- //

void CArsenal::RemoveWeapon( HWEAPON hWeapon )
{
	if( !m_pWeapons || !hWeapon )
		return;

	//if this is our current weapon, deselect it
	if (m_hCurWeapon == hWeapon)
		DeselectCurWeapon();

	//unacquire the weapon
	uint32 nWeaponIndex	= g_pWeaponDB->GetRecordIndex( hWeapon );
	if( m_pWeapons && m_pWeapons[nWeaponIndex] )
	{
		m_pWeapons[nWeaponIndex]->Drop();
	}

#ifdef PROJECT_FEAR	//!!ARL: FEAR wants all ammo to disappear when you drop weapons, the DARK wants to keep the ammo around in case you pick the weapon back up.  (see also CPlayerStats::RemoveWeapon)
	//step through the ammo types used by this weapon, and remove those that are only used by this weapon
	HWEAPONDATA hWpnData = g_pWeaponDB->GetWeaponData(hWeapon,IsAI(m_hObject));
	uint32 nNumAmmos = g_pWeaponDB->GetNumValues( hWpnData, WDB_WEAPON_rAmmoName );
	for( uint32 nAmmo = 0; nAmmo < nNumAmmos; ++nAmmo )
	{

		HAMMO hAmmo = g_pWeaponDB->GetRecordLink( hWpnData, WDB_WEAPON_rAmmoName, nAmmo );
		if (hAmmo && !CanUseAmmo(hAmmo))
		{
			uint32 nAmmoIndex = g_pWeaponDB->GetRecordIndex( hAmmo );
			m_pAmmo[nAmmoIndex] = 0;
		}
	}
#endif

	// Notify the client if this is a player's weapon
	if( m_pPlayer)
	{
		// Send the appropriate message to the client...

		HCLIENT hClient = m_pPlayer->GetClient();
		if (hClient)
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8(MID_PLAYER_INFOCHANGE);
			cMsg.Writeuint8(IC_WEAPON_REMOVE_ID);
			cMsg.WriteDatabaseRecord( g_pLTDatabase, hWeapon );
			g_pLTServer->SendToClient(cMsg.Read(), hClient, MESSAGE_GUARANTEED);
		}

		// If there isn't currently a weapon selected, select a new weapon..

		if( m_hCurWeapon == NULL )
		{
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CArsenal::GetWeaponAttachmentProperties()
//
//	PURPOSE:	Returns by parameter a weapon attachment properties based 
//				on the index. Returns true if the result is valid, false 
//				if it is not.
//
// ----------------------------------------------------------------------- //

bool CArsenal::GetWeaponAttachmentProperties( uint32 index, HWEAPON& hOutWeapon, HAMMO& hOutAmmo, const char** pszOutSocketName )
{
	if ( index < 0 || index > m_ReadPropWeaponList.size() || !pszOutSocketName )
	{
		return false;
	}

	hOutWeapon		= m_ReadPropWeaponList[index].m_hWeapon;
	hOutAmmo		= m_ReadPropWeaponList[index].m_hAmmo;
	*pszOutSocketName = m_ReadPropWeaponList[index].m_sSocket.c_str();
	return true;
}
