// ----------------------------------------------------------------------- //
//
// MODULE  : PickupItem.cpp
//
// PURPOSE : Item that any player can walk across and potentially pick up -
//			 Implementation
//
// CREATED : 10/27/97
//
// (c) 1997-2003 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "PickupItem.h"
#include "MsgIDs.h"
#include "iltserver.h"
#include "ServerUtilities.h"
#include "ClientServerShared.h"
#include "GameServerShell.h"
#include "PlayerObj.h"
#include "VarTrack.h"
#include "SoundMgr.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "CharacterHitBox.h"
#include "ServerSoundMgr.h"
#include "VersionMgr.h"
#include "PropsDB.h"
#include "Spawner.h"
#include "AIUtils.h"
#include "LTEulerAngles.h"
#include "PhysicsUtilities.h"
#include "GameModeMgr.h"
#include "iltphysicssim.h"
#include "CollisionsDB.h"
#include "TeamMgr.h"

LINKFROM_MODULE( PickupItem );

extern CGameServerShell* g_pGameServerShell;

VarTrack g_RespawnScaleTrack;

const char* c_aWorldAnimations[] =
{
	"World",
	"BindPos",
};

// mrice.3.25.2005. the dark does not pick up weapons on collision
#if defined ( PROJECT_DARK )
	#define PICKUP_ON_TOUCHED	(0)
#else
	#define PICKUP_ON_TOUCHED	(1)
#endif

#define PICKUP_MAX_WORLD_ANIS	2

#define PICKUP_ITEM_SHADOW_LOD	eEngineLOD_Medium

#define ModelNone	"<none>"

// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //
//
//	CLASS:		PickupItem
//
//	PURPOSE:	Any in-game object that the player can pick up
//
// ----------------------------------------------------------------------- //
// ----------------------------------------------------------------------- //

BEGIN_CLASS(PickupItem)
	ADD_STRINGPROP_FLAG(Filename, "", PF_FILENAME | PF_MODEL | PF_HIDDEN, "Specifies the model filename for this pickup item")
	ADD_STRINGPROP_FLAG(Material, "", PF_FILENAME | PF_HIDDEN, "Specifies the material to use with the pickup model")
	ADD_STRINGPROP_FLAG(Model, ModelNone, PF_STATICLIST | PF_DIMS | PF_LOCALDIMS, "Contains a list of prop objects that can be used for the model.")
	ADD_REALPROP(Scale, 1.0f, "This value changes the size of the object. It is a multiplicative value based on the original size of the object. The default scale is 1.0.")
	ADD_VISIBLE_FLAG(1, 0)
	ADD_SHADOW_FLAG(1, 0)
	ADD_SOLID_FLAG(0, PF_HIDDEN)
	ADD_GRAVITY_FLAG(0, PF_HIDDEN)
	ADD_COMMANDPROP_FLAG(PickupCommand, "", PF_NOTIFYCHANGE, "This is the command that is sent when the player picks up the WeaponItem.")
	ADD_BOOLPROP(SPRespawn, false, "Singleplayer: Item will respawn in RespawnTime.")
	ADD_BOOLPROP(MPRespawn, true, "Multiplayer: Item will respawn in RespawnTime.")
	ADD_REALPROP(RespawnTime, 10.0f, "The amount of time in seconds before a powerup reappears after being picked up by a player.")
	ADD_REALPROP_FLAG(LifeTime, -1.0f, PF_HIDDEN, "The amount of time in seconds before the powerup disappears after being dropped by a player.")
    ADD_BOOLPROP(MoveToFloor, true, "Whether or not the object is moved to the floor when created in the game.")
	ADD_BOOLPROP_FLAG(TouchPickup, true, PF_HIDDEN, "TODO:PROPDESC" ) // Hidden (all pickups can be touchable)
	ADD_BOOLPROP_FLAG(DMTouchPickup, true, PF_HIDDEN, "TODO:PROPDESC" ) // Hidden (we want pickups to be touchable in DM)
	ADD_STRINGPROP_FLAG(ModelOverride, ModelNone, PF_STATICLIST /*| PF_DIMS | PF_LOCALDIMS*/, "Contains a list of ModelBute objects (type=3) that can be used to override the default model.")
	
	ADD_STRINGPROP_FLAG( WorldAnimation, "World", PF_DYNAMICLIST, "Specifies the name of the animation to use for the model.  The default 'World' animation is typically lying flat on the ground.  The 'BinPos' animation should be an animation created in model edit with the 'Create anim from Bind Pose' menu option.  The model that is visible in WorldEdit is using the models bind pose so setting this property to 'BindPos' will allow LD's to rotate the model and have it look exactly the same in game as it does in WorldEdit." )
	ADD_STRINGPROP_FLAG(Team, "NoTeam", PF_STATICLIST, "This is a dropdown list that allows you to specify if a team owns this item.  Only players on the owning team will be able to pickup this item.  If NoTeam is selected then all players can pickup the item.")
	ADD_BOOLPROP_FLAG(Locked, false, 0, "Determines if this pickup starts out as locked (i.e. can't be picked up)." )
	ADD_BOOLPROP_FLAG(Placed, true, PF_HIDDEN, "Determines whether the item was placed in world edit or spawned at runtime" ) // Hidden

END_CLASS_FLAGS_PLUGIN(PickupItem, GameBase, CF_HIDDEN, CPickupItemPlugin, "Defines an object that can be picked up")


CMDMGR_BEGIN_REGISTER_CLASS( PickupItem )

	ADD_MESSAGE( ACTIVATE,	1,	NULL,	MSG_HANDLER( PickupItem, HandleActivateMsg ),	"ACTIVATE", "TODO:CMDDESC", "TODO:CMDEXP" )
	ADD_MESSAGE( TEAM,		2,	NULL,	MSG_HANDLER( PickupItem, HandleTeamMsg ),		"TEAM <0, 1, -1>", "TODO:CMDDESC", "TODO:CMDEXP" )
	ADD_MESSAGE( LOCKED,	2,	NULL,	MSG_HANDLER( PickupItem, HandleLockedMsg ),		"LOCKED <0/1>", "TODO:CMDDESC", "TODO:CMDEXP" )
	ADD_MESSAGE( RESPAWN,	1,	NULL,	MSG_HANDLER( PickupItem, HandleRespawnMsg ),	"RESPAWN", "Forces a respawn of the PickupItem after seconds.", "msg GearItem00 RESPAWN" )

CMDMGR_END_REGISTER_CLASS( PickupItem, GameBase )


LTRESULT CPickupItemPlugin::PreHook_PropChanged( const char *szObjName, 
												 const char *szPropName,
												 const int nPropType,
												 const GenericProp &gpPropValue,
												 ILTPreInterface *pInterface,
												 const char *szModifiers )
{
	// See if we can handle this property...

	if( LTStrIEquals( "PickupCommand", szPropName ))
	{
		if( m_CommandMgrPlugin.PreHook_PropChanged( szObjName,
													szPropName,
													nPropType,
													gpPropValue,
													pInterface,
													szModifiers ) == LT_OK )
		{
			return LT_OK;
		}
	}
	return LT_UNSUPPORTED;
}

LTRESULT CPickupItemPlugin::PreHook_EditStringList(const char* szRezPath,
												   const char* szPropName,
												   char** aszStrings,
												   uint32* pcStrings,
												   const uint32 cMaxStrings,
												   const uint32 cMaxStringLength)
{
	if( LTStrIEquals( "Model", szPropName ) || LTStrIEquals( "ModelOverride", szPropName ))
	{
		LTStrCpy(aszStrings[(*pcStrings)++], ModelNone, cMaxStringLength );

		uint32 cModels = g_pPropsDB->GetNumProps();
		LTASSERT(cMaxStrings >= cModels, "TODO: Add description here");
		for ( uint32 iModel = 0 ; iModel < cModels ; iModel++ )
		{
			// exit out early if we can't hold any more strings
			if( *pcStrings >= cMaxStrings )
				break;

			// Only list model templates in WorldEdit that have been
			// set in modelbutes.txt to type = generic prop.
			PropsDB::HPROP hProp = g_pPropsDB->GetProp( iModel );
			LTStrCpy( aszStrings[(*pcStrings)++], g_pPropsDB->GetRecordName( hProp ), cMaxStringLength );
		}

		qsort( aszStrings + 1, *pcStrings - 1, sizeof( char * ), CaseInsensitiveCompare );

		return LT_OK;
	}
	else if( LTStrIEquals( "WorldAnimation", szPropName ))
	{
		for( int i = 0; i < PICKUP_MAX_WORLD_ANIS; ++i )
		{
			LTStrCpy( aszStrings[(*pcStrings)++], c_aWorldAnimations[i], cMaxStringLength );
		}

		return LT_OK;
	}
	else if( LTStrIEquals( "Team", szPropName ))
	{
		TeamPopulateEditStringList( aszStrings, pcStrings, cMaxStrings, cMaxStringLength );
		return LT_OK;
	}


	return LT_UNSUPPORTED;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::PickupItem()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

PickupItem::PickupItem() 
:	GameBase				(OT_MODEL),
	m_hPlayerObj			( NULL ),
	m_fScale				( 1.0f ),
	m_fRespawnDelay			( 10.0f ),
	m_fLifeTime				( -1.0f ),
	m_bExpired				( false ),
	m_bMoveToFloor			( true ),
	m_bTouchPickup			( true ),
	m_bRespawn				( false ),
	m_bControlledRespawn	( false ),
	m_dwFlags				( 0 ),
	m_dwUserFlags			( 0 ),
	m_sPickupCommand		( ),
	m_sWorldAniName			( ),
	m_bWasPickedUp			( false ),
	m_bWasPlaced			( true ),
	m_bWasTouched			( false ),
	m_bDropped				( false ),
	m_fDropTime				( 0.0 ),
	m_hOriginalPickupObject	( NULL ),
	m_hDroppedBy			( NULL ),
	m_Shared				( )
{
	MakeTransitionable();

	m_bRespawn = (IsMultiplayerGameServer( ) ? true : false);
	m_hOverrideProp	= NULL;

	// assume they are moveable so they won't show bullet holes...
	m_dwUserFlags	= USRFLG_MOVEABLE;  

	m_bFullyCreated = false;

	m_nFrameCounter = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::~PickupItem()
//
//	PURPOSE:	Deallocate object
//
// ----------------------------------------------------------------------- //
PickupItem::~PickupItem()
{
	if( m_hOriginalPickupObject )
	{
		g_pCmdMgr->QueueMessage( m_hObject, m_hOriginalPickupObject, "RESPAWN" );
		m_hOriginalPickupObject = NULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 PickupItem::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_TOUCHNOTIFY:
		{
#if !PICKUP_ON_TOUCHED
			break;
#endif
			if( !m_bTouchPickup || !m_bFullyCreated || m_bExpired || m_bWasPickedUp)
				break;
			if( m_Shared.m_bLocked )
				break;


			//see if we've been touched already
			if (m_bWasTouched)
			{
				//see if we were touched by a player
				if (m_hPlayerObj && IsPlayer(m_hPlayerObj))
				{
					CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(m_hPlayerObj);
					//bail out if that player is still around
					if( pPlayer && pPlayer->GetClient())
						break;
				}
			}


			HOBJECT hObj = (HOBJECT)pData;

			// If this is a character hit box, use its object...

			CCharacterHitBox* pHitBox = NULL;
			if (IsCharacterHitBox(hObj))
			{
				pHitBox = (CCharacterHitBox*)g_pLTServer->HandleToObject(hObj);
				if (pHitBox)
				{
					hObj = pHitBox->GetModelObject();
				}
			}


			// If the object is dead, it can't pick up stuff...

			if (IsPlayer(hObj))
			{
                CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(hObj);

				if (!pPlayer || !pPlayer->IsAlive()) break;

				SetPlayerObj(hObj);
				m_bWasTouched = true;
			}
			else
			{
                SetPlayerObj(NULL);
				m_bWasTouched = false;
				break;
			}

			ObjectTouch(hObj);
		}
		break;

		case MID_PRECREATE:
		{
			uint32 nRet = GameBase::EngineMessageFn(messageID, pData, fData);
			if( !nRet )
				return nRet;

			ObjectCreateStruct* pInfo = (ObjectCreateStruct*)pData;
	
			if (fData == PRECREATE_WORLDFILE)
			{
				ReadProp(&pInfo->m_cProperties);
			}
			else if (fData == PRECREATE_STRINGPROP)
			{
				ReadProp(&pInfo->m_cProperties);

				// Show ourself...

				pInfo->m_Flags |= FLAG_VISIBLE;
			}

			PostPropRead(pInfo);

			return nRet;
		}
		break;

		case MID_INITIALUPDATE:
		{
			if (fData != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}
		}
		break;

		case MID_SAVEOBJECT:
		{
            Save((ILTMessage_Write*)pData, (uint32)fData);
		}
		break;

		case MID_LOADOBJECT:
		{
            Load((ILTMessage_Read*)pData, (uint32)fData);
		}
		break;

		case MID_SAVESPECIALEFFECTMESSAGE:
		{
			SaveSFXMessage( static_cast<ILTMessage_Write*>( pData ), static_cast<uint32>( fData ) );
		}
		break;

		case MID_LOADSPECIALEFFECTMESSAGE:
		{
			LoadSFXMessage( static_cast<ILTMessage_Read*>( pData ), static_cast<uint32>( fData ) );
		}
		break;

		case MID_ALLOBJECTSCREATED:
		{
			// Don't allow picking up until we are fully created.  This prevents spawned
			// pickup items from getting picked up before the spawning object is ready for it.
			// The Bombable object was creating pickup items which were getting picked up before
			// returning from SpawnObject and so Bombable wasn't able to set itself up correctly.
			m_bFullyCreated = true;
		}
		break;

		default : break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::ObjectMessageFn
//
//	PURPOSE:	Handle object messages
//
// ----------------------------------------------------------------------- //

uint32 PickupItem::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
{
	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();

	switch(messageID)
	{
		case MID_PICKEDUP:
		{
			bool bWasPickedUp = pMsg->Readbool( );
			bool bWeaponsStay = pMsg->Readbool( );

			if (!bWasPickedUp)
			{
				m_bWasPickedUp = false;
			}
			PickedUp( bWasPickedUp, bWeaponsStay );
			if (!bWasPickedUp)
			{
				//clear flags so someone else can try
				m_bWasTouched = false;
			}
		}
		break;

		default: break;
	}

	return GameBase::ObjectMessageFn(hSender, pMsg);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::HandleActivateMsg
//
//	PURPOSE:	Handle a ACTIVATE message...
//
// ----------------------------------------------------------------------- //

void PickupItem::HandleActivateMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Can't activate before we've respawned.
	if( m_bWasPickedUp )
		return;

	if( m_Shared.m_bLocked )
		return;

	// If the activating character is dead, he can't pick up stuff...
	if (IsPlayer(hSender))
	{
		CPlayerObj* pPlayer = (CPlayerObj*)g_pLTServer->HandleToObject(hSender);
		if( !pPlayer || !pPlayer->IsAlive() )
			return;

		SetPlayerObj(hSender);
		ObjectTouch(hSender);
	}
	else
	{
		if (IsAI(hSender))
		{
			ObjectTouch(hSender);
		}
		SetPlayerObj(NULL);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::HandleTeamMsg
//
//	PURPOSE:	Handle a TEAM message...
//
// ----------------------------------------------------------------------- //

void PickupItem::HandleTeamMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount( ) > 1 )
	{
		uint32 nTeamId = atoi( crParsedMsg.GetArg( 1 ));
		if( nTeamId < MAX_TEAMS )
		{
			ASSERT( nTeamId == ( uint8 )nTeamId );
			SetTeamId(( uint8 )LTMIN( nTeamId, 255 ));
		}
		else
		{
			SetTeamId( INVALID_TEAM );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::HandleLockedMsg
//
//	PURPOSE:	Handle a LOCKED message...
//
// ----------------------------------------------------------------------- //

void PickupItem::HandleLockedMsg( HOBJECT hSender, const CParsedMsg& crParsedMsg )
{
	SetLocked( !!atoi( crParsedMsg.GetArg( 1 ) ) );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::HandleRespawnMsg
//
//	PURPOSE:	Handle a RESPAWN message...
//
// ----------------------------------------------------------------------- //

void PickupItem::HandleRespawnMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Force a respawn on the PickupItem...

	Respawn( );
	SetNextUpdate( UPDATE_NEVER );
	m_RespawnTimer.Stop( );
	m_bWasPickedUp = false;	

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

bool PickupItem::ReadProp(const GenericPropList *pProps)
{
	if( !pProps )
		return false;

	m_sPickupCommand	= pProps->GetCommand( "PickupCommand", "" );

	m_bMoveToFloor = pProps->GetBool( "MoveToFloor", m_bMoveToFloor );

	// Whether we can pickup by touching depends on the game type...
	if( !IsMultiplayerGameServer() )
	{
		m_bTouchPickup = pProps->GetBool( "TouchPickup", m_bTouchPickup );
	}
	else
	{
		m_bTouchPickup = pProps->GetBool( "DMTouchPickup", m_bTouchPickup );
	}

	// Respawning is different for multiplayer gametypes...
	if( !IsMultiplayerGameServer( ))
	{
		m_bRespawn = pProps->GetBool( "SPRespawn", m_bRespawn );
	}
	else
	{
		m_bRespawn = pProps->GetBool( "MPRespawn", m_bRespawn );
	}

	m_fRespawnDelay		= pProps->GetReal( "RespawnTime", m_fRespawnDelay );
	m_fLifeTime			= pProps->GetReal( "LifeTime", m_fLifeTime );
	m_fScale			= pProps->GetReal( "Scale", m_fScale );
	m_sWorldAniName		= pProps->GetString( "WorldAnimation", "" );
	m_Shared.m_bLocked	= pProps->GetBool( "Locked", m_Shared.m_bLocked );
	m_bWasPlaced		= pProps->GetBool( "Placed", m_bWasPlaced );

	// Get the team this object belongs to.
	if( GameModeMgr::Instance( ).m_grbUseTeams )
	{
		m_Shared.m_nTeamId = TeamStringToTeamId( pProps->GetString( "Team", "" ));
	}
	else
	{
		m_Shared.m_nTeamId = INVALID_TEAM;
	}

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::PostPropRead()
//
//	PURPOSE:	Update Properties
//
// ----------------------------------------------------------------------- //

void PickupItem::PostPropRead(ObjectCreateStruct *pStruct)
{
	if (!pStruct) return;
	
	const GenericPropList *pProps = &pStruct->m_cProperties;

	pStruct->m_Flags |= FLAG_RAYHIT;
	//pStruct->m_Flags |= (m_bTouchPickup ? FLAG_TOUCH_NOTIFY : 0);

	// [RP] 8/31/02 The above line is commented out because if the object doesn't 
	//		have touch notify set it won't get added to containers and therefore won't 
	//		transition :(  Since we won't even process the touch message unless m_bTouchPickup
	//		is set it should be fine to set this flag to allow pickups to transition.

	pStruct->m_Flags |= FLAG_TOUCH_NOTIFY | FLAG_REMOVEIFOUTSIDE;

	m_dwFlags |= pStruct->m_Flags;

	// Always do autopickup.
	m_bTouchPickup = true;

	PropsDB::HPROP hProp = NULL;
	static CParsedMsg::CToken s_cTok_None( ModelNone );
	CParsedMsg::CToken cTokModel( pProps->GetString( "Model", "" ));
	if( cTokModel != s_cTok_None && cTokModel.c_str()[0] )
		hProp = g_pPropsDB->GetPropByRecordName( cTokModel.c_str());
	if( hProp )
	{
		pStruct->SetFileName(g_pPropsDB->GetPropFilename(hProp));
		g_pPropsDB->CopyMaterialFilenames(hProp, pStruct->m_Materials[0], LTARRAYSIZE( pStruct->m_Materials ), LTARRAYSIZE( pStruct->m_Materials[0]));
	}

	CParsedMsg::CToken cTokModelOverride( pProps->GetString( "ModelOverride", "" ));
	if( cTokModelOverride != s_cTok_None && cTokModelOverride.c_str()[0]  )
		m_hOverrideProp = g_pPropsDB->GetPropByRecordName( cTokModelOverride.c_str());

	CheckForOverrideModel( pStruct );

	// Setup or physics group to pickups.
	pStruct->m_eGroup = PhysicsUtilities::ePhysicsGroup_UserPickup;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::CheckForOverrideModel()
//
//	PURPOSE:	See if we have an override model and if so set us up
//				to use it
//
// ----------------------------------------------------------------------- //

void PickupItem::CheckForOverrideModel(ObjectCreateStruct *pStruct)
{
	if( !m_hOverrideProp )
		return;

	// See if we have an override model prop-type, if so get the model
	// information and set it in pStruct
	pStruct->SetFileName(g_pPropsDB->GetPropFilename(m_hOverrideProp));
	g_pPropsDB->CopyMaterialFilenames(m_hOverrideProp, pStruct->m_Materials[0], LTARRAYSIZE( pStruct->m_Materials ), LTARRAYSIZE( pStruct->m_Materials[0]));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::InitialUpdate()
//
//	PURPOSE:	First update
//
// ----------------------------------------------------------------------- //

bool PickupItem::InitialUpdate()
{
    SetNextUpdate(UPDATE_NEVER);

	m_LifeTimeTimer.SetEngineTimer(SimulationTimer::Instance());
	m_RespawnTimer.SetEngineTimer(SimulationTimer::Instance());

	if (!g_RespawnScaleTrack.IsInitted())
	{
		g_RespawnScaleTrack.Init(GetServerDE(), "RespawnScale", NULL, 1.0f);
	}


	// Set up our user flags...

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, m_dwUserFlags, m_dwUserFlags);

	g_pLTServer->SetObjectShadowLOD( m_hObject, PICKUP_ITEM_SHADOW_LOD );


	// set a special fx message so that the client knows about the pickup item...
	// (so it can handle bouncing, rotating, and displaying the proper targeting text...)

	CreateSpecialFX();

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, m_dwFlags, m_dwFlags);

	// Look for an animation specified in WorldEdit...

	uint32 dwAni = INVALID_ANI;
	if( !m_sWorldAniName.empty())
	{
		dwAni = g_pLTServer->GetAnimIndex(m_hObject, m_sWorldAniName.c_str());
	}
	
	if (dwAni != INVALID_ANI)
	{
		g_pLTServer->GetModelLT()->SetCurAnim(m_hObject, MAIN_TRACKER, dwAni, true);
	}
	else
	{
		// If we couldn't find the ani look for the world ani.  ALL PickUps should have a "World" ani...

		dwAni = g_pLTServer->GetAnimIndex( m_hObject, "World" );
		if( dwAni != INVALID_ANI )
		{
			g_pLTServer->GetModelLT()->SetCurAnim( m_hObject, MAIN_TRACKER, dwAni, true);
		}
	}

	// Set the dims based on the current animation...

    LTVector vDims;
    g_pModelLT->GetModelAnimUserDims(m_hObject, g_pLTServer->GetModelAnimation(m_hObject), &vDims);

	// Set object dims based on scale value...

    LTVector vNewDims;
	vNewDims.x = m_fScale * vDims.x;
	vNewDims.y = m_fScale * vDims.y;
	vNewDims.z = m_fScale * vDims.z;

	// If the Dims are 0, the weapon won't be pickable.  Report this to 
	// animators so they can fix the dims, but set the dims to something 
	// potentially sane just in case an animation is missed.

	if ( vNewDims.x == 0.0f 
		|| vNewDims.y == 0.0f
		|| vNewDims.z == 0.0f )
	{

#ifndef _FINAL
		char szFileName[128];
		g_pModelLT->GetModelFilename( m_hObject, szFileName, LTARRAYSIZE( szFileName ) ); 
		
		char szErrorMsg[512];
		LTSNPrintF( szErrorMsg, LTARRAYSIZE(szErrorMsg), "Art Error.  PickupItem : Model %s has an animation with dims: %f %f %f.  These dims are used for picking up the weapon; update these to be non-zero.", szFileName, vNewDims.x, vNewDims.y, vNewDims.z );
		g_pLTServer->CPrint( szErrorMsg );
#endif

		vNewDims.x = 30.0f;
		vNewDims.y = 30.0f;
		vNewDims.z = 30.0f;
	}

	g_pLTServer->SetObjectScale(m_hObject, m_fScale);
	g_pPhysicsLT->SetObjectDims(m_hObject, &vNewDims, 0);

	// Make sure object starts on floor if the gravity flag is set...

	if(m_bMoveToFloor)
	{
		MoveObjectToFloor(m_hObject);
	}


	if (!m_bRespawn && m_fLifeTime > 0.0f)
	{
		if (m_sCountdownFX.length())
		{
			SetClientFX(m_sCountdownFX.c_str());
		}
		m_LifeTimeTimer.Start(m_fLifeTime);
		SetNextUpdate(UPDATE_NEXT_FRAME);
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::Update()
//
//	PURPOSE:	Update
//
// ----------------------------------------------------------------------- //

bool PickupItem::Update()
{
	//keep updating by default
	SetNextUpdate(UPDATE_NEXT_FRAME);

	if (m_bExpired)
	{
		// Check if it's ok to remove the pickup now.
		if( m_nFrameCounter > 0 )
			m_nFrameCounter--;
		if( m_nFrameCounter == 0 )
		{
			g_pLTServer->RemoveObject(m_hObject);
			SetNextUpdate(UPDATE_NEVER);

			if( m_hOriginalPickupObject )
			{
				g_pCmdMgr->QueueMessage( m_hObject, m_hOriginalPickupObject, "RESPAWN" );
				m_hOriginalPickupObject = NULL;
			}

			return true;
		}
	}

	// If we aren't visible it must be time to respawn...
	if( m_bWasPickedUp && m_RespawnTimer.IsStarted() && m_RespawnTimer.IsTimedOut() && !m_bControlledRespawn )
	{
		Respawn( );
		SetNextUpdate(UPDATE_NEVER);
		m_RespawnTimer.Stop();
		m_bWasPickedUp = false;
		return true;
	}

	//if we're not supposed to respawn, and we had a lifetime... it must have just expired
	if (!m_bRespawn && m_LifeTimeTimer.IsStarted() && m_LifeTimeTimer.IsTimedOut() )
	{
		if (m_sEndFX.length() > 0)
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8( MID_SFX_MESSAGE );
			cMsg.Writeuint8( SFX_PICKUPITEM_ID );
			cMsg.WriteObject( m_hObject );
			cMsg.Writeuint8( PUFX_ENDFX );
			cMsg.WriteString( m_sEndFX.c_str() );
			g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );

		}
		m_LifeTimeTimer.Stop();
		m_nFrameCounter = 1;
		m_bExpired = true;
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::PickedUp()
//
//	PURPOSE:	Called when an object tells this item that the object
//				picked it up.
//
// ----------------------------------------------------------------------- //

void PickupItem::PickedUp( bool bWasPickedUp, bool bWeaponsStay )
{
	//if we weren't picked up, bail out early
	if (!bWasPickedUp)
	{
		m_bWasPickedUp = false;
		//if the player activated us, let him know he can't have us
		if (!m_bWasTouched && m_hPlayerObj)
		{
			CPlayerObj* pPlayer = (CPlayerObj*) g_pLTServer->HandleToObject(m_hPlayerObj);
			if (pPlayer && pPlayer->IsAlive())
			{
				HCLIENT hClient = pPlayer->GetClient();
				if (hClient)
				{
					CAutoMessage cMsg;
					cMsg.Writeuint8( MID_PICKUPITEM_ACTIVATE );
					cMsg.Writebool( false );
					g_pLTServer->SendToClient( cMsg.Read( ), hClient, MESSAGE_GUARANTEED );
				}
			}
		}

		return;
	}

	// Let the world know what happened.
	SendPickedUp();

	// If we're supposed to process a command, do it here.
	if( !m_sPickupCommand.empty() )
	{
		g_pCmdMgr->QueueCommand( m_sPickupCommand.c_str(), m_hPlayerObj, m_hObject );
	}

	// Clear our player obj, we no longer need this link.
	SetPlayerObj(NULL);

	// If this is marked as weaponsstay, then we're done.  The item will
	// stay around for another player to pickup.
	if( bWeaponsStay )
		return;

	if( m_bRespawn )
	{
		// Make the item invisible and non touchable until the next update.
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAG_VISIBLE|FLAG_TOUCH_NOTIFY|FLAG_RAYHIT);

		// Hide the shadow...
		g_pLTServer->SetObjectShadowLOD( m_hObject, eEngineLOD_Never );

		// Controlled respawns don't use the RespawnTime and are instead controlled by other events...
		if( !m_bControlledRespawn )
			m_RespawnTimer.Start(m_fRespawnDelay / g_RespawnScaleTrack.GetFloat(1.0f));
		
	}
	else
	{
		// Set us as expired so we get removed the next frame.  Can't remove yet since 
		// we have messages that need to get down.  Give ourselves 2 updates, since
		// this PickedUp method could have been called right before this frame's update.
		m_bExpired = true;
		m_nFrameCounter = 2;
	}

	SetNextUpdate(UPDATE_NEXT_FRAME);

	// Consider ourselves picked up.
	m_bWasPickedUp = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::SendPickedUp()
//
//	PURPOSE:	Sends the pickedup message.
//
// ----------------------------------------------------------------------- //

void PickupItem::SendPickedUp()
{
	// Let the world know what happened...
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_PICKUPITEM_ID );
	cMsg.WriteObject( m_hObject );
	cMsg.Writeuint8( PUFX_PICKEDUP );
	g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::SetPlayerObj
//
//	PURPOSE:	Set our player obj data member
//
// ----------------------------------------------------------------------- //

void PickupItem::SetPlayerObj(HOBJECT hObj)
{
	m_hPlayerObj = hObj;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::CreateSpecialFX
//
//	PURPOSE:	Send the special fx message for this object
//
// ----------------------------------------------------------------------- //

void PickupItem::CreateSpecialFX( bool bUpdateClients /* = false  */ )
{
	CAutoMessage cMsg;
	cMsg.Writeuint8(SFX_PICKUPITEM_ID);
	m_Shared.Write( cMsg );
	g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::SetClientFX
//
//	PURPOSE:	Set the clientfx for this pickupitem...
//
// ----------------------------------------------------------------------- //

void PickupItem::SetClientFX( const char *pszFX )
{
	m_Shared.m_sClientFX = pszFX;

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_PICKUPITEM_ID );
	cMsg.WriteObject( m_hObject );
	cMsg.Writeuint8( PUFX_CLIENTFX );
	cMsg.WriteString( m_Shared.m_sClientFX.c_str() );
	g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );

	CreateSpecialFX( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::SetDroppedClientFX
//
//	PURPOSE:	Set the clientfx for this pickupitem...
//
// ----------------------------------------------------------------------- //

void PickupItem::SetDroppedClientFX( const char *pszFX )
{
	m_Shared.m_sDroppedClientFX = pszFX;

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_PICKUPITEM_ID );
	cMsg.WriteObject( m_hObject );
	cMsg.Writeuint8( PUFX_DROPPEDCLIENTFX );
	cMsg.WriteString( m_Shared.m_sDroppedClientFX.c_str() );
	g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );

	CreateSpecialFX( );

	if( m_bDropped )
		m_Shared.m_sDroppedClientFX.clear( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::SetLocked
//
//	PURPOSE:	Changed the locked status of this volume.
//
// ----------------------------------------------------------------------- //

void PickupItem::SetLocked( bool bLocked )
{
	m_Shared.m_bLocked = bLocked;

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_PICKUPITEM_ID );
	cMsg.WriteObject( m_hObject );
	cMsg.Writeuint8( PUFX_LOCKED );
	cMsg.Writebool( m_Shared.m_bLocked );
	g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::SetTeamId
//
//	PURPOSE:	Set the teamid for this pickupitem...
//
// ----------------------------------------------------------------------- //

void PickupItem::SetTeamId( uint8 nTeamId )
{
	m_Shared.m_nTeamId = nTeamId;

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_PICKUPITEM_ID );
	cMsg.WriteObject( m_hObject );
	cMsg.Writeuint8( PUFX_TEAMID );
	cMsg.Writeuint8( m_Shared.m_nTeamId );
	g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );

	CreateSpecialFX( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::Respawn
//
//	PURPOSE:	Handle "respawning" the model (make it visible, switch skins, etc.)...
//
// ----------------------------------------------------------------------- //

void PickupItem::Respawn( )
{
	// Make us visible...

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE);
	g_pCommonLT->SetObjectFlags( m_hObject, OFT_Flags2, 0, FLAG2_FORCETRANSLUCENT );
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_TOUCH_NOTIFY, FLAG_TOUCH_NOTIFY);

	g_pLTServer->SetObjectShadowLOD( m_hObject, PICKUP_ITEM_SHADOW_LOD );


	// Turn on the relative flags...
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_RAYHIT, FLAG_RAYHIT);

 	// Let the world know what happened...
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_PICKUPITEM_ID );
	cMsg.WriteObject( m_hObject );
	cMsg.Writeuint8( PUFX_RESPAWN );
	g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );

	m_bWasPickedUp = false;
	m_bWasTouched = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void PickupItem::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
	if (!pMsg) return;

	m_Shared.Save( pMsg );

	SAVE_HOBJECT(m_hPlayerObj);
	SAVE_FLOAT(m_fScale);
	SAVE_FLOAT(m_fRespawnDelay);
	SAVE_BOOL(m_bRespawn);
	SAVE_BOOL( m_bControlledRespawn );
	SAVE_DWORD(m_dwUserFlags);
	SAVE_DWORD(m_dwFlags);
	SAVE_STDSTRING( m_sPickupCommand );
	SAVE_HRECORD( m_hOverrideProp );
	SAVE_BOOL(m_bTouchPickup);
	SAVE_bool(m_bWasPickedUp);
	SAVE_bool(m_bWasPlaced);
	SAVE_bool(m_bWasTouched);
	SAVE_STDSTRING( m_sWorldAniName );
	SAVE_HOBJECT( m_hOriginalPickupObject );
	m_LifeTimeTimer.Save(*pMsg);
	m_RespawnTimer.Save(*pMsg);
	SAVE_bool( m_bDropped );
	SAVE_HOBJECT(m_hDroppedBy);
	SAVE_DOUBLE(m_fDropTime);

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void PickupItem::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
	if (!pMsg) return;

	m_Shared.Load( pMsg );

	LOAD_HOBJECT(m_hPlayerObj);
	LOAD_FLOAT(m_fScale);
    LOAD_FLOAT(m_fRespawnDelay);
	LOAD_BOOL(m_bRespawn);
	LOAD_BOOL( m_bControlledRespawn );
    LOAD_DWORD(m_dwUserFlags);
    LOAD_DWORD(m_dwFlags);
	LOAD_STDSTRING( m_sPickupCommand );
	LOAD_HRECORD( m_hOverrideProp, g_pPropsDB->GetPropsCategory());
	LOAD_BOOL(m_bTouchPickup);
	LOAD_bool(m_bWasPickedUp);
	LOAD_bool(m_bWasPlaced);
	LOAD_bool(m_bWasTouched);
	LOAD_STDSTRING( m_sWorldAniName );
	LOAD_HOBJECT( m_hOriginalPickupObject );
	m_LifeTimeTimer.Load(*pMsg);
	m_RespawnTimer.Load(*pMsg);
	LOAD_bool( m_bDropped );
	LOAD_HOBJECT(m_hDroppedBy);
	
	if	(g_pVersionMgr->GetCurrentSaveVersion( ) > CVersionMgr::kSaveVersion__1_04)
	{
		LOAD_DOUBLE( m_fDropTime );
	}
	else
	{
		m_fDropTime = 0.0;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::SaveSFXMessage
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void PickupItem::SaveSFXMessage( ILTMessage_Write *pMsg, uint32 dwSaveFlags )
{
	if( !pMsg )
		return;

	CAutoMessage cGetMsg;
	g_pLTServer->GetObjectSFXMessage( m_hObject, cGetMsg );
	CLTMsgRef_Read pSFXMsg = cGetMsg.Read( );

	if( pSFXMsg->Size( ) == 0 )
		return;
	
	// Make sure the proper message ID is saved.
	pMsg->Writeuint8( pSFXMsg->Readuint8( ) );

	PICKUPITEMCREATESTRUCT PickupItemCS;
	PickupItemCS.Load( pSFXMsg );
	PickupItemCS.Save( pMsg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::LoadSFXMessage
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void PickupItem::LoadSFXMessage( ILTMessage_Read *pMsg, uint32 dwSaveFlags )
{
	if( !pMsg || pMsg->Size( ) == 0 )
		return;

	CAutoMessage cSFXMsg;

	cSFXMsg.Writeuint8( pMsg->Readuint8( ) );

	PICKUPITEMCREATESTRUCT PickupItemCS;
	PickupItemCS.Load( pMsg );
	PickupItemCS.Save( cSFXMsg );
	g_pLTServer->SetObjectSFXMessage( m_hObject, cSFXMsg.Read( ) );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::DropItem
//
//	PURPOSE:	handle dropping the item into the world (from a character's hand for instance)
//
// ----------------------------------------------------------------------- //

void PickupItem::DropItem(const LTVector& vImpulse, const LTVector& vInitialVelocity, const LTVector& vInitialAngularVelocity, HOBJECT hDroppedBy )
{
	// make sure we are setup for physics
	uint32 nNumRigidBodies = 0;
	g_pLTServer->PhysicsSim()->GetNumModelRigidBodies(m_hObject, nNumRigidBodies);
	LTASSERT(nNumRigidBodies > 0,"No rigid bodies associated with pickup item model");
	if (nNumRigidBodies == 0)
	{
		//no physics so just drop it onto the ground
		LTRotation rRot;
		g_pLTServer->GetObjectRotation(m_hObject,&rRot);

		EulerAngles EA = Eul_FromQuat(rRot,EulOrdYXZr);
		EA.y = 0.0f; //clear pitch
		EA.z = MATH_HALFPI; //hack roll

		rRot = Eul_ToQuat(EA);
		g_pLTServer->SetObjectRotation(m_hObject, rRot);

		LTVector vOldDims, vNewDims(10.0f,10.0f,10.0f);
		g_pPhysicsLT->GetObjectDims(m_hObject, &vOldDims);
		g_pPhysicsLT->SetObjectDims(m_hObject,&vNewDims,0);

		MoveObjectToFloor(m_hObject);

		g_pPhysicsLT->SetObjectDims(m_hObject,&vOldDims,0);
		return;
	}

	// Go unguaranteed since we'll be moving a lot.
	g_pLTServer->SetNetFlags(m_hObject, NETFLAG_POSUNGUARANTEED|NETFLAG_ROTUNGUARANTEED);

	// Visibility needs to be delayed on the client so prediction can interpolate it.
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_DELAYCLIENTVISIBLE, FLAG_DELAYCLIENTVISIBLE);

	// We don't want to be in the normal physics simulation so clear the
	// physics related flags...
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAG_GRAVITY | FLAG_SOLID);

	// Clear our velocity in the old physics system
	g_pPhysicsLT->SetVelocity(m_hObject, LTVector(0, 0, 0));


	//setup our collision properties
	HRECORD hCollisionProperty = GetCollisionProperty();
	uint32 nUserFlags = CollisionPropertyRecordToUserFlag( hCollisionProperty );
	g_pLTServer->Common( )->SetObjectFlags( m_hObject, OFT_User, nUserFlags, USRFLG_COLLISIONPROPMASK );


	// Set us up to use the physics simulation...
	PhysicsUtilities::SetPhysicsWeightSet(m_hObject, PhysicsUtilities::WEIGHTSET_RIGID_BODY, false);

	// Apply our velocity to our rigid bodies...
	//	EPhysicsGroup eGroup;
	//	uint32 nSystem;
	LTRigidTransform tBody;
	for (uint32 i=0; i < nNumRigidBodies; i++)
	{
		HPHYSICSRIGIDBODY hRigidBody;
		if (LT_OK == g_pLTServer->PhysicsSim()->GetModelRigidBody( m_hObject, i, hRigidBody ))
		{
			g_pLTServer->PhysicsSim()->SetRigidBodySolid(hRigidBody,true);

			g_pLTServer->PhysicsSim()->SetRigidBodyAngularVelocity(hRigidBody, vInitialAngularVelocity);
			g_pLTServer->PhysicsSim()->SetRigidBodyVelocity(hRigidBody, vInitialVelocity);


			if ( vImpulse != LTVector::GetIdentity() )
			{
				LTRigidTransform tTrans;
				g_pLTServer->PhysicsSim()->GetRigidBodyTransform(hRigidBody,tTrans);

				if( LTIsNaN( vImpulse ) || vImpulse.MagSqr() > 1000000.0f * 1000000.0f )
				{
					LTERROR( "Invalid impulse detected." );
					g_pLTServer->PhysicsSim()->ApplyRigidBodyImpulseWorldSpace(hRigidBody, tTrans.m_vPos, LTVector( 0.0f, 10.0f, 0.0f ));
				}
				else
				{
					g_pLTServer->PhysicsSim()->ApplyRigidBodyImpulseWorldSpace(hRigidBody, tTrans.m_vPos, vImpulse);
				}
			}

			if (i == 0)
			{
				LTRigidTransform tTrans;
				g_pLTServer->PhysicsSim()->GetRigidBodyTransform(hRigidBody,tTrans);
				tBody = tTrans;
			}

			g_pLTServer->PhysicsSim()->ReleaseRigidBody(hRigidBody);
		}
	}

	//get a rigid transform of the game object
	LTRigidTransform tObject;
	g_pLTServer->GetObjectTransform(m_hObject,&tObject);

	//get the offest transform
	m_tBodyOffset.Difference(tBody,tObject);

	m_bDropped = true;
	m_hDroppedBy = hDroppedBy;
	m_fDropTime = SimulationTimer::Instance().GetTimerAccumulatedS();

	SetNextUpdate(UPDATE_NEXT_FRAME);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PickupItem::GetCollisionProperty
//
//	PURPOSE:	get collision property for dropped items
//
// ----------------------------------------------------------------------- //

HRECORD PickupItem::GetCollisionProperty()
{
	return DATABASE_CATEGORY( Collisions ).GETRECORDATTRIB( DATABASE_CATEGORY( Collisions ).GetGlobalRecord(), Default );
}

