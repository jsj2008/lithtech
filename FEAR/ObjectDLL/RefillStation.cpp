// ----------------------------------------------------------------------- //
//
// MODULE  : RefillStation.cpp
//
// PURPOSE : Creates a designated area where players can refill their inventory supply...
//
// CREATED : 09/29/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

#include "Stdafx.h"
#include "RefillStation.h"
#include "PropsDB.h"
#include "Spawner.h"
#include "CharacterMgr.h"
#include "Prop.h"
#include "PlayerObj.h"
#include "ServerSoundMgr.h"

LINKFROM_MODULE( RefillStation )

//
// Register properties with the WorldEdit...
//

// Hide this object in Dark.
#if defined ( PROJECT_DARK )

	#define CF_HIDDEN_REFILLSTATION CF_HIDDEN

#elif defined ( PROJECT_FEAR )

	// JSC this has to be set to a value or else the parameter will be invalid
	#define CF_HIDDEN_REFILLSTATION 0

#endif

BEGIN_CLASS( RefillStation )
	ADD_STRINGPROP_FLAG( Filename, "", PF_HIDDEN | PF_MODEL, "This hidden property is needed in order to get the model visible within WorldEdit." )
	ADD_STRINGPROP_FLAG( RefillStation, SELECTION_NONE, PF_STATICLIST | PF_DIMS | PF_LOCALDIMS, "Record within the game database to use as the template for this RefillStation." )
	ADD_BOOLPROP_FLAG( MoveToFloor, true, 0, "Tells the RefillStation to appear on the floor when the level opens, even if you placed it in the air inside WorldEdit." )
	ADD_PREFETCH_RESOURCE_PROPS()
END_CLASS_FLAGS_PLUGIN_PREFETCH(RefillStation, GameBase, CF_HIDDEN_REFILLSTATION, RefillStationPlugin, DefaultPrefetch<RefillStation>, "Creates a designated area where players can refill their inventory supply." )


//
// Register messages with the CommandMgr...
//

CMDMGR_BEGIN_REGISTER_CLASS( RefillStation )
CMDMGR_END_REGISTER_CLASS( RefillStation, GameBase )


//
// RefillStationPlugin implementation...
//

LTRESULT RefillStationPlugin::PreHook_EditStringList( const char *szRezPath,
													  const char *szPropName,
													  char **aszStrings,
													  uint32 *pcStrings,
													  const uint32 cMaxStrings,
													  const uint32 cMaxStringLen )
{
	if( !aszStrings || !pcStrings || !g_pWeaponDB )
	{
		LTERROR( "Invalid input parameters" );
		return LT_UNSUPPORTED;
	}

	if( LTStrEquals( szPropName, "RefillStation" ))
	{
		// Fill the first string in the list with a <none> selection...
		LTStrCpy( aszStrings[(*pcStrings)++], SELECTION_NONE, cMaxStringLen );

		// Add an entry for each turret type...

		uint8 nNumRefillStations = DATABASE_CATEGORY( RefillStations ).GetNumRecords( );
		for( uint8 nRefillStation = 0; nRefillStation < nNumRefillStations; ++nRefillStation )
		{
			LTASSERT( cMaxStrings > (*pcStrings) + 1, "Too many refill stations to fit in the list.  Enlarge list size?" );

			HRECORD hRefillStation = DATABASE_CATEGORY( RefillStations ).GetRecordByIndex( nRefillStation );
			if( !hRefillStation )
				continue;

			const char *pszRefillStationName = DATABASE_CATEGORY( RefillStations ).GetRecordName( hRefillStation );
			if( !pszRefillStationName )
				continue;

			if( (LTStrLen( pszRefillStationName ) < cMaxStringLen) && ((*pcStrings) + 1 < cMaxStrings) )
			{
				LTStrCpy( aszStrings[(*pcStrings)++], pszRefillStationName, cMaxStringLen );
			}
		}
		
		// Sort the list so RefillStation types are easier to find...
		qsort( aszStrings, *pcStrings, sizeof(char *), CaseInsensitiveCompare );

		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

LTRESULT RefillStationPlugin::PreHook_Dims( const char* szRezPath,
										   const char* szPropName, 
											const char* szPropValue,
											char* szModelFilenameBuf,
											int nModelFilenameBufLen,
											LTVector &vDims,
											const char* pszObjName, 
											ILTPreInterface *pInterface)
{
	if( !szModelFilenameBuf || nModelFilenameBufLen < 1 )
		return LT_UNSUPPORTED;

	szModelFilenameBuf[0] = '\0';
	
	// Selecting the none option is ok...
	if( LTStrIEquals( szPropValue, SELECTION_NONE ))
		return LT_OK;

	HRECORD hRefillStation = DATABASE_CATEGORY( RefillStations ).GetRecordByName( szPropValue );
	if( !hRefillStation )
		return LT_UNSUPPORTED;

	HRECORD hProp = DATABASE_CATEGORY( RefillStations ).GETRECORDATTRIB( hRefillStation, Model );
	const char *pszModel = g_pPropsDB->GetPropFilename( hProp );
	if( !pszModel )
		return LT_UNSUPPORTED;

	LTStrCpy( szModelFilenameBuf, pszModel, nModelFilenameBufLen );
	
	// Fill out the object dims using the radius value...
	uint32 nRadius = DATABASE_CATEGORY( RefillStations ).GETRECORDATTRIB( hRefillStation, Radius );
	vDims.x = vDims.y = vDims.z = (float)nRadius;
	
	return LT_OK;
}


//
// RefillStation implementation
//

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	RefillStation::RefillStation
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

RefillStation::RefillStation( )
:	GameBase			( OT_NORMAL ),
	m_hRefillStation	( NULL ),
	m_iHealth			( 0 ),
	m_iArmor			( 0 ),
	m_iSlowMo			( 0 ),
	m_bUpdating			( false ),
	m_hRefillSound		( NULL ),
	m_hModelObject		( NULL )
{

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	RefillStation::~RefillStation
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

RefillStation::~RefillStation( )
{
	g_pLTServer->SoundMgr( )->KillSoundLoop( m_hRefillSound );

	if( m_hModelObject )
	{
		g_pLTServer->RemoveObject( m_hModelObject );
		m_hModelObject = NULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RefillStation::EngineMessageFn
//
//	PURPOSE:	Handle messages from the engine...
//
// ----------------------------------------------------------------------- //

uint32 RefillStation::EngineMessageFn( uint32 messageID, void *pData, float fData )
{
	switch( messageID )
	{
		case MID_PRECREATE:
		{
			if( fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP )
			{
				ObjectCreateStruct *pOCS = (ObjectCreateStruct*)pData;
				if( !ReadProp( &pOCS->m_cProperties ))
					return 0;

				PostReadProp( pOCS );
			}
		}
		break;

		case MID_OBJECTCREATED:
		{
			if( fData == OBJECTCREATED_WORLDFILE || fData == OBJECTCREATED_STRINGPROP )
			{
				ObjectCreated( (GenericPropList*)pData );
			}
		}
		break;

		case MID_TOUCHNOTIFY:
		{
			ObjectTouch( (HOBJECT)pData );
		}
		break;

		case MID_UPDATE:
		{
			Update( );
		}
		break;

		case MID_SAVEOBJECT:
		{
			Save( (ILTMessage_Write*)pData, (uint32)fData );
		}
		break;

		case MID_LOADOBJECT:
		{
			Load( (ILTMessage_Read*)pData, (uint32)fData );
		}
		break;

		default:
		break;
	}

	return GameBase::EngineMessageFn( messageID, pData, fData );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RefillStation::ReadProp
//
//	PURPOSE:	Read in the properties of the object...
//
// ----------------------------------------------------------------------- //

bool RefillStation::ReadProp( const GenericPropList *pProps )
{
	if( !pProps )
		return false;

	// Obtain the template used for this refill station...
	const char *pszRefillStation = pProps->GetString( "RefillStation", "" );
	m_hRefillStation = DATABASE_CATEGORY( RefillStations ).GetRecordByName( pszRefillStation );
	if( !m_hRefillStation )
		return false;

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RefillStation::PostReadProp
//
//	PURPOSE:	Update the create struct of the object andother data after obtaining the property values...
//
// ----------------------------------------------------------------------- //

void RefillStation::PostReadProp( ObjectCreateStruct *pOCS )
{
	if( !pOCS )
		return;

	// Start with the max amounts of inventory...

	HATTRIBUTE hHealth = DATABASE_CATEGORY( RefillStations ).GetAttribute( m_hRefillStation, "Health" );
	m_iHealth = DATABASE_CATEGORY( RefillStations ).GETSTRUCTATTRIB( Health, hHealth, 0, TotalAmount );

	// Infinite health if set negative...
	if( m_iHealth < 0 )
		m_iHealth = kRefillInfinite;

	HATTRIBUTE hArmor = DATABASE_CATEGORY( RefillStations ).GetAttribute( m_hRefillStation, "Armor" );
	m_iArmor = DATABASE_CATEGORY( RefillStations ).GETSTRUCTATTRIB( Armor, hArmor, 0, TotalAmount );

	// Infinite armor if set negative...
	if( m_iArmor < 0 )
		m_iArmor = kRefillInfinite;

	HATTRIBUTE hSlowMo = DATABASE_CATEGORY( RefillStations ).GetAttribute( m_hRefillStation, "SlowMo" );
	m_iSlowMo = DATABASE_CATEGORY( RefillStations ).GETSTRUCTATTRIB( SlowMo, hSlowMo, 0, TotalAmount );

	// Infinite slow-mo if set negative...
	if( m_iSlowMo < 0 )
		m_iSlowMo = kRefillInfinite;

	// Read in the ammo amounts...
	HATTRIBUTE hAmmo = DATABASE_CATEGORY( RefillStations ).GetAttribute( m_hRefillStation, "Ammo" );
	uint32 nNumAmmo = DATABASE_CATEGORY( RefillStations ).GetNumValues( hAmmo );

	m_vecAmmo.resize( nNumAmmo );
	for( uint32 nAmmo = 0; nAmmo < nNumAmmo; ++nAmmo )
	{
		m_vecAmmo[nAmmo] = DATABASE_CATEGORY( RefillStations ).GETSTRUCTATTRIB( Ammo, hAmmo, nAmmo, TotalAmount );
		if( m_vecAmmo[nAmmo] < 0 )
			m_vecAmmo[nAmmo] = kRefillInfinite;
	}

	// Set object flags...
	pOCS->m_Flags = FLAG_TOUCH_NOTIFY | FLAG_GOTHRUWORLD;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RefillStation::ObjectCreated
//
//	PURPOSE:	Handle any adjustments to the object after it has been created...
//
// ----------------------------------------------------------------------- //

void RefillStation::ObjectCreated( const GenericPropList *pProps )
{
	if( !pProps )
		return;

	bool bMoveToFloor = pProps->GetBool( "MoveToFloor", true );

	float fRadius = (float)DATABASE_CATEGORY( RefillStations ).GETRECORDATTRIB( m_hRefillStation, Radius );

	LTVector vDims( fRadius, fRadius, fRadius );
	g_pPhysicsLT->SetObjectDims( m_hObject, &vDims, 0 );

	if( bMoveToFloor )
		MoveObjectToFloor( m_hObject );

	// Create the Prop object associated with the RefillStation, if one exists...
	HRECORD hProp = DATABASE_CATEGORY( RefillStations ).GETRECORDATTRIB( m_hRefillStation, Model );
	if( hProp )
	{
		LTRigidTransform tProp;
		g_pLTServer->GetObjectTransform( m_hObject, &tProp );

		char szSpawn[256] = {0};
		LTSNPrintF( szSpawn, LTARRAYSIZE(szSpawn), "Prop Model %s; MoveToFloor %i; Solid 1; NeverDestroy 1;",
					g_pPropsDB->GetRecordName( hProp ), bMoveToFloor );

		Prop *pProp = dynamic_cast<Prop*>(SpawnObject( szSpawn, tProp.m_vPos, tProp.m_rRot ));
		if( pProp && pProp->m_hObject )
		{
			// Make sure the prop is solid on the server and client...
			g_pCommonLT->SetObjectFlags( pProp->m_hObject, OFT_Flags, FLAG_SOLID, FLAG_SOLID | FLAG_CLIENTNONSOLID );

			// Cache the model object so we can remove it if the refill station goes away...
			m_hModelObject = pProp->m_hObject;
		}
	}

	// Don't update untill a player is close enough to us...
	SetNextUpdate( UPDATE_NEVER );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RefillStation::ObjectTouch
//
//	PURPOSE:	Handle an object touching the dims of the station...
//
// ----------------------------------------------------------------------- //

void RefillStation::ObjectTouch( HOBJECT hObject )
{
	if( !hObject )
		return;

	// Currently updating so don't reset the next update...
	if( m_bUpdating )
		return;

	if( IsPlayer( hObject ))
	{
		float fUpdateRate = DATABASE_CATEGORY( RefillStations ).GETRECORDATTRIB( m_hRefillStation, UpdateRate ) / 1000.0f;
		SetNextUpdate( fUpdateRate );

		// Begin updating...
		m_bUpdating = true;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RefillStation::Update
//
//	PURPOSE:	Handle an update for the refill station...
//
// ----------------------------------------------------------------------- //

void RefillStation::Update( )
{
	// Check if the player is within our radius...

	LTVector vPos;
	g_pLTServer->GetObjectPos( m_hObject, &vPos );

	float fRadius = (float)DATABASE_CATEGORY( RefillStations ).GETRECORDATTRIB( m_hRefillStation, Radius );

	uint32 nTestPlayerCount = 0;
	
	// Don't update anymore if no players in radius this update...
	m_bUpdating = false;

	bool bUpdatedPlayer = false;

	CTList<CCharacter*>	lstChars;
	if( g_pCharacterMgr->FindCharactersWithinRadius( &lstChars, vPos, fRadius, NULL, CCharacterMgr::kList_Players  ))
	{
		CCharacter **pCur = lstChars.GetItem( TLIT_FIRST );
		while( pCur )
		{
			CPlayerObj *pPlayer = dynamic_cast<CPlayerObj*>(*pCur);
			if( pPlayer && pPlayer->IsAlive( ))
			{
				bUpdatedPlayer |= UpdatePlayerRefill( pPlayer );
				m_bUpdating = true;
			}

			pCur = lstChars.GetItem( TLIT_NEXT );
		}
	}

	if( IsEmpty( ))
	{
		// Supply is fully depleted...
		HRECORD hDepletedSound = DATABASE_CATEGORY( RefillStations ).GETRECORDATTRIB( m_hRefillStation, DepletedSound );
		g_pServerSoundMgr->PlayDBSoundFromPos( vPos, hDepletedSound );
		
		m_bUpdating = false;
	}

	if( m_bUpdating )
	{
		float fUpdateRate = DATABASE_CATEGORY( RefillStations ).GETRECORDATTRIB( m_hRefillStation, UpdateRate ) / 1000.0f;
		SetNextUpdate( fUpdateRate );

		if( bUpdatedPlayer && !m_hRefillSound )
		{
			HRECORD hRefillSound = DATABASE_CATEGORY( RefillStations ).GETRECORDATTRIB( m_hRefillStation, RefillSound );
			m_hRefillSound = g_pServerSoundMgr->PlayDBSoundFromPos( vPos, hRefillSound, SMGR_INVALID_RADIUS, SOUNDPRIORITY_INVALID, PLAYSOUND_LOOP | PLAYSOUND_GETHANDLE );
		}
		else if( !bUpdatedPlayer )
		{
			g_pLTServer->SoundMgr( )->KillSoundLoop( m_hRefillSound );
			m_hRefillSound = NULL;
		}
	}
	else
	{
		g_pLTServer->SoundMgr( )->KillSoundLoop( m_hRefillSound );
		m_hRefillSound = NULL;

		// No players in radius so don't update anymore...
		SetNextUpdate( UPDATE_NEVER );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RefillStation::IsEmpty
//
//	PURPOSE:	Test to see if the RefillStation has any inventory left...
//
// ----------------------------------------------------------------------- //

bool RefillStation::IsEmpty( )
{
	bool bHasHealth = false;
	if( (m_iHealth == kRefillInfinite) || (m_iHealth > 0) )
		bHasHealth = true;

	bool bHasArmor = false;
	if( (m_iArmor == kRefillInfinite) || (m_iArmor > 0) )
		bHasArmor = true;

	bool bHasSlowMo = false;
	if( (m_iSlowMo == kRefillInfinite) || (m_iSlowMo > 0) )
		bHasSlowMo = true;

	bool bHasAmmo = false;
	int32Array::iterator iter;
	for( iter = m_vecAmmo.begin( ); iter != m_vecAmmo.end( ); ++iter )
	{
		if( (*iter == kRefillInfinite) || (*iter > 0) )
			bHasAmmo = true;
	}

	return !(bHasHealth || bHasArmor || bHasSlowMo || bHasAmmo);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RefillStation::UpdatePlayerRefill
//
//	PURPOSE:	Send the player the refill data and update inventory accordingly...
//
// ----------------------------------------------------------------------- //

bool RefillStation::UpdatePlayerRefill( CPlayerObj *pPlayer )
{
	if( !pPlayer )
		return false;

	bool bUpdatedInventory = false;

	if( (m_iHealth > 0) || (m_iHealth == kRefillInfinite) )
	{
		// Add health to the players' inventory...
		HATTRIBUTE hHealth = DATABASE_CATEGORY( RefillStations ).GetAttribute( m_hRefillStation, "Health" );
		uint32 nHealthPerUpdate = DATABASE_CATEGORY( RefillStations ).GETSTRUCTATTRIB( Health, hHealth, 0, AmountPerUpdate );

		bool bAddedHealth = pPlayer->GetInventory( )->AddHealth( nHealthPerUpdate );
		bUpdatedInventory |= bAddedHealth;
		
		// Reduce health supply if added to player successfully and don't have infinite health...
		if( bAddedHealth && (m_iHealth != kRefillInfinite) )
			m_iHealth -= nHealthPerUpdate;
	}

	if( (m_iArmor > 0) || (m_iArmor == kRefillInfinite) )
	{
		// Add armor to the players' inventory...
		HATTRIBUTE hArmor = DATABASE_CATEGORY( RefillStations ).GetAttribute( m_hRefillStation, "Armor" );
		uint32 nArmorPerUpdate = DATABASE_CATEGORY( RefillStations ).GETSTRUCTATTRIB( Armor, hArmor, 0, AmountPerUpdate );
		
		bool bAddedArmor = pPlayer->GetInventory( )->AddArmor( nArmorPerUpdate );
		bUpdatedInventory |= bAddedArmor;

		// Reduce armor supply if added to player successfully and don't have infinite armor...
		if( bAddedArmor && (m_iArmor != kRefillInfinite) )
			m_iArmor -= nArmorPerUpdate;
	}

	if( (m_iSlowMo > 0) || (m_iSlowMo == kRefillInfinite) )
	{
		// Add slow-mo time to the players' inventory...
		HATTRIBUTE hSlowMo = DATABASE_CATEGORY( RefillStations ).GetAttribute( m_hRefillStation, "SlowMo" );
		uint32 nSlowMoPerUpdate = DATABASE_CATEGORY( RefillStations ).GETSTRUCTATTRIB( SlowMo, hSlowMo, 0, AmountPerUpdate );

		bool bAddedSlowMo = pPlayer->GetInventory( )->AddSlowMo( nSlowMoPerUpdate );
		bUpdatedInventory |= bAddedSlowMo;

		// REduce slow-mo supply if added to player successfully and don't have infinite slow-mo...
		if( bAddedSlowMo && (m_iSlowMo != kRefillInfinite) )
			m_iSlowMo -= nSlowMoPerUpdate;
	}

	HATTRIBUTE hAmmo = DATABASE_CATEGORY( RefillStations ).GetAttribute( m_hRefillStation, "Ammo" );
	for( uint32 nAmmo = 0; nAmmo < m_vecAmmo.size( ); ++nAmmo )
	{
		if( (m_vecAmmo[nAmmo] > 0) || (m_vecAmmo[nAmmo] == kRefillInfinite) )
		{
			HAMMO hAmmoType = DATABASE_CATEGORY( RefillStations ).GETSTRUCTATTRIB( Ammo, hAmmo, nAmmo, Ammo );
			uint32 nAmmoPerUpdate = DATABASE_CATEGORY( RefillStations ).GETSTRUCTATTRIB( Ammo, hAmmo, nAmmo, AmountPerUpdate );

			bool bAddedAmmo = pPlayer->GetInventory( )->AddAmmo( hAmmoType, nAmmoPerUpdate );
			bUpdatedInventory |= bAddedAmmo;

			if( bAddedAmmo && (m_vecAmmo[nAmmo] != kRefillInfinite) )
				m_vecAmmo[nAmmo] -= nAmmoPerUpdate;
		}
	}

	return bUpdatedInventory;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RefillStation::Save
//
//	PURPOSE:	Save the object...
//
// ----------------------------------------------------------------------- //

void RefillStation::Save( ILTMessage_Write *pMsg, uint32 dwSaveFlags )
{
	if( !pMsg )
		return;

	SAVE_HRECORD( m_hRefillStation );
	SAVE_INT( m_iHealth );
	SAVE_INT( m_iArmor );
	SAVE_INT( m_iSlowMo );
	SAVE_BOOL( m_bUpdating );
	SAVE_HOBJECT( m_hModelObject );

	SAVE_INT( m_vecAmmo.size( ));

	uint32 nNumAmmo = m_vecAmmo.size( );
	for( uint32 nAmmo = 0; nAmmo < nNumAmmo; ++nAmmo )
	{
		SAVE_INT( m_vecAmmo[nAmmo] );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RefillStation::Load
//
//	PURPOSE:	Load the object...
//
// ----------------------------------------------------------------------- //

void RefillStation::Load( ILTMessage_Read *pMsg, uint32 dwLoadFlags )
{
	if( !pMsg )
		return;

	LOAD_HRECORD( m_hRefillStation, DATABASE_CATEGORY( RefillStations ).GetCategory( ));
	LOAD_INT( m_iHealth );
	LOAD_INT( m_iArmor );
	LOAD_INT( m_iSlowMo );
	LOAD_BOOL( m_bUpdating );
	LOAD_HOBJECT( m_hModelObject );

	uint32 nNumAmmo;
	LOAD_INT( nNumAmmo );

	m_vecAmmo.resize( nNumAmmo );
	for( uint32 nAmmo = 0; nAmmo < nNumAmmo; ++nAmmo )
	{
		LOAD_INT( m_vecAmmo[nAmmo] );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	RefillStation::GetPrefetchResourceList
//
//	PURPOSE:	determines the list of all needed resources
//
// ----------------------------------------------------------------------- //

void RefillStation::GetPrefetchResourceList(const char* pszObjectName, IObjectResourceGatherer* pInterface, ResourceList& Resources )
{
	// get the refill station type
	char szRefillStation[MAX_PATH];
	pInterface->GetPropString(pszObjectName, "RefillStation", szRefillStation, LTARRAYSIZE(szRefillStation), "");

	if (!LTStrEmpty(szRefillStation))
	{
		// gather resources from the database record for this type
		HRECORD hRefillStation = DATABASE_CATEGORY( RefillStations ).GetRecordByName( szRefillStation );
		GetRecordResources(Resources, hRefillStation, true);
	}
}

// EOF

