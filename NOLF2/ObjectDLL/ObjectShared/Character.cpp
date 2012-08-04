// ----------------------------------------------------------------------- //
//
// MODULE  : Character.cpp
//
// PURPOSE : Base class for player and AI
//
// CREATED : 10/6/97
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "Character.h"
#include "iltserver.h"
#include "ServerUtilities.h"
#include "stdio.h"
#include "Body.h"
#include "VolumeBrush.h"
#include "Spawner.h"
#include "SurfaceFunctions.h"
#include "CharacterMgr.h"
#include "CVarTrack.h"
#include "SoundMgr.h"
#include "iltmodel.h"
#include "ilttransform.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "SFXMsgIds.h"
#include "Attachments.h"
#include "SurfaceMgr.h"
#include "Animator.h"
#include "MsgIDs.h"
#include "GameServerShell.h"
#include "AIVolumeMgr.h"
#include "CharacterHitBox.h"
#include "Camera.h"
#include "AIState.h"
#include "ServerSoundMgr.h"
#include "ObjectRelationMgr.h"
#include "AIStimulusMgr.h"
#include "RelationMgr.h"
#include "AIVolume.h"
#include "AIRegion.h"
#include "AIStimulusMgr.h"
#include "AIInformationVolumeMgr.h"
#include "Weapon.h"
#include "PlayerObj.h"
#include "InventoryButeMgr.h"
#include "KeyMgr.h"
#include "SharedFXStructs.h"
#include "TeleportPoint.h"
#include "Searchable.h"
#include "ServerMissionMgr.h"
#include "ActiveWorldModel.h"
#include "VersionMgr.h"

LINKFROM_MODULE( Character );

BEGIN_CLASS(CCharacter)
	ADD_DESTRUCTIBLE_AGGREGATE(PF_GROUP(1), 0)
	ADD_ATTACHMENTS_AGGREGATE()
	ADD_REALPROP(HitPoints, -1.0f)
	ADD_REALPROP(Energy, -1.0f)
	ADD_REALPROP(ArmorPoints, -1.0f)
    ADD_BOOLPROP(MoveToFloor, LTTRUE)
	ADD_BOOLPROP(MakeBody, 1)
	ADD_BOOLPROP(PermanentBody, 0)
	ADD_REALPROP(BodyLifetime, -1.0f)
	ADD_STRINGPROP(SpawnItem, "")
	ADD_STRINGPROP(HeadExtension, "")
	ADD_SEARCHABLE_AGGREGATE(PF_GROUP(5), 0)
	ADD_STRINGPROP_FLAG(SearchSoundName, "Interface\\Snd\\SearchBodyLoop.wav",PF_GROUP(5) | PF_FILENAME)

	PROP_DEFINEGROUP(AddChildModel, (PF_GROUP(7)) | (0)) 
		ADD_STRINGPROP_FLAG(ChildModel_1, "", (PF_GROUP(7)) | (PF_FILENAME)) 
		ADD_STRINGPROP_FLAG(ChildModel_2, "", (PF_GROUP(7)) | (PF_FILENAME)) 
		ADD_STRINGPROP_FLAG(ChildModel_3, "", (PF_GROUP(7)) | (PF_FILENAME)) 
	ADD_GENERAL_INVENTORY_PROP(PF_GROUP(2))
END_CLASS_DEFAULT_FLAGS_PLUGIN(CCharacter, GameBase, NULL, NULL, CF_HIDDEN, CCharacterPlugin)

extern CAIStimulusMgr* g_pAIStimulusMgr;
extern CAIInformationVolumeMgr* g_pAIInformationVolumeMgr;

LTBOOL s_bCharacterPluginInitted = LTFALSE;
CInventoryButeMgr s_InventoryButeMgr;

const float kDialogueTimeFudgeFactor = 5.0f;

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LTBOOL ValidateTelePortMsg
//
//  PURPOSE:	Make sure the TELEPORT message is valid
//
// ----------------------------------------------------------------------- //

static LTBOOL ValidateTelePortMsg( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !pInterface ) return LTFALSE;

	if( IsVector( cpMsgParams.m_Args[1] ))
	{
		return LTTRUE;
	}
	else
	{
		if( LT_NOTFOUND == pInterface->FindObject( cpMsgParams.m_Args[1] ))
		{
			if( CCommandMgrPlugin::s_bShowMsgErrors )
			{
				pInterface->ShowDebugWindow( LTTRUE );
				pInterface->CPrint( "ERROR! - ValidateTelePortMsg()" );
				pInterface->CPrint( "    MSG - TELEPORT - Could not find object '%s'!", cpMsgParams.m_Args[1] );
			}
			
			return LTFALSE;
		}

		char const* pszObjClass = pInterface->GetObjectClass( cpMsgParams.m_Args[1] );
		if( pszObjClass && !_stricmp( pszObjClass, "TeleportPoint" ))
		{
			return LTTRUE;
		}

		if( CCommandMgrPlugin::s_bShowMsgErrors )
		{
			pInterface->ShowDebugWindow( LTTRUE );
			pInterface->CPrint( "ERROR! - ValidateTelePortMsg()" );
			pInterface->CPrint( "    MSG - TELEPORT - Object '%s'is not a TeleportPoint or a vector.", cpMsgParams.m_Args[1] );
		}

		return LTFALSE;
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LTBOOL ValidateBooleanMsg
//
//  PURPOSE:	Make sure any boolean message is valid
//
// ----------------------------------------------------------------------- //

static LTBOOL ValidateBooleanMsg( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !pInterface ) return LTFALSE;

	if( IsTrueChar( *cpMsgParams.m_Args[1] ) ||
		IsFalseChar( *cpMsgParams.m_Args[1] ))
	{
		return LTTRUE;
	}

	if( CCommandMgrPlugin::s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint( "ERROR! - ValidateBooleanMsg()" );
		pInterface->CPrint( "    MSG - %s - '%s' is not a valid boolean value!", _strupr(cpMsgParams.m_Args[0]), cpMsgParams.m_Args[1] );
	}
	
	return LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	LTBOOL ValidateCrosshairMsg
//
//  PURPOSE:	Make sure the cross hair message is valid
//
// ----------------------------------------------------------------------- //

static LTBOOL ValidateCrosshairMsg( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !pInterface ) return LTFALSE;

	if( !_stricmp( cpMsgParams.m_Args[1], "GOOD" ) ||
		!_stricmp( cpMsgParams.m_Args[1], "BAD" ) ||
		!_stricmp( cpMsgParams.m_Args[1], "NEUTRAL" ) ||
		!_stricmp( cpMsgParams.m_Args[1], "UNKNOWN" ))
	{
		return LTTRUE;
	}

	if( CCommandMgrPlugin::s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( LTTRUE );
		pInterface->CPrint( "ERROR! - ValidateCrosshairMsg()" );
		pInterface->CPrint( "    MSG - CROSSHAIR - Crosshair style '%s' is not valid!", cpMsgParams.m_Args[1] );
	}
	
	return LTFALSE;
}

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( CCharacter )

	CMDMGR_ADD_MSG( PLAYSOUND, 2, NULL, "PLAYSOUND <sound name>" )	
	CMDMGR_ADD_MSG( TELEPORT, 2, ValidateTelePortMsg, "TELEPORT <teleport point> | <vector>" )
	CMDMGR_ADD_MSG( ATTACH, 3, NULL, "ATTACH <attach pos> <attachment name>" )
	CMDMGR_ADD_MSG( DETACH, 2, NULL, "DETACH <attach pos>" )
	CMDMGR_ADD_MSG( GADGET, 2, NULL, "GADGET <ammo id>" )
	CMDMGR_ADD_MSG( CANDAMAGE, 2, ValidateBooleanMsg, "CANDAMAGE <bool>" )
	CMDMGR_ADD_MSG( CROSSHAIR, 2, ValidateCrosshairMsg, "CROSSHAIR <style>" )

CMDMGR_END_REGISTER_CLASS( CCharacter, GameBase )



#define KEY_FOOTSTEP_SOUND		"FOOTSTEP_KEY"
#define KEY_SET_DIMS			"SETDIMS"
#define KEY_MOVE				"MOVE"
#define KEY_PLAYSOUND			"PLAYSOUND"
#define KEY_COMMAND				"CMD"
#define KEY_BUTE_SOUND			"BUTE_SOUND_KEY"
#define KEY_ON					"ON"
#define KEY_OFF					"OFF"
#define KEY_DEFLECT				"DEFLECT"

#define TRIGGER_PLAY_SOUND		"PLAYSOUND"
#define TRIGGER_TELEPORT		"TELEPORT"

#define DEFAULT_SOUND_RADIUS		1000.0f
#define FOOTSTEP_SOUND_RADIUS		1000.0f
#define DEFAULT_LADDER_VEL			400.0f
#define DEFAULT_SWIM_VEL			175.0f
#define DEFAULT_RUN_VEL				100.0f
#define DEFAULT_WALK_VEL			60.0f
#define DEFAULT_JUMP_VEL			50.0f
#define DEFAULT_FALL_VEL			200.0f
#define DEFAULT_MOVE_ACCEL			3000.0f

#define DIMS_EPSILON				0.5f
#define FALL_LANDING_TIME			0.5f

static CVarTrack g_VolumeDebugTrack;
static CVarTrack s_BodyStickAngle;

CVarTrack g_BodyStickDist;
CVarTrack g_BodyStateTimeout;

// Globals (save space) used for parsing messages (used in sub classes as well)...
// g_pCommandPos is global to make sure the command position is correctly
// updated in multiple calls to Parse()

char g_tokenSpace[PARSE_MAXTOKENS*PARSE_MAXTOKENSIZE];
char *g_pTokens[PARSE_MAXTOKENS];
const char *g_pCommandPos;

extern CGameServerShell* g_pGameServerShell;

int32 CCharacter::sm_cAISnds = 0;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DamageFilterFunctionHook()
//
//	PURPOSE:	Function that the m_destructable aggregate calls (if a
//				function is registered) when damage has been taken.
//
// ----------------------------------------------------------------------- //

static bool DamageFilterHook( GameBase *pObject, DamageStruct *pDamageStruct )
{
	// cast to a game base object
	CCharacter *pMyObj = dynamic_cast< CCharacter* >( pObject );

	// call the most derived class
	return pMyObj->FilterDamage( pDamageStruct );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::CCharacter()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //


CCharacter::CCharacter() : GameBase(OT_MODEL)
{
	AddAggregate(&m_damage);
	AddAggregate(&m_editable);

	m_pSearch = debug_new( CSearchable );
	AddAggregate(m_pSearch);
	m_pSearch->SetIsBody(true);

	// damage filtering
	m_damage.RegisterFilterFunction( DamageFilterHook, this );

	// Setup the TransitionAggregate
	MakeTransitionable();

	m_bInitializedAnimation		= LTFALSE;

	m_bBlink					= LTFALSE;

	m_bShortRecoil				= LTFALSE;
	m_bShortRecoiling			= LTFALSE;

	m_ccCrosshair				= UNKNOWN;

	m_dwFlags					= FLAG_STAIRSTEP | FLAG_SHADOW | FLAG_TOUCH_NOTIFY | FLAG_SOLID | FLAG_GRAVITY |
								  FLAG_MODELKEYS | FLAG_RAYHIT | FLAG_VISIBLE;

	m_fLadderVel				= DEFAULT_LADDER_VEL;
	m_fSwimVel					= DEFAULT_SWIM_VEL;
	m_fRunVel					= DEFAULT_RUN_VEL;
	m_fWalkVel					= DEFAULT_WALK_VEL;
	m_fJumpVel					= DEFAULT_JUMP_VEL;
	m_fSuperJumpVel				= DEFAULT_JUMP_VEL;
	m_fFallVel					= DEFAULT_FALL_VEL;
	m_fBaseMoveAccel			= DEFAULT_MOVE_ACCEL;
	m_eModelNodeLastHit			= eModelNodeInvalid;
    m_bUsingHitDetection        = LTTRUE;

	m_fSoundRadius				= DEFAULT_SOUND_RADIUS;
	m_eSoundPriority			= SOUNDPRIORITY_AI_HIGH;

	m_eMusicMoodMin				= CMusicMgr::eMoodRoutine;
	m_eMusicMoodMax				= CMusicMgr::eMoodAggressive;

	m_byFXFlags					= 0;

    m_bRolling                  = LTFALSE;
    m_bPivoting                 = LTFALSE;
    m_bOnGround                 = LTTRUE;
	m_eStandingOnSurface		= ST_UNKNOWN;
    m_bAllowRun                 = LTTRUE;
    m_bAllowMovement            = LTTRUE;
	m_eContainerCode			= CC_NO_CONTAINER;
	m_eLastContainerCode		= CC_NO_CONTAINER;
    m_bBodyInLiquid             = LTFALSE;
    m_bBodyWasInLiquid          = LTFALSE;
    m_bBodyOnLadder             = LTFALSE;
    m_bLeftFoot                 = LTTRUE;
    m_bPlayingTextDialogue      = LTFALSE;

	m_eEnemyVisibleStimID			= kStimID_Unset;
	m_eUndeterminedVisibleStimID	= kStimID_Unset;

	m_fLastPainTime				= -(float)INT_MAX;
	m_fLastPainVolume			= 0.0f;

	VEC_INIT(m_vOldCharacterColor);
	m_fOldCharacterAlpha		= 1.0f;
    m_bCharacterHadShadow       = LTFALSE;

    m_bMoveToFloor              = LTTRUE;

    m_hstrSpawnItem             = LTNULL;

	m_hstrHeadExtension			= LTNULL;

    // save out extra childmodel names.
    m_nExtraChildModels         = 0;
    for( int i= 0 ; i < MAX_CHILD_MODELS ; i++ )
	{
        m_hstrExtraChildModels[i] = LTNULL ;
	}

    m_pAttachments              = LTNULL;

	// Debug bounding box...

	m_pHandName					= "GUNHAND";

    m_hCurDlgSnd                = LTNULL;
	m_eCurDlgSndType			= CST_NONE;

    m_bStartedDeath             = LTFALSE;
	m_eDeathType				= CD_NORMAL;

	m_eModelId					= eModelIdInvalid;
	m_eModelSkeleton			= eModelSkeletonInvalid;

	m_fDefaultHitPts			= -1.0f;
	m_fDefaultEnergy			= -1.0f;
	m_fDefaultArmor				= -1.0f;
	m_fMoveMultiplier			= 1.0f;
	m_fJumpMultiplier			= 1.0f;

	m_pCurrentVolume			= LTNULL;
	m_pLastVolume				= LTNULL;
    m_vLastVolumePos            = LTVector(0,0,0);

	m_pLastInformationVolume	= LTNULL;
	m_pCurrentInformationVolume = LTNULL;
	m_vLastInformationVolumePos = LTVector(0,0,0);;

    m_hHitBox                   = LTNULL;

	m_cSpears					= 0;

	m_bWallStick				= LTFALSE;
	m_bStuckInFront				= LTFALSE;

	m_bArmored					= LTFALSE;

    m_pAnimator                 = LTNULL;

	m_cActive					= 0;


	m_nDamageFlags				= 0;
	m_nInstantDamageFlags		= 0;

	m_iPermissionSet			= 255; // Full permission set (uint8)

	// Create the relation Mgr
	m_pRelationMgr				= debug_new( CObjectRelationMgr );
	m_pcs						= debug_new(CHARCREATESTRUCT);
	
	m_fPitch					= 0.5f;
	m_fLastPitch				= 0.0f;

	// Initialize our spears to notify us if their objects get removed.
	for( int nSpear = 0; nSpear < ARRAY_LEN( m_aSpears ); nSpear++ )
	{
		m_aSpears[nSpear].hObject.SetReceiver( *this );
	}

	m_bMakeBody = true;
	m_bPermanentBody = false;

	m_eTeleportTriggerState = eTeleporTriggerStateNone;

	m_eDeathDamageType			= DT_INVALID;
	m_fBodyLifetime				= -1.0f;

	m_nUniqueDialogueId			= 0;

	m_bTracking = false;
	m_bRadarVisible = false;

	m_nCarrying = CFX_CARRY_NONE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::ResetAfterDeath()
//
//	PURPOSE:	Reset
//
// ----------------------------------------------------------------------- //

void CCharacter::ResetAfterDeath()
{
    m_bStartedDeath     = LTFALSE;

	KillDlgSnd();

	// Since we were dead, we need to reset our solid flag...

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_SOLID, FLAG_SOLID);

	// Also update our hit box in case we have moved...

	UpdateHitBox();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::~CCharacter()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CCharacter::~CCharacter()
{
	DestroyAttachments();

	KillDlgSnd();

	FREE_HSTRING(m_hstrHeadExtension);

    for( int i =0 ; i < m_nExtraChildModels ; i++ )
	{
        FREE_HSTRING(m_hstrExtraChildModels[i]);
	}

	FREE_HSTRING(m_hstrSpawnItem);

	// Get rid of the spears if they're still around

	for ( uint32 iSpear = 0 ; iSpear < m_cSpears ; iSpear++ )
	{
		HATTACHMENT hAttachment;
		HOBJECT hSpear = m_aSpears[iSpear].hObject;
		if ( hSpear )
		{
			if ( LT_OK == g_pLTServer->FindAttachment(m_hObject, hSpear, &hAttachment) )
			{
				g_pLTServer->RemoveAttachment(hAttachment);
			}

			g_pLTServer->RemoveObject(hSpear);
			m_aSpears[iSpear].hObject = LTNULL;
		}
	}

	m_cSpears = 0;

	// Take us out of the charactermgr

	g_pCharacterMgr->Remove(this);

	if (m_hHitBox)
	{
		g_pLTServer->RemoveObject(m_hHitBox);
	}

	if ( m_pRelationMgr != NULL )
	{
		debug_delete(m_pRelationMgr);
		m_pRelationMgr = NULL;
	}

	if ( m_pcs != NULL )
	{
		debug_delete(m_pcs);
		m_pcs = NULL;
	}

	RemoveAggregate(m_pSearch);
	debug_delete(m_pSearch);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::EngineMessageFn()
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CCharacter::EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData)
{
	switch(messageID)
	{
        case MID_ACTIVATING:
		{
			m_cActive++;
			g_pCharacterMgr->Add(this);

			if ( m_cActive != 1 )
			{
				g_pLTServer->CPrint("Active count out of sync!!!!!!");
			}
		}
		break;

		case MID_DEACTIVATING:
		{
			m_cActive--;
			g_pCharacterMgr->Remove(this);

			if ( m_cActive != 0 )
			{
				g_pLTServer->CPrint("Active count out of sync!!!!!!");
			}
		}
		break;

		case MID_PARENTATTACHMENTREMOVED:
		{
			g_pLTServer->RemoveObject( m_hObject );
		}
		break;

		case MID_UPDATE:
		{
			Update();
		}
		break;

		case MID_MODELSTRINGKEY:
		{
			HandleModelString((ArgList*)pData);
		}
		break;

		case MID_PRECREATE:
		{
			CreateAttachments();

			if ( m_pAttachments )
			{
				AddAggregate(m_pAttachments);
			}

			ObjectCreateStruct* pStruct = (ObjectCreateStruct*)pData;

			if (pStruct)
			{
				if (fData == PRECREATE_WORLDFILE || fData == PRECREATE_STRINGPROP)
				{
					ReadProp(pStruct);
				}
			}

			m_pSearch->Enable(false);
		}
		break;

		case MID_INITIALUPDATE:
		{
            uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);
			InitialUpdate((int)fData);
			return dwRet;
		}
		break;

		// RegisterStimulus depends on g_pWorldProperties already existing.
		case MID_ALLOBJECTSCREATED:
		{
			//force update of info vols here so that stimuli may be registered properly.
			UpdateCurrentInformationVolume(true);

			// The player will register when it is done spawning in.
			if( !IsPlayer( m_hObject ))
			{
				// Register all persistent stimuli now, as there are 
				// dependancies on object creation in the registration process.
				RegisterPersistentStimuli();
			}
		
			// The crosshair depends on the player being created.  Send the
			// crosshair your crosshair to the player now.  Rely on standard
			// updates for any objects created after this point.
			
			// Sending a message to ourselves to set our crosshair to UNKNOWN
			// will cause the crosshair to be reset.
			SendTriggerMsgToObject( this, m_hObject, LTFALSE, "CROSSHAIR UNKNOWN" );
		}
		break;

		case MID_SAVEOBJECT:
		{
			// Let aggregates go first...

            uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);

			Save((ILTMessage_Write*)pData);

			return dwRet;
		}
		break;

		case MID_LOADOBJECT:
		{
			// Let aggregates go first...

            uint32 dwRet = GameBase::EngineMessageFn(messageID, pData, fData);

			Load((ILTMessage_Read*)pData);

			// We need to reset our sfx message since values
			// could have changed across save versions.
			
			CreateSpecialFX( );

			return dwRet;
		}
		break;

		default : break;
	}

	return GameBase::EngineMessageFn(messageID, pData, fData);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::ObjectMessageFn()
//
//	PURPOSE:	Handler for object to object messages.
//
// --------------------------------------------------------------------------- //

uint32 CCharacter::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
{
	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();
	switch (messageID)
	{
		case MID_DAMAGE:
		{
			// Save and then restore the message position since it may change after sending it to the base...

			uint32 dwMsgPos = pMsg->Tell();
            uint32 dwRet = GameBase::ObjectMessageFn(hSender, pMsg);
			pMsg->SeekTo( dwMsgPos );
		
			ProcessDamageMsg(pMsg);
			return dwRet;
		}
		break;

		case MID_SFX_MESSAGE:
		{
			HandleSfxMessage( hSender, pMsg );
		}
		break;
	}

	return GameBase::ObjectMessageFn(hSender, pMsg);
}

void CCharacter::HandleSfxMessage( HOBJECT hSender, ILTMessage_Read *pMsg )
{
	uint8 nSfxId = pMsg->Readuint8( );
	switch( nSfxId )
	{
		case SFX_CHARACTER_ID:
		{
			uint8 nCfxMsg = pMsg->Readuint8( );
			switch( nCfxMsg )
			{
				// Client telling us sound is over.
				case CFX_NODECONTROL_LIP_SYNC:
				case CFX_DIALOGUE_MSG:
				{
					// Kill the dialogue sound the first time
					// we get a matching id.  If more than one client sends
					// this message, it will get ignored.
					uint8 nDialogueUniqueId = pMsg->Readuint8( );
					if( nDialogueUniqueId == m_nUniqueDialogueId )
					{
						CPlayerObj* pPlayerObj = dynamic_cast< CPlayerObj* >( g_pLTServer->HandleToObject( hSender ));
						if( pPlayerObj )
						{
							m_PlayerTrackerDialogue.RemoveClient( pPlayerObj->GetClient( ));

							// When all the clients have reported in,
							// we can kill the sound.
							if( m_PlayerTrackerDialogue.IsEmpty( ))
								KillDlgSnd( );
						}
					}
				}
				break;
			}
		}
		break;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::ReadProp
//
//	PURPOSE:	Set property value
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacter::ReadProp(ObjectCreateStruct *pStruct)
{
	GenericProp genProp;
    if (!pStruct) return LTFALSE;

    if ( g_pLTServer->GetPropGeneric( "MoveToFloor", &genProp ) == LT_OK )
	{
		m_bMoveToFloor = genProp.m_Bool;
	}
    if ( g_pLTServer->GetPropGeneric( "MakeBody", &genProp ) == LT_OK )
	{
		m_bMakeBody = genProp.m_Bool;
	}
    if ( g_pLTServer->GetPropGeneric( "PermanentBody", &genProp ) == LT_OK )
	{
		m_bPermanentBody = genProp.m_Bool;
	}
	if ( g_pLTServer->GetPropGeneric( "BodyLifetime", &genProp ) == LT_OK )
	{
		m_fBodyLifetime = genProp.m_Float;
	}

    if ( g_pLTServer->GetPropGeneric( "SpawnItem", &genProp ) == LT_OK )
	{
		if ( genProp.m_String[0] )
			m_hstrSpawnItem = g_pLTServer->CreateString( genProp.m_String );
	}

    if ( g_pLTServer->GetPropGeneric( "HitPoints", &genProp ) == LT_OK )
	{
		m_fDefaultHitPts = genProp.m_Float;
	}

	if ( g_pLTServer->GetPropGeneric( "Energy", &genProp ) == LT_OK )
	{
		m_fDefaultEnergy = genProp.m_Float;
	}

    if ( g_pLTServer->GetPropGeneric( "ArmorPoints", &genProp ) == LT_OK )
	{
		m_fDefaultArmor = genProp.m_Float;
	}

    if ( g_pLTServer->GetPropGeneric( "HeadExtension", &genProp ) == LT_OK )
		if ( genProp.m_String[0] ) 
		{
            m_hstrHeadExtension = g_pLTServer->CreateString( genProp.m_String );
		}

	// read extra childmodel prop
    // make sure we are putting the name into an empty slot.
    char szPropValue[256];

    //  3 = number of childmodels definable in dedit.  t.f fix magic number...
    for( uint32 cm_cnt = 0 ,i = 0 ; i < 3 ; i++ )
    {
        sprintf(szPropValue,"ChildModel_%d", i+1 );
        // find a null child model
        if ( g_pLTServer->GetPropGeneric( szPropValue, &genProp ) == LT_OK )
	{
		if ( genProp.m_String[0] )
		{
                // find slot
                while( m_hstrExtraChildModels[cm_cnt] != NULL )
	{
                    cm_cnt++ ;
                    if( cm_cnt >= MAX_CHILD_MODELS )
		{
                        // t.f fix
                        // we ran out of slots!
                        ASSERT( 0 );
                        return LTTRUE;
		}
	}

                m_hstrExtraChildModels[cm_cnt] = g_pLTServer->CreateString( genProp.m_String );
                m_nExtraChildModels++;

            }
		}
	}

	// Read the general inventory
	char buf[64];
	GEN_INVENTORY_ITEM item;
	for(int nInv=1;nInv<=CHARACTER_MAX_INVENTORY;nInv++)
	{
		sprintf(buf,"Item%dCount",nInv);
		if(g_pLTServer->GetPropGeneric(buf,&genProp)==LT_OK)
		{
			item.nCount = (uint8)genProp.m_Long;
			if(item.nCount > 0)
			{
				// Read in the item
				sprintf(buf,"Item%dID",nInv);
				if(g_pLTServer->GetPropGeneric(buf,&genProp)==LT_OK)
				{
					// Translate the item string to an ID
					item.nItemID = s_InventoryButeMgr.GetItemIndex(genProp.m_String);
					if(item.nItemID <= 0)
					{
						g_pLTServer->CPrint("ERROR - Inventory Item %d had a bad ID\n",nInv);
						TRACE("ERROR - Inventory Item %d had an ID of 0\n",nInv);
						ASSERT(FALSE);
					}
					else
					{
						m_lstInventory.push_back(item);
					}
				}
			}
		}
	}

    return LTTRUE;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::OnTrigger()
//
//	PURPOSE:	Process a trigger message
//
// --------------------------------------------------------------------------- //

bool CCharacter::OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg)
{
	static CParsedMsg::CToken s_cTok_Play_Sound(TRIGGER_PLAY_SOUND);
	static CParsedMsg::CToken s_cTok_Teleport(TRIGGER_TELEPORT);

	static CParsedMsg::CToken s_cTok_Attach(KEY_ATTACH);
	static CParsedMsg::CToken s_cTok_Detach(KEY_DETACH);
	static CParsedMsg::CToken s_cTok_Gadget("GADGET");
	static CParsedMsg::CToken s_cTok_CanDamage("CANDAMAGE");
	static CParsedMsg::CToken s_cTok_Hidden("HIDDEN");
	static CParsedMsg::CToken s_cTok_Crosshair("CROSSHAIR");
	static CParsedMsg::CToken s_cTok_Good("GOOD");
	static CParsedMsg::CToken s_cTok_Bad("BAD");
	static CParsedMsg::CToken s_cTok_Neutral("NEUTRAL");
	static CParsedMsg::CToken s_cTok_Unknown("UNKNOWN");
	static CParsedMsg::CToken s_cTok_Find("FIND");
	static CParsedMsg::CToken s_cTok_Remove("REMOVE");

	if ( (cMsg.GetArg(0) == s_cTok_Play_Sound) && (cMsg.GetArgCount() > 1) )
	{
		// Get sound name from message...
		PlayDialogSound(cMsg.GetArg(1));
		return true;
	}
	else if ( cMsg.GetArg(0) == s_cTok_Teleport )
	{
		// Save the teleportpt off so we can do it in our next update.  We can't
		// teleport now, since we may have been moving when we caused the trigger.  You can't
		// move again if you are already moving.
		if ( !IsVector(cMsg.GetArg(1)) )
		{
			HOBJECT hObject = NULL;
			if ( LT_OK == FindNamedObject(cMsg.GetArg(1), hObject) )
			{
				TeleportPoint* pTeleportPt = dynamic_cast< TeleportPoint* >( g_pLTServer->HandleToObject(hObject));
				if( !pTeleportPt )
					return true;

				m_eTeleportTriggerState = eTeleporTriggerStatePoint;
				m_hTeleportPoint = pTeleportPt->m_hObject;
		        return true;
			}
		}
		else
		{
			sscanf(cMsg.GetArg(1), "%f,%f,%f", &m_vTeleportPos.x, &m_vTeleportPos.y, &m_vTeleportPos.z);
			m_eTeleportTriggerState = eTeleporTriggerStateVector;

			// Clear the teleportpt since we are using the vector instead.
			m_hTeleportPoint = NULL;

			return true;
		}
	}
	else if ( cMsg.GetArg(0) == s_cTok_Attach )
	{
		if( m_pAttachments )
		{
			m_pAttachments->Attach(cMsg.GetArg(1), cMsg.GetArg(2));
			HandleAttach();
		}
        return true;
	}
	else if ( cMsg.GetArg(0) == s_cTok_Detach )
	{
		if( m_pAttachments )
		{
			m_pAttachments->Detach(cMsg.GetArg(1));
			HandleDetach();
		}
        return true;
	}
	else if ( cMsg.GetArg(0) == s_cTok_Gadget )
	{
		HandleGadget(atoi(cMsg.GetArg(1)));
        return true;
	}
	else if ( cMsg.GetArg(0) == s_cTok_CanDamage )
	{
		m_damage.SetCanDamage(IsTrueChar(*cMsg.GetArg(1)));
		return true;
	}
	else if ( cMsg.GetArg(0) == s_cTok_Hidden )
	{
		if( cMsg.GetArgCount() > 1 )
		{
			HideCharacter( IsTrueChar( *cMsg.GetArg(1) ) );
		}

		// Intentionally do NOT return here.
		// We want GameBase to process this message too.
	}
	else if ( cMsg.GetArg(0) == s_cTok_Find )
	{
		LTVector vPos;
		g_pLTServer->GetObjectPos( m_hObject, &vPos );
		vPos = ConvertToDEditPos( vPos );
		g_pLTServer->CPrint("FIND: %s is at pos (%.2f %.2f %.2f)", GetObjectName(m_hObject), vPos.x, vPos.y, vPos.z );
		return true;
	}
	else if ( cMsg.GetArg(0) == s_cTok_Remove )
	{
		AITRACE( AIShowCharacters, ( m_hObject, "Received Remove command." ) );
		// Intentionally falling through.
	}
	else if ( cMsg.GetArg(0) == s_cTok_Crosshair )
	{
		if ( cMsg.GetArg(1) == s_cTok_Good )
		{
			m_ccCrosshair = GOOD;
		}
		else if ( cMsg.GetArg(1) == s_cTok_Bad )
		{
			m_ccCrosshair = BAD;
		}
		else if ( cMsg.GetArg(1) == s_cTok_Neutral )
		{
			m_ccCrosshair = NEUTRAL;
		}
		else if ( cMsg.GetArg(1) == s_cTok_Unknown )
		{
			m_ccCrosshair = UNKNOWN;
		}

		ResetCrosshair( );
		return true;
	}

	return GameBase::OnTrigger( hSender, cMsg );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::ResetCrosshair()
//
//	PURPOSE:	Tell client about crosshair alignment change.
//
// --------------------------------------------------------------------------- //

void CCharacter::ResetCrosshair( )
{
	// If our Crosshair is unknown, then the effect gets the result of
	// our Crosshair relative to the player (Crosshairs are always relative
	// to the player).  m_ccCrosshair can be UNKNOWN if no subclass sets it
	// and if a level designer does not set it explicitly.
	if ( m_ccCrosshair == UNKNOWN )
	{
		// We want to store information relative to the player -- that is,
		// what the player things we are (player may think we are good or 
		// bad or neutral currently)
		RelationSet RS;
		CRelationMgr::GetGlobalRelationMgr()->GetButeMgr()->FillRelationSet(DEFAULT_PLAYERNAME, &RS);
		m_ccCrosshair = GetRelativeAlignment( RS, GetRelationData());
	}

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_SFX_MESSAGE);
	cMsg.Writeuint8(SFX_CHARACTER_ID);
	cMsg.WriteObject(m_hObject);
	cMsg.Writeuint8(CFX_CROSSHAIR_MSG);
	cMsg.Writeuint8(m_ccCrosshair);
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

	CreateSpecialFX();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetTracking
//
//	PURPOSE:	Sets the tracking icon on the radar.
//
// ----------------------------------------------------------------------- //

void CCharacter::SetTracking( bool bTracking )
{
	if( m_bTracking != bTracking )
	{
		m_bTracking = bTracking;

		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_SFX_MESSAGE );
		cMsg.Writeuint8( SFX_CHARACTER_ID );
		cMsg.WriteObject( m_hObject );
		cMsg.Writeuint8( CFX_CHARACTER_TRACKING );
		cMsg.Writebool( bTracking );
		g_pLTServer->SendToClient( cMsg.Read(), LTNULL, MESSAGE_GUARANTEED );

		CreateSpecialFX();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetRadarVisible
//
//	PURPOSE:	Sets the RadarVisible icon on the radar.
//
// ----------------------------------------------------------------------- //

void CCharacter::SetRadarVisible( bool bRadarVisible )
{
	if( m_bRadarVisible != bRadarVisible )
	{
		m_bRadarVisible = bRadarVisible;

		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_SFX_MESSAGE );
		cMsg.Writeuint8( SFX_CHARACTER_ID );
		cMsg.WriteObject( m_hObject );
		cMsg.Writeuint8( CFX_CHARACTER_RADAR );
		cMsg.Writebool( bRadarVisible );
		g_pLTServer->SendToClient( cMsg.Read(), LTNULL, MESSAGE_GUARANTEED );

		CreateSpecialFX();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetCarrying
//
//	PURPOSE:	Sets what the character is carrying
//
// ----------------------------------------------------------------------- //

void CCharacter::SetCarrying( uint8 nCarrying )
{
	if( m_nCarrying != nCarrying )
	{
		m_nCarrying = nCarrying;

		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_SFX_MESSAGE );
		cMsg.Writeuint8( SFX_CHARACTER_ID );
		cMsg.WriteObject( m_hObject );
		cMsg.Writeuint8( CFX_CARRY );
		cMsg.Writeuint8( m_nCarrying );
		g_pLTServer->SendToClient( cMsg.Read(), LTNULL, MESSAGE_GUARANTEED );

		CreateSpecialFX();
	}
}





// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::ProcessDamageMsg()
//
//	PURPOSE:	Process a damage message.
//
// --------------------------------------------------------------------------- //

void CCharacter::ProcessDamageMsg(ILTMessage_Read *pMsg)
{
	if (!pMsg || Camera::IsActive()) return;

	DamageStruct damage;
	damage.InitFromMessage(pMsg);

	if ( !m_damage.IsCantDamageType(damage.eType) && m_damage.GetCanDamage() )
	{
		// Set our pain information

		m_fLastPainTime = g_pLTServer->GetTime();
		m_fLastPainVolume = 1.0f;

		// AIs modify their pain volume.
		// Register AllyPainSound stimulus.
		if(!IsAI(m_hObject))
		{
			LTVector vPainPos;
			g_pLTServer->GetObjectPos(m_hObject, &vPainPos);
			g_pAIStimulusMgr->RegisterStimulus( kStim_AllyPainSound, m_hObject, vPainPos, m_fLastPainVolume );
		}

		// Play a damage sound...

		if (!m_damage.IsDead() && m_damage.GetCanDamage())
		{
			// Play our damage sound

			PlayDamageSound(damage.eType);
		}
	}

	if ( m_damage.IsDead() && m_bWallStick )
	{
		// Should we do it still?

		m_bWallStick = ShouldWallStick();
	}
	else
	{
		m_bWallStick = LTFALSE;
	}


	// [KLS - 2/28/02] - Update our user flags to reflect changes to our surface type
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, GetUserFlagSurfaceType(), 0XFF000000);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetUserFlagSurfaceType()
//
//	PURPOSE:	Return our surface type as a user flag
//
// --------------------------------------------------------------------------- //

uint32 CCharacter::GetUserFlagSurfaceType()
{
	SurfaceType eType = ((m_damage.GetArmorPoints() > 0.0f) ?	g_pModelButeMgr->GetArmorSurfaceType(m_eModelId) : 
																g_pModelButeMgr->GetFleshSurfaceType(m_eModelId));

	return SurfaceToUserFlag(eType);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::ShouldWallStick()
//
//	PURPOSE:	Should we wall stick
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacter::ShouldWallStick()
{
	// No see if we're going to get pinned on the wall

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	LTRotation rRot;
	g_pLTServer->GetObjectRotation(m_hObject, &rRot);

	LTVector vForward = rRot.Forward();

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	float fDir = (m_bStuckInFront ? 1.0f : -1.0f);

	IQuery.m_From	  = vPos;
	IQuery.m_To		  = (vPos - (fDir * vForward * g_BodyStickDist.GetFloat()));
	IQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;
	IQuery.m_FilterFn = WorldFilterFn;
	IQuery.m_PolyFilterFn = LTNULL;

	// Has to hit something

	if ( g_pLTServer->IntersectSegment(&IQuery, &IInfo) )
	{
		// Can the arrow stick into the surface?

		SurfaceType eSurf = GetSurfaceType(IInfo);
		SURFACE* pSurf = g_pSurfaceMgr->GetSurface(eSurf);

		if (pSurf && (pSurf->eType != ST_SKY))
		{
			// Has to be more or less same plane normal as character's forward

			// TODO: bute normal/fwd dp threshhold

			vForward *= fDir;
			if ( IInfo.m_Plane.Normal().Dot(vForward) > s_BodyStickAngle.GetFloat() )
			{
				g_pLTServer->CPrint("plane dot charfwd > %.2f", s_BodyStickAngle.GetFloat());

				// We already know arrow.fwd is within the threshhold, we tested this in CCharacter::AddSpear

				return LTTRUE;
			}
		}
	}

	return LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::InitialUpdate()
//
//	PURPOSE:	Initialize object
//
// ----------------------------------------------------------------------- //

void CCharacter::InitialUpdate(int nInfo)
{
	// Volume debugging

	if (IsPlayer(m_hObject))
	{
		if (!g_VolumeDebugTrack.IsInitted())
		{
            g_VolumeDebugTrack.Init(g_pLTServer, "VolumeDebug", LTNULL, 0.0f);
		}
	}

	if (!s_BodyStickAngle.IsInitted())
	{
        s_BodyStickAngle.Init(g_pLTServer, "BodyStickAngle", LTNULL, 0.8f);
	}

	if(!g_BodyStickDist.IsInitted())
	{
        g_BodyStickDist.Init(g_pLTServer, "BodyStickDist", NULL, 150.0f);
	}

	if(!g_BodyStateTimeout.IsInitted())
	{
        g_BodyStateTimeout.Init(g_pLTServer, "BodyStateTimeout", NULL, 5.0f);
	}


	// Init the animator

	if (nInfo == INITIALUPDATE_SAVEGAME) return;

	// If the character was spawned, he will never get a MID_ALLOBJECTSCREATED,
	// so we need to register persistent stimuli here to make character visible. 

	if( ( !IsPlayer(m_hObject) ) && ( nInfo != INITIALUPDATE_WORLDFILE ) )
	{
		RegisterPersistentStimuli();
	}

	// Create the box used for weapon impact detection...

	CreateHitBox();

	// Make sure this object is added to the global CharacterMgr...

//	g_pCharacterMgr->Add(this);

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, m_dwFlags, FLAGMASK_ALL);
    g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags2, FLAG2_DYNAMICDIRLIGHT, FLAG2_DYNAMICDIRLIGHT);
//    g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags2, FLAG2_DYNAMICDIRLIGHT, FLAGMASK_ALL);

	//see if this object is translucent
	if(g_pModelButeMgr->IsModelTranslucent(m_eModelId))
	{
		//it is, setup the flag
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags2, FLAG2_FORCETRANSLUCENT, FLAG2_FORCETRANSLUCENT);
	}

	m_damage.Init(m_hObject);
	m_damage.SetMass(g_pModelButeMgr->GetModelMass(m_eModelId));

	if (m_fDefaultHitPts >= 0.0f)
	{
		m_damage.SetHitPoints(m_fDefaultHitPts);
		m_damage.SetMaxHitPoints(m_fDefaultHitPts);
	}
	else
	{
		m_damage.SetHitPoints(g_pModelButeMgr->GetModelHitPoints(m_eModelId));
		m_damage.SetMaxHitPoints(g_pModelButeMgr->GetModelMaxHitPoints(m_eModelId));
	}

	if (m_fDefaultArmor >= 0.0f)
	{
		m_damage.SetArmorPoints(m_fDefaultArmor);
		m_damage.SetMaxArmorPoints(m_fDefaultArmor);
	}
	else
	{
		m_damage.SetArmorPoints(g_pModelButeMgr->GetModelArmor(m_eModelId));
		m_damage.SetMaxArmorPoints(g_pModelButeMgr->GetModelMaxArmor(m_eModelId));
	}

	if (m_fDefaultEnergy >= 0.0f)
	{
		m_damage.SetEnergy(m_fDefaultEnergy);
		m_damage.SetMaxEnergy(m_fDefaultEnergy);
	}
	else
	{
		m_damage.SetEnergy(g_pModelButeMgr->GetModelEnergy(m_eModelId));
		m_damage.SetMaxEnergy(g_pModelButeMgr->GetModelMaxEnergy(m_eModelId));
	}

	// Set this as an object that can be seen with spy vision...

    uint32 nFlags = USRFLG_MOVEABLE | /*USRFLG_SPY_VISION |*/ USRFLG_CHARACTER | GetUserFlagSurfaceType();
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, nFlags, nFlags);

	// Set our initial dims based on the current animation...
	// TODO! does this need to change?

    LTVector vDims;
	g_pCommonLT->GetModelAnimUserDims(m_hObject, &vDims, g_pLTServer->GetModelAnimation(m_hObject));
	SetDims(&vDims);

	if (m_bMoveToFloor)
	{
		MoveObjectToFloor(m_hObject);
	}


	// Create the special fx message...

	CreateSpecialFX();

	// Update the hitbox

	UpdateClientHitBox();

	// [KLS - 2/28/02] - Update our user flags to reflect changes to our surface type
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, GetUserFlagSurfaceType(), 0XFF000000);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::CreateSpecialFX()
//
//	PURPOSE:	Add client-side special fx
//
// ----------------------------------------------------------------------- //
void CCharacter::CreateSpecialFX(LTBOOL bUpdateClients /* =LTFALSE */)
{
	// Create the special fx...

	m_pcs->Clear();

	m_pcs->eModelId					= m_eModelId;
	m_pcs->byFXFlags				= m_byFXFlags;
	m_pcs->nTrackers				= 0;								// Subclasses need to fill this in in precreate
	m_pcs->nDimsTracker				= MAIN_TRACKER;						// Main tracker
	m_pcs->fStealthPercent			= m_damage.GetStealthModifier();
	m_pcs->bTracking				= m_bTracking;
	m_pcs->bRadarVisible			= m_bRadarVisible;
	m_pcs->nCarrying				= m_nCarrying;
	

	// If our Crosshair is unknown, then the effect gets the result of
	// our Crosshair relative to the player (Crosshairs are always relative
	// to the player).  m_ccCrosshair can be UNKNOWN if no subclass sets it
	// and if a level designer does not set it explicitly
	if ( m_ccCrosshair == UNKNOWN )
	{
		// We want to store information relative to the player -- that is,
		// what the player things we are (player may think we are good or 
		// bad or neutral currently)
		RelationSet RS;
		CRelationMgr::GetGlobalRelationMgr()->GetButeMgr()->FillRelationSet(DEFAULT_PLAYERNAME, &RS);
		m_ccCrosshair = GetRelativeAlignment( RS, GetRelationData());
	}

	m_pcs->eCrosshairCharacterClass = m_ccCrosshair;
	m_pcs->nDamageFlags				= m_nDamageFlags;
	m_pcs->fPitch					= m_fPitch;


	if( m_hHitBox )
	{
		CCharacterHitBox *pHitBox = dynamic_cast<CCharacterHitBox*>(g_pLTServer->HandleToObject( m_hHitBox ));
		if( pHitBox )
		{
			m_pcs->vHitBoxOffset = pHitBox->GetOffset();
			g_pPhysicsLT->GetObjectDims( m_hHitBox, &m_pcs->vHitBoxDims );
		}
	}
	
	PreCreateSpecialFX(*m_pcs);

	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(SFX_CHARACTER_ID);
		m_pcs->Write(cMsg);
		g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
	}

	// Tell the client about the new info...

	if (bUpdateClients)
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_SFX_MESSAGE);
		cMsg.Writeuint8(SFX_CHARACTER_ID);
		cMsg.WriteObject(m_hObject);
		cMsg.Writeuint8(CFX_ALLFX_MSG);
        m_pcs->Write(cMsg);
		g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SendStealthToClients()
//
//	PURPOSE:	Send our stealth variable to the clients
//
// ----------------------------------------------------------------------- //

void CCharacter::SendStealthToClients()
{
	// Update clients with new info...

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_SFX_MESSAGE);
	cMsg.Writeuint8(SFX_CHARACTER_ID);
	cMsg.WriteObject(m_hObject);
	cMsg.Writeuint8(CFX_STEALTH_MSG);
	cMsg.Writefloat(m_damage.GetStealthModifier());
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

	// Update our special fx message so new clients will get the updated
	// info as well...

	CreateSpecialFX();
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCharacter::SendDamageFlagsToClients
//
//  PURPOSE:	Notify the clients of our damage flags
//
// ----------------------------------------------------------------------- //

void CCharacter::SendDamageFlagsToClients( )
{
	// Update clients with new info...

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_SFX_MESSAGE);
	cMsg.Writeuint8(SFX_CHARACTER_ID);
	cMsg.WriteObject(m_hObject);
	cMsg.Writeuint8(CFX_DMGFLAGS_MSG);
	cMsg.Writeuint64(m_nDamageFlags);
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

	// Update our special fx message so new clients will get the updated
	// info as well...

	CreateSpecialFX();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCharacter::SetDamageFlags
//
//  PURPOSE:	Set our damage flags and send message if changed
//
// ----------------------------------------------------------------------- //

void CCharacter::SetDamageFlags( const DamageFlags nDmgFlags )
{
	// If we are not changing anything just quit out

	if( nDmgFlags == m_nDamageFlags ) return;

	// Set 'em and send 'em...

	m_nDamageFlags = nDmgFlags;
	SendDamageFlagsToClients();
}

void CCharacter::SetInstantDamageFlags( const DamageFlags nDmgFlags )
{
	m_nInstantDamageFlags = nDmgFlags;

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_CHARACTER_ID );
	cMsg.WriteObject( m_hObject );
	cMsg.Writeuint8( CFX_INSTANTDMGFLAGS_MSG );
	cMsg.Writeuint64( m_nInstantDamageFlags );
	g_pLTServer->SendToClient( cMsg.Read(), LTNULL, MESSAGE_GUARANTEED );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::*Cigarette()
//
//	PURPOSE:	Creates/Destroys cigarette sfx on client
//
// ----------------------------------------------------------------------- //

void CCharacter::CreateCigarette(LTBOOL bSmoke)
{
	_ASSERT(!(m_byFXFlags & CHARCREATESTRUCT::eCigarette));

	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_SFX_MESSAGE);
		cMsg.Writeuint8(SFX_CHARACTER_ID);
		cMsg.WriteObject(m_hObject);
		cMsg.Writeuint8(CFX_CIGARETTE_CREATE_MSG);
		g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
	}

	m_byFXFlags |= CHARCREATESTRUCT::eCigarette;

	if ( bSmoke )
	{
		m_byFXFlags |= CHARCREATESTRUCT::eCigaretteSmoke;

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_SFX_MESSAGE);
		cMsg.Writeuint8(SFX_CHARACTER_ID);
		cMsg.WriteObject(m_hObject);
		cMsg.Writeuint8(CFX_CIGARETTESMOKE_CREATE_MSG);
		g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
	}
	else
	{
		m_byFXFlags &= ~CHARCREATESTRUCT::eCigaretteSmoke;
	}

	CreateSpecialFX();
}

void CCharacter::DestroyCigarette()
{
	_ASSERT(m_byFXFlags & CHARCREATESTRUCT::eCigarette);

	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_SFX_MESSAGE);
		cMsg.Writeuint8(SFX_CHARACTER_ID);
		cMsg.WriteObject(m_hObject);
		cMsg.Writeuint8(CFX_CIGARETTE_DESTROY_MSG);
		g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
	}

	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_SFX_MESSAGE);
		cMsg.Writeuint8(SFX_CHARACTER_ID);
		cMsg.WriteObject(m_hObject);
		cMsg.Writeuint8(CFX_CIGARETTESMOKE_DESTROY_MSG);
		g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
	}

	m_byFXFlags &= ~CHARCREATESTRUCT::eCigarette;
	m_byFXFlags &= ~CHARCREATESTRUCT::eCigaretteSmoke;

	CreateSpecialFX();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::*Zzz()
//
//	PURPOSE:	Creates/Destroys sleeping sfx on client
//
// ----------------------------------------------------------------------- //

void CCharacter::CreateZzz()
{
	_ASSERT(!(m_byFXFlags & CHARCREATESTRUCT::eZzz));

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_SFX_MESSAGE);
	cMsg.Writeuint8(SFX_CHARACTER_ID);
	cMsg.WriteObject(m_hObject);
	cMsg.Writeuint8(CFX_ZZZ_CREATE_MSG);
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

	m_byFXFlags |= CHARCREATESTRUCT::eZzz;

	CreateSpecialFX();
}

void CCharacter::DestroyZzz()
{
	_ASSERT(m_byFXFlags & CHARCREATESTRUCT::eZzz);

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_SFX_MESSAGE);
	cMsg.Writeuint8(SFX_CHARACTER_ID);
	cMsg.WriteObject(m_hObject);
	cMsg.Writeuint8(CFX_ZZZ_DESTROY_MSG);
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

	m_byFXFlags &= ~CHARCREATESTRUCT::eZzz;

	CreateSpecialFX();
}

bool CCharacter::HasZzz() const
{
	return !!(m_byFXFlags & CHARCREATESTRUCT::eZzz);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::*FlashLight()
//
//	PURPOSE:	Creates/Destroys flashlight sfx on client
//
// ----------------------------------------------------------------------- //
LTBOOL CCharacter::IsFlashLightOn() const
{
	return (LTBOOL)(!!(m_byFXFlags & CHARCREATESTRUCT::eFlashLight));
}

LTBOOL CCharacter::HasFlashLight()
{
	return (m_byFXFlags & CHARCREATESTRUCT::eFlashLight);
}

void CCharacter::CreateFlashLight()
{
	_ASSERT(!(m_byFXFlags & CHARCREATESTRUCT::eFlashLight));

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_SFX_MESSAGE);
	cMsg.Writeuint8(SFX_CHARACTER_ID);
	cMsg.WriteObject(m_hObject);
	cMsg.Writeuint8(CFX_FLASHLIGHT_CREATE_MSG);
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

	m_byFXFlags |= CHARCREATESTRUCT::eFlashLight;

	CreateSpecialFX();
}

void CCharacter::DestroyFlashLight()
{
	_ASSERT(m_byFXFlags & CHARCREATESTRUCT::eFlashLight);

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_SFX_MESSAGE);
	cMsg.Writeuint8(SFX_CHARACTER_ID);
	cMsg.WriteObject(m_hObject);
	cMsg.Writeuint8(CFX_FLASHLIGHT_DESTROY_MSG);
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

	m_byFXFlags &= ~CHARCREATESTRUCT::eFlashLight;

	CreateSpecialFX();
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCharacter::SetPitch
//
//  PURPOSE:	Assign our pitch value and determine if we should notify the clients...
//
// ----------------------------------------------------------------------- //

void CCharacter::SetPitch( float fPitch )
{
	m_fPitch = fPitch;

	// We only care about player pitch if we are in a multiplayer game...
	
	if( IsMultiplayerGame() )
	{
		if( m_fLastPitch != m_fPitch )
		{
			m_fLastPitch = m_fPitch;

			LTRESULT ltResult;
			CAutoMessage cMsg;

			cMsg.Writeuint8(MID_SFX_MESSAGE);

			cMsg.Writeuint8( SFX_CHARACTER_ID );

			cMsg.WriteObject( m_hObject );

			cMsg.Writeuint8( CFX_PITCH );

			// and write it to the message.
			cMsg.Writeuint8( CompressAngleToByte( fPitch ) );
			
			// then send the message
			ltResult = g_pLTServer->SendToClient( cMsg.Read(), LTNULL, 0);
			ASSERT( LT_OK == ltResult );

			CreateSpecialFX();
		}
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCharacter::GetRotationWithPitch
//
//  PURPOSE:	Gets the rotation of the object including the pitch.
//
// ----------------------------------------------------------------------- //

LTRotation CCharacter::GetRotationWithPitch( ) const
{
	LTRotation rRot;
	g_pLTServer->GetObjectRotation( m_hObject, &rRot );

	// Convert the pitch percentage into radians.
	float fPitchRadians = ( m_fPitch - 0.5f ) * MATH_PI;

	// Rotate with the pitch.
	rRot.Rotate( rRot.Right( ), fPitchRadians );

	return rRot;
}
		
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::*Armor()
//
//	PURPOSE:	Creates/Destroys armor sfx on client
//
// ----------------------------------------------------------------------- //

void CCharacter::CreateArmor()
{
	m_bArmored = LTTRUE;
}

void CCharacter::DestroyArmor()
{
	m_bArmored = LTFALSE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandHeldWeaponFirePos()
//
//	PURPOSE:	Get the fire (flash) position of the hand-held weapon
//
// ----------------------------------------------------------------------- //

LTVector CCharacter::HandHeldWeaponFirePos(CWeapon *pWeapon)
{
    LTVector vPos;

	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	if (!g_pLTServer || !pWeapon || !g_pWeaponMgr) return vPos;

	HATTACHMENT hAttachment;
    if (g_pLTServer->FindAttachment(m_hObject, pWeapon->GetModelObject(), &hAttachment) != LT_OK)
	{
		return vPos;
	}

	HMODELSOCKET hSocket;

    if (g_pModelLT->GetSocket(pWeapon->GetModelObject(), "Flash", hSocket) == LT_OK)
	{
		LTransform transform;
		g_pCommonLT->GetAttachedModelSocketTransform(hAttachment, hSocket, transform);

		vPos = transform.m_Pos;

	}

	return vPos;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::InitAnimation()
//
//	PURPOSE:	Initializes our animation
//
// ----------------------------------------------------------------------- //

void CCharacter::InitAnimation()
{
	// Init the animator if we haven't done so yet

	if ( m_pAnimator && !m_pAnimator->IsInitialized() )
	{
		m_pAnimator->Init(m_hObject);
	}

	// Add the recoil tracker

	if ( m_bShortRecoil )
	{
		// Use an arbitrary tracker ID (FIXTRACKER)
		m_RecoilAnimTracker = 44;
        g_pModelLT->AddTracker(m_hObject, m_RecoilAnimTracker);

		if ( LT_OK != g_pModelLT->FindWeightSet(m_hObject, "Null", m_hNullWeightset) )
		{
#ifndef _FINAL
			g_pLTServer->CPrint("Critical error, no Null weightset on Character!");
#endif
		}

		if ( LT_OK != g_pModelLT->FindWeightSet(m_hObject, "Blink", m_hBlinkWeightset) )
		{
#ifndef _FINAL
			g_pLTServer->CPrint("Critical error, no Blink weightset on Character!");
#endif
			m_hBlinkWeightset = m_hNullWeightset;
		}

		if ( LT_OK != g_pModelLT->FindWeightSet(m_hObject, "Twitch", m_hTwitchWeightset) )
		{
#ifndef _FINAL
			g_pLTServer->CPrint("Critical error, no Twitch weightset on Character!");
#endif
			m_hTwitchWeightset = m_hNullWeightset;
		}
		else
		{
			g_pModelLT->SetWeightSet(m_hObject, m_RecoilAnimTracker, m_hTwitchWeightset);
		}

        g_pModelLT->SetCurAnim(m_hObject, m_RecoilAnimTracker, g_pLTServer->GetAnimIndex(m_hObject, "Base"));
        g_pModelLT->SetLooping(m_hObject, m_RecoilAnimTracker, LTFALSE);
	}

	// Add the blink tracker

	if ( m_bBlink )
	{
		// Use an arbitrary tracker number (FIXTRACKER)
		m_BlinkAnimTracker = 45;
        g_pModelLT->AddTracker(m_hObject, m_BlinkAnimTracker);
        g_pModelLT->SetCurAnim(m_hObject, m_BlinkAnimTracker, g_pLTServer->GetAnimIndex(m_hObject, "Blink"));
        g_pModelLT->SetLooping(m_hObject, m_BlinkAnimTracker, LTTRUE);

		SetBlinking(LTTRUE);
	}

	// We're initted

	m_bInitializedAnimation = LTTRUE;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CCharacter::UpdateRelationMgr()
//              
//	PURPOSE:	Does the RelationMgr updating.  Happens every frame.  
//				Currently relations are always allowed to be removed.
//              
//----------------------------------------------------------------------------
void CCharacter::UpdateRelationMgr()
{
	m_pRelationMgr->Update( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::Update()
//
//	PURPOSE:	Update the object
//
// ----------------------------------------------------------------------- //

void CCharacter::Update()
{
	UpdateTeleport( );

	UpdateRelationMgr();

	if ( !m_bInitializedAnimation )
	{
		InitAnimation();
	}

	// Update the recoil

	if ( m_bShortRecoiling )
	{
		uint32 dwFlags;
        if ( LT_OK == g_pModelLT->GetPlaybackState(m_hObject, m_RecoilAnimTracker, dwFlags) )
		{
			if ( MS_PLAYDONE & dwFlags )
			{
				g_pModelLT->SetWeightSet(m_hObject, m_RecoilAnimTracker, m_hNullWeightset);
				m_bShortRecoiling = LTFALSE;
			}
		}
	}

	// Update our Information Volume position
	UpdateCurrentInformationVolume();
	

	// Update our last volume position
	if ( g_pAIVolumeMgr->IsInitialized() && IsVisible() )
	{
        LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);
		AIVolume* pVolume = g_pAIVolumeMgr->FindContainingVolume(LTNULL, vPos, eAxisAll, GetVerticalThreshold(), m_pLastVolume);

		if ( pVolume )
		{
			if( !m_pCurrentVolume )
			{
				AITRACE( AIShowVolumes, ( m_hObject, "Starting in volume: %s (%s)", 
					pVolume->GetName(), pVolume->GetRegion() ? pVolume->GetRegion()->GetName() : "No Region" ) );
			}

			if ( m_pLastVolume && m_pLastVolume != pVolume )
			{
				HandleVolumeExit(m_pLastVolume);
			}

			if ( m_pLastVolume != pVolume )
			{
				HandleVolumeEnter(pVolume);
			}

			m_pCurrentVolume = pVolume;
			m_pLastVolume = m_pCurrentVolume;
			m_vLastVolumePos = vPos;

#ifndef _FINAL
			if ( IsPlayer(m_hObject) && g_VolumeDebugTrack.GetFloat(0.0f) == 1.0f )
			{
				g_pLTServer->CPrint("Player in volume \"%s\"", pVolume->GetName());
			}
#endif
		}
		else
		{
			m_pCurrentVolume = LTNULL;
#ifndef _FINAL
			if ( IsPlayer(m_hObject) && g_VolumeDebugTrack.GetFloat(0.0f) == 1.0f )
			{
				g_pLTServer->CPrint("Player not in volume");
			}
#endif
		}
	}

	// Update our sounds

	UpdateSounds();

	// Keep track of frame to frame changes...

	m_eLastContainerCode	= m_eContainerCode;
	m_bBodyWasInLiquid		= m_bBodyInLiquid;

    m_bBodyInLiquid         = LTFALSE;
    m_bBodyOnLadder         = LTFALSE;

	// Update our container code info...

	UpdateContainerCode();

	// Make sure our hit box is in the correct position...

	UpdateHitBox();

	// Update our animation

	UpdateAnimation();

	// Update our playertracker for dialogue.
	m_PlayerTrackerDialogue.Update( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::RecalcLastVolumePos()
//
//	PURPOSE:	Update the last volume position
//
// ----------------------------------------------------------------------- //

void CCharacter::RecalcLastVolumePos()
{
	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	AIVolume* pVolume = g_pAIVolumeMgr->FindContainingVolume(LTNULL, vPos, eAxisAll, GetVerticalThreshold(), m_pLastVolume);

	if ( pVolume )
	{
		m_pCurrentVolume = pVolume;
		m_pLastVolume = m_pCurrentVolume;
		m_vLastVolumePos = vPos;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CCharacter::RegisterPersistentStimuli()
//              
//	PURPOSE:	Register all stimuli which live as long as the character
//              
//----------------------------------------------------------------------------
void CCharacter::RegisterPersistentStimuli(void)
{
	// Register for SeeEnemy stimulus.  
	// Save the registration ID in case we want to toggle visibility.
	if( ( m_eEnemyVisibleStimID == kStimID_Unset ) || 
		( !g_pAIStimulusMgr->StimulusExists( m_eEnemyVisibleStimID ) ) )
	{
		m_eEnemyVisibleStimID
			= g_pAIStimulusMgr->RegisterStimulus( kStim_EnemyVisible, m_hObject, LTNULL, CAIStimulusRecord::kDynamicPos_TrackSource, 1.f );
		AITRACE( AIShowCharacters, ( m_hObject, "Registered persistent stimulus EnemyVisible = %d", m_eEnemyVisibleStimID ) );
	}
	else if( g_pAIStimulusMgr->StimulusExists( m_eEnemyVisibleStimID ) )
	{
		AITRACE( AIShowCharacters, ( m_hObject, "NOT Registering persistent stimulus EnemyVisible, because StimulusID %d already exists!", m_eEnemyVisibleStimID ) );
	}

	// Register for UndeterminedVisible stimulus.
	// Save the registration ID in case we want to toggle visibility.
	if( ( m_eUndeterminedVisibleStimID == kStimID_Unset ) ||
		( !g_pAIStimulusMgr->StimulusExists( m_eUndeterminedVisibleStimID ) ) )
	{
		AITRACE( AIShowCharacters, ( m_hObject, "Registering persistent stimulus UndeterminedVisible" ) );
		m_eUndeterminedVisibleStimID
			= g_pAIStimulusMgr->RegisterStimulus( kStim_UndeterminedVisible, m_hObject, LTNULL, CAIStimulusRecord::kDynamicPos_TrackSource, 1.f );
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CCharacter::RemovePersistentStimuli()
//              
//	PURPOSE:	Remove all stimuli which live as long as the character
//              
//----------------------------------------------------------------------------
void CCharacter::RemovePersistentStimuli(void)
{
	if( m_eEnemyVisibleStimID != kStimID_Unset )
	{
		AITRACE( AIShowCharacters, ( m_hObject, "Removed persistent stimulus EnemyVisible = %d", m_eEnemyVisibleStimID ) );
		g_pAIStimulusMgr->RemoveStimulus( m_eEnemyVisibleStimID );
		m_eEnemyVisibleStimID = kStimID_Unset;
	}

	if( m_eUndeterminedVisibleStimID != kStimID_Unset )
	{
		AITRACE( AIShowCharacters, ( m_hObject, "Removing persistent stimulus UndeterminedVisible" ) );
		g_pAIStimulusMgr->RemoveStimulus( m_eUndeterminedVisibleStimID );
		m_eUndeterminedVisibleStimID = kStimID_Unset;
	}
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CCharacter::IsVisible()
//              
//	PURPOSE:	Character is invisible if it has no visible stimuli active
//              
//----------------------------------------------------------------------------
bool CCharacter::IsVisible(void)
{
	return ( m_eUndeterminedVisibleStimID != kStimID_Unset && m_eEnemyVisibleStimID != kStimID_Unset );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HideCharacter
//
//	PURPOSE:	Hide/Show character.
//
// ----------------------------------------------------------------------- //

void CCharacter::HideCharacter(LTBOOL bHide)
{
	if( bHide )
	{
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAG_VISIBLE | FLAG_TOUCH_NOTIFY | FLAG_RAYHIT );
		g_pCommonLT->SetObjectFlags(m_hHitBox, OFT_Flags, 0, FLAG_RAYHIT );
		HideAttachments( LTTRUE );

		// Hide any attached spears.

		for ( uint32 iSpear = 0 ; iSpear < m_cSpears ; iSpear++ )
		{
			HOBJECT hSpear = m_aSpears[iSpear].hObject;
			if ( hSpear )
			{
				g_pCommonLT->SetObjectFlags( hSpear, OFT_Flags, FLAG_FORCECLIENTUPDATE, FLAG_FORCECLIENTUPDATE | FLAG_VISIBLE );
			}
		}
	}
	else
	{
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_VISIBLE | FLAG_TOUCH_NOTIFY | FLAG_RAYHIT, FLAG_VISIBLE | FLAG_TOUCH_NOTIFY | FLAG_RAYHIT );
		g_pCommonLT->SetObjectFlags(m_hHitBox, OFT_Flags, FLAG_RAYHIT, FLAG_RAYHIT );
		HideAttachments( LTFALSE );

		// Unhide any attached spears.

		for ( uint32 iSpear = 0 ; iSpear < m_cSpears ; iSpear++ )
		{
			HOBJECT hSpear = m_aSpears[iSpear].hObject;
			if ( hSpear )
			{
				g_pCommonLT->SetObjectFlags( hSpear, OFT_Flags, FLAG_VISIBLE, FLAG_FORCECLIENTUPDATE | FLAG_VISIBLE );
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleShortRecoil()
//
//	PURPOSE:	Handles doing a short recoil
//
// ----------------------------------------------------------------------- //

void CCharacter::HandleShortRecoil()
{
	if ( !m_bShortRecoil || m_bShortRecoiling ) return;
	if ( eModelNodeInvalid == m_eModelNodeLastHit ) return;

	const char* szRecoil = LTNULL;
	if ( HitFromFront(m_damage.GetLastDamageDir()) )
	{
		szRecoil = g_pModelButeMgr->GetSkeletonNodeFrontShortRecoilAni(m_eModelSkeleton, m_eModelNodeLastHit);
	}
	else
	{
		szRecoil = g_pModelButeMgr->GetSkeletonNodeBackShortRecoilAni(m_eModelSkeleton, m_eModelNodeLastHit);
	}

	HMODELANIM hAni;
	if ( !szRecoil || (INVALID_MODEL_ANIM == (hAni = g_pLTServer->GetAnimIndex(m_hObject, (char*)szRecoil))) )
	{
		return;
	}

    g_pModelLT->SetCurAnim(m_hObject, m_RecoilAnimTracker, hAni);
    g_pModelLT->SetWeightSet(m_hObject, m_RecoilAnimTracker, m_hTwitchWeightset);
    g_pModelLT->ResetAnim(m_hObject, m_RecoilAnimTracker);

	m_bShortRecoiling = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleModelString()
//
//	PURPOSE:	Handles model keyframe strings
//
// ----------------------------------------------------------------------- //

void CCharacter::HandleModelString(ArgList* pArgList)
{
	static CParsedMsg::CToken s_cTok_KEY_FOOTSTEP_SOUND( KEY_FOOTSTEP_SOUND );
	static CParsedMsg::CToken s_cTok_KEY_PLAYSOUND( KEY_PLAYSOUND );
	static CParsedMsg::CToken s_cTok_KEY_ATTACH( KEY_ATTACH );
	static CParsedMsg::CToken s_cTok_KEY_DETACH( KEY_DETACH );
	static CParsedMsg::CToken s_cTok_KEY_ON( KEY_ON );
	static CParsedMsg::CToken s_cTok_KEY_OFF( KEY_OFF );
	static CParsedMsg::CToken s_cTok_KEY_SET_DIMS( KEY_SET_DIMS );
	static CParsedMsg::CToken s_cTok_KEY_MOVE( KEY_MOVE );
	static CParsedMsg::CToken s_cTok_KEY_COMMAND( KEY_COMMAND );
	static CParsedMsg::CToken s_cTok_KEY_BUTE_SOUND( KEY_BUTE_SOUND );
	static CParsedMsg::CToken s_cTok_KEY_DEFLECT( KEY_DEFLECT );
	static CParsedMsg::CToken s_cTok_HITBOX_DIMS( "HITBOX_DIMS" );
	static CParsedMsg::CToken s_cTok_HITBOX_OFFSET( "HITBOX_OFFSET" );
	static CParsedMsg::CToken s_cTok_HITBOX_DEFAULT( "HITBOX_DEFAULT" );
		
	if (!pArgList || !pArgList->argv || pArgList->argc == 0) return;

	char* pKey = pArgList->argv[0];
	if (!pKey) return;

	CParsedMsg::CToken tok( pKey );

	// Only update the footstep info if we are on the ground...(NOTE:  The
	// character footstep sounds are now played on the client. see CCharacterFX
	// if you're interested)

	if ( tok == s_cTok_KEY_FOOTSTEP_SOUND )
	{
		if( m_bOnGround || m_bBodyOnLadder )
		{
			LTBOOL bInWater = (m_bBodyInLiquid && !IsLiquid(m_eContainerCode));

			if (m_bBodyOnLadder)
			{
				m_eStandingOnSurface = ST_LADDER;
			}
			else if (bInWater)
			{
				m_eStandingOnSurface = ST_LIQUID;
			}

			LTFLOAT fVolume = GetFootstepVolume();

			// Adjust the footstep volume by our stealth modifier...

			fVolume *= m_damage.GetStealthModifier();

			SURFACE* pSurf = g_pSurfaceMgr->GetSurface(m_eStandingOnSurface);
			_ASSERT(pSurf);
			if (pSurf)
			{
				fVolume *= pSurf->fMovementNoiseModifier;

				// TODO: reduce this to a bool function call
				if ( *pSurf->szLtFootPrintSpr && *pSurf->szRtFootPrintSpr )
				{
					// If this is a surface that creates footprints, add a footprint to our list

					LTVector vFootprintPos;
					g_pLTServer->GetObjectPos(m_hObject, &vFootprintPos);

					// Register EnemyFootprintVisible stimulus.
					g_pAIStimulusMgr->RegisterStimulus( kStim_EnemyFootprintVisible, m_hObject, vFootprintPos, 1.0f, pSurf->fFootPrintLifetime );
				}
			}

			// Register EnemyFootstepSound stimulus.
			LTVector vMovementPos;
			g_pLTServer->GetObjectPos(m_hObject, &vMovementPos);
			g_pAIStimulusMgr->RegisterStimulus( kStim_EnemyFootstepSound, m_hObject, vMovementPos, fVolume );
		}
	}
	else if ( tok == s_cTok_KEY_PLAYSOUND )
	{
		if( pArgList->argc > 1 )
	{
		// Get sound name from message...

		char* pSoundName = pArgList->argv[1];

		if (pSoundName)
		{
			// See if sound radius was in message..

			LTFLOAT fRadius = 1000;

			if (pArgList->argc > 3 && pArgList->argv[2])
			{
				fRadius = (LTFLOAT) atoi(pArgList->argv[2]);
			}

			fRadius = fRadius > 0.0f ? fRadius : m_fSoundRadius;

            PlaySound(pSoundName, fRadius, LTTRUE);
		}
	}
	}
	else if ( tok == s_cTok_KEY_ATTACH)
	{
		if( (pArgList->argc != 3) || !m_pAttachments )
			return;

		m_pAttachments->Attach(pArgList->argv[1], pArgList->argv[2]);
		HandleAttach();
	}
	else if ( tok == s_cTok_KEY_DETACH)
	{
		if( (pArgList->argc != 2) || !m_pAttachments )
			return;

		m_pAttachments->Detach(pArgList->argv[1]);
		HandleDetach();
	}
	else if ( tok == s_cTok_KEY_ON )
	{
		if (pArgList->argc != 2) return;

		HOBJECT hObject = LTNULL;
		if ( LT_OK == FindNamedObject(pArgList->argv[1], hObject) )
		{
			SendTriggerMsgToObject(this, hObject, LTFALSE, "ON");
		}
	}
	else if ( tok == s_cTok_KEY_OFF )
	{
		if (pArgList->argc != 2) return;

		HOBJECT hObject = LTNULL;
		if ( LT_OK == FindNamedObject(pArgList->argv[1], hObject) )
		{
			SendTriggerMsgToObject(this, hObject, LTFALSE, "OFF");
		}
	}
	else if ( tok == s_cTok_KEY_SET_DIMS )
	{
		if (pArgList->argc < 2) return;

		// Set up so we can set one or more dims...

        LTVector vDims;
		g_pPhysicsLT->GetObjectDims(m_hObject, &vDims);

		if (pArgList->argv[1])
		{
			vDims.x = (LTFLOAT) atof(pArgList->argv[1]);
		}
		if (pArgList->argc > 2 && pArgList->argv[2])
		{
			vDims.y = (LTFLOAT) atof(pArgList->argv[2]);
		}
		if (pArgList->argc > 3 && pArgList->argv[3])
		{
			vDims.z = (LTFLOAT) atof(pArgList->argv[3]);
		}

		// Set the new dims

		SetDims(&vDims);
	}
	else if ( tok == s_cTok_KEY_MOVE )
	{
		if (pArgList->argc < 2) return;

		// Set up so we move in one or more directions

        LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);

        LTRotation rRot;
		g_pLTServer->GetObjectRotation(m_hObject, &rRot);

		LTFLOAT fOffset;

		if (pArgList->argv[1])
		{
			// Forward...

			fOffset = (LTFLOAT) atof(pArgList->argv[1]);

			vPos += rRot.Forward() * fOffset;
		}
		if (pArgList->argc > 2 && pArgList->argv[2])
		{
			// Up...

			fOffset = (LTFLOAT) atof(pArgList->argv[2]);

			vPos += rRot.Up() * fOffset;
		}
		if (pArgList->argc > 3 && pArgList->argv[3])
		{
			// Right...

			fOffset = (LTFLOAT) atof(pArgList->argv[3]);

			vPos += rRot.Right() * fOffset;
		}

		// Set the new position

		g_pLTServer->MoveObject(m_hObject, &vPos);
	}
	else if ( tok == s_cTok_KEY_COMMAND )
	{
		LTBOOL bAddParen = LTFALSE;

		char buf[255] = "";
		sprintf(buf, "%s", pArgList->argv[1]);
		for (int i=2; i < pArgList->argc; i++)
		{
			bAddParen = LTFALSE;
			strcat(buf, " ");
			if (strstr(pArgList->argv[i], " "))
			{
				strcat(buf, "(");
				bAddParen = LTTRUE;
			}

			strcat(buf, pArgList->argv[i]);

			if (bAddParen)
			{
				strcat(buf, ")");
			}
		}

		g_pLTServer->CPrint("KEY COMMAND: %s", buf);
		if (buf[0] && g_pCmdMgr->IsValidCmd(buf))
		{
			g_pCmdMgr->Process(buf, m_hObject, m_hObject);
		}
    }
	else if( tok == s_cTok_KEY_BUTE_SOUND )
	{
		if( pArgList->argc > 1 && pArgList->argv[1] )
		{
			PlaySound( pArgList->argv[1], -1 );
		}
	}
	else if( tok == s_cTok_KEY_DEFLECT )
	{
		if( pArgList->argc > 1 && pArgList->argv[1] )
		{
			// Will deflect ammo that is set as bCanBeDeflected == true.
			SetDeflecting(( float )atof( pArgList->argv[1] ));
		}
	}
	else if( tok == s_cTok_HITBOX_DIMS )
	{
		if( pArgList->argc > 2 )
		{
			CCharacterHitBox* pHitBox = dynamic_cast<CCharacterHitBox*>(g_pLTServer->HandleToObject(m_hHitBox));
			if( pHitBox )
			{
				HMODELANIM hCurAnim = INVALID_ANI;
				if( LT_OK == g_pModelLT->GetCurAnim( m_hObject, MAIN_TRACKER, hCurAnim ))
				{
					LTVector vDims;
					vDims.x = vDims.z = (float)atoi(pArgList->argv[1]);
					vDims.y = (float)atoi(pArgList->argv[2]);

					g_pPhysicsLT->SetObjectDims(m_hHitBox, &vDims, 0);
					pHitBox->SetAnimControllingDims( true, hCurAnim );
					
					UpdateClientHitBox();
				}
			}				
		}
	}
	else if( tok == s_cTok_HITBOX_OFFSET )
	{
		if( pArgList->argc > 3 )
		{
			CCharacterHitBox* pHitBox = dynamic_cast<CCharacterHitBox*>(g_pLTServer->HandleToObject(m_hHitBox));
			if( pHitBox )
			{
				HMODELANIM hCurAnim = INVALID_ANI;
				if( LT_OK == g_pModelLT->GetCurAnim( m_hObject, MAIN_TRACKER, hCurAnim ))
				{
					LTVector vOffset;
					vOffset.x = (float)atoi(pArgList->argv[1]);
					vOffset.y = (float)atoi(pArgList->argv[2]);
					vOffset.z = (float)atoi(pArgList->argv[3]);

					pHitBox->SetOffset(vOffset);
					pHitBox->SetAnimControllingOffset( true, hCurAnim );

					UpdateClientHitBox();
				}
			}
		}
	}
	else if( tok == s_cTok_HITBOX_DEFAULT )
	{
		CCharacterHitBox* pHitBox = dynamic_cast<CCharacterHitBox*>(g_pLTServer->HandleToObject(m_hHitBox));
		if( pHitBox )
		{
			pHitBox->SetDimsToModel();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateAnimation()
//
//	PURPOSE:	Update the current animation
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateAnimation()
{
	// If we're dead, we do that first

	if( m_damage.IsDead() )
	{
		SetDeathAnimation();
		return;
	}
    else if( m_pAnimator )
	{
		m_pAnimator->Update();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetDeathAnimation()
//
//	PURPOSE:	Set animation to death
//
// ----------------------------------------------------------------------- //

void CCharacter::SetDeathAnimation()
{
	if (m_bStartedDeath) return;

	StartDeath();

	// Figure out if this was a death from behind or from front

    LTBOOL bFront = HitFromFront(m_damage.GetDeathDir());

	// Choose the appropriate death ani

	m_eDeathType = CD_NORMAL;

	LTFLOAT fDeathDamage = m_damage.GetDeathDamage();
	LTFLOAT fMaxHitPts   = m_damage.GetMaxHitPoints();

	m_eDeathDamageType = m_damage.GetDeathType();
    LTBOOL bGibDeath   = LTFALSE;

	// Virtual GetAlternateDeathAnimation gives AIs a chance to choose their
	// own death animation.

	HMODELANIM hAni = GetAlternateDeathAnimation();

	// If no alternate was found, do the normal selection.

	if( hAni == INVALID_ANI )
	{
		hAni = GetDeathAni(bFront);
	}

	// Set the death animation

	if ( hAni != INVALID_ANI )
	{
	    g_pLTServer->SetModelLooping(m_hObject, LTFALSE);

		// Set model dims based on animation...

		LTVector vDims;
		if( LT_OK == g_pCommonLT->GetModelAnimUserDims( m_hObject, &vDims, hAni ))
			SetDims( &vDims );

		g_pLTServer->SetModelAnimation(m_hObject, hAni);

	}
	else {
		AIASSERT( 0, m_hObject, "No death animation found." );	
	}	

	// Make us nonsolid...

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAG_SOLID);

	// Handle dead

    HandleDead(LTTRUE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetDeathAni()
//
//	PURPOSE:	Gets the location based death animation
//
// ----------------------------------------------------------------------- //

HMODELANIM CCharacter::GetDeathAni(LTBOOL bFront)
{
	HMODELANIM hAni = INVALID_ANI;
	const char* szDeathAni = NULL;

	if ( bFront )
	{
		// Look for a death ani specific to this node

		if ( eModelNodeInvalid != m_eModelNodeLastHit )
		{
			szDeathAni = g_pModelButeMgr->GetSkeletonNodeFrontDeathAni(m_eModelSkeleton, m_eModelNodeLastHit);
			if ( szDeathAni )
			{
				hAni = g_pLTServer->GetAnimIndex(m_hObject, (char*)szDeathAni);
			}
		}

		// If the given node-specific ani could not be found, just use the default (which better be there)

		if ( hAni == INVALID_ANI )
		{
			g_pLTServer->CPrint( "Character %s is missing animation %s.", GetObjectName(m_hObject), szDeathAni );

			szDeathAni = g_pModelButeMgr->GetSkeletonDefaultFrontDeathAni(m_eModelSkeleton);
			if ( szDeathAni )
			{
				hAni = g_pLTServer->GetAnimIndex(m_hObject, (char*)szDeathAni);
			}
		}

		if( hAni == INVALID_ANI )
		{
			AIASSERT( 0, m_hObject, "No death animation found." );	
			g_pLTServer->CPrint( "Character %s is missing animation %s.", GetObjectName(m_hObject), szDeathAni );
		}

	}
	else
	{
		// Look for a death ani specific to this node

		if ( eModelNodeInvalid != m_eModelNodeLastHit )
		{
			szDeathAni = g_pModelButeMgr->GetSkeletonNodeBackDeathAni(m_eModelSkeleton, m_eModelNodeLastHit);
			if ( szDeathAni )
			{
				hAni = g_pLTServer->GetAnimIndex(m_hObject, (char*)szDeathAni);
			}
		}

		// If the given node-specific ani could not be found, just use the default (which better be there)

		if ( hAni == INVALID_ANI )
		{
			g_pLTServer->CPrint( "Character %s is missing animation %s.", GetObjectName(m_hObject), szDeathAni );

			szDeathAni = g_pModelButeMgr->GetSkeletonDefaultBackDeathAni(m_eModelSkeleton);
			if ( szDeathAni )
			{
				hAni = g_pLTServer->GetAnimIndex(m_hObject, (char*)szDeathAni);
			}
		}

		if( hAni == INVALID_ANI )
		{
			AIASSERT( 0, m_hObject, "No death animation found." );	
			g_pLTServer->CPrint( "Character %s is missing animation %s.", GetObjectName(m_hObject), szDeathAni );
		}
	}

	return hAni;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateMovement
//
//	PURPOSE:	Update character movement
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateMovement(LTBOOL bUpdatePhysics)
{
	// Update m_bOnGround data member...

	UpdateOnGround();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::PushCharacter
//
//	PURPOSE:	Push character from some position
//
// ----------------------------------------------------------------------- //

void CCharacter::PushCharacter(const LTVector &vPos, LTFLOAT fRadius, LTFLOAT fStartDelay, LTFLOAT fDuration, LTFLOAT fStrength)
{
	AIASSERT(0, m_hObject, "CCharacter::PushCharacter: Currently, only the Player can be pushed.");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateOnLadder
//
//	PURPOSE:	Update if we're on a ladder
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateOnLadder(VolumeBrush* pBrush, ContainerPhysics* pCPStruct)
{
    m_bBodyOnLadder = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateInLiquid
//
//	PURPOSE:	Update if we're in liquid
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateInLiquid(VolumeBrush* pBrush, ContainerPhysics* pCPStruct)
{
    m_bBodyInLiquid = LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateOnGround
//
//	PURPOSE:	Update m_bOnGround data member
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateOnGround()
{
	// See if we're standing on any breakable objects...

	CollisionInfo Info;
	g_pLTServer->GetStandingOn(m_hObject, &Info);

	if (Info.m_hObject && IsKindOf(Info.m_hObject, "Breakable"))
	{
        SendTriggerMsgToObject(this, Info.m_hObject, LTFALSE, "TOUCHNOTIFY");
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateSounds()
//
//	PURPOSE:	Update the currently playing sounds
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateSounds()
{
	// Check if we had a server side sound played.
	if (m_hCurDlgSnd)
	{
        bool bIsDone = LTFALSE;
		if (g_pLTServer->SoundMgr()->IsSoundDone(m_hCurDlgSnd, bIsDone) != LT_OK || bIsDone)
		{
			KillDlgSnd();
		}
	}

	// See if we are coming out of a liquid...

	if (!m_bBodyInLiquid && m_bBodyWasInLiquid)
	{
        PlaySound("Chars\\Snd\\splash1.wav", m_fSoundRadius, LTFALSE);
	}
	else if (!m_bBodyWasInLiquid && m_bBodyInLiquid)  // or going into
	{
        PlaySound("Chars\\Snd\\splash2.wav", m_fSoundRadius, LTFALSE);
	}

	if ( m_tmrDialogue.On() && m_tmrDialogue.Stopped() )
	{
		m_tmrDialogue.Stop();
		m_PlayerTrackerDialogue.Term();
		KillDlgSnd();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::PlaySound
//
//	PURPOSE:	Play the specified sound
//
// ----------------------------------------------------------------------- //

void CCharacter::PlaySound(char *pSoundName, LTFLOAT fRadius, LTBOOL bAttached)
{
    LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	g_pServerSoundMgr->PlaySoundFromPos(vPos, pSoundName, fRadius, m_eSoundPriority);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::PlayDamageSound
//
//	PURPOSE:	Play a damage sound
//
// ----------------------------------------------------------------------- //

void CCharacter::PlayDamageSound(DamageType eType)
{
	if (m_eCurDlgSndType == CST_DAMAGE) return;

	PlayDialogSound(GetDamageSound(eType), CST_DAMAGE);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::DoLipSyncingOnDlgSound
//
//	PURPOSE:	Check if we should be doing lip syncing on the dialog.
//
// ----------------------------------------------------------------------- //
bool CCharacter::DoLipSyncingOnDlgSound( HOBJECT hSpeaker )
{
    bool bLipSync = (CanLipSync() && ( IsMultiplayerGame( ) || 
		!IsPlayer(m_hObject)));

	return bLipSync;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::PlayDialogSound
//
//	PURPOSE:	Play a dialog sound
//
// ----------------------------------------------------------------------- //

void CCharacter::PlayDialogSound(const char* pSound, CharacterSoundType eType)
{
	if (!pSound || !pSound[0]) return;
	if ((m_damage.IsDead() && eType != CST_DEATH)) return;
	if (eType == CST_AI_SOUND && (sm_cAISnds >= 2)) return;


	// Kill current sound...

	KillDlgSnd();


	// Check if character is supposed to lipsync.  Also, don't do lipsyncing on the 
	// player in singleplayer.
    bool bLipSync = DoLipSyncingOnDlgSound( m_hObject );


	//HACK: We need to determine the duration of the dialogue sound so we can effectively time out
	//		in case one or more of the clients is not responding. In order to determine the duration,
	//		we have to start playing the sound...
	LTFLOAT fDuration = 0.0;
	if (IsMultiplayerGame() && (bLipSync || DoDialogueSubtitles()) )
	{
	
		uint32 dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_TIME;
		LTFLOAT fRadius = 1.0f;
		uint8 nVolume = SMGR_DEFAULT_VOLUME;
		HLTSOUND hTmpSnd = g_pServerSoundMgr->PlaySoundFromObject(m_hObject, const_cast<char*>(pSound),
			fRadius, m_eSoundPriority, dwFlags, nVolume, 1.0f, -1.0f, SPEECH_SOUND_CLASS );

		g_pLTServer->SoundMgr()->GetSoundDuration(hTmpSnd, fDuration);

		g_pLTServer->SoundMgr()->KillSound(hTmpSnd);

		fDuration += kDialogueTimeFudgeFactor;
		m_tmrDialogue.Start(fDuration);

	}


	//special case for handling death sounds. We can't track these sounds because both the
	// character object and the characterFX object may be destroyed before the sound finishes
	// and we don't want wither of these events to kill the sound
	if ((bLipSync || DoDialogueSubtitles()) && (eType == CST_DEATH) )
	{
		HSTRING hStr = g_pLTServer->CreateString(const_cast<char*>(pSound));

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_SFX_MESSAGE);
		cMsg.Writeuint8(SFX_CHARACTER_ID);
		cMsg.WriteObject(m_hObject);
		cMsg.Writeuint8(CFX_DIALOGUE_MSG);
		cMsg.WriteHString(hStr);
		cMsg.Writefloat(m_fSoundRadius);
		m_nUniqueDialogueId++;
		cMsg.Writeuint8( m_nUniqueDialogueId );
		cMsg.Writeuint8( eType );
		g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

		g_pLTServer->FreeString(hStr);
	}
	else if (bLipSync)
	{
		// Tell the client to play the sound (lip synched)...
		// TO DO: Remove sending of sound path, send sound id instead...

		HSTRING hStr = g_pLTServer->CreateString(const_cast<char*>(pSound));

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_SFX_MESSAGE);
		cMsg.Writeuint8(SFX_CHARACTER_ID);
		cMsg.WriteObject(m_hObject);
		cMsg.Writeuint8(CFX_NODECONTROL_LIP_SYNC);
		cMsg.WriteHString(hStr);
		cMsg.Writefloat(m_fSoundRadius);
		m_nUniqueDialogueId++;
		cMsg.Writeuint8( m_nUniqueDialogueId );
		cMsg.Writeuint8( eType );
		g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

		g_pLTServer->FreeString(hStr);

		// Recored who we sent this message to.
		m_PlayerTrackerDialogue.Init( *this );
	}
	else if (DoDialogueSubtitles() || (eType == CST_DAMAGE))
	{
		HSTRING hStr = g_pLTServer->CreateString(const_cast<char*>(pSound));

		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_SFX_MESSAGE);
		cMsg.Writeuint8(SFX_CHARACTER_ID);
		cMsg.WriteObject(m_hObject);
		cMsg.Writeuint8(CFX_DIALOGUE_MSG);
		cMsg.WriteHString(hStr);
		cMsg.Writefloat(m_fSoundRadius);
		m_nUniqueDialogueId++;
		cMsg.Writeuint8( m_nUniqueDialogueId );
		cMsg.Writeuint8( eType );
		g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

		g_pLTServer->FreeString(hStr);

		// Recored who we sent this message to.
		m_PlayerTrackerDialogue.Init( *this );
	}
	else
	{
		// Just do a server side sound.

		uint32 dwFlags = PLAYSOUND_GETHANDLE | PLAYSOUND_TIME;

		// Play it inside the speaking player's head.
		if( IsPlayer( m_hObject ))
		{
			dwFlags |= PLAYSOUND_CLIENTLOCAL;
		}

		LTFLOAT fRadius = m_fSoundRadius;
		uint8 nVolume = SMGR_DEFAULT_VOLUME;

		// Only player dialogue sounds once...

		if (eType == CST_DIALOG)
		{
			dwFlags |= PLAYSOUND_ONCE;
		}

		m_hCurDlgSnd = g_pServerSoundMgr->PlaySoundFromObject(m_hObject, const_cast<char*>(pSound),
			fRadius, m_eSoundPriority, dwFlags, nVolume, 1.0f, -1.0f, SPEECH_SOUND_CLASS );
		if( !m_hCurDlgSnd )
		{
			return;
		}
	}

	m_eCurDlgSndType = eType;

	if( m_eCurDlgSndType == CST_AI_SOUND )
	{
		sm_cAISnds++;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::KillDlgSnd
//
//	PURPOSE:	Kill the current dialog sound
//
// ----------------------------------------------------------------------- //

void CCharacter::KillDlgSnd()
{
	if( m_eCurDlgSndType == CST_NONE )
		return;

	// Clean up if we had a server sound.
	if (m_hCurDlgSnd)
	{
		g_pLTServer->SoundMgr()->KillSound(m_hCurDlgSnd);
        m_hCurDlgSnd = LTNULL;

	}

	if ( m_eCurDlgSndType == CST_AI_SOUND )
	{
		sm_cAISnds--;
	}

	if (m_eCurDlgSndType == CST_DEATH)
		return;

	m_eCurDlgSndType = CST_NONE;

	// Make sure the client knows to stop lip syncing...

    bool bLipSync = DoLipSyncingOnDlgSound( m_hObject );

	if (bLipSync)
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_SFX_MESSAGE);
		cMsg.Writeuint8(SFX_CHARACTER_ID);
		cMsg.WriteObject(m_hObject);
		cMsg.Writeuint8(CFX_NODECONTROL_LIP_SYNC);
		cMsg.WriteHString(LTNULL);
		cMsg.Writefloat(0.0f);
		g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
	}
	else if (DoDialogueSubtitles())
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_SFX_MESSAGE);
		cMsg.Writeuint8(SFX_CHARACTER_ID);
		cMsg.WriteObject(m_hObject);
		cMsg.Writeuint8(CFX_DIALOGUE_MSG);
		cMsg.WriteHString(LTNULL);
		cMsg.Writefloat(0.0f);
		g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::PlayDialogue
//
//	PURPOSE:	Do dialogue sound/window
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacter::PlayDialogue( DWORD dwID )
{
    if (!dwID) return LTFALSE;

	char szSound[128] = "";
	g_pServerSoundMgr->GetSoundFilenameFromId("Dialogue", dwID, szSound, sizeof(szSound));
	PlayDialogSound(szSound);

	return TRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::PlayDialogue
//
//	PURPOSE:	Do dialogue sound/window
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacter::PlayDialogue( char *szDialogue )
{
    if (!szDialogue) return LTFALSE;

	DWORD dwID = 0;
	if ((szDialogue[0] >= '0') && (szDialogue[0] <= '9'))
	{
		char szSound[128] = "";
		// It's an ID
		dwID = atoi(szDialogue);
		g_pServerSoundMgr->GetSoundFilenameFromId("Dialogue", dwID, szSound, sizeof(szSound));
		PlayDialogSound(szSound);
	}
	else
	{
		// It's a sound
		PlayDialogSound(szDialogue);
	}
	
    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::StopDialogue
//
//	PURPOSE:	Stop the dialogue
//
// ----------------------------------------------------------------------- //

void CCharacter::StopDialogue(LTBOOL bCinematicDone)
{
	KillDialogueSound();
    m_bPlayingTextDialogue = LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::PlayDeathSound()
//
//	PURPOSE:	Play the death sound
//
// ----------------------------------------------------------------------- //

void CCharacter::PlayDeathSound()
{
	KillDlgSnd();
    PlayDialogSound(GetDeathSound(), CST_DEATH);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleDead()
//
//	PURPOSE:	Okay, death animation is done...
//
// ----------------------------------------------------------------------- //

void CCharacter::HandleDead(LTBOOL bRemoveObj)
{
	if( m_bMakeBody )
	{
		CreateBody();
	}

	if (bRemoveObj)
	{
		g_pLTServer->RemoveObject( m_hObject );
	}

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleVectorImpact()
//
//	PURPOSE:	Last chance to reject getting hit
//
// ----------------------------------------------------------------------- //

void CCharacter::HandleVectorImpact(IntersectInfo& iInfo, LTVector& vDir, LTVector& vFrom, ModelNode& eModelNode)
{
	// Let the character/body prop's attachments handle the impact...

//	if ( m_pAttachments )
//	{
//		m_pAttachments->HandleProjectileImpact(iInfo, vDir, vFrom, m_eModelSkeleton, eModelNode);
//	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::CreateBody()
//
//	PURPOSE:	Create the body prop
//
// ----------------------------------------------------------------------- //

void CCharacter::CreateBody()
{
    LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	HCLASS hClass = g_pLTServer->GetClass("Body");
	if (!hClass) return;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	// retreive modeldb/skins data from model instance.
	g_pModelLT->GetModelDBFilename(m_hObject, theStruct.m_Filename, MAX_CS_FILENAME_LEN);
    
    for( uint32 i = 0 ; i < MAX_MODEL_TEXTURES ; i++ )
    {
        g_pModelLT->GetSkinFilename(m_hObject, i,theStruct.m_SkinNames[i], MAX_CS_FILENAME_LEN+1);
    }
    
	
	g_pModelButeMgr->CopyRenderStyleFilenames(m_eModelId, &theStruct);


	LTSNPrintF(theStruct.m_Name, sizeof(theStruct.m_Name), "%s_body", GetObjectName(m_hObject));

	VEC_SET(theStruct.m_Pos, 0.0f, 0.1f, 0.0f);
	VEC_ADD(theStruct.m_Pos, theStruct.m_Pos, vPos);
	g_pLTServer->GetObjectRotation(m_hObject, &theStruct.m_Rotation);

	
	// Allocate an object...

	Body* pProp = (Body *)g_pLTServer->CreateObject(hClass, &theStruct);
	if (!pProp) 
		return;

	SetupBody(pProp);

}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetupBody()
//
//	PURPOSE:	Sets up the body prop
//
// ----------------------------------------------------------------------- //

void CCharacter::SetupBody(Body* pProp)
{
	_ASSERT(pProp);
	if ( !pProp ) return;

	BODYINITSTRUCT bi;
	bi.eBodyState = GetBodyState();
	bi.pCharacter = this;
	bi.bPermanentBody = m_bPermanentBody;
	bi.fBodyLifetime = m_fBodyLifetime;
	bi.bCanRevive = false;

	pProp->Init(bi);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetBodyState()
//
//	PURPOSE:	Gets the state of our body prop
//
// ----------------------------------------------------------------------- //

EnumAIStateType CCharacter::GetBodyState()
{
	if ( m_bWallStick )
	{
		return kState_BodyArrow;
	}
	else if ( m_damage.GetLastDamageType() == DT_EXPLODE )
	{
		return kState_BodyExplode;
	}
	else if ( m_damage.GetLastDamageType() == DT_BURN )
	{
		return kState_BodyAcid;
	}
	else if ( m_damage.GetLastDamageType() == DT_CRUSH )
	{
		return kState_BodyCrush;
	}
	else if ( m_damage.GetLastDamageType() == DT_POISON )
	{
		return kState_BodyPoison;
	}
	else if ( m_damage.GetLastDamageType() == DT_ASSS )
	{
		return kState_BodyNormal;
	}
	else if ( HasLastVolume() )
	{
		AIVolume* pVolume = GetLastVolume();
		if ( pVolume->GetVolumeType() == AIVolume::kVolumeType_Stairs )
		{
			return kState_BodyStairs;
		}
		else if ( pVolume->GetVolumeType() == AIVolume::kVolumeType_Ledge )
		{
			return kState_BodyLedge;
		}
		else if ( pVolume->GetVolumeType() == AIVolume::kVolumeType_Ladder )
		{
			return kState_BodyLadder;
		}
		else if ( IsLiquid(m_eContainerCode) )
		{
			return kState_BodyUnderwater;
		}
	}

	return kState_BodyNormal;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetPriorityBodyState()
//
//	PURPOSE:	Returns the body state with higher "priority"
//
// ----------------------------------------------------------------------- //

EnumAIStateType CCharacter::GetPriorityBodyState(EnumAIStateType bs1, EnumAIStateType bs2)
{
	switch ( bs1 )
	{
		case kState_BodyArrow:
			return bs1;
		case kState_BodyExplode:
			return bs1;
		case kState_BodyCrush:
			return bs1;
		case kState_BodyChair:
			return bs1;
		case kState_BodyUnderwater:
			return bs1;
		case kState_BodyLedge:
			return bs1;
		case kState_BodyLadder:
			return bs1;
		case kState_BodyStairs:
			return bs1;
		case kState_BodyLaser:
			return bs1;
		case kState_BodyPoison:
			return bs1;
		case kState_BodyAcid:
			return bs1;
	}

	switch ( bs2 )
	{
		case kState_BodyArrow:
			return bs2;
		case kState_BodyExplode:
			return bs2;
		case kState_BodyCrush:
			return bs2;
		case kState_BodyChair:
			return bs2;
		case kState_BodyUnderwater:
			return bs2;
		case kState_BodyLedge:
			return bs2;
		case kState_BodyLadder:
			return bs2;
		case kState_BodyStairs:
			return bs2;
		case kState_BodyLaser:
			return bs2;
		case kState_BodyPoison:
			return bs2;
		case kState_BodyAcid:
			return bs2;
	}

	_ASSERT(bs1 == bs2);

	return bs1;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetDims()
//
//	PURPOSE:	Set the dims for the character
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacter::SetDims(LTVector* pvDims, LTBOOL bSetLargest)
{
    if (!pvDims) return LTFALSE;

    LTBOOL bRet = LTTRUE;

	// Calculate what the dims should be based on our model size...

    LTVector vNewDims;
	VEC_MULSCALAR(vNewDims, *pvDims, 1.0f);


    LTVector vOldDims;
	g_pPhysicsLT->GetObjectDims(m_hObject, &vOldDims);


	// Only update dims if they have changed...

	if ((vNewDims.x > vOldDims.x - DIMS_EPSILON && vNewDims.x < vOldDims.x + DIMS_EPSILON) &&
		(vNewDims.y > vOldDims.y - DIMS_EPSILON && vNewDims.y < vOldDims.y + DIMS_EPSILON) &&
		(vNewDims.z > vOldDims.z - DIMS_EPSILON && vNewDims.z < vOldDims.z + DIMS_EPSILON))
	{
		return LTTRUE;  // Setting of dims didn't actually fail
	}


	// Try to set our new dims...

    if (g_pPhysicsLT->SetObjectDims(m_hObject, &vNewDims, SETDIMS_PUSHOBJECTS) == LT_ERROR)
	{
		if (bSetLargest)
		{
			g_pPhysicsLT->SetObjectDims(m_hObject, &vNewDims, 0);

		}

        bRet = LTFALSE; // Didn't set to new dims...
	}


	// [RP] 2/25/03 - MoveObjectToFloor() teleports the object to the floor.  We care about bounding box
	//				  collisions here so this block of code is exactly the same as MoveObjectToFloor() with 
	//				  the exception of calling MoveObject() rather than SetObjectPos().

	LTVector vPos, vDims;
	g_pLTServer->GetObjectPos( m_hObject, &vPos );
	g_pPhysicsLT->GetObjectDims( m_hObject, &vDims );

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_From	= vPos;
	IQuery.m_To		= vPos + LTVector(0.0f, -10000.0f, 0.0f);
	IQuery.m_Flags	= IGNORE_NONSOLID | INTERSECT_OBJECTS | INTERSECT_HPOLY;
	
    if( g_pLTServer->IntersectSegment( &IQuery, &IInfo ))
	{
        float fDist = vPos.y - IInfo.m_Point.y;
		if( fDist > vDims.y )
		{
			vPos.y -= (fDist - (vDims.y + 0.1f));
			g_pLTServer->MoveObject( m_hObject, &vPos );
		}
	}

	// This forces the client to move to the server's position because it's teleporting.
	
	g_pLTServer->GetObjectPos( m_hObject, &vPos );
	g_pLTServer->SetObjectPos( m_hObject, &vPos );


	// Update the dims of our hit box...

	if (m_hHitBox)
	{
		CCharacterHitBox *pHitBox = dynamic_cast<CCharacterHitBox*>(g_pLTServer->HandleToObject( m_hHitBox ));
		if( pHitBox )
		{
			pHitBox->EnlargeAndSetDims( vNewDims );
		}
	}

	return bRet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetRadius()
//
//	PURPOSE:	Get radius
//
// ----------------------------------------------------------------------- //

LTFLOAT CCharacter::GetRadius()
{
	LTVector vDims;
	g_pPhysicsLT->GetObjectDims(m_hObject, &vDims);
    return Max<LTFLOAT>(vDims.x, vDims.z);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::CreateHitBox()
//
//	PURPOSE:	Create our hit box
//
// ----------------------------------------------------------------------- //

void CCharacter::CreateHitBox()
{
	if (m_hHitBox) return;

    LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	HCLASS hClass = g_pLTServer->GetClass("CCharacterHitBox");
	if (!hClass) return;

	ObjectCreateStruct theStruct;
	INIT_OBJECTCREATESTRUCT(theStruct);

	theStruct.m_Pos = vPos;
	g_pLTServer->GetObjectRotation(m_hObject, &theStruct.m_Rotation);

	// Allocate an object...

	CCharacterHitBox* pHitBox = (CCharacterHitBox *)g_pLTServer->CreateObject(hClass, &theStruct);
	if (!pHitBox) return;

	m_hHitBox = pHitBox->m_hObject;

	pHitBox->Init(m_hObject, this);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateHitBox()
//
//	PURPOSE:	Update our hit box position
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateHitBox()
{
	if (!m_hHitBox) return;

	CCharacterHitBox* pHitBox = (CCharacterHitBox*) g_pLTServer->HandleToObject(m_hHitBox);
	if (pHitBox)
	{
		pHitBox->Update();
	}
}


void CCharacter::UpdateClientHitBox()
{
	if (m_hHitBox == INVALID_HOBJECT)
		return;

	LTVector vHitDims, vHitOffset(0.0f, 0.0f, 0.0f);
	g_pPhysicsLT->GetObjectDims(m_hHitBox, &vHitDims);

	bool bCanBeSearched = false;

	CCharacterHitBox* pHitBox = dynamic_cast<CCharacterHitBox*>(g_pLTServer->HandleToObject(m_hHitBox));
	if (pHitBox)
	{
		vHitOffset = pHitBox->GetOffset();
		bCanBeSearched = pHitBox->CanBeSearched();
	}

	// Send the hitbox message
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_SFX_MESSAGE);
	cMsg.Writeuint8(SFX_CHARACTER_ID);
	cMsg.WriteObject(m_hObject);
	cMsg.Writeuint8(CFX_HITBOX_MSG);
	cMsg.WriteCompLTVector( vHitDims );
	cMsg.WriteCompLTVector( vHitOffset );
	cMsg.Writebool( bCanBeSearched );
	g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);

	// Update the SFX
	CreateSpecialFX();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SpawnItem()
//
//	PURPOSE:	Spawn the specified item
//
// ----------------------------------------------------------------------- //

void CCharacter::SpawnItem(char* pItem, LTVector & vPos, LTRotation & rRot)
{
	if (!pItem) return;

	LPBASECLASS pObj = SpawnObject(pItem, vPos, rRot);

	if (pObj && pObj->m_hObject)
	{
        LTVector vAccel;
		VEC_SET(vAccel, GetRandom(0.0f, 300.0f), GetRandom(100.0f, 200.0f), GetRandom(0.0f, 300.0f));
		g_pPhysicsLT->SetAcceleration(pObj->m_hObject, &vAccel);

        LTVector vVel;
		vVel.Init(GetRandom(0.0f, 100.0f), GetRandom(200.0f, 400.0f), GetRandom(0.0f, 100.0f));
		g_pPhysicsLT->SetVelocity(pObj->m_hObject, &vVel);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::StartDeath()
//
//	PURPOSE:	Start dying
//
// ----------------------------------------------------------------------- //

void CCharacter::StartDeath()
{
	if (m_bStartedDeath) return;

    m_bStartedDeath = LTTRUE;

	PlayDeathSound();

	// Spawn any special item we were instructed to

	if (m_hstrSpawnItem)
	{
		const char* pItem = g_pLTServer->GetStringData(m_hstrSpawnItem);
		if (pItem)
		{
			// Add gravity to the item...

			char buf[300];
			sprintf(buf, "%s Gravity 1", pItem);

            LTVector vPos;
			g_pLTServer->GetObjectPos(m_hObject, &vPos);

			SpawnItem(buf, vPos, LTRotation());
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateContainerCode()
//
//	PURPOSE:	Update our container code
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateContainerCode()
{
    LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	vPos += GetHeadOffset();

	uint32 nIgnoreFlags = (CC_DYNAMIC_OCCLUDER_VOLUME_FLAG | CC_PLAYER_IGNORE_FLAGS);
    m_eContainerCode = ::GetContainerCode(vPos, nIgnoreFlags);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetHeadOffset()
//
//	PURPOSE:	Update the offset from our position to our head
//
// ----------------------------------------------------------------------- //

LTVector CCharacter::GetHeadOffset()
{
    LTVector vOffset;
	VEC_INIT(vOffset);

    LTVector vDims;
	g_pPhysicsLT->GetObjectDims(m_hObject, &vDims);

	// Just make the default offset a bit above the waist...

	vOffset.y = vDims.y * 0.75f;

	return vOffset;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetLastFireInfo()
//
//	PURPOSE:	Set the last fire info
//
// ----------------------------------------------------------------------- //

void CCharacter::SetLastFireInfo(CharFireInfo* pInfo)
{
	if (!pInfo) return;

	m_LastFireInfo.hObject		= pInfo->hObject;
	m_LastFireInfo.vFiredPos	= pInfo->vFiredPos;
	m_LastFireInfo.vImpactPos	= pInfo->vImpactPos;
	m_LastFireInfo.nWeaponId	= pInfo->nWeaponId;
	m_LastFireInfo.nAmmoId		= pInfo->nAmmoId;
	m_LastFireInfo.fTime		= pInfo->fTime;
	m_LastFireInfo.bSilenced	= pInfo->bSilenced;
	m_LastFireInfo.eSurface		= pInfo->eSurface;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetLastFireInfo()
//
//	PURPOSE:	Get the last fire info
//
// ----------------------------------------------------------------------- //

void CCharacter::GetLastFireInfo(CharFireInfo & info)
{
	info.hObject	= m_LastFireInfo.hObject;
	info.vFiredPos  = m_LastFireInfo.vFiredPos;
	info.vImpactPos = m_LastFireInfo.vImpactPos;
	info.nWeaponId	= m_LastFireInfo.nWeaponId;
	info.nAmmoId	= m_LastFireInfo.nAmmoId;
	info.fTime		= m_LastFireInfo.fTime;
	info.bSilenced	= m_LastFireInfo.bSilenced;
	info.eSurface	= m_LastFireInfo.eSurface;
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CCharacter::GetRelationData()
//              
//	PURPOSE:	Accessor functions for the Relation System components
//              
//----------------------------------------------------------------------------
const RelationData&	CCharacter::GetRelationData() const
{
	return m_pRelationMgr->GetData();
}
CObjectRelationMgr* CCharacter::GetRelationMgr()
{
	return m_pRelationMgr;
}
const RelationSet& CCharacter::GetRelationSet() const
{
	return m_pRelationMgr->GetRelationUser()->GetRelations();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CCharacter::Save(ILTMessage_Write *pMsg)
{
	if (!pMsg) return;

    m_pcs->Write(pMsg);

	m_LastFireInfo.Save(pMsg);

	SAVE_HOBJECT(m_hHitBox);

	SAVE_BOOL(m_bMoveToFloor);
	SAVE_BYTE(m_eDeathType);
	SAVE_BOOL(m_bStartedDeath);
	SAVE_BOOL(m_bRolling);
	SAVE_BOOL(m_bPivoting);
	SAVE_BOOL(m_bAllowRun);
	SAVE_BOOL(m_bAllowMovement);
	SAVE_BYTE(m_eStandingOnSurface);
	SAVE_BOOL(m_bOnGround);
	SAVE_BYTE(m_eContainerCode);
	SAVE_BYTE(m_eLastContainerCode);
	SAVE_BOOL(m_bBodyInLiquid);
	SAVE_BOOL(m_bBodyWasInLiquid);
	SAVE_BOOL(m_bBodyOnLadder);
	SAVE_VECTOR(m_vOldCharacterColor);
	SAVE_FLOAT(m_fOldCharacterAlpha);
	SAVE_BOOL(m_bCharacterHadShadow);
	SAVE_BYTE(m_eModelNodeLastHit);
	SAVE_BOOL(m_bLeftFoot);
	SAVE_BYTE(m_eModelId);
	SAVE_BYTE(m_eModelSkeleton);
	SAVE_HSTRING(m_hstrSpawnItem);
	SAVE_FLOAT(m_fDefaultHitPts);
	SAVE_FLOAT(m_fDefaultArmor);
	SAVE_FLOAT(m_fDefaultEnergy);
	SAVE_FLOAT(m_fSoundRadius);
	SAVE_DWORD(m_eMusicMoodMin);
	SAVE_DWORD(m_eMusicMoodMax);
	SAVE_FLOAT(m_fBaseMoveAccel);
	SAVE_FLOAT(m_fLadderVel);
	SAVE_FLOAT(m_fSwimVel);
	SAVE_FLOAT(m_fRunVel);
	SAVE_FLOAT(m_fWalkVel);
	SAVE_FLOAT(m_fJumpVel);
	SAVE_FLOAT(m_fSuperJumpVel);
	SAVE_FLOAT(m_fFallVel);
	SAVE_BOOL(m_bUsingHitDetection);
	m_pRelationMgr->Save(pMsg);

	SAVE_BYTE(m_ccCrosshair);
	SAVE_DWORD(m_dwFlags);
	SAVE_TIME(m_fLastPainTime);
	SAVE_FLOAT(m_fLastPainVolume);

	SAVE_COBJECT(m_pLastVolume);
	SAVE_COBJECT(m_pCurrentVolume);
	SAVE_COBJECT(m_pLastInformationVolume);
	SAVE_COBJECT(m_pCurrentInformationVolume);

	SAVE_VECTOR(m_vLastVolumePos);
	SAVE_VECTOR(m_vLastInformationVolumePos);
	SAVE_BYTE(m_byFXFlags);
	SAVE_BOOL(m_bShortRecoiling);
	SAVE_HSTRING(m_hstrHeadExtension);
	
   
	SAVE_DWORD(m_cSpears);
	for ( uint32 iSpear = 0 ; iSpear < m_cSpears ; iSpear++ )
	{
		SAVE_HOBJECT(m_aSpears[iSpear].hObject);
		SAVE_DWORD(m_aSpears[iSpear].eModelNode);
		SAVE_ROTATION(m_aSpears[iSpear].rRot);
	}

	SAVE_BOOL(m_bStuckInFront);
	SAVE_BOOL(m_bWallStick);
	SAVE_BOOL(m_bArmored);
	SAVE_INT(m_cActive);
	SAVE_QWORD(m_nDamageFlags);
	SAVE_FLOAT(m_fPitch);
	SAVE_FLOAT(m_fLastPitch);

	// Inventory
	SAVE_INT(m_lstInventory.size());

	GEN_INVENTORY_LIST::iterator iter;
	for(iter=m_lstInventory.begin();iter!=m_lstInventory.end();iter++)
	{
		SAVE_INT(iter->nItemID);
		SAVE_INT(iter->nCount);
	}
	
	m_Keys.Save(pMsg);

	SAVE_bool(m_bMakeBody);
	SAVE_bool(m_bPermanentBody);
	SAVE_BYTE(m_iPermissionSet);

	SAVE_DWORD( m_eTeleportTriggerState );
	SAVE_VECTOR( m_vTeleportPos );
	SAVE_HOBJECT( m_hTeleportPoint );
	
	SAVE_BYTE( m_eDeathDamageType );

	SAVE_DWORD( m_eEnemyVisibleStimID );
	SAVE_DWORD( m_eUndeterminedVisibleStimID );

	SAVE_bool( m_bTracking );

	SAVE_bool( m_bRadarVisible );

	SAVE_BYTE( m_nCarrying);
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CCharacter::Load(ILTMessage_Read *pMsg)
{
	if (!pMsg) return;

    m_pcs->Read(pMsg);

	m_LastFireInfo.Load(pMsg);

	LOAD_HOBJECT(m_hHitBox);
	LOAD_BOOL(m_bMoveToFloor);
	LOAD_BYTE_CAST(m_eDeathType, CharacterDeath);
	LOAD_BOOL(m_bStartedDeath);
	LOAD_BOOL(m_bRolling);
	LOAD_BOOL(m_bPivoting);
	LOAD_BOOL(m_bAllowRun);
	LOAD_BOOL(m_bAllowMovement);
	LOAD_BYTE_CAST(m_eStandingOnSurface, SurfaceType);
	LOAD_BOOL(m_bOnGround);
	LOAD_BYTE_CAST(m_eContainerCode, ContainerCode);
	LOAD_BYTE_CAST(m_eLastContainerCode, ContainerCode);
	LOAD_BOOL(m_bBodyInLiquid);
	LOAD_BOOL(m_bBodyWasInLiquid);
	LOAD_BOOL(m_bBodyOnLadder);
	LOAD_VECTOR(m_vOldCharacterColor);
	LOAD_FLOAT(m_fOldCharacterAlpha);
	LOAD_BOOL(m_bCharacterHadShadow);
	LOAD_BYTE_CAST(m_eModelNodeLastHit, ModelNode);
	LOAD_BOOL(m_bLeftFoot);
	LOAD_BYTE_CAST(m_eModelId, ModelId);
	LOAD_BYTE_CAST(m_eModelSkeleton, ModelSkeleton);
	LOAD_HSTRING(m_hstrSpawnItem);
	LOAD_FLOAT(m_fDefaultHitPts);
	LOAD_FLOAT(m_fDefaultArmor);
	LOAD_FLOAT(m_fDefaultEnergy);
	LOAD_FLOAT(m_fSoundRadius);
	LOAD_DWORD_CAST(m_eMusicMoodMin, CMusicMgr::Mood);
	LOAD_DWORD_CAST(m_eMusicMoodMax, CMusicMgr::Mood);
	LOAD_FLOAT(m_fBaseMoveAccel);
	LOAD_FLOAT(m_fLadderVel);
	LOAD_FLOAT(m_fSwimVel);
	LOAD_FLOAT(m_fRunVel);
	LOAD_FLOAT(m_fWalkVel);
	LOAD_FLOAT(m_fJumpVel);
	LOAD_FLOAT(m_fSuperJumpVel);
	LOAD_FLOAT(m_fFallVel);
    LOAD_BOOL(m_bUsingHitDetection);
	m_pRelationMgr->Load(pMsg);
	LOAD_BYTE_CAST(m_ccCrosshair, CharacterClass);
	LOAD_DWORD(m_dwFlags);
	LOAD_TIME(m_fLastPainTime);
	LOAD_FLOAT(m_fLastPainVolume);

	LOAD_COBJECT(m_pLastVolume, AIVolume);
	LOAD_COBJECT(m_pCurrentVolume, AIVolume);
	LOAD_COBJECT(m_pLastInformationVolume, AIInformationVolume);
	LOAD_COBJECT(m_pCurrentInformationVolume, AIInformationVolume);

	// If we're loading from a transition, then our positional information is invalid.
	if( g_pGameServerShell->GetLGFlags( ) == LOAD_TRANSITION )
	{
		m_pLastVolume			= LTNULL;
		m_pCurrentVolume		= LTNULL;
		m_pLastInformationVolume = LTNULL;
		m_pCurrentInformationVolume = LTNULL;
	}

	LOAD_VECTOR(m_vLastVolumePos);
	LOAD_VECTOR(m_vLastInformationVolumePos);
	LOAD_BYTE(m_byFXFlags);
	LOAD_BOOL(m_bShortRecoiling);
	LOAD_HSTRING(m_hstrHeadExtension);

	LOAD_DWORD(m_cSpears);
	for ( uint32 iSpear = 0 ; iSpear < m_cSpears ; iSpear++ )
	{
		LOAD_HOBJECT(m_aSpears[iSpear].hObject);
		LOAD_DWORD_CAST(m_aSpears[iSpear].eModelNode, ModelNode);
		LOAD_ROTATION(m_aSpears[iSpear].rRot);
	}

	LOAD_BOOL(m_bStuckInFront);
	LOAD_BOOL(m_bWallStick);
	LOAD_BOOL(m_bArmored);
	LOAD_INT(m_cActive);
	LOAD_QWORD(m_nDamageFlags);
	LOAD_FLOAT(m_fPitch);
	LOAD_FLOAT(m_fLastPitch);

	// Inventory
	m_lstInventory.clear();
	int nNumInventoryItems;
	LOAD_INT(nNumInventoryItems);

	GEN_INVENTORY_ITEM item;
	int nTemp;
	for(int i=0;i<nNumInventoryItems;i++)
	{
		LOAD_INT(nTemp);
		item.nItemID = (uint8)nTemp;
		LOAD_INT(nTemp);
		item.nCount = (uint8)nTemp;
	}

	m_Keys.Load( pMsg );

	LOAD_bool(m_bMakeBody);
	LOAD_bool(m_bPermanentBody);
	LOAD_BYTE(m_iPermissionSet);

	LOAD_DWORD_CAST(m_eTeleportTriggerState, TeleportTriggerStates);
	LOAD_VECTOR(m_vTeleportPos);
	LOAD_HOBJECT(m_hTeleportPoint);

	LOAD_BYTE_CAST( m_eDeathDamageType, DamageType );

	LOAD_DWORD_CAST( m_eEnemyVisibleStimID, EnumAIStimulusID );
	LOAD_DWORD_CAST( m_eUndeterminedVisibleStimID, EnumAIStimulusID );

	if( g_pVersionMgr->GetCurrentSaveVersion( ) < CVersionMgr::kSaveVersion__1_2 )
	{
		// The player used to call SetTracking in SetClientLoaded, now it
		// calls SetRadarVisible.  For the player, consider it not tracked,
		// but load the radarvisible flag.
		if( dynamic_cast< CPlayerObj* >( this ))
		{
			m_bTracking = false;
			LOAD_bool( m_bRadarVisible );

			m_pcs->bTracking = false;
			m_pcs->bRadarVisible = m_bRadarVisible;
		}
		else
		{
			LOAD_bool( m_bTracking );
		}
	}
	else
	{
		LOAD_bool( m_bTracking );
		LOAD_bool( m_bRadarVisible );
	}


	if( g_pVersionMgr->GetCurrentSaveVersion( ) < CVersionMgr::kSaveVersion__1_3 )
	{
		if( !IsPlayer(m_hObject) )
			m_nCarrying = CFX_CARRY_NONE;
	}
	else
	{
		LOAD_BYTE( m_nCarrying );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::DestroyAttachments
//
//	PURPOSE:	Destroys our attachments aggregate
//
// ----------------------------------------------------------------------- //

void CCharacter::DestroyAttachments()
{
	if ( m_pAttachments )
	{
		RemoveAggregate(m_pAttachments);
		CAttachments::Destroy(m_pAttachments);
        m_pAttachments = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HideAttachments
//
//	PURPOSE:	Hide/Show our attachments.
//
// ----------------------------------------------------------------------- //

void CCharacter::HideAttachments(LTBOOL bHide)
{
	if ( m_pAttachments )
	{
		m_pAttachments->HideAttachments(bHide);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::TransferAttachments
//
//	PURPOSE:	Transfer our attachments aggregate (i.e., clear our
//				attachments aggregate, but don't remove it)
//
// ----------------------------------------------------------------------- //

CAttachments* CCharacter::TransferAttachments( bool bRemove )
{
	if (m_pAttachments)
	{
		m_pAttachments->HandleDeath();
	}

	CAttachments* pAtt = LTNULL;
	if (m_pAttachments && bRemove)
	{
		pAtt = m_pAttachments;
		RemoveAggregate(m_pAttachments);
        m_pAttachments = LTNULL;
	}
	else if( m_pAttachments )
	{
		// If we are not removing our own attachments then we must create a new one.

		pAtt = CAttachments::Create( m_pAttachments->GetType() );
	}
	else
	{
		ASSERT( !"Transfering NULL Attachments" );
	}

	return pAtt;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::TransferWeapons
//
//	PURPOSE:	Transfers weapons to body.
//
// ----------------------------------------------------------------------- //

void CCharacter::TransferWeapons(Body *pBody, bool bRemove )
{
	if( !m_pAttachments )
		return;

	CWeapon* apWeapons[kMaxWeapons];
	CAttachmentPosition* apAttachmentPositions[kMaxWeapons];
	uint32 cWeapons = m_pAttachments->EnumerateWeapons(apWeapons, apAttachmentPositions, kMaxWeapons);

	while ( cWeapons )
	{
		char szSpawn[1024];
		cWeapons--;

		int nId = apWeapons[cWeapons]->GetId();
		if (!g_pWeaponMgr->IsPlayerWeapon(nId))
			continue;

		HOBJECT hWpnModel = apWeapons[cWeapons]->GetModelObject();
		uint32 dwAni = g_pLTServer->GetModelAnimation(hWpnModel);

		apAttachmentPositions[cWeapons]->GetAttachment()->CreateSpawnString(szSpawn);

        BaseClass* pObj = SpawnObject(szSpawn, LTVector(-10000,-10000,-10000), LTRotation());
		if ( pObj && pObj->m_hObject )
		{
			if (dwAni != INVALID_ANI)
			{
				LTVector vDims;
				g_pCommonLT->GetModelAnimUserDims(pObj->m_hObject, &vDims, dwAni);
				g_pPhysicsLT->SetObjectDims(pObj->m_hObject, &vDims, 0);

				g_pLTServer->SetModelAnimation(pObj->m_hObject, dwAni);
			}
			pBody->AddWeapon(pObj->m_hObject, (char *)apAttachmentPositions[cWeapons]->GetName() );

			if( bRemove )
			{
				apAttachmentPositions[cWeapons]->RemoveWeapon();
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::TransferSpears
//
//	PURPOSE:	Transfer our attachments aggregate (i.e., clear our
//				attachments aggregate, but don't remove it)
//
// ----------------------------------------------------------------------- //

void CCharacter::TransferSpears(Body* pBody)
{
	for ( uint32 iSpear = 0 ; iSpear < m_cSpears ; iSpear++ )
	{
		HATTACHMENT hAttachment;
		HOBJECT hSpear = m_aSpears[iSpear].hObject;

		if ( hSpear )
		{
			if ( LT_OK == g_pLTServer->FindAttachment(m_hObject, hSpear, &hAttachment) )
			{
				if ( LT_OK == g_pLTServer->RemoveAttachment(hAttachment) )
				{
					// Attach it to the body prop and break our link

					pBody->AddSpear(hSpear, m_aSpears[iSpear].rRot, m_aSpears[iSpear].eModelNode);

					m_aSpears[iSpear].hObject= LTNULL;

					continue;
				}
			}

			// If any of this failed, just remove the spear

			g_pLTServer->RemoveObject(hSpear);
		}
	}

	// No more spears.
	m_cSpears = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetLastVolume
//
//	PURPOSE:	Gets the last attachment we were standing in
//
// ----------------------------------------------------------------------- //

AIVolume* CCharacter::GetLastVolume()
{
	if ( g_pAIVolumeMgr->IsInitialized() && m_pLastVolume != LTNULL )
	{
		return m_pLastVolume;
	}
	else
	{
        return LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetCurrentVolume
//
//	PURPOSE:	Gets the volume we are currently standing in
//
// ----------------------------------------------------------------------- //

AIVolume* CCharacter::GetCurrentVolume()
{
	if ( g_pAIVolumeMgr->IsInitialized() && m_pCurrentVolume != LTNULL )
	{
		return m_pCurrentVolume;
	}
	else
	{
        return LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetLastInformationVolume
//
//	PURPOSE:	Gets the last attachment we were standing in
//
// ----------------------------------------------------------------------- //

AIInformationVolume* CCharacter::GetLastInformationVolume()
{
	if ( g_pAIInformationVolumeMgr->IsInitialized() && m_pLastInformationVolume != LTNULL )
	{
		return m_pLastInformationVolume;
	}
	else
	{
        return LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetCurrentInformationVolume
//
//	PURPOSE:	Gets the volume we are currently standing in
//
// ----------------------------------------------------------------------- //

AIInformationVolume* CCharacter::GetCurrentInformationVolume()
{
	if ( g_pAIInformationVolumeMgr->IsInitialized() && m_pCurrentInformationVolume != LTNULL )
	{
		return (AIInformationVolume*)m_pCurrentInformationVolume;
	}
	else
	{
        return LTNULL;
	}
}
// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateCurrentInformationVolume
//
//	PURPOSE:	Sets the volume we are currently standing in
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateCurrentInformationVolume(bool bForce /* = false */)
{
	if ( g_pAIInformationVolumeMgr->IsInitialized() && (bForce || IsVisible()) )
	{
        LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);
		AIInformationVolume* pInformationVolume = g_pAIInformationVolumeMgr->FindContainingVolume(m_hObject, vPos, eAxisAll, GetInfoVerticalThreshold(), m_pLastInformationVolume);

		if ( pInformationVolume )
		{
			if ( m_pLastInformationVolume && m_pLastInformationVolume != pInformationVolume )
			{
				HandleVolumeExit(m_pLastInformationVolume);
			}

			if ( m_pLastInformationVolume != pInformationVolume )
			{
				HandleVolumeEnter(pInformationVolume);
			}

			m_pCurrentInformationVolume = pInformationVolume;
			m_pLastInformationVolume = m_pCurrentInformationVolume;
			m_vLastInformationVolumePos = vPos;

#ifndef _FINAL
			if ( IsPlayer(m_hObject) && g_VolumeDebugTrack.GetFloat(0.0f) == 1.0f )
			{
				g_pLTServer->CPrint("Player in InformationVolume \"%s\"", pInformationVolume->GetName());
			}
#endif
		}
		else
		{
			m_pCurrentInformationVolume = LTNULL;
#ifndef _FINAL
			if ( IsPlayer(m_hObject) && g_VolumeDebugTrack.GetFloat(0.0f) == 1.0f )
			{
				g_pLTServer->CPrint("Player not in an InformationVolume");
			}
#endif
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HitFromFront
//
//	PURPOSE:	Tells whether the vector is coming at us from the front
//				or back assuming it passes through us.
//
// ----------------------------------------------------------------------- //

LTBOOL CCharacter::HitFromFront(const LTVector& vDir)
{
    LTRotation rRot;
	g_pLTServer->GetObjectRotation(m_hObject, &rRot);
	return vDir.Dot(rRot.Forward()) < 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::ComputeDamageModifier
//
//	PURPOSE:	Adjust the amount of damage based on the node hit...
//
// ----------------------------------------------------------------------- //

LTFLOAT CCharacter::ComputeDamageModifier(ModelNode eModelNode)
{
	LTFLOAT fModifier = g_pModelButeMgr->GetSkeletonNodeDamageFactor(m_eModelSkeleton, eModelNode);
	return fModifier;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::AddSpear
//
//	PURPOSE:	Stick a spear into us
//
// ----------------------------------------------------------------------- //

bool CCharacter::AddSpear(HOBJECT hSpear, ModelNode eModelNode, const LTRotation& rRot, bool bCanWallStick )
{
	if ( m_cSpears < kMaxSpears )
	{
		// Check the node to see if it can attach a spear
		// If we are dead the spear should always attach.
		
		if( !m_damage.IsDead() )
		{
			if( !g_pModelButeMgr->GetSkeletonNodeAttachSpears( m_eModelSkeleton, eModelNode ))
			{
				// We can't attach a spear to the node so remove the spear and fail...

				g_pLTServer->RemoveObject(hSpear);
				return false;
			}
		}

		char* szNode = (char *)g_pModelButeMgr->GetSkeletonNodeName(m_eModelSkeleton, eModelNode);

		// Get the node transform because we need to make rotation relative

		HMODELNODE hNode;
		if ( szNode && LT_OK == g_pModelLT->GetNode(m_hObject, szNode, hNode) )
		{
			LTransform transform;
			if ( LT_OK == g_pModelLT->GetNodeTransform(m_hObject, hNode, transform, LTTRUE) )
			{
				LTRotation rRotNode = transform.m_Rot;
				LTRotation rAttachment = ~rRotNode*rRot;

				LTVector vSpearDims;

				// Get the dims of the spear from the model animation since the dims may
				// have been adjusted for the pickup box and we want to use the small
				// dims for this calculation...

				uint32 dwAni = g_pLTServer->GetModelAnimation(hSpear);
			 	if (dwAni != INVALID_ANI)
				{
					g_pCommonLT->GetModelAnimUserDims(hSpear, &vSpearDims, dwAni);
				}
				else // use normal dims...
				{
					g_pPhysicsLT->GetObjectDims( hSpear, &vSpearDims );
				}
						
				LTVector vAttachment = rAttachment.Forward() * -(vSpearDims.z * 2.0f);

				HATTACHMENT hAttachment;
				if ( LT_OK == g_pLTServer->CreateAttachment(m_hObject, hSpear, szNode, &vAttachment, &rAttachment, &hAttachment) )
				{
					m_aSpears[m_cSpears].hObject = hSpear;
					m_aSpears[m_cSpears].eModelNode = eModelNode;
					m_aSpears[m_cSpears].rRot = rRot;

					m_cSpears++;

					//if (!m_damage.IsDead())
					//	g_pLTServer->CPrint("spear stuck");

					//flag the spear to be hidden on low violence clients
					g_pCommonLT->SetObjectFlags(hSpear, OFT_User, USRFLG_ATTACH_HIDEGORE, USRFLG_ATTACH_HIDEGORE);

					//tell everybody we've got a new spear,so they can decide whether to draw it
					CAutoMessage cMsg;
					cMsg.Writeuint8(MID_SFX_MESSAGE);
					cMsg.Writeuint8(SFX_CHARACTER_ID);
					cMsg.WriteObject(m_hObject);
					cMsg.Writeuint8(CFX_UPDATE_ATTACHMENTS);
					g_pLTServer->SendToClient(cMsg.Read(), LTNULL, MESSAGE_GUARANTEED);


					// Try to stick us to a wall if we are dead and a node that will stick was hit...
					
					if ( bCanWallStick && m_damage.IsDead() && (NODEFLAG_WALLSTICK & g_pModelButeMgr->GetSkeletonNodeFlags(m_eModelSkeleton, eModelNode)) )
					{
						// TODO: this might be a bit dangerous... can't really think of a better way to do this right now.
						// Only stick if projectile forward is within threshhold of character forward

						LTRotation rCharacterRot;
						LTVector vSpearForward, vCharacterForward;

						g_pLTServer->GetObjectRotation(m_hObject, &rCharacterRot);
						vCharacterForward = rCharacterRot.Forward();
						vSpearForward = rRot.Forward();

						// TODO: bute -normal/fdw dp threshhold
						
						// Handle sticking in back or in front...
						float fForwardThreshhold = -vSpearForward.Dot(vCharacterForward);
						float fBackwardThreshhold = vSpearForward.Dot(vCharacterForward);

						//g_pLTServer->CPrint("Backward Threshhold = %.4f", fBackwardThreshhold);
						//g_pLTServer->CPrint("Forward  Threshhold = %.4f", fForwardThreshhold);

						bool bTryWallStick = false;
						if ( fForwardThreshhold > s_BodyStickAngle.GetFloat() )
						{
							bTryWallStick = true;
							m_bStuckInFront = true;
						}
						else if ( fBackwardThreshhold > s_BodyStickAngle.GetFloat() )
						{
							bTryWallStick = true;
							m_bStuckInFront = false;
						}
							
						if ( bTryWallStick )
						{
							m_bWallStick = ShouldWallStick();
						}
					}

					return true;
				}
			}
		}
	}

	// Unless we actually stuck the spear into us, we'll fall through into here.

	g_pLTServer->RemoveObject(hSpear);
	return false;
}

void CCharacter::SetBlinking(LTBOOL bBlinking)
{
    g_pModelLT->SetWeightSet(m_hObject, m_BlinkAnimTracker, bBlinking ? m_hBlinkWeightset : m_hNullWeightset);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCharacter::AddToObjectList
//
//  PURPOSE:	Add any objects we create to the list...
//
// ----------------------------------------------------------------------- //

void CCharacter::AddToObjectList( ObjectList *pObjList, eObjListControl eControl )
{
	if( !pObjList ) return;

	// Send to base first to add our object...

	GameBase::AddToObjectList( pObjList, eControl );

	// Add our hit box...

	AddObjectToList( pObjList, m_hHitBox, eControl );

	// Add our attachments...

	if( m_pAttachments )
	{
		m_pAttachments->AddToObjectList( pObjList, eControl );
	}

	// Add any spears attached to us...

	for ( uint32 iSpear = 0 ; iSpear < m_cSpears ; iSpear++ )
	{
		HOBJECT hSpear = m_aSpears[iSpear].hObject;

		if ( hSpear )
		{
			AddObjectToList( pObjList, hSpear, eControl );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::FilterDamage()
//
//	PURPOSE:	Change the damage struct before damage is dealt
//
// ----------------------------------------------------------------------- //

bool CCharacter::FilterDamage( DamageStruct *pDamageStruct )
{
	//
	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateKeys
//
//	PURPOSE:	Add to/Remove from/Clear the key list
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateKeys(uint8 nType, uint16 nId)
{
	switch (nType)
	{
		case ITEM_ADD_ID:
		{
			if (g_pKeyMgr->IsValidKey(nId))
			{
				m_Keys.Add(nId);
				
				// If we are a player check to see if obtaining this key will enable any objects with an activate type handler...

				if( IsPlayer( m_hObject ))
				{
					// Run through all the objects in the key control list...

					CKeyMgr::KeyControlMap::const_iterator iter;
					for( iter = g_pKeyMgr->GetKeyControlMap().begin(); iter != g_pKeyMgr->GetKeyControlMap().end(); ++iter )
					{
						HOBJECT hObj = (*iter).first;

						// Currently we only care about ActiveWorldModels such as doors...

						if( IsKindOf( hObj, "ActiveWorldModel" ))
						{
							if( g_pKeyMgr->CanCharacterControlObject( m_hObject, hObj ))
							{
								// Ok, the object is unlocked for us.  Let our client know we can activate the object...
								
								CPlayerObj *pPlayer = dynamic_cast<CPlayerObj*>(g_pLTServer->HandleToObject( m_hObject ));
								ActiveWorldModel *pWorldModel = dynamic_cast<ActiveWorldModel*>(g_pLTServer->HandleToObject( hObj ));

								if( !pPlayer || !pWorldModel )
									break;

								pWorldModel->GetActivateTypeHandler()->SetDisabled( false, true, pPlayer->GetClient() );
							}
						}
					}
				}
			}
		}
		break;

		case ITEM_REMOVE_ID:
		{
			if (g_pKeyMgr->IsValidKey(nId))
			{
				m_Keys.Remove(nId);
			}
		}
		break;

		case ITEM_CLEAR_ID:
		{
			m_Keys.Clear();
		}
		break;

		default :
			ASSERT(!"Invalid key request encountered");
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::RemoveAttachments
//
//	PURPOSE:	Remove the attachments aggregate with the option of destroying it.
//
// ----------------------------------------------------------------------- //

void CCharacter::RemoveAttachments( bool bDestroyAttachments )
{
	if( m_pAttachments )
	{
		RemoveAggregate( m_pAttachments );

		if( bDestroyAttachments )
		{
			DestroyAttachments();
		}

		m_pAttachments = LTNULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::RemoveWeapons
//
//	PURPOSE:	Remove all the weapons attached to this character.
//
// ----------------------------------------------------------------------- //

void CCharacter::RemoveWeapons()
{
	if( m_pAttachments )
	{
		CWeapon* apWeapons[kMaxWeapons];
		CAttachmentPosition* apAttachmentPositions[kMaxWeapons];
		uint32 cWeapons = m_pAttachments->EnumerateWeapons(apWeapons, apAttachmentPositions, kMaxWeapons);

		while ( cWeapons )
		{
			apAttachmentPositions[--cWeapons]->RemoveWeapon();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::OnLinkBroken
//
//	PURPOSE:	Add to/Remove from/Clear the key list
//
// ----------------------------------------------------------------------- //

void CCharacter::OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj )
{
	for ( uint32 iSpear = 0 ; iSpear < m_cSpears ; iSpear++ )
	{
		if ( &m_aSpears[iSpear].hObject == pRef )
		{
			HATTACHMENT hAttachment;
			if ( LT_OK == g_pLTServer->FindAttachment(m_hObject, hObj, &hAttachment) )
			{
				g_pLTServer->RemoveAttachment(hAttachment);
			}

			m_aSpears[iSpear].hObject = LTNULL;
		}
	}

	GameBase::OnLinkBroken( pRef, hObj );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CCharacter::GetVerticalThreshold()
//              
//	PURPOSE:	Base class function!!  Should be pure virtual, but unable to 
//				because of how DEdit must instantiate classes.
//              
//----------------------------------------------------------------------------
float CCharacter::GetVerticalThreshold() const
{
	AIASSERT( 0, m_hObject, "CCharacter::GetVerticalThreshold: Should not get here" ); 
	return 0.0f;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CCharacter::GetInfoVerticalThreshold()
//              
//	PURPOSE:	Base class function!!  Should be pure virtual, but unable to 
//				because of how DEdit must instantiate classes.
//              
//----------------------------------------------------------------------------
float CCharacter::GetInfoVerticalThreshold() const
{
	AIASSERT( 0, m_hObject, "CCharacter::GetVerticalThreshold: Should not get here" ); 
	return 0.0f;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetHitPointsMod
//
//	PURPOSE:	Modify the current hit pts and max hit pts by a percentage..
//
// ----------------------------------------------------------------------- //

void CCharacter::SetHitPointsMod( float fMod )
{
	float fHitPts =  m_damage.GetHitPoints();
	float fMaxHitPts = m_damage.GetMaxHitPoints();

	m_damage.SetHitPoints( fHitPts + (fHitPts * fMod) );
	m_damage.SetMaxHitPoints( fMaxHitPts + (fMaxHitPts * fMod) );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetArmorPointsMod
//
//	PURPOSE:	Modify the current armor pts and max armor pts by a percentage..
//
// ----------------------------------------------------------------------- //

void CCharacter::SetArmorPointsMod( float fMod )
{
	float fArmor = m_damage.GetArmorPoints();
	float fMaxArmor = m_damage.GetMaxArmorPoints();

	m_damage.SetArmorPoints( fArmor + (fArmor * fMod) );
	m_damage.SetMaxArmorPoints(  fMaxArmor + (fMaxArmor * fMod) );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::IsDeflecting
//
//	PURPOSE:	Checks if this character is deflecting ammo set to be
//				deflectable.
//
// ----------------------------------------------------------------------- //

bool CCharacter::IsDeflecting( )
{
	return !m_tmrDeflecting.Stopped( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetDeflecting
//
//	PURPOSE:	Sets the time we will be deflecting.
//
// ----------------------------------------------------------------------- //

void CCharacter::SetDeflecting( float fDuration )
{
	m_tmrDeflecting.Start( fDuration );
}


LTRESULT CCharacterPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	if (!s_bCharacterPluginInitted)
	{
		char szFile[256];
		if (g_pInventoryButeMgr == LTNULL )
		{
			sprintf(szFile, "%s\\Attributes\\Inventory.txt", szRezPath);
			s_InventoryButeMgr.SetInRezFile(LTFALSE);
			s_InventoryButeMgr.Init(szFile);
		}

		s_bCharacterPluginInitted = LTTRUE;
	}

	char buf[64];
	for(int i=1;i<=CHARACTER_MAX_INVENTORY;i++)
	{
		sprintf(buf,"Item%dID",i);
		if(!_strcmpi(buf, szPropName))
		{
			uint32 cItems = s_InventoryButeMgr.GetNumItems();
			ASSERT(cMaxStrings >= cItems);
			for(uint32 iItem=0;iItem<cItems;iItem++)
			{
				strcpy(aszStrings[(*pcStrings)++],s_InventoryButeMgr.GetItemName(iItem));
			}
	
			return LT_OK;
		}
	}

	return LT_UNSUPPORTED;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateTeleport()
//
//	PURPOSE:	Updates teleport status.  We may have been triggered in the
//				previous frame.
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateTeleport()
{
	// See if we don't need to teleport.
	if( m_eTeleportTriggerState == eTeleporTriggerStateNone )
		return;

	if( m_eTeleportTriggerState == eTeleporTriggerStateVector )
	{
		HandleTeleport( m_vTeleportPos );
	}
	else
	{
		TeleportPoint* pTeleportPt = dynamic_cast< TeleportPoint* >( g_pLTServer->HandleToObject( m_hTeleportPoint ));
		if( pTeleportPt )
			HandleTeleport( pTeleportPt );
	}

	// Clear the flag.
	m_eTeleportTriggerState = eTeleporTriggerStateNone;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CServerSaveLoadMgr::OnPlayerTrackerAbort
//
//	PURPOSE:	Handle when we have to abort a playertracker.
//
// --------------------------------------------------------------------------- //

void CCharacter::OnPlayerTrackerAbort( )
{
	// All remaining clients broke their links by disconnecting.
	if( m_PlayerTrackerDialogue.IsEmpty( ))
		KillDlgSnd( );
}

