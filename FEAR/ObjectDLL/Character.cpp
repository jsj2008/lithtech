// ----------------------------------------------------------------------- //
//
// MODULE  : Character.cpp
//
// PURPOSE : Base class for player and AI
//
// CREATED : 10/6/97
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "Character.h"
#include "iltserver.h"
#include "ServerUtilities.h"
#include "stdio.h"
#include "VolumeBrush.h"
#include "Spawner.h"
#include "SurfaceFunctions.h"
#include "CharacterMgr.h"
#include "VarTrack.h"
#include "SoundMgr.h"
#include "iltmodel.h"
#include "ObjectMsgs.h"
#include "ParsedMsg.h"
#include "SFXMsgIds.h"
#include "Attachments.h"
#include "SurfaceDB.h"
#include "Animator.h"
#include "MsgIDs.h"
#include "GameServerShell.h"
#include "CharacterHitBox.h"
#include "Camera.h"
#include "AIState.h"
#include "ServerSoundMgr.h"
#include "AIStimulusMgr.h"
#include "AINavMesh.h"
#include "AINavMeshLinkAbstract.h"
#include "AIQuadTree.h"
#include "AIStimulusMgr.h"
#include "CharacterDB.h"
#include "Weapon.h"
#include "SharedFXStructs.h"
#include "PlayerObj.h"
#include "TeleportPoint.h"
#include "ServerMissionMgr.h"
#include "ActiveWorldModel.h"
#include "VersionMgr.h"
#include "AIUtils.h"
#include "AISoundDB.h"
#include "WeaponItems.h"
#include "iltphysicssim.h"
#include "FxDefs.h"
#include "ServerDB.h"
#include "Prop.h"
#include "CollisionsDB.h"
#include "ProjectileTypes.h"
#include "sys/win/mpstrconv.h"
#include "StringUtilities.h"
#include "WeaponFireInfo.h"
#include "Ladder.h"

#include <queue>
#include <vector>

LINKFROM_MODULE( Character );

BEGIN_CLASS(CCharacter)
	ADD_DESTRUCTIBLE_AGGREGATE(PF_GROUP(1), 0)
	ADD_ATTACHMENTS_AGGREGATE( PF_GROUP(2) )

	ADD_BOOLPROP(MoveToFloor, true, "Should the character move to the floor at the start of the level?")
	ADD_BOOLPROP(MakeBody, 1, "Make a body after death.")
	ADD_BOOLPROP(PermanentBody, 0, "Don't remove body when doing body density capicity limiting.")
	ADD_REALPROP(BodyLifetime, -1.0f, "How long the body will remain visible after being created.  -1 for infinite lifetime.")
	ADD_STRINGPROP(SpawnItem, "", "Spawn string to run after death.")
	
END_CLASS_FLAGS(CCharacter, GameBase, CF_HIDDEN, "This object is the base object for all Characters inthe game.  Players and AI are derived from Character.  An instance of Character should not be created in the game.")

extern CAIStimulusMgr* g_pAIStimulusMgr;

const float kfFlashlightUpdateRate = 0.3f;

CCharacter::CharacterList CCharacter::m_lstCharacters;
CCharacter::CharacterList CCharacter::m_lstBodies;
CCharacter::CharacterList CCharacter::m_lstSeverBodies;

static VarTrack s_vtBodyCapRadius;
static VarTrack s_vtBodyCapRadiusCount;
static VarTrack s_vtBodyCapTotalCount;
extern VarTrack g_vtBodySeverTest;
extern VarTrack g_vtBodyGibTest;
extern VarTrack g_vtDebugRemote;
static VarTrack s_vtRagdollDisable;
static VarTrack s_vtRagdollDelay;

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	bool ValidateTelePortMsg
//
//  PURPOSE:	Make sure the TELEPORT message is valid
//
// ----------------------------------------------------------------------- //

static bool ValidateTelePortMsg( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !pInterface ) return false;

	if( IsVector( cpMsgParams.m_Args[1] ))
	{
		return true;
	}
	else
	{
		if( !CCommandMgrPlugin::DoesObjectExist( pInterface, cpMsgParams.m_Args[1] ))
		{
			WORLDEDIT_ERROR_MSG1( pInterface, cpMsgParams, "Could not find object '%s'!", cpMsgParams.m_Args[1] );
			return false;
		}

		char const* pszObjClass = CCommandMgrPlugin::GetObjectClass( pInterface, cpMsgParams.m_Args[1] );
		if( pszObjClass && !LTStrICmp( pszObjClass, "TeleportPoint" ))
		{
			return true;
		}

		WORLDEDIT_ERROR_MSG1( pInterface, cpMsgParams, "Object '%s'is not a TeleportPoint or a vector.", cpMsgParams.m_Args[1] );
		return false;
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	bool ValidateBooleanMsg
//
//  PURPOSE:	Make sure any boolean message is valid
//
// ----------------------------------------------------------------------- //

static bool ValidateBooleanMsg( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !pInterface ) return false;

	if( IsTrueChar( *cpMsgParams.m_Args[1] ) ||
		IsFalseChar( *cpMsgParams.m_Args[1] ))
	{
		return true;
	}

	if( CCommandMgrPlugin::s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( true );
		pInterface->CPrint( "ERROR! - ValidateBooleanMsg()" );
		pInterface->CPrint( "    MSG - %s - '%s' is not a valid boolean value!", LTStrUpr(cpMsgParams.m_Args[0]), cpMsgParams.m_Args[1] );
	}
	
	return false;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	bool ValidateCrosshairMsg
//
//  PURPOSE:	Make sure the cross hair message is valid
//
// ----------------------------------------------------------------------- //

static bool ValidateCrosshairMsg( ILTPreInterface *pInterface, ConParse &cpMsgParams )
{
	if( !pInterface ) return false;

	if( !LTStrICmp( cpMsgParams.m_Args[1], "GOOD" ) ||
		!LTStrICmp( cpMsgParams.m_Args[1], "BAD" ) ||
		!LTStrICmp( cpMsgParams.m_Args[1], "NEUTRAL" ) ||
		!LTStrICmp( cpMsgParams.m_Args[1], "UNKNOWN" ))
	{
		return true;
	}

	if( CCommandMgrPlugin::s_bShowMsgErrors )
	{
		pInterface->ShowDebugWindow( true );
		pInterface->CPrint( "ERROR! - ValidateCrosshairMsg()" );
		pInterface->CPrint( "    MSG - CROSSHAIR - Crosshair style '%s' is not valid!", cpMsgParams.m_Args[1] );
	}
	
	return false;
}

//
// Register the class with the command mgr plugin and add any messages to the class
//

CMDMGR_BEGIN_REGISTER_CLASS( CCharacter )

	ADD_MESSAGE( PLAYSOUND,	2,	NULL,					MSG_HANDLER( CCharacter, HandlePlaySoundMsg ),	"PLAYSOUND <DialogueID>", "Tells the character to play a line of dialogue. <DialogueID> specifies an entry in the string database to use.", "msg <CharacterName> (PLAYSOUND 15076)" )	
	ADD_MESSAGE( TELEPORT,	2,	ValidateTelePortMsg,	MSG_HANDLER( CCharacter, HandleTeleportMsg ),	"TELEPORT <teleport point> | <vector>", "Moves the character to the position of the specified TeleportPoint object, or location.", "msg <CharacterName> (TELEPORT TeleportPoint00)<BR>msg <CharacterName> (TELEPORT (1000, 50, 300)) " )
	ADD_MESSAGE( ATTACH,	-1,	NULL,					MSG_HANDLER( CCharacter, HandleAttachMsg ),		"ATTACH <attachment name> [attach pos]", "Creates an attachment on the character. <Attachment name> specifies an Attachment record in the game database. The optional [attach pos] parameter allows you to override the default socket set in the record.", "msg <CharacterName> (ATTACH Bomb)" )
	ADD_MESSAGE( DETACH,	-1,	NULL,					MSG_HANDLER( CCharacter, HandleDetachMsg ),		"DETACH <attachment name> [attach pos]", "Removes an attachment from the character. <Attachment name> specifies an Attachment record in the game database. Use the [attach pos] parameter if the object is attached to a socket other than the default set in the record.", "msg <CharacterName> (DETACH Bomb)" )
	ADD_MESSAGE( CANDAMAGE,	2,	ValidateBooleanMsg,		MSG_HANDLER( CCharacter, HandleCanDamageMsg ),	"CANDAMAGE <bool>", "Sets whether the character can be damaged.", "msg <CharacterName> (CANDAMAGE 0)" )
	ADD_MESSAGE( CROSSHAIR,	2,	ValidateCrosshairMsg,	MSG_HANDLER( CCharacter, HandleCrosshairMsg ),	"CROSSHAIR <alignment>", "Sets the characters alignment or side. The <alignment> can be GOOD (friendly to the player), BAD (unfriendly), or NEUTRAL", "msg <CharacterName> (CROSSHAIR BAD)" )
	ADD_MESSAGE( HIDDEN,	2,	NULL,					MSG_HANDLER( CCharacter, HandleHiddenMsg ),		"HIDDEN <bool>", "Toggles whether the object is visible, solid, and rayhit.", "msg <CharacterName> (HIDDEN 1)<BR>msg <CharacterName> (HIDDEN 0)" )
	ADD_MESSAGE( FIND,		1,	NULL,					MSG_HANDLER( CCharacter, HandleFindMsg ),		"FIND", "Displays the position of the character in the console. Used for debugging", "msg <CharacterName> FIND" )	
	ADD_MESSAGE( REMOVE,	1,	NULL,					MSG_HANDLER( CCharacter, HandleRemoveMsg ),		"REMOVE", "Removes the character from the game permanently.", "msg <CharacterName> REMOVE" )
	ADD_MESSAGE( MOVETOFLOOR, 2, 	NULL, 				MSG_HANDLER( CCharacter, HandleMoveToFloorMsg ),	"MOVETOFLOOR <bool>", "Toggles whether the character is automatically moved to the floor.", "msg <CharacterName> (MOVETOFLOOR 1)<BR>msg <CharacterName> (MOVETOFLOOR 0)" )
	ADD_MESSAGE( DETONATOR,	1,	NULL,					MSG_HANDLER( CCharacter, HandleDetonatorMsg ),	"DETONATOR", "Detonates remote charges set by the character.", "msg Character DETONATOR" )
	ADD_MESSAGE( SHOW_ATTACHFX,	-1,	NULL,				MSG_HANDLER( CCharacter, HandleShowAttachFXMsg ),	"SHOW_ATTACHFX [index]", "Shows the attached ClientFX identified the the given index. This index refers to the array of PersistentClientFX specified in the character model's database record.", "msg <CharacterName> (SHOW_ATTACHFX 1)" )
	ADD_MESSAGE( HIDE_ATTACHFX,	-1,	NULL,				MSG_HANDLER( CCharacter, HandleHideAttachFXMsg ),	"HIDE_ATTACHFX [index]", "Hides the attached ClientFX identified the the given index. This index refers to the array of PersistentClientFX specified in the character model's database record.", "msg <CharacterName> (HIDE_ATTACHFX 1)" )
	
	ADD_MESSAGE_ARG_RANGE( FX,	2,	3,	NULL,	MSG_HANDLER( CCharacter, HandleFXMsg ),	"FX <ClientFX name> [socket]", "Plays the specified ClientFX on the character.  If the optional socket is specified the ClientFX will play at that socket position.", "msg Character (FireFX Left_Leg);" )

CMDMGR_END_REGISTER_CLASS( CCharacter, GameBase )



#define KEY_FOOTSTEP			"FOOTSTEP_KEY"
#define KEY_SET_DIMS			"SETDIMS"
#define KEY_MOVE				"MOVE"
#define KEY_ON					"ON"
#define KEY_OFF					"OFF"
#define KEY_HITBOX_DIMS			"HITBOX_DIMS"
#define	KEY_HITBOX_OFFSET		"HITBOX_OFFSET"
#define KEY_HITBOX_DEFAULT		"HITBOX_DEFAULT"
#define KEY_COOL_MOVE			"COOL_MOVE"

#define TRIGGER_PLAY_SOUND		"PLAYSOUND"
#define TRIGGER_TELEPORT		"TELEPORT"

#define DEFAULT_SOUND_RADIUS		2000.0f
#define DEFAULT_SOUND_INNER_RADIUS	500.0f
#define FOOTSTEP_SOUND_RADIUS		2000.0f
#define DEFAULT_LADDER_VEL			400.0f
#define DEFAULT_SWIM_VEL			175.0f
#define DEFAULT_RUN_VEL				100.0f
#define DEFAULT_WALK_VEL			60.0f
#define DEFAULT_JUMP_VEL			50.0f
#define DEFAULT_FALL_VEL			200.0f
#define DEFAULT_CRAWL_VEL			50.0f
#define DEFAULT_MOVE_ACCEL			3000.0f

#define DIMS_EPSILON				0.05f
#define FALL_LANDING_TIME			0.5f

// Since there is no actual invalid animation tracker id the main tracker is used
// to signify a tracker has not been set up yet.  The define is just used to convey
// the invalid concept.
#define INVALID_TRACKER				MAIN_TRACKER

static VarTrack g_NavMeshDebugTrack;
VarTrack g_BodyStateTimeout;

extern CGameServerShell* g_pGameServerShell;

int32 CCharacter::sm_cAISnds = 0;
uint32 CCharacter::sm_nGibCounter = 0;
uint32 CCharacter::sm_nSeverCounter = 0;

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


CCharacter::CCharacter() 
:	GameBase					( OT_MODEL ),
	m_bInitializedAnimation		( false ),
	m_bShortRecoil				( false ),
	m_bShortRecoiling			( false ),
	m_RecoilAnimTracker			( INVALID_TRACKER ),
	m_BlendAnimTracker			( INVALID_TRACKER ),
	m_pAIAttributes				( NULL ),
	m_ccCrosshair				( kCharStance_Undetermined ),
	m_eAlignment				( kCharAlignment_Invalid ),
	m_fLadderVel				( DEFAULT_LADDER_VEL ),
	m_fSwimVel					( DEFAULT_SWIM_VEL ),
	m_fRunVel					( DEFAULT_RUN_VEL ),
	m_fWalkVel					( DEFAULT_WALK_VEL ),
	m_fJumpVel					( DEFAULT_JUMP_VEL ),
	m_fFallVel					( DEFAULT_FALL_VEL ),
	m_fCrawlVel					( DEFAULT_CRAWL_VEL ),
	m_fBaseMoveAccel			( DEFAULT_MOVE_ACCEL ),
	m_hModelNodeLastHit			( NULL ),
	m_fSoundOuterRadius			( DEFAULT_SOUND_RADIUS ),
	m_fSoundInnerRadius			( DEFAULT_SOUND_INNER_RADIUS ),
	m_eSoundPriority			( SOUNDPRIORITY_AI_HIGH ),
	m_byFXFlags					( 0 ),
	m_bOnGround					( true ),
	m_eStandingOnSurface		( ST_UNKNOWN ),
	m_eContainerSurface			( ST_UNKNOWN ),
	m_eContainerCode			( CC_NO_CONTAINER ),
	m_bBodyInLiquid				( false ),
	m_bBodyWasInLiquid			( false ),
	m_bBodySpecialMove			( false ),
	m_bPlayingTextDialogue		( false ),
	m_eEnemyVisibleStimID		( kStimID_Unset ),
	m_eFlashlightStimID			( kStimID_Unset ),
	m_fNextFlashlightBeamPosUpdateTime	( 0.0f ),
	m_fLastPainVolume			( 0.0f ),
	m_bMoveToFloor				( true ),
	m_sSpawnItem				( ),
	m_pAttachments				( NULL ),
	m_eCurDlgSndType			( CST_NONE ),
	m_bStartedDeath				( false ),
	m_bDelayRagdollDeath		( false ),
	m_eDeathType				( CD_NORMAL ),
	m_hModel					( NULL ),
	m_hModelSkeleton			( NULL ),
	m_fMoveMultiplier			( 1.0f ),
	m_fJumpMultiplier			( 1.0f ),
	m_eCurrentNavMeshPoly		( kNMPoly_Invalid ),
	m_eLastNavMeshPoly			( kNMPoly_Invalid ),
	m_eCurrentNavMeshLink		( kNMLink_Invalid ),
	m_vLastNavMeshPos			( 0.0f, 0.0f, 0.0f ),
	m_fLastCoolMoveTime			( 0.0f ),
	m_hHitBox					( NULL ),
	m_cSpears					( 0 ),
	m_cActive					( 0 ),
	m_nDamageFlags				( 0 ),
	m_nInstantDamageFlags		( 0 ),
	m_bMakeBody					( true ),
	m_bPermanentBody			( false ),
	m_bIsOnBodiesList			( false ),
	m_bIsOnSeveredList			( false ),
	m_eTeleportTriggerState		( eTeleporTriggerStateNone ),
	m_eDeathDamageType			( DT_INVALID ),
	m_nUniqueDialogueId			( 0 ),
	m_nFootstepTrackerId		( MAIN_TRACKER ),
	m_flBlockWindowEndTime		( 0.0f ),
	m_flDodgeWindowEndTime		( 0.0f ),
	m_bFirstDeathUpdate			( false ),
	m_fBodyLifetime				( -1.0f ),
	m_eDeathStimID				( kStimID_Unset ),
	m_bFireWeaponDuringDeath	( false ),
	m_fDropWeaponDuringDeathHeight ( 0.f ),
	m_bGibbed					( false ),
	m_bSevered					( false ),
	m_bDeathEffect				( false ),
	m_bDecapitated				( false ),
	m_bIsRagdolling				( false ),
	m_bIsSolidToAI				( true ),
	m_vLastHitBoxDims			( 0.0f, 0.0f, 0.0f ),
	m_vLastHitBoxOffset			( 0.0f, 0.0f, 0.0f ),
	m_hLadderObject				( INVALID_HOBJECT ),
	m_hLoadLadderObject			( INVALID_HOBJECT )
{
	AddAggregate( &m_Arsenal );
	AddAggregate(&m_damage);
	AddAggregate(&m_MeleeCollisionController);

	// damage filtering
	m_damage.RegisterFilterFunction( DamageFilterHook, this );

	// Setup the TransitionAggregate
	MakeTransitionable();

	m_dwFlags					= FLAG_TOUCH_NOTIFY | FLAG_SOLID | FLAG_GRAVITY |
								  FLAG_MODELKEYS | FLAG_RAYHIT | FLAG_VISIBLE;

    m_bMoveToFloor              = true;

	m_Keys.Clear();

	// Initialize our spears to notify us if their objects get removed.
	for( int nSpear = 0; nSpear < ARRAY_LEN( m_aSpears ); nSpear++ )
	{
		m_aSpears[nSpear].hObject.SetReceiver( *this );
	}

	// Add this instance to a list of all Character's.
	m_lstCharacters.push_back( this );

	//initialize our damage tracker
	m_DamageTracker.Init(this);

	// Clear out ragdoll collision notifiers.
	for (uint32 nCallbacks = 0; nCallbacks < kMaxRagdollCallbacks; nCallbacks++)
	{
		m_aRagdollCollisionParms[nCallbacks].hCollisionNotifier = INVALID_PHYSICS_COLLISION_NOTIFIER;
	}
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
	const char* pPhysicsWeightSetName = g_pModelsDB->GetDefaultPhysicsWeightSet(m_hModel);
	if (pPhysicsWeightSetName)
	{
		SetPhysicsWeightSet(pPhysicsWeightSetName);

		if( IsAI( m_hObject ))
		{
			m_CharacterPhysics.Reset( pPhysicsWeightSetName, PhysicsUtilities::ePhysicsGroup_UserAI, false );
		}
		else
		{
			m_CharacterPhysics.Reset( pPhysicsWeightSetName, PhysicsUtilities::ePhysicsGroup_UserPlayer, IsMultiplayerGameServer( ));
		}
	}

	m_bStartedDeath = false;

	KillDlgSnd();

	// Since we were dead, we need to reset our solid flag...

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_SOLID, FLAG_SOLID);

	// Also update our hit box in case we have moved...
	UpdateHitBox();


	//remove any remotes stuck to us after we died
	ObjRefNotifierList::iterator iter = m_AttachedRemoteCharges.begin();
	while (iter != m_AttachedRemoteCharges.end()) 
	{
		HATTACHMENT hAttachment;
		HOBJECT hRemote = *iter;
		if( hRemote )
		{
			if ( LT_OK == g_pLTServer->FindAttachment(m_hObject, hRemote, &hAttachment) )
			{
				g_pLTServer->RemoveAttachment(hAttachment);
			}
			g_pLTServer->RemoveObject(hRemote);
		}
		++iter;
	}
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
			m_aSpears[iSpear].hObject = NULL;
		}
	}
	m_cSpears = 0;

	//remove any remotes stuck to us
	ObjRefNotifierList::iterator iter = m_AttachedRemoteCharges.begin();
	while (iter != m_AttachedRemoteCharges.end()) 
	{
		HATTACHMENT hAttachment;
		HOBJECT hRemote = *iter;
		if( hRemote )
		{
			if ( LT_OK == g_pLTServer->FindAttachment(m_hObject, hRemote, &hAttachment) )
			{
				g_pLTServer->RemoveAttachment(hAttachment);
			}

			g_pLTServer->RemoveObject(hRemote);
		}
		++iter;
	}
	m_AttachedRemoteCharges.clear();

	RemoveRagdollCollisionNotifications();

	// Take us out of the charactermgr

	g_pCharacterMgr->Remove(this);

	if (m_hHitBox)
	{
		g_pLTServer->RemoveObject(m_hHitBox);
	}

	// Erase this instance from the list of all Character's.
	CharacterList::iterator it = m_lstCharacters.begin( );
	while( it != m_lstCharacters.end( ))
	{
		if( *it == this )
		{
			m_lstCharacters.erase( it );
			break;
		}

		it++;
	}

	if( m_bIsOnBodiesList )
	{
		it = m_lstBodies.begin( );
		while( it != m_lstBodies.end( ))
		{
			if( *it == this )
			{
				m_lstBodies.erase( it );
				break;
			}

			it++;
		}
	}

	if( m_bIsOnSeveredList )
	{
		it = m_lstSeverBodies.begin( );
		while( it != m_lstSeverBodies.end( ))
		{
			if( *it == this )
			{
				m_lstSeverBodies.erase( it );
				break;
			}

			it++;
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::EngineMessageFn()
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CCharacter::EngineMessageFn(uint32 messageID, void *pData, float fData)
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
			HandleModelString( (ArgList*)pData,  (ANIMTRACKERID)fData );
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
					ReadProp(&pStruct->m_cProperties);
				}
			}
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
			g_pCmdMgr->QueueMessage( this, this, "CROSSHAIR UNKNOWN" );

			// Update our NavMesh position.

			UpdateNavMeshPosition();
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
		
			DamageStruct damage;
			damage.InitFromMessage(pMsg);
		
			ProcessDamageMsg( damage );
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
			uint8 nCfxMsg = pMsg->ReadBits( FNumBitsExclusive<CFX_COUNT>::k_nValue );
			switch( nCfxMsg )
			{
				// Client telling character they are exposed to a block
				case CFX_BLOCKWINDOW_MSG:
				{
					m_flBlockWindowEndTime = g_pLTServer->GetTime() + pMsg->Readfloat();
				}
				break;

				// Client telling character they are exposed to a dodge
				case CFX_DODGEWINDOW_MSG:
				{
					m_flDodgeWindowEndTime = g_pLTServer->GetTime() + pMsg->Readfloat();
				}
				break;

				// Client player telling server AI that they kicked them during a berserker attack
				case CFX_BERSERKERKICK_MSG:
				{
					HOBJECT hAI = pMsg->ReadObject();
					CAI* pAI = CAI::DynamicCast(hAI);
					if (pAI)
					{
						pAI->HandleBerserkerKicked();
					}
				}
				break;

				// Client player telling server AI that they are getting "finished" and should play an appropriate action.
				case CFX_FINISHINGMOVE_MSG:
				{
					HOBJECT hAI = pMsg->ReadObject();
					CAI* pAI = CAI::DynamicCast(hAI);
					if (pAI)
					{
						pAI->HandleFinishingMove(pMsg->ReadDatabaseRecord(g_pLTDatabase, g_pModelsDB->GetSyncActionCategory()));
					}
				}
				break;

				// Client telling character to destroy a projectile
				case CFX_KILLPROJECTILE_MSG:
				{
					HOBJECT hProj = pMsg->ReadObject();
					CProjectile* pProj = CProjectile::DynamicCast(hProj);
					if (pProj)
					{
						pProj->Kill();
					}
				}
				break;

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

bool CCharacter::ReadProp(const GenericPropList *pProps)
{
	if( !pProps )
		return false;

	m_bMoveToFloor		= pProps->GetBool( "MoveToFloor", m_bMoveToFloor );
	m_bMakeBody			= pProps->GetBool( "MakeBody", m_bMakeBody );
	m_bPermanentBody	= pProps->GetBool( "PermanentBody", m_bPermanentBody );
	m_fBodyLifetime		= pProps->GetReal( "BodyLifetime", m_fBodyLifetime );
	m_sSpawnItem		= pProps->GetString( "SpawnItem", "" );

	return true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandlePlaySoundMsg()
//
//	PURPOSE:	Handle a PLAYSOUND message...
//
// --------------------------------------------------------------------------- //

void CCharacter::HandlePlaySoundMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		// Get sound name from message...
		PlayDialogSound( crParsedMsg.GetArg(1) );
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleTeleportMsg()
//
//	PURPOSE:	Handle a TELEPORT message...
//
// --------------------------------------------------------------------------- //

void CCharacter::HandleTeleportMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Save the teleportpt off so we can do it in our next update.  We can't
	// teleport now, since we may have been moving when we caused the trigger.  You can't
	// move again if you are already moving.
	if ( !IsVector(crParsedMsg.GetArg(1)) )
	{
		HOBJECT hObject = NULL;
		if ( LT_OK == FindNamedObject(crParsedMsg.GetArg(1), hObject) )
		{
			TeleportPoint* pTeleportPt = dynamic_cast< TeleportPoint* >( g_pLTServer->HandleToObject(hObject));
			if( !pTeleportPt )
				return;

			m_eTeleportTriggerState = eTeleporTriggerStatePoint;
			m_hTeleportPoint = pTeleportPt->m_hObject;
		}
	}
	else
	{
		sscanf(crParsedMsg.GetArg(1), "%f,%f,%f", &m_vTeleportPos.x, &m_vTeleportPos.y, &m_vTeleportPos.z);
		m_eTeleportTriggerState = eTeleporTriggerStateVector;

		// Clear the teleportpt since we are using the vector instead.
		m_hTeleportPoint = NULL;
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleAttachMsg()
//
//	PURPOSE:	Handle a ATTACH message...
//
// --------------------------------------------------------------------------- //

void CCharacter::HandleAttachMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( m_pAttachments )
	{
		m_pAttachments->Attach( crParsedMsg.GetArg(1), (crParsedMsg.GetArgCount() >= 3 ? crParsedMsg.GetArg(2) : NULL) );
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleDetachMsg()
//
//	PURPOSE:	Handle a DETACH message...
//
// --------------------------------------------------------------------------- //

void CCharacter::HandleDetachMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( m_pAttachments )
	{
		m_pAttachments->Detach( crParsedMsg.GetArg(1), (crParsedMsg.GetArgCount() >= 3 ? crParsedMsg.GetArg(2) : NULL) );
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleCanDamageMsg()
//
//	PURPOSE:	Handle a CANDAMAGE message...
//
// --------------------------------------------------------------------------- //

void CCharacter::HandleCanDamageMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	m_damage.SetCanDamage( IsTrueChar( *crParsedMsg.GetArg(1) ));
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleHiddenMsg()
//
//	PURPOSE:	Handle a HIDDEN message...
//
// --------------------------------------------------------------------------- //

void CCharacter::HandleHiddenMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if( crParsedMsg.GetArgCount() > 1 )
	{
		HideCharacter( IsTrueChar( *crParsedMsg.GetArg(1) ) );
	}

	// Let the base handle it...
	GameBase::HandleHiddenMsg( hSender, crParsedMsg );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleFindMsg()
//
//	PURPOSE:	Handle a FIND message...
//
// --------------------------------------------------------------------------- //

void CCharacter::HandleFindMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	LTVector vPos;
	g_pLTServer->GetObjectPos( m_hObject, &vPos );
	vPos = ConvertToDEditPos( vPos );
	g_pLTServer->CPrint("FIND: %s is at pos (%.2f %.2f %.2f)", GetObjectName(m_hObject), vPos.x, vPos.y, vPos.z );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleRemoveMsg()
//
//	PURPOSE:	Handle a REMOVE message...
//
// --------------------------------------------------------------------------- //

void CCharacter::HandleRemoveMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	HandleCharacterRemoval();

	// Let the base handle it...
	GameBase::HandleRemoveMsg( hSender, crParsedMsg );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleCrosshairMsg()
//
//	PURPOSE:	Handle a CROSSHAIR message...
//
// --------------------------------------------------------------------------- //

void CCharacter::HandleCrosshairMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	static CParsedMsg::CToken s_cTok_Good("GOOD");
	static CParsedMsg::CToken s_cTok_Bad("BAD");
	static CParsedMsg::CToken s_cTok_Neutral("NEUTRAL");
	static CParsedMsg::CToken s_cTok_Unknown("UNKNOWN");

	if( crParsedMsg.GetArg(1) == s_cTok_Good )
	{
		m_ccCrosshair = kCharStance_Like;
	}
	else if( crParsedMsg.GetArg(1) == s_cTok_Bad )
	{
		m_ccCrosshair = kCharStance_Hate;
	}
	else if( crParsedMsg.GetArg(1) == s_cTok_Neutral )
	{
		m_ccCrosshair = kCharStance_Tolerate;
	}
	else if( crParsedMsg.GetArg(1) == s_cTok_Unknown )
	{
		m_ccCrosshair = kCharStance_Undetermined;
	}

	ResetCrosshair( m_ccCrosshair );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleMoveToFloorMsg()
//
//	PURPOSE:	Handle a MOVETOFLOOR message...
//
// --------------------------------------------------------------------------- //

void CCharacter::HandleMoveToFloorMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
      if( crParsedMsg.GetArgCount() <= 1 )
		return;

    SetMoveToFloor( IsTrueChar( *crParsedMsg.GetArg(1) ) != 0 );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleFXMsg()
//
//	PURPOSE:	Handle a FX message...
//
// --------------------------------------------------------------------------- //

void CCharacter::HandleFXMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	// Read the FX name and optional socket and send message to client to play...

	const char *pszClientFX = crParsedMsg.GetArg( 1 );

	const char *pszSocket = NULL;
	if( crParsedMsg.GetArgCount( ) > 1 )
		pszSocket = crParsedMsg.GetArg( 2 );

	HMODELSOCKET hSocket = INVALID_MODEL_SOCKET;
	if( !LTStrEmpty( pszSocket ))
	{
		if( g_pModelLT->GetSocket( m_hObject, pszSocket, hSocket ) != LT_OK )
		{
			char szError[256] = {0};
			LTSNPrintF( szError, LTARRAYSIZE(szError), "Socket (%s) does not exist on character model.", pszSocket );
			LTERROR( szError );
		}
	}

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_CHARACTER_ID );
	cMsg.WriteObject( m_hObject );
	cMsg.WriteBits( CFX_FX_MSG, FNumBitsExclusive<CFX_COUNT>::k_nValue );
	cMsg.WriteString( pszClientFX );
	cMsg.Writeuint32( hSocket );
	
	g_pLTServer->SendToClient( cMsg.Read( ), NULL, MESSAGE_GUARANTEED );
	
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleShowAttachFXMsg()
//
//	PURPOSE:	Handle a SHOW_ATTACHFX message...
//
// --------------------------------------------------------------------------- //

void CCharacter::HandleShowAttachFXMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	uint32 nElement = (uint32)-1;
	if( crParsedMsg.GetArgCount() > 1 )
	{
		nElement = atoi( crParsedMsg.GetArg(1) );
	}

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_CHARACTER_ID );
	cMsg.WriteObject( m_hObject );
	cMsg.WriteBits( CFX_SHOW_ATTACH_FX, FNumBitsExclusive<CFX_COUNT>::k_nValue );
	cMsg.Writeuint32( nElement );

	g_pLTServer->SendToClient( cMsg.Read( ), NULL, MESSAGE_GUARANTEED );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleHideAttachFXMsg()
//
//	PURPOSE:	Handle a HIDE_ATTACHFX message...
//
// --------------------------------------------------------------------------- //

void CCharacter::HandleHideAttachFXMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	uint32 nElement = (uint32)-1;
	if( crParsedMsg.GetArgCount() > 1 )
	{
		nElement = atoi( crParsedMsg.GetArg(1) );
	}

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_CHARACTER_ID );
	cMsg.WriteObject( m_hObject );
	cMsg.WriteBits( CFX_HIDE_ATTACH_FX, FNumBitsExclusive<CFX_COUNT>::k_nValue );
	cMsg.Writeuint32( nElement );

	g_pLTServer->SendToClient( cMsg.Read( ), NULL, MESSAGE_GUARANTEED );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleCharacterRemoval()
//
//	PURPOSE:	Handle removing a character.
//
// --------------------------------------------------------------------------- //

void CCharacter::HandleCharacterRemoval()
{
	AITRACE( AIShowCharacters, ( m_hObject, "Received Remove command." ) );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::ResetCrosshair()
//
//	PURPOSE:	Tell client about crosshair alignment change.
//
// --------------------------------------------------------------------------- //

void CCharacter::ResetCrosshair( EnumCharacterStance ccCrosshair )
{
	// Check if already set.
	if( m_ccCrosshair == ccCrosshair )
		return;

	m_ccCrosshair = ccCrosshair;

	// If our Crosshair is unknown, then the effect gets the result of
	// our Crosshair relative to the player (Crosshairs are always relative
	// to the player).  m_ccCrosshair can be kStance_Undetermined if no subclass sets it
	// and if a level designer does not set it explicitly.
	if ( m_ccCrosshair == kCharStance_Undetermined )
	{
		CPlayerObj* pPlayer = g_pCharacterMgr->FindPlayer();
		if( pPlayer )
		{
			m_ccCrosshair = g_pCharacterDB->GetStance( pPlayer->GetAlignment(), m_eAlignment );
		}
	}

	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_CHARACTER_ID );
	cMsg.WriteObject( m_hObject );
	cMsg.WriteBits(CFX_CROSSHAIR_MSG, FNumBitsExclusive<CFX_COUNT>::k_nValue );
	cMsg.WriteBits(m_ccCrosshair, FNumBitsExclusive<kCharStance_Count>::k_nValue);
	g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);

	CreateSpecialFX();
}



// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::ProcessDamageMsg()
//
//	PURPOSE:	Process a damage message.
//
// --------------------------------------------------------------------------- //

void CCharacter::ProcessDamageMsg( DamageStruct &rDamage )
{
	if( Camera::IsActive( ))
		return;

	if (rDamage.eType == DT_WORLDONLY) 
	{
		return;
	}


	if ( !m_damage.IsCantDamageType(rDamage.eType) && m_damage.GetCanDamage() )
	{
		// Set our pain information

		m_fLastPainVolume = 1.0f;

		// AIs modify their pain volume.
		// Register AllyPainSound stimulus.
		if(!IsAI(m_hObject))
		{
			LTVector vPainPos;
			g_pLTServer->GetObjectPos(m_hObject, &vPainPos);

			StimulusRecordCreateStruct scs(kStim_PainSound, GetAlignment(), vPainPos, m_hObject);
			scs.m_flRadiusScalar = m_fLastPainVolume;
			g_pAIStimulusMgr->RegisterStimulus( scs );
		}

		// Play a damage sound...

		if (IsAlive() && m_damage.GetCanDamage())
		{
			// Play our damage sound

			PlayDamageSound(rDamage.eType);
		}

		m_DamageTracker.ProcessDamage(rDamage);
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

uint32 CCharacter::GetUserFlagSurfaceType() const
{
	SurfaceType eType = ST_UNKNOWN;
	if (m_hModel)
	{
		eType = ((m_damage.GetArmorPoints() > 0.0f) ?	g_pModelsDB->GetArmorSurfaceType(m_hModel) : 
														g_pModelsDB->GetFleshSurfaceType(m_hModel));
	}

	return SurfaceToUserFlag(eType);
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetCollisionPropertyRecord()
//
//	PURPOSE:	Return our surface type as a user flag
//
// --------------------------------------------------------------------------- //

HRECORD CCharacter::GetCollisionPropertyRecord() const
{
	if (!m_hModel)
	{
		return NULL;
	}
	return g_pModelsDB->GetCollisionProperty( m_hModel );
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
	// Initialize console variables...

	if(!g_BodyStateTimeout.IsInitted())
        g_BodyStateTimeout.Init(g_pLTServer, "BodyStateTimeout", NULL, 5.0f);

	// Need to always intialize our characterphysics, since it's not part of save/load.
	m_CharacterPhysics.Init( m_hObject );

	if (nInfo == INITIALUPDATE_SAVEGAME) return;

	m_tmrDialogue.SetEngineTimer( ObjectContextTimer( m_hObject ));
	m_tmrBodyFade.SetEngineTimer( RealTimeTimer::Instance( ));
	m_tmrLifetime.SetEngineTimer( RealTimeTimer::Instance( ));

	// If the character was spawned, he will never get a MID_ALLOBJECTSCREATED,
	// so we need to register persistent stimuli here to make character visible. 

	if( ( !IsPlayer(m_hObject) ) && ( nInfo != INITIALUPDATE_WORLDFILE ) )
	{
		RegisterPersistentStimuli();
	}

	// Create the box used for weapon impact detection...

	CreateHitBox();

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, m_dwFlags, FLAGMASK_ALL);

	m_damage.Init(m_hObject);

	if( m_hModel )
	{
		//see if this object is translucent
		if(g_pModelsDB->IsModelTranslucent(m_hModel))
		{
			//it is, setup the flag
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags2, FLAG2_FORCETRANSLUCENT, FLAG2_FORCETRANSLUCENT);
		}

		m_damage.SetMass(g_pModelsDB->GetModelMass(m_hModel));

		m_damage.SetHitPoints(g_pModelsDB->GetModelHitPoints(m_hModel));
		m_damage.SetMaxHitPoints(g_pModelsDB->GetModelMaxHitPoints(m_hModel));

		m_damage.SetArmorPoints(g_pModelsDB->GetModelArmor(m_hModel));
		m_damage.SetMaxArmorPoints(g_pModelsDB->GetModelMaxArmor(m_hModel));
	
		// See if this model supports short recoiling.
		m_bShortRecoil = g_pModelsDB->GetModelShortRecoils( m_hModel );
	}

	// Set up userflags.
	uint32 nFlags = USRFLG_MOVEABLE | USRFLG_CHARACTER | GetUserFlagSurfaceType() | 
		CollisionPropertyRecordToUserFlag( GetCollisionPropertyRecord( ));
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, nFlags, USRFLG_MOVEABLE | USRFLG_CHARACTER | 
		USRFLG_SURFACEMASK | USRFLG_COLLISIONPROPMASK );

	// Set our initial dims based on the current animation...
	// TODO! does this need to change?

    LTVector vDims;
	g_pModelLT->GetModelAnimUserDims(m_hObject, g_pLTServer->GetModelAnimation(m_hObject), &vDims);
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

	//clear out our accumulated damage
	m_DamageTracker.Clear();

	m_MeleeCollisionController.Init( m_hObject );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::CreateSpecialFX()
//
//	PURPOSE:	Add client-side special fx
//
// ----------------------------------------------------------------------- //
void CCharacter::CreateSpecialFX(bool bUpdateClients /* =false */)
{
	// Create the special fx...

	m_cs.Clear();

	m_cs.hServerObj				= m_hObject;
	m_cs.hModel					= m_hModel;
	m_cs.byFXFlags				= m_byFXFlags;
	m_cs.fStealthPercent			= GetStealthModifier();
	m_cs.bIsDead					= m_damage.IsDead();
	m_cs.vDeathDir				= m_damage.GetDeathDir();
	m_cs.fDeathImpulseForce		= m_damage.GetDeathImpulseForce();
	m_cs.hModelNodeLastHit		= m_hModelNodeLastHit;
	m_cs.fDeathNodeImpulseForceScale = m_damage.GetDeathHitNodeImpulseForceScale();
	m_cs.hDeathAmmo				= m_damage.GetDeathAmmo();
	m_cs.eDeathDamageType			= m_eDeathDamageType;
	m_cs.hCurWeaponRecord			= m_Arsenal.GetCurWeaponRecord( );
	m_cs.bIsSpectating			= IsSpectating();
	m_cs.bPermanentBody			= m_bPermanentBody;

	// If our Crosshair is unknown, then the effect gets the result of
	// our Crosshair relative to the player (Crosshairs are always relative
	// to the player).  m_ccCrosshair can be kStance_Undetermined if no subclass 
	// sets it and if a level designer does not set it explicitly
	if ( m_ccCrosshair == kCharStance_Undetermined )
	{
		CPlayerObj* pPlayer = g_pCharacterMgr->FindPlayer();
		if( pPlayer )
		{
			m_ccCrosshair = g_pCharacterDB->GetStance( pPlayer->GetAlignment(), m_eAlignment );
		}
	}

	m_cs.eCrosshairPlayerStance	= m_ccCrosshair;
	m_cs.nDamageFlags				= m_nDamageFlags;


	if( m_hHitBox )
	{
		CCharacterHitBox *pHitBox = CCharacterHitBox::DynamicCast( m_hHitBox );
		if( pHitBox )
		{
			m_cs.vHitBoxOffset = pHitBox->GetOffset();
			g_pPhysicsLT->GetObjectDims( m_hHitBox, &m_cs.vHitBoxDims );
		}
	}
	
	PreCreateSpecialFX(m_cs);

	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(SFX_CHARACTER_ID);
		m_cs.Write(cMsg);
		g_pLTServer->SetObjectSFXMessage(m_hObject, cMsg.Read());
	}

	// Tell the client about the new info...

	if (bUpdateClients)
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_SFX_MESSAGE);
		cMsg.Writeuint8(SFX_CHARACTER_ID);
		cMsg.WriteObject(m_hObject);
		cMsg.WriteBits(CFX_ALLFX_MSG, FNumBitsExclusive<CFX_COUNT>::k_nValue );
        m_cs.Write(cMsg);
		g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);
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
	cMsg.WriteBits(CFX_STEALTH_MSG, FNumBitsExclusive<CFX_COUNT>::k_nValue );
	cMsg.Writefloat(GetStealthModifier());
	g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);

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
	cMsg.WriteBits(CFX_DMGFLAGS_MSG, FNumBitsExclusive<CFX_COUNT>::k_nValue );
	if( m_nDamageFlags == 0 )
	{
		cMsg.Writebool( false );
	}
	else
	{
		cMsg.Writebool( true );
		cMsg.WriteBits(m_nDamageFlags, kNumDamageTypes);
	}
	g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);

	// Update our special fx message so new clients will get the updated
	// info as well...

	CreateSpecialFX();
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCharacter::SendWeaponRecordToClients
//
//  PURPOSE:	Notify the clients of our current weapon...
//
// ----------------------------------------------------------------------- //

void CCharacter::SendWeaponRecordToClients( )
{
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_CHARACTER_ID );
	cMsg.WriteObject( m_hObject );
	cMsg.WriteBits(CFX_CHANGE_WEAPON, FNumBitsExclusive<CFX_COUNT>::k_nValue );
	cMsg.WriteDatabaseRecord( g_pLTDatabase, m_Arsenal.GetCurWeaponRecord( ));
	g_pLTServer->SendToClient( cMsg.Read( ), NULL, MESSAGE_GUARANTEED );

	CreateSpecialFX( false );
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

	if( g_pGameServerShell->ShouldSendInstantDamageToClient( nDmgFlags ))
	{
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_CHARACTER_ID );
	cMsg.WriteObject( m_hObject );
		cMsg.WriteBits(CFX_INSTANTDMGFLAGS_MSG, FNumBitsExclusive<CFX_COUNT>::k_nValue );
		cMsg.WriteBits(m_nInstantDamageFlags, kNumDamageTypes);
	g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );
}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::*FlashLight()
//
//	PURPOSE:	Creates/Destroys flashlight sfx on client
//
// ----------------------------------------------------------------------- //

void CCharacter::CreateFlashLight()
{
	LTASSERT(!(m_byFXFlags & CHARCREATESTRUCT::eFlashLight), "TODO: Add description here");

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_SFX_MESSAGE);
	cMsg.Writeuint8(SFX_CHARACTER_ID);
	cMsg.WriteObject(m_hObject);
	cMsg.WriteBits(CFX_FLASHLIGHT_CREATE_MSG, FNumBitsExclusive<CFX_COUNT>::k_nValue );
	g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);

	m_byFXFlags |= CHARCREATESTRUCT::eFlashLight;

	CreateSpecialFX();

	// Create a stimulus to let AI see the flashlight beam.

	if( IsPlayer( m_hObject ) )
	{
		StimulusRecordCreateStruct scs( kStim_FlashlightBeamVisible, GetAlignment(), LTVector( 0.f, 0.f, 0.f ), m_hObject );
		m_eFlashlightStimID = g_pAIStimulusMgr->RegisterStimulus( scs );
		UpdateFlashLight();
	}
}

void CCharacter::DestroyFlashLight()
{
	LTASSERT(m_byFXFlags & CHARCREATESTRUCT::eFlashLight, "TODO: Add description here");

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_SFX_MESSAGE);
	cMsg.Writeuint8(SFX_CHARACTER_ID);
	cMsg.WriteObject(m_hObject);
	cMsg.WriteBits(CFX_FLASHLIGHT_DESTROY_MSG, FNumBitsExclusive<CFX_COUNT>::k_nValue );
	g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);

	m_byFXFlags &= ~CHARCREATESTRUCT::eFlashLight;

	CreateSpecialFX();

	// Remove the stimulus that lets AI see the flashlight beam.

	if( m_eFlashlightStimID != kStimID_Unset )
	{
		g_pAIStimulusMgr->RemoveStimulus( m_eFlashlightStimID );
		m_eFlashlightStimID = kStimID_Unset;
		m_fNextFlashlightBeamPosUpdateTime = 0.f;
	}
}

void CCharacter::UpdateFlashLight()
{
	// No flashlight stimulus to update.

	if( m_eFlashlightStimID == kStimID_Unset )
	{
		return;
	}

	// Stimulus record does not exist.

	CAIStimulusRecord* pStimulus = g_pAIStimulusMgr->GetStimulusRecord( m_eFlashlightStimID );
	if( !pStimulus )
	{
		return;
	}

	// Not time to update yet.

	double fCurTime = g_pLTServer->GetTime();
	if( fCurTime >= m_fNextFlashlightBeamPosUpdateTime )
	{
		// Flashlight points along the character's view.

		LTRigidTransform tfView;
		GetViewTransform( tfView );
		LTVector vForward = tfView.m_rRot.Forward();

		// Find where the end of the beam hits the world.

		IntersectQuery IQuery;
		IntersectInfo IInfo;

		IQuery.m_From	  = tfView.m_vPos;
		IQuery.m_To		  = tfView.m_vPos + ( vForward * 10000.f );
		IQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID;

		pStimulus->m_hStimulusTarget = NULL;
		pStimulus->m_vStimulusPos = tfView.m_vPos;
		if( g_pLTServer->IntersectSegment( IQuery, &IInfo ) )
		{
			// Record if the beam is hitting an AI.

			if( IsCharacterHitBox( IInfo.m_hObject ) )
			{
				CCharacterHitBox* pHitBox = (CCharacterHitBox*)g_pLTServer->HandleToObject(IInfo.m_hObject);
				if( pHitBox )
				{
					pStimulus->m_hStimulusTarget = pHitBox->GetModelObject();
				}
			}

			// Move the stimulus slightly away from the geometry.

			pStimulus->m_vStimulusPos = IInfo.m_Point;
			pStimulus->m_vStimulusPos += vForward * -1.f;
		}

		// Do not update the flashlight position every frame.

		m_fNextFlashlightBeamPosUpdateTime = fCurTime + kfFlashlightUpdateRate;
	}

	// Refresh stimulus to allow AI to sense it again.

	pStimulus->m_fTimeStamp = fCurTime;
	pStimulus->m_lstCurResponders.resize( 0 );
}

bool CCharacter::IsFlashlightOn()
{
	return ( m_byFXFlags & CHARCREATESTRUCT::eFlashLight );
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

	if (!g_pLTServer || !pWeapon) return vPos;

	HATTACHMENT hAttachment;
    if (g_pLTServer->FindAttachment(m_hObject, pWeapon->GetModelObject(), &hAttachment) != LT_OK)
	{
		return vPos;
	}

	HMODELSOCKET hSocket;

    if (g_pModelLT->GetSocket(pWeapon->GetModelObject(), "Flash", hSocket) == LT_OK)
	{
		LTTransform transform;
		g_pCommonLT->GetAttachedModelSocketTransform(hAttachment, hSocket, transform);

		vPos = transform.m_vPos;

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
	// Characters must provide a weightset named 'Null'.  This weightset is 
	// used for trackers which are not in use or have not yet been configured.

	const char* const pszNullWeightSetName = g_pModelsDB->GetNullWeightSetName();
	if ( LT_OK != g_pModelLT->FindWeightSet(m_hObject, pszNullWeightSetName, m_hNullWeightset) )
	{
#ifndef _FINAL
		g_pLTServer->CPrint("Critical error, no Null weightset on Character!  Missing: %s", pszNullWeightSetName);
#endif
		LTERROR( "Critical error, no 'Null' weightset on Character!" );
		return;
	}

	// Add a blend tracker, if this character uses blending.  This tracker 
	// needs to be added BEFORE the twitch tracker, as the twitch tracker 
	// uses an additive blend and this uses an interpolative blend.  This 
	// should be done in the AI code, but that hides tracker creation order 
	// dependency.  Initially set the weightset to 'Null', as the weightset
	// is set per animation when the blend tracker is used.

	if ( m_pAIAttributes && m_pAIAttributes->bEnableAnimationBlending )
	{
		m_BlendAnimTracker = kAD_TRK_Blend; 
		if ( LT_OK != g_pModelLT->AddTracker( m_hObject, m_BlendAnimTracker, true ) )
		{
			LTASSERT_PARAM1( g_pLTBase, "CAnimationContext::SetBlendTracker - Failed to add animation tracker: %d.", m_BlendAnimTracker );
			m_BlendAnimTracker = INVALID_TRACKER;
		}
		else
		{
			g_pModelLT->SetWeightSet( m_hObject, m_BlendAnimTracker, m_hNullWeightset );
		}
	}

	AddRecoilTracker();

	// We're initted
	m_bInitializedAnimation = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::AddRecoilTracker()
//
//	PURPOSE:	Initialize the tracker used for playing recoil animations...
//
// ----------------------------------------------------------------------- //

void CCharacter::AddRecoilTracker( )
{
	if( !m_bShortRecoil )
		return;

	// Remove any previously added recoil tracker...
	RemoveRecoilTracker( );

	m_RecoilAnimTracker = kAD_TRK_Twitch;
    if( LT_OK != g_pModelLT->AddTracker( m_hObject, m_RecoilAnimTracker, true ))
	{
		g_pLTServer->CPrint( "Failed to add recoil tracker!" );       
	}
		

	if ( LT_OK != g_pModelLT->FindWeightSet(m_hObject, "Null", m_hNullWeightset) )
	{
#ifndef _FINAL
		g_pLTServer->CPrint("Critical error, no Null weightset on Character!");
#endif
		LTERROR( "Critical error, no 'Null' weightset on Character!" );
		return;
	}

	if ( LT_OK != g_pModelLT->FindWeightSet(m_hObject, "Twitch", m_hTwitchWeightset) )
	{
#ifndef _FINAL
		g_pLTServer->CPrint("Critical error, no Twitch weightset on Character!");
#endif
		LTERROR( "Critical error, no 'Twitch' weightset on Character!" );
		m_hTwitchWeightset = m_hNullWeightset;
		return;
	}

	if( LT_OK != g_pModelLT->SetWeightSet(m_hObject, m_RecoilAnimTracker, m_hTwitchWeightset) )
	{
		LTERROR( "SetWeightSet - Failed" );
		return;
	}

	// It is Ok for a character to NOT have an anim called Base,
	// so do not bail if it is not found.

	HMODELANIM hBaseAnim = g_pLTServer->GetAnimIndex( m_hObject, "Base" );

	g_pModelLT->SetCurAnim(m_hObject, m_RecoilAnimTracker, hBaseAnim, true);

    g_pModelLT->SetLooping(m_hObject, m_RecoilAnimTracker, false);

	g_pModelLT->SetPlaying( m_hObject, m_RecoilAnimTracker, false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::RemoveRecoilTracker()
//
//	PURPOSE:	Remove the recoil tracker.  This is needed to ensure the recoil tracker is 
//				always the last tracker added...
//
// ----------------------------------------------------------------------- //

void CCharacter::RemoveRecoilTracker( )
{
	// Remove any previously added recoil tracker...
	if( m_RecoilAnimTracker != INVALID_TRACKER )
	{
		g_pModelLT->RemoveTracker( m_hObject, m_RecoilAnimTracker );

		m_RecoilAnimTracker = INVALID_TRACKER;
		m_bShortRecoiling = false;
	}
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
	if (m_damage.IsDead() && m_bStartedDeath)
	{
		UpdateDead( true );
	}

	UpdateTeleport( );

	if ( !m_bInitializedAnimation )
	{
		InitAnimation();
	}

	// Update the recoil

	if( m_bShortRecoiling )
	{
		uint32 dwFlags;
        if ( LT_OK == g_pModelLT->GetPlaybackState(m_hObject, m_RecoilAnimTracker, dwFlags) )
		{
			if ( MS_PLAYDONE & dwFlags )
			{
				g_pModelLT->SetWeightSet(m_hObject, m_RecoilAnimTracker, m_hNullWeightset);
				g_pModelLT->SetLooping( m_hObject, m_RecoilAnimTracker, false );
				g_pModelLT->SetPlaying( m_hObject, m_RecoilAnimTracker, false );

				m_bShortRecoiling = false;
			}
		}
	}

	// Update our NavMesh position.

	UpdateNavMeshPosition();

	// Update our sounds

	UpdateSounds();

	// Keep track of frame to frame changes...

	m_bBodyWasInLiquid		= m_bBodyInLiquid;
    m_bBodyInLiquid         = false;

	m_eContainerSurface = ST_UNKNOWN;

	// Update our container code info...

	UpdateContainerCode();

	// Make sure our hit box is in the correct position...

	UpdateHitBox();

	// Update our animation

	UpdateAnimation();

	// Update where our flashlight beam is pointing.

	if( IsFlashlightOn() )
	{
		UpdateFlashLight();
	}

	// Update our playertracker for dialogue.
	m_PlayerTrackerDialogue.Update( );

	// Update physics weight set if necessary...(doing this in the update
	// ensures that we won't be in the middle of an animation when the
	// physics weight set changes)...
	
	if (!m_sPhysicsWeightSet.empty())
	{
		SetPhysicsWeightSet(m_sPhysicsWeightSet.c_str());
		m_sPhysicsWeightSet = ""; // clear out
	}

	//clear out our accumulated damage
	//	(this should be done after all other update code that will need the damage info)
	m_DamageTracker.Clear();

	// Update any character specific physics...
	m_CharacterPhysics.Update( );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateDead()
//
//	PURPOSE:	Update the object while the character is dead
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateDead( bool bCanBeRemoved )
{
	if (!m_damage.IsDead())
		return;

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);

	if( !IsPlayer( m_hObject ))
	{
		LTVector vMin, vMax;
		g_pLTServer->GetWorldBox(vMin, vMax);

		if (vPos.x < vMin.x || vPos.y < vMin.y || vPos.z < vMin.z ||
			vPos.x > vMax.x || vPos.y > vMax.y || vPos.z > vMax.z)
		{
			g_pLTServer->RemoveObject( m_hObject );
			return;
		}
	}

	if ( m_bFirstDeathUpdate )
	{
		m_bFirstDeathUpdate = false;

		if( IsMultiplayerGameServer( ))
		{
			m_fBodyLifetime = g_pServerDB->GetPlayerFloat( SrvDB_fMultiplayerBodyLifetime );
		}

		if (m_fBodyLifetime > 0.0f && !m_bPermanentBody)
		{
			m_tmrLifetime.Start(m_fBodyLifetime);
		}

		// Remove CharacterVisible stimulus.
		RemovePersistentStimuli();

		// Register AllyDeathVisible stimulus.
		StimulusRecordCreateStruct scs( kStim_DeathVisible, m_eAlignment, vPos, m_hObject );
		m_eDeathStimID = g_pAIStimulusMgr->RegisterStimulus( scs );

	}
	else if (m_tmrLifetime.IsStarted()) //if the body has a lifetime, check to see if it has expired...
	{
		if (m_tmrLifetime.IsTimedOut())
		{
			m_tmrLifetime.Stop();
			StartFade();
		}
	}
	else if (m_tmrBodyFade.IsStarted()) //if the body is fading...
	{
		if (m_tmrBodyFade.IsTimedOut() && bCanBeRemoved )
		{
			g_pAIStimulusMgr->RemoveStimulus( m_eDeathStimID );
			g_pLTServer->RemoveObject( m_hObject );
			return;
		}
	}

	// Fire weapon while dying.

	if( m_bFireWeaponDuringDeath )
	{
		UpdateFireWeaponDuringDeath();
	}
	
	// Always update to support shooting dead bodies (make sure hit box is moved
	// with ragdoll AI) 
	SetNextUpdate(UPDATE_NEXT_FRAME);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateFireWeaponsDuringDeath()
//
//	PURPOSE:	Update weapon firing while dying.
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateFireWeaponDuringDeath()
{
	// Bail if killed by a melee attack.

	if( ( m_damage.GetLastDamageType() == DT_MELEE ) ||
		( m_damage.GetLastDamageType() == DT_RIFLE_BUTT ) )
	{
		DropWeapons();
		m_bFireWeaponDuringDeath = false;
		return;
	}

	// Bail if no current weapon.

	CWeapon* pWeapon = m_Arsenal.GetCurWeapon();
	if( !pWeapon )
	{
		DropWeapons();
		m_bFireWeaponDuringDeath = false;
		return;
	}

	// Bail if we fail to find the weapon's socket.

	HMODELSOCKET hSocket = INVALID_MODEL_SOCKET;
	if( LT_OK != g_pModelLT->GetSocket( m_hObject, pWeapon->GetActiveWeapon()->GetSocketName(), hSocket ) )
	{
		DropWeapons();
		pWeapon->KillLoopSound();
		m_bFireWeaponDuringDeath = false;
		return;
	}

	// Bail if we fail to find the socket's transform.

	LTTransform transform;
	if( LT_OK != g_pModelLT->GetSocketTransform( m_hObject, hSocket, transform, true ) )
	{
		DropWeapons();
		pWeapon->KillLoopSound();
		m_bFireWeaponDuringDeath = false;
		return;
	}

	// Bail if weapon dips below lowest quarter of dim's height.
	// This helps prevent clipping the weapon into the ground.

	if( transform.m_vPos.y < m_fDropWeaponDuringDeathHeight )
	{
		pWeapon->KillLoopSound();
		DropWeapons();
		m_bFireWeaponDuringDeath = false;
		return;
	}

	// Bail if clip is empty.

	if( pWeapon->GetAmmoInClip() <= 0 )
	{
		pWeapon->KillLoopSound();
		DropWeapons();
		m_bFireWeaponDuringDeath = false;
		return;
	}

	// Fire the weapon.

	WeaponFireInfo weaponFireInfo;
	static uint8 s_nCount = GetRandom( 0, 255 );
	s_nCount++;

	weaponFireInfo.hFiredFrom  		= m_hObject;
	weaponFireInfo.vPath       		= transform.m_rRot.Forward();
	weaponFireInfo.vFirePos    		= transform.m_vPos;
	weaponFireInfo.vFlashPos   		= transform.m_vPos;
	weaponFireInfo.hFiringWeapon	= pWeapon->GetModelObject();
	weaponFireInfo.fPerturb			= 0.0f;
	weaponFireInfo.nSeed			= (uint8)GetRandom( 2, 255 );
	weaponFireInfo.nPerturbCount	= s_nCount;
	weaponFireInfo.nFireTimestamp	= g_pLTServer->GetRealTimeMS( );

	WeaponState eWeaponState = pWeapon->UpdateWeapon( weaponFireInfo, true );

	// Play the weapon's fire animation (to play the looping sound).

	uint32 dwFireAni = pWeapon->GetFireAni();
	if( dwFireAni != INVALID_ANI )
	{
		pWeapon->PlayAnimation( dwFireAni, true, false );
	}

	// Stop firing after a single shot if weapon is not automatic.

	if( ( eWeaponState == W_FIRED ) &&
		g_pWeaponDB->GetBool( pWeapon->GetWeaponData(), WDB_WEAPON_bSemiAuto ) )
	{
		pWeapon->KillLoopSound();
		DropWeapons();
		m_bFireWeaponDuringDeath = false;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::RagdollDeath()
//
//	PURPOSE:	Ragdoll the character on death
//
// ----------------------------------------------------------------------- //

void CCharacter::RagdollDeath( bool bClampVelocity )
{
	// Don't do ragdoll death if disabled (used for debugging)
	if( !m_CharacterPhysics.CanRagDoll( )) 
	{
		return;
	}


	// Rag-doll time...
	SetPhysicsWeightSet(PhysicsUtilities::WEIGHTSET_RIGID_BODY);
	
	bool bClientOnly = IsMultiplayerGameServer( );
	m_CharacterPhysics.RagDollDeath( m_damage.GetDeathDir( ), m_damage.GetDeathImpulseForce( ),
									 m_hModelNodeLastHit, m_damage.GetDeathHitNodeImpulseForceScale( ), 
									 bClampVelocity, bClientOnly );

	// Send the ragdoll message...
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_CHARACTER_ID );
	cMsg.WriteObject( m_hObject );
	cMsg.WriteBits(CFX_RAGDOLL, FNumBitsExclusive<CFX_COUNT>::k_nValue );
	cMsg.WriteCompLTPolarCoord( LTPolarCoord( m_damage.GetDeathDir( )));
	cMsg.Writefloat( m_damage.GetDeathImpulseForce( ));
	cMsg.WriteDatabaseRecord( g_pLTDatabase, m_hModelNodeLastHit );
	cMsg.Writefloat( m_damage.GetDeathHitNodeImpulseForceScale( ));
	g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CCharacter::AddRagdollCollisionNotifications()
//              
//	PURPOSE:	Add collision notifications for RagdollDamage.
//              
//----------------------------------------------------------------------------
void CCharacter::AddRagdollCollisionNotifications()
{
	// Get rid of existing notifications.
	RemoveRagdollCollisionNotifications();

#if 0 //!!ARL: Is there a server-side performance flag?
	if (!g_pProfileMgr->GetCurrentProfile()->m_bPhysics)
		return;
#endif

	HRECORD hModelDecalSet = g_pModelsDB->GetRecordLink(m_hModel, "ModelDecalSet");
	if (!hModelDecalSet)
		return;

	HRECORD hDamageType = g_pModelsDB->GetRecordLink(m_hModel, "RagdollDamageType");
	if (!hDamageType)
		return;

	uint32 nCallbacks = 0;

	// Iterate through all the ModelDecalNodes
	HATTRIBUTE hNodes = g_pModelsDB->GetAttribute(hModelDecalSet, "Nodes");
	uint32 iNumNodes = g_pLTDatabase->GetNumValues(hNodes);
	for (uint32 iNode = 0; iNode < iNumNodes; iNode++)
	{
		// Limit number of collision callbacks (for performance and also so we can use a fixed array for the parameters).
		if (nCallbacks >= kMaxRagdollCallbacks)
			break;

		HRECORD hModelDecalNode = g_pModelsDB->GetRecordLink(hNodes, iNode);
		if (!hModelDecalNode)
			continue;

		// Use the "Node" RecordLink to look up the HMODELNODE so we can get its rigidbody (if any).
		HRECORD hNode = g_pModelsDB->GetRecordLink(hModelDecalNode, "Node");
		if (!hNode)
			continue;

		HMODELNODE hModelNode;
		const char* szNodeName = g_pModelsDB->GetNodeName(hNode);
		if (g_pModelLT->GetNode(m_hObject, szNodeName, hModelNode) != LT_OK)
			continue;

		if (hModelNode == INVALID_MODEL_NODE)
			continue;

		HPHYSICSRIGIDBODY hRigidBody;
		if (g_pLTBase->PhysicsSim()->GetModelNodeRigidBody(m_hObject, hModelNode, hRigidBody) != LT_OK)
			continue;

		if (hRigidBody == INVALID_PHYSICS_RIGID_BODY)
			continue;

		// Now find a decal type for the damage type we want applied to ragdolls.
		HATTRIBUTE hDecals = g_pModelsDB->GetAttribute(hModelDecalNode, "Decals");
		uint32 iNumDecals = g_pLTDatabase->GetNumValues(hDecals);
		for (uint32 iDecal = 0; iDecal < iNumDecals; iDecal++)
		{
			if (nCallbacks >= kMaxRagdollCallbacks)
				break;

			HATTRIBUTE hDamageTypes = g_pModelsDB->GetStructAttribute(hDecals, iDecal, "DamageTypes");
			uint32 iNumDamageTypes = g_pLTDatabase->GetNumValues(hDamageTypes);
			for (uint32 iDamageType = 0; iDamageType < iNumDamageTypes; iDamageType++)
			{
				HRECORD hDT = g_pModelsDB->GetRecordLink(hDamageTypes, iDamageType);
				if (hDT != hDamageType)
					continue;

#if 0 //!!ARL: We have no way of knowing the client gore settings on the server.
				// Filter out gore decals in low gore settings
				if (!g_pProfileMgr->GetCurrentProfile()->m_bGore)
				{
					HATTRIBUTE hIsGore = g_pModelsDB->GetStructAttribute(hDecals, iDecal, "IsGore");
					bool bGore = g_pModelsDB->GetBool(hIsGore);
					if (bGore)
						continue;
				}
#endif
				// Finally, setup our params and add our notification for this rigid body.
				HATTRIBUTE hVelocityThreshold = g_pModelsDB->GetStructAttribute(hDecals, iDecal, "VelocityThreshold");
				HATTRIBUTE hNumImpacts = g_pModelsDB->GetStructAttribute(hDecals, iDecal, "DecalLimit");
				HATTRIBUTE hDecalType = g_pModelsDB->GetStructAttribute(hDecals, iDecal, "Decal");

				RAGDOLLCALLBACKSTRUCT* pParms = &m_aRagdollCollisionParms[nCallbacks++];
				pParms->fVelocityThresold = g_pModelsDB->GetFloat(hVelocityThreshold);
				pParms->nNumImpacts = g_pModelsDB->GetInt32(hNumImpacts);
				pParms->hObject = m_hObject;
				pParms->hNode = hModelNode;
				pParms->hDecalType = g_pModelsDB->GetRecordLink(hDecalType);
				pParms->hCollisionNotifier = g_pLTBase->PhysicsSim()->RegisterCollisionNotifier(hRigidBody, RagdollCollisionNotifier, pParms);

				break;	// found a damage type match stop looking for more for this decal entry
			}
		}
		g_pLTBase->PhysicsSim()->ReleaseRigidBody(hRigidBody);
	}
}

	
//----------------------------------------------------------------------------
//              
//	ROUTINE:	CCharacter::RemoveRagdollCollisionNotifications()
//              
//	PURPOSE:	Remove any existing collision notifications (used for RagdollDamage)
//              
//----------------------------------------------------------------------------
void CCharacter::RemoveRagdollCollisionNotifications()
{
	for (uint32 nCallbacks = 0; nCallbacks < kMaxRagdollCallbacks; nCallbacks++)
	{
		if (m_aRagdollCollisionParms[nCallbacks].hCollisionNotifier != INVALID_PHYSICS_COLLISION_NOTIFIER)
		{
			g_pLTServer->PhysicsSim()->ReleaseCollisionNotifier(m_aRagdollCollisionParms[nCallbacks].hCollisionNotifier);
			m_aRagdollCollisionParms[nCallbacks].hCollisionNotifier = INVALID_PHYSICS_COLLISION_NOTIFIER;
		}
	}
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CCharacter::RagdollCollisionNotifier()
//              
//	PURPOSE:	RigidBody callback function for adding decals upon impact while ragdolling.
//              
//----------------------------------------------------------------------------
void CCharacter::RagdollCollisionNotifier(	HPHYSICSRIGIDBODY hBody1, HPHYSICSRIGIDBODY hBody2,
											const LTVector& vCollisionPt, const LTVector& vCollisionNormal,
											float fVelocity, bool& bIgnoreCollision, void* pUser )
{
	CGameServerShell::CServerShellScopeTracker cScopeTracker;

	RAGDOLLCALLBACKSTRUCT* pParms = (RAGDOLLCALLBACKSTRUCT*)pUser;

	if (fVelocity < pParms->fVelocityThresold)
		return;

	if (pParms->nNumImpacts == 0)
		return;

	if (pParms->nNumImpacts > 0)
		pParms->nNumImpacts--;

	g_pGameServerShell->ApplyDecal(pParms->hObject, pParms->hNode, pParms->hDecalType, vCollisionPt, vCollisionNormal);
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
		LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);

		StimulusRecordCreateStruct scs(kStim_CharacterVisible, GetAlignment(), vPos, m_hObject);
		scs.m_dwDynamicPosFlags |= CAIStimulusRecord::kDynamicPos_TrackSource;
		m_eEnemyVisibleStimID = g_pAIStimulusMgr->RegisterStimulus( scs );
		AITRACE( AIShowCharacters, ( m_hObject, "Registered persistent stimulus EnemyVisible = %d", m_eEnemyVisibleStimID ) );
	}
	else if( g_pAIStimulusMgr->StimulusExists( m_eEnemyVisibleStimID ) )
	{
		AITRACE( AIShowCharacters, ( m_hObject, "NOT Registering persistent stimulus EnemyVisible, because StimulusID %d already exists!", m_eEnemyVisibleStimID ) );
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
	return ( m_eEnemyVisibleStimID != kStimID_Unset );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HideCharacter
//
//	PURPOSE:	Hide/Show character.
//
// ----------------------------------------------------------------------- //

void CCharacter::HideCharacter(bool bHide)
{
	if( bHide )
	{
		g_pCommonLT->SetObjectFlags(m_hHitBox, OFT_Flags, 0, FLAG_RAYHIT );
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAG_VISIBLE | FLAG_TOUCH_NOTIFY | FLAG_RAYHIT );
		g_pLTServer->SetObjectShadowLOD( m_hObject, eEngineLOD_Never );

		HideAttachments( true );
		m_Arsenal.HideWeapons( true );

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
		g_pCommonLT->SetObjectFlags(m_hHitBox, OFT_Flags, FLAG_RAYHIT, FLAG_RAYHIT );
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_VISIBLE | FLAG_TOUCH_NOTIFY | FLAG_RAYHIT, FLAG_VISIBLE | FLAG_TOUCH_NOTIFY | FLAG_RAYHIT );		
		g_pLTServer->SetObjectShadowLOD( m_hObject, eEngineLOD_Medium );

		HideAttachments( false );
		m_Arsenal.HideWeapons( false );

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

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_SFX_MESSAGE);
	cMsg.Writeuint8(SFX_CHARACTER_ID);
	cMsg.WriteObject(m_hObject);
	cMsg.WriteBits(CFX_UPDATE_ATTACHMENTS, FNumBitsExclusive<CFX_COUNT>::k_nValue );
	g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);

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
	if ( !m_hModelNodeLastHit ) 
	{
		m_hModelNodeLastHit  = g_pModelsDB->GetSkeletonDefaultHitNode(m_hModelSkeleton);
	}

	const char* szRecoil = NULL;
	if ( HitFromFront(m_damage.GetLastDamageDir()) )
	{
		szRecoil = g_pModelsDB->GetNodeFrontShortRecoilAni( m_hModelNodeLastHit );
	}
	else
	{
		szRecoil = g_pModelsDB->GetNodeBackShortRecoilAni( m_hModelNodeLastHit );
	}

	HMODELANIM hAni;
	if ( !szRecoil || (INVALID_MODEL_ANIM == (hAni = g_pLTServer->GetAnimIndex(m_hObject, (char*)szRecoil))) )
	{
		char szError[256] = {0};
		LTSNPrintF( szError, LTARRAYSIZE(szError), "Could not find short recoil animation %s", szRecoil );
		LTERROR( szError );
		return;
	}

	g_pModelLT->SetWeightSet(m_hObject, m_RecoilAnimTracker, m_hTwitchWeightset);

	// Play the recoil animation...
	g_pModelLT->SetCurAnim(m_hObject, m_RecoilAnimTracker, hAni, true);
	
	g_pModelLT->SetPlaying( m_hObject, m_RecoilAnimTracker, true );

	// Don't loop recoil animations...
	g_pModelLT->SetLooping( m_hObject, m_RecoilAnimTracker, false );

	// This call should not be necessary and should be removed when the SetCurAnim function properly handles
	// calling the same animation twice in a row.
	g_pModelLT->ResetAnim(m_hObject, m_RecoilAnimTracker);
 
	m_bShortRecoiling = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleModelString()
//
//	PURPOSE:	Handles model keyframe strings
//
// ----------------------------------------------------------------------- //

void CCharacter::HandleModelString( ArgList* pArgList, ANIMTRACKERID nTrackerId )
{
	static CParsedMsg::CToken s_cTok_KEY_FOOTSTEP_SOUND( KEY_FOOTSTEP );
	static CParsedMsg::CToken s_cTok_KEY_ATTACH( KEY_ATTACH );
	static CParsedMsg::CToken s_cTok_KEY_DETACH( KEY_DETACH );
	static CParsedMsg::CToken s_cTok_KEY_SET_DIMS( KEY_SET_DIMS );
	static CParsedMsg::CToken s_cTok_KEY_HITBOX_DIMS( KEY_HITBOX_DIMS );
	static CParsedMsg::CToken s_cTok_KEY_HITBOX_OFFSET( KEY_HITBOX_OFFSET );
	static CParsedMsg::CToken s_cTok_KEY_HITBOX_DEFAULT( KEY_HITBOX_DEFAULT );
	static CParsedMsg::CToken s_cTok_COOL_MOVE( KEY_COOL_MOVE );
	static CParsedMsg::CToken s_cTok_PhysicsWeightSet( "SET_WEIGHT_SET" );


	if (!pArgList || !pArgList->argv || pArgList->argc == 0) return;

	char* pKey = pArgList->argv[0];
	if (!pKey) return;

	CParsedMsg::CToken tok( pKey );

	// Only update the footstep info if we are on the ground...(NOTE:  The
	// character footstep sounds are now played on the client. see CCharacterFX
	// if you're interested)

	if ( tok == s_cTok_KEY_FOOTSTEP_SOUND )
	{
		if( (m_bOnGround || IsOnLadder( ) ) && (nTrackerId == m_nFootstepTrackerId) )
		{
			SurfaceType eSurfaceType = m_eStandingOnSurface;

			if( IsOnLadder( ) )
			{
				eSurfaceType = ST_LADDER;
			}

			float fVolume = GetFootstepVolume();

			// Adjust the footstep volume by our stealth modifier...

			fVolume *= GetStealthModifier();
			
			HSURFACE hSurf = g_pSurfaceDB->GetSurface( eSurfaceType );
			LTASSERT(hSurf,"Invalid surface");
			if (hSurf)
			{
				fVolume *= g_pSurfaceDB->GetFloat(hSurf,SrfDB_Srf_fMoveNoiseMod);
				if ( ShowsTracks(hSurf) )
				{
					// If this is a surface that creates footprints, add a footprint to our list
	
					LTVector vFootprintPos;
					g_pLTServer->GetObjectPos(m_hObject, &vFootprintPos);
	
					// Register EnemyFootprintVisible stimulus.
					float fLifeTime = float(g_pSurfaceDB->GetInt32(hSurf,SrfDB_Srf_nFootPrintLifetime )) / 1000.0f;
					StimulusRecordCreateStruct scs( kStim_FootprintVisible, GetAlignment(), vFootprintPos, m_hObject );
					scs.m_flDurationScalar = fLifeTime;
					g_pAIStimulusMgr->RegisterStimulus( scs );
				}
   			}

			// Register EnemyFootstepSound stimulus.
			LTVector vMovementPos;
			g_pLTServer->GetObjectPos(m_hObject, &vMovementPos);

			StimulusRecordCreateStruct scs( kStim_FootstepSound, GetAlignment(), vMovementPos, m_hObject );
			scs.m_flRadiusScalar = fVolume;
			g_pAIStimulusMgr->RegisterStimulus( scs );
		}
	}
	else if ( tok == s_cTok_KEY_ATTACH)
	{
		if( !(pArgList->argc >= 2) || !m_pAttachments )
			return;

		m_pAttachments->Attach(pArgList->argv[1], (pArgList->argc >= 3 ? pArgList->argv[2] : NULL));
	}
	else if ( tok == s_cTok_KEY_DETACH)
	{
		if( !(pArgList->argc >= 2) || !m_pAttachments )
			return;

		m_pAttachments->Detach( pArgList->argv[1], (pArgList->argc >= 3 ? pArgList->argv[2] : NULL) );
	}
	else if ( tok == s_cTok_KEY_SET_DIMS )
	{
		LTERROR( "Trying to set Dims in animation.  Why??" );

		if (pArgList->argc < 2) return;

		// Set up so we can set one or more dims...

        LTVector vDims;
		g_pPhysicsLT->GetObjectDims(m_hObject, &vDims);

		if (pArgList->argv[1])
		{
			vDims.x = (float) atof(pArgList->argv[1]);
		}
		if (pArgList->argc > 2 && pArgList->argv[2])
		{
			vDims.y = (float) atof(pArgList->argv[2]);
		}
		if (pArgList->argc > 3 && pArgList->argv[3])
		{
			vDims.z = (float) atof(pArgList->argv[3]);
		}

		// Set the new dims

		SetDims(&vDims);
	}
	else if( tok == s_cTok_KEY_HITBOX_DIMS )
	{
		if( pArgList->argc > 2 )
		{
			CCharacterHitBox* pHitBox = CCharacterHitBox::DynamicCast(m_hHitBox);
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
	else if( tok == s_cTok_KEY_HITBOX_OFFSET )
	{
		if( pArgList->argc > 3 )
		{
			CCharacterHitBox* pHitBox = CCharacterHitBox::DynamicCast(m_hHitBox);
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
	else if( tok == s_cTok_KEY_HITBOX_DEFAULT )
	{
		CCharacterHitBox* pHitBox = CCharacterHitBox::DynamicCast(m_hHitBox);
		if( pHitBox )
		{
			pHitBox->SetDimsToModel();
		}
	}	
	else if ( tok == s_cTok_COOL_MOVE )
	{
		m_fLastCoolMoveTime = g_pLTServer->GetTime();
	}
	else if (tok == s_cTok_PhysicsWeightSet)
	{
		// Don't allow physics weight set changes if we're dead (as this will mess up
		// rag-dolling)
		if (!m_damage.IsDead())
		{
			// Save the weight set and set it in our update (not during an animation)...
			m_sPhysicsWeightSet = pArgList->argv[1];
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
		SetDeathAnimation( true );
		return;
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetDeathAnimation()
//
//	PURPOSE:	Set animation to death
//
// ----------------------------------------------------------------------- //

void CCharacter::SetDeathAnimation( bool bClampRagdollVelocity )
{
	if (m_bStartedDeath && !m_bDelayRagdollDeath) return;

	StartDeath();
	if(!s_vtRagdollDisable.IsInitted())
		s_vtRagdollDisable.Init(g_pLTServer, "RagdollDisable", NULL, 0.0f);
	if(!s_vtRagdollDelay.IsInitted())
		s_vtRagdollDelay.Init(g_pLTServer, "RagdollDelay", NULL, 1.0f);

	if ((m_bSevered || m_bDeathEffect) && !m_bDelayRagdollDeath && s_vtRagdollDelay.GetFloat(1.0f) > 0.0f )
	{
		m_bDelayRagdollDeath = true;
		return;
	}
	m_bDelayRagdollDeath = false;

	// Choose the appropriate death ani

	m_eDeathType = CD_NORMAL;

	m_eDeathDamageType = m_damage.GetDeathType();

	// Attempt to ragdoll.  If we fail to ragdoll, attempt to set a death 
	// animation as a fallback.  If the ragdolling is successful, attempt 
	// to stick the character to geometry



	if (s_vtRagdollDisable.GetFloat() < 1.0f)
	{
		// Ragdoll our body...
		RagdollDeath( bClampRagdollVelocity );

		if ( m_bIsRagdolling )
		{
			// Determine if the character should stick to world geoemtry...
			if (g_pModelsDB->GetModelCanWallStick(m_hModel))
			{
				m_CharacterPhysics.StickToGeometry( m_hModelNodeLastHit, m_damage.GetDeathDir( ), m_damage.GetDeathAmmo( ));
			}
			

			// Set an animation to play so non-rigidbody nodes aren't in awkward positions...
			if( GetConsoleFloat( "UseAnimOnRagdollDeath", 1.0f ) > 0.0f )
			{
				const char* pszRagdollDeathAni = g_pModelsDB->GetRagdollDeathAnimationName( );
				if( !LTStrEmpty( pszRagdollDeathAni ))
				{
					HMODELANIM hAni = INVALID_MODEL_ANIM;
					g_pModelLT->GetAnimIndex( m_hObject, pszRagdollDeathAni, hAni );

					if( hAni != INVALID_MODEL_ANIM )
					{
						g_pLTServer->GetModelLT()->SetCurAnim( m_hObject, MAIN_TRACKER, hAni, true );
					}
				}
			}
		}
		else
		{
			// Figure out if this was a death from behind or from front

			bool bFront = HitFromFront(m_damage.GetDeathDir());

			// Virtual GetAlternateDeathAnimation gives AIs a chance to choose their
			// own death animation.

			HMODELANIM hAni = GetAlternateDeathAnimation();

			// If no alternate was found, do the normal selection.

			if( hAni == INVALID_MODEL_ANIM )
			{
				hAni = GetDeathAni(bFront);
			}

			// Set the death animation

			if ( hAni != INVALID_MODEL_ANIM )
			{
				// Set model dims based on animation...

				LTVector vDims;
				if( LT_OK == g_pModelLT->GetModelAnimUserDims( m_hObject, hAni, &vDims ))
				{
					SetDims( &vDims );
				}


				// Only set the death animation if we aren't doing ragdoll...

				g_pLTServer->GetModelLT()->SetCurAnim(m_hObject, MAIN_TRACKER, hAni, true);
			}
			else 
			{
				AIASSERT( 0, m_hObject, "No death animation found." );	
			}	
		}
	}
	// No matter what, stop any animation looping.

	g_pLTServer->SetModelLooping(m_hObject, false);

	// Make us nonsolid...

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAG_SOLID);

	// We need to update our physics solid flag.  Since we are set to ~FLAG_SOLID,
	// the engine automatically sets us to physics non-solid.
	uint32 nNumRigidBodies = 0;
	g_pLTServer->PhysicsSim( )->GetNumModelRigidBodies( m_hObject, nNumRigidBodies );
	for( uint32 nIndex = 0; nIndex < nNumRigidBodies; nIndex++ )
	{
		HPHYSICSRIGIDBODY hRigidBody;
		if (LT_OK == g_pLTServer->PhysicsSim( )->GetModelRigidBody( m_hObject, nIndex, hRigidBody ))
		{
			g_pLTServer->PhysicsSim( )->SetRigidBodySolid( hRigidBody, true );
			g_pLTServer->PhysicsSim( )->ReleaseRigidBody(hRigidBody);
		}
	}

	// Handle dead
    HandleDead();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetDeathAni()
//
//	PURPOSE:	Gets the location based death animation
//
// ----------------------------------------------------------------------- //

HMODELANIM CCharacter::GetDeathAni(bool bFront)
{
	HMODELANIM hAni = INVALID_ANI;
	const char* szDeathAni = NULL;

	if ( bFront )
	{
		// Look for a death ani specific to this node

		if ( m_hModelNodeLastHit )
		{
			szDeathAni = g_pModelsDB->GetNodeFrontDeathAni( m_hModelNodeLastHit );
			if ( szDeathAni )
			{
				hAni = g_pLTServer->GetAnimIndex(m_hObject, (char*)szDeathAni);
			}
		}

		// If the given node-specific ani could not be found, just use the default (which better be there)

		if ( hAni == INVALID_ANI )
		{
			DebugCPrint(1, "Character %s is missing animation %s.", GetObjectName(m_hObject), szDeathAni );

			szDeathAni = g_pModelsDB->GetSkeletonDefaultFrontDeathAni(m_hModelSkeleton);
			if ( szDeathAni )
			{
				hAni = g_pLTServer->GetAnimIndex(m_hObject, (char*)szDeathAni);
			}
		}

		if( hAni == INVALID_ANI )
		{
			AIASSERT( 0, m_hObject, "No death animation found." );	
			DebugCPrint(1, "Character %s is missing animation %s.", GetObjectName(m_hObject), szDeathAni );
		}

	}
	else
	{
		// Look for a death ani specific to this node

		if ( m_hModelNodeLastHit )
		{
			szDeathAni = g_pModelsDB->GetNodeBackDeathAni( m_hModelNodeLastHit );
			if ( szDeathAni )
			{
				hAni = g_pLTServer->GetAnimIndex(m_hObject, (char*)szDeathAni);
			}
		}

		// If the given node-specific ani could not be found, just use the default (which better be there)

		if ( hAni == INVALID_ANI )
		{
			DebugCPrint(1, "Character %s is missing animation %s.", GetObjectName(m_hObject), szDeathAni );

			szDeathAni = g_pModelsDB->GetSkeletonDefaultBackDeathAni(m_hModelSkeleton);
			if ( szDeathAni )
			{
				hAni = g_pLTServer->GetAnimIndex(m_hObject, (char*)szDeathAni);
			}
		}

		if( hAni == INVALID_ANI )
		{
			AIASSERT( 0, m_hObject, "No death animation found." );	
			DebugCPrint(1, "Character %s is missing animation %s.", GetObjectName(m_hObject), szDeathAni );
		}
	}

	return hAni;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateNavMeshPosition
//
//	PURPOSE:	Update character's position in the NavMesh
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateNavMeshPosition()
{
	if( g_pAIQuadTree->IsAIQuadTreeInitialized() && IsVisible() )
	{
		LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);

		CAI* pAI = CAI::DynamicCast(m_hObject);

		ENUM_NMPolyID ePoly = g_pAIQuadTree->GetContainingNMPoly( vPos, GetCharTypeMask(), m_eLastNavMeshPoly, pAI );

		if( ePoly != kNMPoly_Invalid )
		{
			if( m_eCurrentNavMeshPoly == kNMPoly_Invalid )
			{
				AITRACE( AIShowNavMesh, ( m_hObject, "Starting in NavMesh poly: %d", ePoly ) );
			}

			int iAIRegion;
			ENUM_AIRegionID eAIRegion;
			CAINavMeshPoly* pPolyLast = g_pAINavMesh->GetNMPoly( m_eLastNavMeshPoly );
			CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( ePoly );

			if( ( m_eLastNavMeshPoly != kNMPoly_Invalid ) && 
				( m_eLastNavMeshPoly != ePoly ) )
			{
				HandleNavMeshPolyExit( m_eLastNavMeshPoly );

				if( pPolyLast && pPolyLast->GetNumAIRegions() )
				{
					for( iAIRegion=0; iAIRegion < pPolyLast->GetNumAIRegions(); ++iAIRegion )
					{
						eAIRegion = pPolyLast->GetAIRegion( iAIRegion );
						if( !pPoly->IsContainedByAIRegion( eAIRegion ) )
						{
							HandleAIRegionExit( eAIRegion );
						}
					}
				}
			}

			if ( m_eLastNavMeshPoly != ePoly )
			{
				HandleNavMeshPolyEnter( ePoly );

				if( pPoly && pPoly->GetNumAIRegions() )
				{
					for( iAIRegion=0; iAIRegion < pPoly->GetNumAIRegions(); ++iAIRegion )
					{
						eAIRegion = pPoly->GetAIRegion( iAIRegion );
						if( !( pPolyLast && pPolyLast->IsContainedByAIRegion( eAIRegion ) ) )
						{
							HandleAIRegionEnter( eAIRegion );
						}
					}
				}
			}

			m_eCurrentNavMeshPoly = ePoly;
			m_eLastNavMeshPoly = m_eCurrentNavMeshPoly;
			m_vLastNavMeshPos = vPos;

	#ifndef _FINAL
			if( IsPlayer(m_hObject) && g_NavMeshDebugTrack.GetFloat(0.0f) == 1.0f )
			{
				g_pLTServer->CPrint("Player in NavMesh poly \"%d\"", ePoly );
			}
	#endif
		}
		else
		{
			m_eCurrentNavMeshPoly = kNMPoly_Invalid;
	#ifndef _FINAL
			if( IsPlayer(m_hObject) && g_NavMeshDebugTrack.GetFloat(0.0f) == 1.0f )
			{
				g_pLTServer->CPrint("Player not in NavMesh");
			}
	#endif
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::UpdateMovement
//
//	PURPOSE:	Update character movement
//
// ----------------------------------------------------------------------- //

void CCharacter::UpdateMovement(bool bUpdatePhysics)
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

void CCharacter::PushCharacter(const LTVector &vPos, float fRadius, float fStartDelay, float fDuration, float fStrength)
{
	AIASSERT(0, m_hObject, "CCharacter::PushCharacter: Currently, only the Player can be pushed.");
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetLadderObject
//
//	PURPOSE:	Update if we're on a ladder
//
// ----------------------------------------------------------------------- //

void CCharacter::SetLadderObject( HOBJECT hLadder )
{
	m_hLadderObject = hLadder;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetSpecialMove
//
//	PURPOSE:	Update if we're playing a special move.
//
// ----------------------------------------------------------------------- //

void CCharacter::SetSpecialMove(bool bSpecialMove)
{
	m_bBodySpecialMove = bSpecialMove;
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
	if( !pBrush )
		return;

    m_bBodyInLiquid = true;
	m_eContainerSurface = ::GetSurfaceType( pBrush->m_hObject );
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
	// See if we are coming out of a liquid...

	if (!m_bBodyInLiquid && m_bBodyWasInLiquid)
	{
        PlayDBSound("SplashJumpOutOf", m_fSoundOuterRadius, false);
	}
	else if (!m_bBodyWasInLiquid && m_bBodyInLiquid)  // or going into
	{
        PlayDBSound("SplashJumpInto", m_fSoundOuterRadius, false);
	}

	if ( m_tmrDialogue.IsStarted() && m_tmrDialogue.IsTimedOut( ))
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

void CCharacter::PlaySound(const char *pSoundName, float fRadius, bool bAttached)
{
    LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	g_pServerSoundMgr->PlaySoundFromPos(vPos, pSoundName, NULL, fRadius, m_eSoundPriority,
		 PLAYSOUND_USEOCCLUSION, SMGR_DEFAULT_VOLUME, 1.0f, -1.0f,
		DEFAULT_SOUND_CLASS, PLAYSOUND_MIX_OBJECTS);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::PlayDBSound
//
//	PURPOSE:	Play the specified sound
//
// ----------------------------------------------------------------------- //

void CCharacter::PlayDBSound(const char *pSoundRecordName, float fRadius, bool bAttached)
{
	HRECORD hSoundRec = g_pSoundDB->GetSoundDBRecord(pSoundRecordName);
	PlayDBSound( hSoundRec, fRadius, bAttached );
}

void CCharacter::PlayDBSound(HRECORD hSoundRec, float fRadius, bool bAttached)
{
	if( !hSoundRec)
		return;

	LTVector vPos;
	g_pLTServer->GetObjectPos(m_hObject, &vPos);
	g_pServerSoundMgr->PlayDBSoundFromPos(vPos, hSoundRec, fRadius, m_eSoundPriority,
		PLAYSOUND_USEOCCLUSION, SMGR_DEFAULT_VOLUME, 1.0f, -1.0f,
		DEFAULT_SOUND_CLASS, PLAYSOUND_MIX_OBJECTS);
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
    bool bLipSync = (CanLipSync() &&  !IsPlayer(m_hObject));

	return bLipSync;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::PlayDialogSound
//
//	PURPOSE:	Play a dialog sound
//
// ----------------------------------------------------------------------- //

void CCharacter::PlayDialogSound(const char* pSound, CharacterSoundType eType/* =CST_DIALOG */, 
								 const char*szIcon /* = NULL */, bool bUseRadioVoice /* = false */  )
{
	if (!pSound || !pSound[0]) return;
	if ((m_damage.IsDead() && eType != CST_DEATH)) return;
	if (eType == CST_AI_SOUND && (sm_cAISnds >= 2)) return;


	// Kill current sound...

	KillDlgSnd();


	// Check if character is supposed to lipsync.  Also, don't do lipsyncing on the 
	// player in singleplayer.
    bool bLipSync = DoLipSyncingOnDlgSound( m_hObject );

	// now get the CCharacter's mix channel

	int32	MixChannel=PLAYSOUND_MIX_SPEECH;

	ModelsDB::HMODEL hModel = GetModel();

	HRECORD hSoundTemplate = g_pModelsDB->GetModelSoundTemplate(hModel);
	if( hSoundTemplate )
	{
		MixChannel = g_pAISoundDB->GetMixChannel(hSoundTemplate);
	}

	//special case for handling death sounds. We can't track these sounds because both the
	// character object and the characterFX object may be destroyed before the sound finishes
	// and we don't want either of these events to kill the sound
	if ((bLipSync || DoDialogueSubtitles()) && (eType == CST_DEATH) )
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_SFX_MESSAGE);
		cMsg.Writeuint8(SFX_CHARACTER_ID);
		cMsg.WriteObject(m_hObject);
		cMsg.WriteBits(CFX_DIALOGUE_MSG, FNumBitsExclusive<CFX_COUNT>::k_nValue );
		cMsg.Writeuint32( IndexFromStringID(pSound) );
		cMsg.Writefloat(m_fSoundOuterRadius);
		cMsg.Writefloat(m_fSoundInnerRadius);
		m_nUniqueDialogueId++;
		cMsg.Writeuint8( m_nUniqueDialogueId );
		cMsg.Writeuint8( eType );
		cMsg.Writeint16( MixChannel );
		cMsg.Writebool( bUseRadioVoice ); // UseRadioSound;
		cMsg.WriteString( szIcon );
		g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);
	}
	else if (bLipSync)
	{
		// Tell the client to play the sound (lip synched)...
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_SFX_MESSAGE);
		cMsg.Writeuint8(SFX_CHARACTER_ID);
		cMsg.WriteObject(m_hObject);
		cMsg.WriteBits(CFX_NODECONTROL_LIP_SYNC, FNumBitsExclusive<CFX_COUNT>::k_nValue );
		cMsg.Writeuint32( IndexFromStringID(pSound) );
		cMsg.Writefloat(m_fSoundOuterRadius);
		cMsg.Writefloat(m_fSoundInnerRadius);
		m_nUniqueDialogueId++;
		cMsg.Writeuint8( m_nUniqueDialogueId );
		cMsg.Writeuint8( eType );
		cMsg.Writeint16( MixChannel );
		cMsg.Writebool(bUseRadioVoice);
		cMsg.WriteString( szIcon );
		g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);

		// Recored who we sent this message to.
		m_PlayerTrackerDialogue.Init( *this );
	}
	else
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_SFX_MESSAGE);
		cMsg.Writeuint8(SFX_CHARACTER_ID);
		cMsg.WriteObject(m_hObject);
		cMsg.WriteBits(CFX_DIALOGUE_MSG, FNumBitsExclusive<CFX_COUNT>::k_nValue );
		cMsg.Writeuint32( IndexFromStringID(pSound) );
		cMsg.Writefloat(m_fSoundOuterRadius);
		cMsg.Writefloat(m_fSoundInnerRadius);
		m_nUniqueDialogueId++;
		cMsg.Writeuint8( m_nUniqueDialogueId );
		cMsg.Writeuint8( eType );
		cMsg.Writeint16( MixChannel );
		cMsg.Writebool( bUseRadioVoice ); // UseRadioSound;
		cMsg.WriteString( szIcon );
		g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);

		// Recored who we sent this message to.
		m_PlayerTrackerDialogue.Init( *this );
	}

	m_eCurDlgSndType = eType;

	if( m_eCurDlgSndType == CST_AI_SOUND )
	{
		sm_cAISnds++;
		AITRACE( AIShowSounds, ( m_hObject, "Incrementing static AISound count: %d", sm_cAISnds ) );
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

	if ( m_eCurDlgSndType == CST_AI_SOUND )
	{
		sm_cAISnds--;
		AITRACE( AIShowSounds, ( m_hObject, "Decrementing static AISound count: %d", sm_cAISnds ) );
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
		cMsg.WriteBits(CFX_NODECONTROL_LIP_SYNC, FNumBitsExclusive<CFX_COUNT>::k_nValue );
		cMsg.Writeuint32((uint32)-1);
		cMsg.Writefloat(0.0f);
		cMsg.Writefloat(0.0f);
		cMsg.Writeuint8( m_nUniqueDialogueId );
		cMsg.Writeuint8( m_eCurDlgSndType );
		cMsg.Writeint16( 0 ); // mixchannel
		cMsg.Writebool(false);
		cMsg.WriteString("");
		g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);
	}
	else if (DoDialogueSubtitles())
	{
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_SFX_MESSAGE);
		cMsg.Writeuint8(SFX_CHARACTER_ID);
		cMsg.WriteObject(m_hObject);
		cMsg.WriteBits(CFX_DIALOGUE_MSG, FNumBitsExclusive<CFX_COUNT>::k_nValue );
		cMsg.Writeuint32((uint32)-1);
		cMsg.Writefloat(0.0f);
		cMsg.Writefloat(0.0f);
		cMsg.Writeuint8( m_nUniqueDialogueId );
		cMsg.Writeuint8( m_eCurDlgSndType );
		cMsg.Writeint16( 0 ); // mixchannel
		cMsg.Writebool( false ); // UseRadioSound;
		cMsg.WriteString("");
		g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::PlayDialogue
//
//	PURPOSE:	Do dialogue sound/window
//
// ----------------------------------------------------------------------- //

bool CCharacter::PlayDialogue( char *szDialogue, const char*szIcon /* = NULL  */,
								bool bUseRadioVoice /* = false */ )
{
    if( !szDialogue ) 
		return false;

	PlayDialogSound( szDialogue, CST_DIALOG, szIcon , bUseRadioVoice);
	
    return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::StopDialogue
//
//	PURPOSE:	Stop the dialogue
//
// ----------------------------------------------------------------------- //

void CCharacter::StopDialogue(bool bCinematicDone)
{
	KillDialogueSound();
    m_bPlayingTextDialogue = false;
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

	if (m_bGibbed)
	{
		PlayDBSound( g_pModelsDB->GetModelDeathGibSnd(m_hModel), m_fSoundOuterRadius, true);
	}
	else if (m_bDecapitated)
	{
		PlayDBSound( g_pModelsDB->GetModelDeathDecapitateSnd(m_hModel), m_fSoundOuterRadius, true);
	}
	else
	{
    PlayDialogSound(GetDeathSound(), CST_DEATH);
}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleDead()
//
//	PURPOSE:	Okay, death animation is done
//
// ----------------------------------------------------------------------- //

void CCharacter::HandleDead()
{
	//check for special case of holding detonator in hand when dying...
	if (m_Arsenal.GetCurWeaponRecord() == g_pWeaponDB->GetDetonatorRecord()) 
	{
		DetonateRemoteCharges();
	}
	else
	{
		// Remove any undetonated remotes from the world if the detonator is not out..
		RemoveRemoteCharges( );
	}

	//detonate any remotes stuck to use when we die
	ObjRefNotifierList::iterator iter = m_AttachedRemoteCharges.begin();
	while (iter != m_AttachedRemoteCharges.end()) 
	{
		HOBJECT hObj = *iter;
		if( hObj )
		{
			CRemoteCharge* pRC = dynamic_cast< CRemoteCharge* >( g_pLTServer->HandleToObject( hObj ) );
			if (pRC)
			{
				pRC->RemoteDetonate();
			}
		}
		++iter;
	}

	// The player handles its drops in its own state machine.
	if( !IsPlayer( m_hObject ))
	{
		// Some characters continue firing while dying.
		// Don't drop the weapon until we've fired off a few rounds.
		if( m_bFireWeaponDuringDeath )
		{
			// If the character's weapon falls below the bottom
			// quarter of his dims, drop it to minimize the odds
			// of the weapon clipping into the floor.

			LTVector vPos;
			g_pLTServer->GetObjectPos( m_hObject, &vPos );

			LTVector vDims;
			g_pPhysicsLT->GetObjectDims(m_hObject, &vDims);

			m_fDropWeaponDuringDeathHeight = vPos.y - ( vDims.y * 0.5f );
		}
		else
		{
			//drop our stuff
			DropWeapons();
		}

		SpawnGearItemsOnDeath( );
	}
	
	if( !m_bMakeBody )
	{
		g_pLTServer->RemoveObject( m_hObject );
		return;
	}

	//we're dead, can't hurt us anymore
	m_damage.SetCanDamage(false);

	// Make sure model doesn't slide all over the place...
	g_pPhysicsLT->SetFrictionCoefficient(m_hObject, 500.0f);

	//update our flags
	uint32 dwFlagsOn = FLAG_TOUCH_NOTIFY | FLAG_RAYHIT;
	uint32 dwFlagsOff = FLAG_SOLID | FLAG_GRAVITY;
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, dwFlagsOn, (dwFlagsOn | dwFlagsOff));
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_AI_CLIENT_SOLID );

	// Add this instance to a list of all bodies. Don't add permanent bodies, because we don't want them capped.
	if (!m_bPermanentBody && !m_bIsOnBodiesList)
	{
		m_bIsOnBodiesList = true;
		m_lstBodies.push_back( this );
	}

	// Keep the number of bodies low in a given area.
	CapNumberOfBodies( );

	//set up things that need a to wait for the next update
	m_bFirstDeathUpdate = true;

	if (m_hHitBox)
	{
		LTVector vDims;
		g_pPhysicsLT->GetObjectDims( m_hObject, &vDims );

		//we need to make it large enough to contain the ragdolled body regardless of it's contortions...
		float fMax = LTMAX(vDims.x,vDims.y);
		vDims.Init(fMax,fMax,fMax);

		g_pPhysicsLT->SetObjectDims( m_hHitBox, &vDims, 0 );
	}

	// Send the dead message
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_SFX_MESSAGE);
	cMsg.Writeuint8(SFX_CHARACTER_ID);
	cMsg.WriteObject(m_hObject);
	cMsg.WriteBits(CFX_DEAD, FNumBitsExclusive<CFX_COUNT>::k_nValue );
	cMsg.WriteBits(m_eDeathDamageType, FNumBitsExclusive<kNumDamageTypes>::k_nValue );
	cMsg.WriteDatabaseRecord( g_pLTDatabase, m_damage.GetDeathAmmo( ));
	g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);

	// Update the SFX
	CreateSpecialFX();

}




// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetDims()
//
//	PURPOSE:	Set the dims for the character
//
// ----------------------------------------------------------------------- //

bool CCharacter::SetDims(LTVector* pvDims, bool bSetLargest)
{
	if (!pvDims) return false;

	bool bRet = true;

	// Calculate what the dims should be based on our model size...

	LTVector vNewDims = *pvDims * 1.0f;


    LTVector vOldDims;
	g_pPhysicsLT->GetObjectDims(m_hObject, &vOldDims);


	// Only update dims if they have changed...

	if ((vNewDims.x > vOldDims.x - DIMS_EPSILON && vNewDims.x < vOldDims.x + DIMS_EPSILON) &&
		(vNewDims.y > vOldDims.y - DIMS_EPSILON && vNewDims.y < vOldDims.y + DIMS_EPSILON) &&
		(vNewDims.z > vOldDims.z - DIMS_EPSILON && vNewDims.z < vOldDims.z + DIMS_EPSILON))
	{
		return true;  // Setting of dims didn't actually fail
	}


	// Try to set our new dims...

    if( g_pPhysicsLT->SetObjectDims( m_hObject, &vNewDims, SETDIMS_PUSHOBJECTS ) != LT_OK )
	{
		if( bSetLargest )
		{
			g_pPhysicsLT->SetObjectDims( m_hObject, &vNewDims, 0 );
		}

		bRet = false; // Didn't set to new dims...
	}

	LTVector vPos;

	// move to floor if necessary, and only if the client says we should be on the ground.  Note that
	// we must not move to floor if the client says it is currently in the air, since that would
	// cause the server's position for the object to get out-of-sync with the client
	if( m_bMoveToFloor && m_bOnGround )
	{
		// [RP] 2/25/03 - MoveObjectToFloor() teleports the object to the floor.  We care about bounding box
		//				  collisions here so this block of code is exactly the same as MoveObjectToFloor() with 
		//				  the exception of calling MoveObject() rather than SetObjectPos().

		LTVector vDims;
		g_pLTServer->GetObjectPos( m_hObject, &vPos );
		g_pPhysicsLT->GetObjectDims( m_hObject, &vDims );

		IntersectQuery IQuery;
		IntersectInfo IInfo;

		IQuery.m_From	= vPos;
		IQuery.m_To		= vPos + LTVector(0.0f, -10000.0f, 0.0f);
		IQuery.m_Flags	= IGNORE_NONSOLID | INTERSECT_OBJECTS | INTERSECT_HPOLY;
		
		if( g_pLTServer->IntersectSegment( IQuery, &IInfo ))
		{
			float fDist = vPos.y - IInfo.m_Point.y;
			if( fDist > vDims.y )
			{
				vPos.y -= (fDist - (vDims.y + 0.1f));
				g_pLTServer->Physics()->MoveObject(m_hObject, vPos, 0);
			}
		}
	}

	// This forces the client to move to the server's position because it's teleporting.
	// Note: This is not done in multiplayer, since it will cause a teleport on all clients,
	// so instead we move toward the position normally on each client.
	if (!IsMultiplayerGameServer())
	{
		g_pLTServer->ForceCurrentObjectPos( m_hObject );
	}

	// Update the dims of our hit box...
	if (m_hHitBox)
	{
		CCharacterHitBox *pHitBox = CCharacterHitBox::DynamicCast(m_hHitBox);
		if( pHitBox )
		{
			pHitBox->EnlargeDims( vNewDims );
			pHitBox->SetDims( vNewDims );
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

float CCharacter::GetRadius()
{
	LTVector vDims;
	g_pPhysicsLT->GetObjectDims(m_hObject, &vDims);
    return LTMAX(vDims.x, vDims.z);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetViewTransform()
//
//	PURPOSE:	Get the direction the character is looking.
//
// ----------------------------------------------------------------------- //

void CCharacter::GetViewTransform( LTRigidTransform &tfView ) const
{
	// NOTE: Eventually the AI should return the head node's forward,
	//       and the Player should return the camera's forward.
	g_pLTServer->GetObjectTransform( m_hObject, &tfView );
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

	theStruct.m_Pos = vPos;
	theStruct.m_UserData = ( uint32 )m_hObject;
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

	CCharacterHitBox* pHitBox = CCharacterHitBox::DynamicCast(m_hHitBox);
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

	CCharacterHitBox* pHitBox = CCharacterHitBox::DynamicCast( m_hHitBox );
	if (pHitBox)
	{
		vHitOffset = pHitBox->GetOffset();
	}

	// compare to the last sent dims and offset to see if we should send them again
	if ((vHitDims != m_vLastHitBoxDims) || (vHitOffset != m_vLastHitBoxOffset))
	{
		// Send the hitbox message
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_SFX_MESSAGE);
		cMsg.Writeuint8(SFX_CHARACTER_ID);
		cMsg.WriteObject(m_hObject);
		cMsg.WriteBits(CFX_HITBOX_MSG, FNumBitsExclusive<CFX_COUNT>::k_nValue );

		bool bUseDefaultHitboxDims = false;
		if( pHitBox )
		{
			LTVector vDims;
			g_pPhysicsLT->GetObjectDims( m_hObject, &vDims );
			pHitBox->EnlargeDims( vDims );
			if( vHitDims == vDims )
				bUseDefaultHitboxDims = true;
		}
		cMsg.Writebool( !bUseDefaultHitboxDims );
		if( !bUseDefaultHitboxDims )
		{
		cMsg.WriteCompLTVector( vHitDims );
		}

		bool bSendHitBoxOffset = vHitOffset != LTVector::GetIdentity();
		cMsg.Writebool( bSendHitBoxOffset );
		if( bSendHitBoxOffset )
		{
		cMsg.WriteCompLTVector( vHitOffset );
		}

		g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);

		// update last sent vectors
		m_vLastHitBoxDims   = vHitDims;
		m_vLastHitBoxOffset = vHitOffset;

	// Update the SFX
	CreateSpecialFX();
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

    m_bStartedDeath = true;

	// Spawn any special item we were instructed to

	if( !m_sSpawnItem.empty() )
	{
		// Add gravity to the item...

		char buf[300];
		LTSNPrintF( buf, LTARRAYSIZE( buf ), "%s Gravity 1;Placed 0", m_sSpawnItem.c_str() );

        LTVector vPos;
		g_pLTServer->GetObjectPos(m_hObject, &vPos);

		LPBASECLASS pObj = SpawnObject(&buf[0], vPos, LTRotation());

		if (pObj && pObj->m_hObject)
		{
			LTVector vAccel(GetRandom(0.0f, 300.0f), GetRandom(100.0f, 200.0f), GetRandom(0.0f, 300.0f));
			g_pPhysicsLT->SetAcceleration(pObj->m_hObject, vAccel);

			LTVector vVel(GetRandom(0.0f, 100.0f), GetRandom(200.0f, 400.0f), GetRandom(0.0f, 100.0f));
			g_pPhysicsLT->SetVelocity(pObj->m_hObject, vVel);
		}
	}


	//check for gibbing
	bool bForceGib = g_pModelsDB->AlwaysGib(m_hModel);
	if ( sm_nGibCounter > 0 && !bForceGib)
	{
		sm_nGibCounter--;
	}
	else if (g_pModelsDB->CanGib(m_hModel) && (bForceGib || m_DamageTracker.CheckForGib()) )
	{
		m_bGibbed = true;
		HandleGib();
	}
	
	bool bDeathFX = false;
	if (!m_bGibbed)
	{
		bDeathFX = HandleDeathFX();
	}

	uint32 n = m_lstSeverBodies.size();
	//check for severing
	if ( sm_nSeverCounter > 0)
	{
		sm_nSeverCounter--;
	}
	else if (	!m_bGibbed && 
				!bDeathFX &&
				m_lstSeverBodies.size() < g_pServerDB->GetSeverTotalCap() &&
				!IsKnockedDown()
				) 
	{
		ModelsDB::HSEVERBODY hBody = g_pModelsDB->GetSeverBodyRecord(m_hModel);
		if (hBody) 
		{
			m_DamageTracker.PrepareForSeverChecks(hBody);
			ModelsDB::HPieceArray s_Pieces;
			s_Pieces.reserve(HL_NUM_LOCS);

			if (m_DamageTracker.CheckForSever(hBody,IsCrouched(),s_Pieces)) 
			{
				m_bSevered = true;

				PrepareToSever();
				//set frequency cap so we don't get another right away...
				sm_nSeverCounter = g_pServerDB->GetSeverFrequencyCap();

				// no cap for testing...
				if (g_vtBodySeverTest.GetFloat() > 0.0f)
				{
					sm_nSeverCounter = 0;
				}

				if( !m_bIsOnSeveredList )
				{
					m_bIsOnSeveredList = true;
					m_lstSeverBodies.push_back( this );
				}

				ModelsDB::HPieceArray::iterator iter = s_Pieces.begin();
				while (iter != s_Pieces.end()) 
				{
					HandleSever(*iter);
					iter++;
				}
				
			}
		}
	}

	//play the death sound after gibbing and severing have been determined
	PlayDeathSound();
	
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

	uint32 nIgnoreFlags = (CC_DYNAMIC_SECTOR_VOLUME_FLAG | CC_PLAYER_IGNORE_FLAGS);
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
	vOffset.Init();

    LTVector vDims;
	g_pPhysicsLT->GetObjectDims(m_hObject, &vDims);

	// Just make the default offset a bit above the waist...

	vOffset.y = vDims.y * 0.75f;

	return vOffset;
}

//----------------------------------------------------------------------------
//
//	ROUTINE:	CCharacter::HandleNavMeshPoly*()
//
//	PURPOSE:	Handle entering and exiting NavMeshPolys.
//
//----------------------------------------------------------------------------

void CCharacter::HandleNavMeshPolyEnter( ENUM_NMPolyID ePoly )
{
	// Character is not in the NavMesh.

	if( ePoly == kNMPoly_Invalid )
	{
		m_eCurrentNavMeshLink = kNMLink_Invalid;
		return;
}

	// Poly does not exist.

	CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( ePoly );
	if( !pPoly )
{
		m_eCurrentNavMeshLink = kNMLink_Invalid;
		return;
}

	// Record the link.

	m_eCurrentNavMeshLink = pPoly->GetNMLinkID();
}

void CCharacter::HandleNavMeshPolyExit( ENUM_NMPolyID ePoly )
{
	// Clear the link.

	m_eCurrentNavMeshLink = kNMLink_Invalid;
} 

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CCharacter::Has*NavMesh*()
//              
//	PURPOSE:	Return true if AI has a valid NavMesh poly.
//              
//----------------------------------------------------------------------------

bool CCharacter::HasLastNavMeshPoly() const
{
	return ( m_eLastNavMeshPoly != kNMPoly_Invalid ); 
}

bool CCharacter::HasCurrentNavMeshPoly() const
{
	return ( m_eCurrentNavMeshPoly != kNMPoly_Invalid ); 
}

bool CCharacter::HasCurrentNavMeshLink() const
{
	return ( m_eCurrentNavMeshLink != kNMLink_Invalid ); 
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
#ifdef PLATFORM_XENON	// JPW saves on Xenon will need to be signed so this will need to be rewritten before ship -- saving to the DVD causes a crash tho, so for now just return
	return;
#endif

	if (!pMsg) return;

    m_cs.Write(pMsg);

	SAVE_HOBJECT(m_hHitBox);

	SAVE_bool(m_bMoveToFloor);
	SAVE_BYTE(m_eDeathType);
	SAVE_BOOL(m_bStartedDeath);
	SAVE_BYTE(m_eStandingOnSurface);
	SAVE_BYTE(m_eContainerSurface);
	SAVE_BOOL(m_bOnGround);
	SAVE_BYTE(m_eContainerCode);
	SAVE_BOOL(m_bBodyInLiquid);
	SAVE_BOOL(m_bBodyWasInLiquid);
	SAVE_BOOL(m_bBodySpecialMove);
	SAVE_HRECORD(m_hModelNodeLastHit);
	SAVE_HRECORD(m_hModel);
	SAVE_HRECORD(m_hModelSkeleton);
	SAVE_STDSTRING( m_sSpawnItem );
	SAVE_FLOAT(m_fSoundOuterRadius);
	SAVE_FLOAT(m_fSoundInnerRadius);
	SAVE_INT(m_eSoundPriority);
	SAVE_FLOAT(m_fBaseMoveAccel);
	SAVE_FLOAT(m_fLadderVel);
	SAVE_FLOAT(m_fSwimVel);
	SAVE_FLOAT(m_fRunVel);
	SAVE_FLOAT(m_fWalkVel);
	SAVE_FLOAT(m_fJumpVel);
	SAVE_FLOAT(m_fFallVel);
	SAVE_FLOAT(m_fCrawlVel);


	pMsg->WriteString( m_pAIAttributes ? m_pAIAttributes->strName.c_str() : "");

	// Save the string, because indices may change if alignment records are added.

	std::string strAlignment = g_pCharacterDB->Alignment2String( m_eAlignment );
	SAVE_STDSTRING( strAlignment );

	SAVE_BYTE(m_ccCrosshair);
	SAVE_DWORD(m_dwFlags);
	SAVE_FLOAT(m_fLastPainVolume);

	SAVE_DWORD(m_eLastNavMeshPoly);
	SAVE_DWORD(m_eCurrentNavMeshPoly);
	SAVE_DWORD(m_eCurrentNavMeshLink);

	SAVE_VECTOR(m_vLastNavMeshPos);

	SAVE_TIME(m_fLastCoolMoveTime);

	SAVE_BYTE(m_byFXFlags);
	SAVE_BOOL(m_bShortRecoiling);
   
	SAVE_DWORD(m_cSpears);
	for ( uint32 iSpear = 0 ; iSpear < m_cSpears ; iSpear++ )
	{
		SAVE_HOBJECT(m_aSpears[iSpear].hObject);
		SAVE_HRECORD(m_aSpears[iSpear].hModelNode);
		SAVE_ROTATION(m_aSpears[iSpear].rRot);
	}

	SAVE_INT(m_cActive);
	SAVE_TYPE( m_nDamageFlags );

	m_Keys.Save(pMsg);

	SAVE_bool(m_bMakeBody);
	SAVE_bool(m_bPermanentBody);

	SAVE_DWORD( m_eTeleportTriggerState );
	SAVE_VECTOR( m_vTeleportPos );
	SAVE_HOBJECT( m_hTeleportPoint );
	
	SAVE_BYTE( m_eDeathDamageType );

	SAVE_DWORD( m_eEnemyVisibleStimID );
	SAVE_DWORD( m_eFlashlightStimID );
	SAVE_TIME( m_fNextFlashlightBeamPosUpdateTime );

	SAVE_TIME( m_flBlockWindowEndTime );
	SAVE_TIME( m_flDodgeWindowEndTime );

	SAVE_BOOL( m_bFirstDeathUpdate);
	SAVE_FLOAT( m_fBodyLifetime);
	m_tmrLifetime.Save(*pMsg);
	m_tmrBodyFade.Save(*pMsg);
	SAVE_DWORD( m_eDeathStimID);
	SAVE_bool( m_bFireWeaponDuringDeath );
	SAVE_FLOAT( m_fDropWeaponDuringDeathHeight );

	SAVE_bool( m_bGibbed );
	SAVE_bool( m_bSevered );
	SAVE_bool( m_bDeathEffect );
	SAVE_bool( m_bDecapitated );
	SAVE_bool( m_bIsSolidToAI );

	SAVE_STDSTRING( m_sPhysicsWeightSet );

	uint32 nUserFlags = 0;
	g_pLTServer->Common( )->GetObjectFlags( m_hObject, OFT_User, nUserFlags );
	HRECORD hRecord = UserFlagToCollisionPropertyRecord( nUserFlags );
	SAVE_CHARSTRING( g_pLTDatabase->GetRecordName( hRecord ));


	SAVE_DWORD(m_ActiveRemoteCharges.size());
	ObjRefNotifierList::iterator iter = m_ActiveRemoteCharges.begin();
	while (iter != m_ActiveRemoteCharges.end()) 
	{
		HOBJECT hObj = *iter;
		SAVE_HOBJECT(hObj);
		++iter;
	}

	SAVE_DWORD(m_AttachedRemoteCharges.size());
	iter = m_AttachedRemoteCharges.begin();
	while (iter != m_AttachedRemoteCharges.end()) 
	{
		HOBJECT hObj = *iter;
		SAVE_HOBJECT(hObj);
		++iter;
	}

	SAVE_HOBJECT( m_hLadderObject );
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

	m_cs.hServerObj = m_hObject;
    m_cs.Read(pMsg);

	LOAD_HOBJECT(m_hHitBox);
	LOAD_bool(m_bMoveToFloor);
	LOAD_BYTE_CAST(m_eDeathType, CharacterDeath);
	LOAD_BOOL(m_bStartedDeath);
	LOAD_BYTE_CAST(m_eStandingOnSurface, SurfaceType);
	LOAD_BYTE_CAST(m_eContainerSurface, SurfaceType);
	LOAD_BOOL(m_bOnGround);
	LOAD_BYTE_CAST(m_eContainerCode, ContainerCode);
	LOAD_BOOL(m_bBodyInLiquid);
	LOAD_BOOL(m_bBodyWasInLiquid);
	LOAD_BOOL(m_bBodySpecialMove);
	LOAD_HRECORD(m_hModelNodeLastHit, g_pModelsDB->GetNodesCategory());
	LOAD_HRECORD(m_hModel, g_pModelsDB->GetModelsCategory());
	LOAD_HRECORD(m_hModelSkeleton, g_pModelsDB->GetSkeletonCategory());
	LOAD_STDSTRING( m_sSpawnItem );
	LOAD_FLOAT(m_fSoundOuterRadius);
	LOAD_FLOAT(m_fSoundInnerRadius);
	LOAD_INT_CAST(m_eSoundPriority, SoundPriority);
	LOAD_FLOAT(m_fBaseMoveAccel);
	LOAD_FLOAT(m_fLadderVel);
	LOAD_FLOAT(m_fSwimVel);
	LOAD_FLOAT(m_fRunVel);
	LOAD_FLOAT(m_fWalkVel);
	LOAD_FLOAT(m_fJumpVel);
	LOAD_FLOAT(m_fFallVel);
	LOAD_FLOAT(m_fCrawlVel);

	// Set the attributes, could be "" if we didn't have an aiattributes.
	char szName[128];
	pMsg->ReadString(szName, LTARRAYSIZE(szName));
	if(szName[0])
		SetAIAttributes(szName);

	// Load the string, because indices may change if alignment records are added.

	std::string strAlignment;
	LOAD_STDSTRING( strAlignment );
	m_eAlignment = g_pCharacterDB->String2Alignment( strAlignment.c_str() );

	LOAD_BYTE_CAST(m_ccCrosshair, EnumCharacterStance);
	LOAD_DWORD(m_dwFlags);
	LOAD_FLOAT(m_fLastPainVolume);

	LOAD_DWORD_CAST(m_eLastNavMeshPoly, ENUM_NMPolyID);
	LOAD_DWORD_CAST(m_eCurrentNavMeshPoly, ENUM_NMPolyID);
	LOAD_DWORD_CAST(m_eCurrentNavMeshLink, ENUM_NMLinkID);

	// If we're loading from a transition, then our positional information is invalid.
	if( g_pGameServerShell->GetLGFlags( ) == LOAD_TRANSITION )
	{
		m_eLastNavMeshPoly		= kNMPoly_Invalid;
		m_eCurrentNavMeshPoly	= kNMPoly_Invalid;
	}

	LOAD_VECTOR(m_vLastNavMeshPos);

	LOAD_TIME(m_fLastCoolMoveTime);

	LOAD_BYTE(m_byFXFlags);
	LOAD_BOOL(m_bShortRecoiling);

	LOAD_DWORD(m_cSpears);
	for ( uint32 iSpear = 0 ; iSpear < m_cSpears ; iSpear++ )
	{
		LOAD_HOBJECT(m_aSpears[iSpear].hObject);
		LOAD_HRECORD(m_aSpears[iSpear].hModelNode, g_pModelsDB->GetNodesCategory());
		LOAD_ROTATION(m_aSpears[iSpear].rRot);
	}

	LOAD_INT(m_cActive);
	LOAD_TYPE( m_nDamageFlags );

	m_Keys.Load( pMsg );

	LOAD_bool(m_bMakeBody);
	LOAD_bool(m_bPermanentBody);

	LOAD_DWORD_CAST(m_eTeleportTriggerState, TeleportTriggerStates);
	LOAD_VECTOR(m_vTeleportPos);
	LOAD_HOBJECT(m_hTeleportPoint);

	LOAD_BYTE_CAST( m_eDeathDamageType, DamageType );

	LOAD_DWORD_CAST( m_eEnemyVisibleStimID, EnumAIStimulusID );
	LOAD_DWORD_CAST( m_eFlashlightStimID, EnumAIStimulusID );
	LOAD_TIME( m_fNextFlashlightBeamPosUpdateTime );

	LOAD_TIME( m_flBlockWindowEndTime );
	LOAD_TIME( m_flDodgeWindowEndTime );

	LOAD_BOOL( m_bFirstDeathUpdate);
	LOAD_FLOAT( m_fBodyLifetime);
	m_tmrLifetime.Load(*pMsg);
	m_tmrBodyFade.Load(*pMsg);

	LOAD_DWORD_CAST( m_eDeathStimID, EnumAIStimulusID );
	LOAD_bool( m_bFireWeaponDuringDeath );
	LOAD_FLOAT( m_fDropWeaponDuringDeathHeight );

	LOAD_bool(m_bGibbed);
	LOAD_bool(m_bSevered);
	LOAD_bool(m_bDeathEffect);
	LOAD_bool(m_bDecapitated);
	LOAD_bool( m_bIsSolidToAI );

	LOAD_STDSTRING( m_sPhysicsWeightSet );

	if (m_damage.IsDead() && m_bStartedDeath )
	{
		m_bIsOnBodiesList = true;
		m_lstBodies.push_back(this);

		if (m_bSevered)
		{
			m_bIsOnSeveredList = true;
			m_lstSeverBodies.push_back( this );
		}
	}

	char szCollisionPropertyRecordName[256];
	LOAD_CHARSTRING( szCollisionPropertyRecordName, LTARRAYSIZE( szCollisionPropertyRecordName ));
	HRECORD hCollisionProperty = g_pLTDatabase->GetRecord( DATABASE_CATEGORY( CollisionProperty ).GetCategory(), szCollisionPropertyRecordName );
	uint32 nUserFlags = CollisionPropertyRecordToUserFlag( hCollisionProperty );
	g_pLTServer->Common( )->SetObjectFlags( m_hObject, OFT_User, nUserFlags, USRFLG_COLLISIONPROPMASK );

	uint32 nNumCharges = 0;
	LOAD_DWORD(nNumCharges);
	m_ActiveRemoteCharges.clear();
	for (uint8 c = 0; c < nNumCharges; ++c)
	{
		LTObjRefNotifier ref( *this );
		HOBJECT hRemote;
		LOAD_HOBJECT(hRemote);

		ref = hRemote;
		m_ActiveRemoteCharges.push_back(ref);
	}

	LOAD_DWORD(nNumCharges);
	m_AttachedRemoteCharges.clear();
	for (uint8 c = 0; c < nNumCharges; ++c)
	{
		LTObjRefNotifier ref( *this );
		HOBJECT hRemote;
		LOAD_HOBJECT(hRemote);

		ref = hRemote;
		m_AttachedRemoteCharges.push_back(ref);
	}

	LOAD_HOBJECT( m_hLadderObject );
	m_hLoadLadderObject = m_hLadderObject;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CCharacter::SetAIAttributes()
//              
//	PURPOSE:	Sets the Attributes record.
//              
//----------------------------------------------------------------------------

bool CCharacter::SetAIAttributes( const char* pszName )
{
	AIASSERT( m_pAIAttributes == NULL, m_hObject, "CCharacter::SetAIAttributes: AIAttributes already set." );
	AIASSERT( pszName != NULL, m_hObject, "CCharacter::SetAIAttributes: Setting AIAttributes to NULL template." );
	AIASSERT( g_pAIDB != NULL, m_hObject, "CCharacter::SetAIAttributes: Setting AIAttributes to NULL template." );
	if( !( pszName && g_pAIDB ) )
	{
		return false;
	}

	uint32 nRecord = g_pAIDB->GetAIAttributesRecordID( pszName ); 
	AIASSERT( nRecord >= 0 && nRecord < g_pAIDB->GetNumAIAttributesRecords(), m_hObject, "Record out of bounds" );

	m_pAIAttributes = g_pAIDB->GetAIAttributesRecord( nRecord );
	AIASSERT( m_pAIAttributes != NULL, m_hObject, "CCharacter::SetAIAttributes: m_pAIAttributes is NULL" );

	// Setup a damage mask.

	if( m_pAIAttributes && 
		m_pAIAttributes->eDamageMaskID != kAIDamageMaskID_Invalid )
	{
		CDestructible* pDestructable = GetDestructible();
		AIASSERT( pDestructable, m_hObject, "CCharacter::SetAIAttributes: No destructable." );
		if( pDestructable )
		{
			DamageFlags df = g_pAIDB->GetAIDamageMaskRecord( m_pAIAttributes->eDamageMaskID )->dfDamageTypes;
			pDestructable->SetCantDamageFlags( (df == 0 ? 0 : ~df) );
		}
	}

	return true;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CCharacter::GetAIAttributes()
//              
//	PURPOSE:	Returns a pointer to the attributes record.
//              
//----------------------------------------------------------------------------

const AIDB_AttributesRecord* const CCharacter::GetAIAttributes() const
{
	return m_pAIAttributes;
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CCharacter::GetCharTypeMask()
//              
//	PURPOSE:	Return the char type bitmask that includes the character type.
//              
//----------------------------------------------------------------------------

uint32 CCharacter::GetCharTypeMask() const
{
	if( m_pAIAttributes )
	{
		return ( 1 << m_pAIAttributes->eAIAttributesID );
	}

	return 0;
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
        m_pAttachments = NULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HideAttachments
//
//	PURPOSE:	Hide/Show our attachments.
//
// ----------------------------------------------------------------------- //

void CCharacter::HideAttachments(bool bHide)
{
	if ( m_pAttachments )
	{
		m_pAttachments->HideAttachments(bHide);
	}

	m_Arsenal.HideWeapons( bHide );
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

	CAttachments* pAtt = NULL;
	if (m_pAttachments && bRemove)
	{
		pAtt = m_pAttachments;
		RemoveAggregate(m_pAttachments);
        m_pAttachments = NULL;
	}
	else if( m_pAttachments )
	{
		// If we are not removing our own attachments then we must create a new one.

		pAtt = CAttachments::Create();
	}
	else
	{
		ASSERT( !"Transfering NULL Attachments" );
	}

	return pAtt;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::RemoveWeapon
//
//	PURPOSE:	Handles removing the passed in weapon from the 
//			character.
//
// ----------------------------------------------------------------------- //

void CCharacter::RemoveWeapon( CActiveWeapon* pActiveWeapon )
{
	m_Arsenal.RemoveActiveWeapon( pActiveWeapon );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HitFromFront
//
//	PURPOSE:	Tells whether the vector is coming at us from the front
//				or back assuming it passes through us.
//
// ----------------------------------------------------------------------- //

bool CCharacter::HitFromFront(const LTVector& vDir)
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

float CCharacter::ComputeDamageModifier( ModelsDB::HNODE hModelNode)
{
	if ( !hModelNode )
	{
		return 1.0f;
	}

	return g_pModelsDB->GetNodeDamageFactor( hModelNode );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::AddSpear
//
//	PURPOSE:	Stick a spear into us
//
// ----------------------------------------------------------------------- //

bool CCharacter::AddSpear(HOBJECT hSpear, ModelsDB::HNODE hModelNode, const LTRotation& rRot, bool bCanWallStick )
{
	if ( m_cSpears < kMaxSpears )
	{
		// Check the node to see if it can attach a spear
		// If we are dead the spear should always attach.
		
		if( !m_damage.IsDead() )
		{
			if( !g_pModelsDB->GetNodeAttachSpears( hModelNode ))
			{
				// We can't attach a spear to the node so remove the spear and fail...

				g_pLTServer->RemoveObject(hSpear);
				return false;
			}
		}

		char* szNode = (char *)g_pModelsDB->GetNodeName( hModelNode );

		// Get the node transform because we need to make rotation relative

		HMODELNODE hNode;
		if ( szNode && LT_OK == g_pModelLT->GetNode(m_hObject, szNode, hNode) )
		{
			LTTransform transform;
			if ( LT_OK == g_pModelLT->GetNodeTransform(m_hObject, hNode, transform, true) )
			{
				LTRotation rRotNode = transform.m_rRot;
				LTRotation rAttachment = ~rRotNode*rRot;

				LTVector vSpearDims;

				// Get the dims of the spear from the model animation since the dims may
				// have been adjusted for the pickup box and we want to use the small
				// dims for this calculation...

				uint32 dwAni = g_pLTServer->GetModelAnimation(hSpear);
			 	if (dwAni != INVALID_ANI)
				{
					g_pModelLT->GetModelAnimUserDims(hSpear, dwAni, &vSpearDims);
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
					m_aSpears[m_cSpears].hModelNode = hModelNode;
					m_aSpears[m_cSpears].rRot = rRot;

					m_cSpears++;

					//if (!m_damage.IsDead())
					//	g_pLTServer->CPrint("spear stuck");

					//flag the spear to be hidden on low violence clients
					g_pCommonLT->SetObjectFlags(hSpear, OFT_User, USRFLG_GORE, USRFLG_GORE);

					//tell everybody we've got a new spear,so they can decide whether to draw it
					CAutoMessage cMsg;
					cMsg.Writeuint8(MID_SFX_MESSAGE);
					cMsg.Writeuint8(SFX_CHARACTER_ID);
					cMsg.WriteObject(m_hObject);
					cMsg.WriteBits(CFX_UPDATE_ATTACHMENTS, FNumBitsExclusive<CFX_COUNT>::k_nValue );
					g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);


					return true;
				}
			}
		}
	}

	// Unless we actually stuck the spear into us, we'll fall through into here.

	g_pLTServer->RemoveObject(hSpear);
	return false;
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

	//add any remotes stuck to us
	ObjRefNotifierList::iterator iter = m_AttachedRemoteCharges.begin();
	while (iter != m_AttachedRemoteCharges.end()) 
	{
		HOBJECT hRemote = *iter;
		if( hRemote )
		{
			AddObjectToList( pObjList, hRemote, eControl );
		}
		++iter;
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

		m_pAttachments = NULL;
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
	// Remove spear attachments...
	for ( uint32 iSpear = 0 ; iSpear < m_cSpears ; iSpear++ )
	{
		if ( &m_aSpears[iSpear].hObject == pRef )
		{
			HATTACHMENT hAttachment;
			if ( LT_OK == g_pLTServer->FindAttachment(m_hObject, hObj, &hAttachment) )
			{
				g_pLTServer->RemoveAttachment(hAttachment);
			}

			m_aSpears[iSpear].hObject = NULL;
		}
	}

	// Remove remote charge attachments...
	ObjRefNotifierList::iterator iter = m_AttachedRemoteCharges.begin();
	while( iter != m_AttachedRemoteCharges.end( )) 
	{
		if( &( *iter ) == pRef )
		{
			HATTACHMENT hAttachment;
			if( LT_OK == g_pLTServer->FindAttachment( m_hObject, hObj, &hAttachment ))
			{
				g_pLTServer->RemoveAttachment(hAttachment);
			}

			m_AttachedRemoteCharges.erase( iter );
			break;
		}
		++iter;
	}

	// Remove remote charges character has out.
	iter = m_ActiveRemoteCharges.begin();
	while( iter != m_ActiveRemoteCharges.end( )) 
	{
		if( &( *iter ) == pRef )
		{
			m_ActiveRemoteCharges.erase( iter );
			break;
		}
		++iter;
	}

	GameBase::OnLinkBroken( pRef, hObj );
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CCharacter::GetVerticalThreshold()
//              
//	PURPOSE:	Base class function!!  Should be pure virtual, but unable to 
//				because of how WorldEdit must instantiate classes.
//              
//----------------------------------------------------------------------------
float CCharacter::GetVerticalThreshold() const
{
	AIASSERT( 0, m_hObject, "CCharacter::GetVerticalThreshold: Should not get here" ); 
	return 0.0f;
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
//	ROUTINE:	CCharacter::OnPlayerTrackerAbort
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

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetMoveToFloor
//
//	PURPOSE:	Sets the move to floor flag and commands a move to floor.
//
// --------------------------------------------------------------------------- //

void CCharacter::SetMoveToFloor( bool bValue )
{
	if( bValue == m_bMoveToFloor )
		return;

	m_bMoveToFloor = bValue;

	// If we turned it off, we're done, otherwise we need to move the object to the floor.
	if( !m_bMoveToFloor)
		return;

	// Move us to the floor.
	MoveObjectToFloor(m_hObject);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::IsPreferredWeapon( )
//
//	PURPOSE:	Checks if WeaponA is preferred to WeaponB.
//
// ----------------------------------------------------------------------- //

bool CCharacter::IsPreferredWeapon( HWEAPON hWeaponA, HWEAPON hWeaponB ) const
{
	return g_pWeaponDB->IsBetterWeapon(hWeaponA,hWeaponB);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::StartFade
//
//	PURPOSE:	Start fading away... dead characters only
//
// ----------------------------------------------------------------------- //
void CCharacter::StartFade()
{
	//bail out if we're not dead
	if( IsAlive( ))
	{
		LTERROR("CCharacter::StartFade( ) - we're not dead");
		return;
	}

	if (!m_tmrBodyFade.IsStarted())
	{
// Characters materials don't support alpha'ing out.
/*
		CAutoMessage cMsg;
		cMsg.Writeuint8(MID_SFX_MESSAGE);
		cMsg.Writeuint8(SFX_CHARACTER_ID);
		cMsg.WriteObject(m_hObject);
		cMsg.WriteBits(CFX_FADE_MSG, FNumBitsExclusive<CFX_COUNT>::k_nValue );
		g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);
*/
		m_tmrBodyFade.Start(3.5f);

		SetNextUpdate(UPDATE_NEXT_FRAME);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::CapNumberOfBodies
//
//	PURPOSE:	Keep body density low where this body is.
//
// ----------------------------------------------------------------------- //

struct BodyDistance
{
	CCharacter*	m_pBody;
	float		m_fDistanceSqr;

	bool operator()( BodyDistance const& a, BodyDistance const& b) const
	{
		// a should be considered "smaller" than b if its distance
		// is less than b.
		return a.m_fDistanceSqr < b.m_fDistanceSqr;
	}
};

void CCharacter::CapNumberOfBodies( )
{
	//bail out if we're not dead
	if (IsAlive( ))
	{
		LTERROR("CCharacter::CapNumberOfBodies( ) - we're not dead");
		return;
	}

	typedef std::vector< BodyDistance, LTAllocator<BodyDistance, LT_MEM_TYPE_OBJECTSHELL> > TBodyDistanceList;
	typedef std::priority_queue< BodyDistance, TBodyDistanceList, BodyDistance > TBodyDistanceQueue;
	TBodyDistanceQueue queRadius;
	TBodyDistanceQueue queTotal;

	LTVector vPos;
	g_pLTServer->GetObjectPos( m_hObject, &vPos );

	LTVector vOtherPos;
	BodyDistance bodyDistance;

	// Initialize console variables.
	if(!s_vtBodyCapRadius.IsInitted())
		s_vtBodyCapRadius.Init(g_pLTServer, "BodyCapRadius", NULL, g_pServerDB->GetBodyCapRadius());
	if(!s_vtBodyCapRadiusCount.IsInitted())
		s_vtBodyCapRadiusCount.Init(g_pLTServer, "BodyCapRadiusCount", NULL, (float)g_pServerDB->GetBodyCapRadiusCount());
	if(!s_vtBodyCapTotalCount.IsInitted())
		s_vtBodyCapTotalCount.Init(g_pLTServer, "BodyCapTotalCount", NULL, (float)g_pServerDB->GetBodyCapTotalCount());

	float fBodyCapRadiusSqr = s_vtBodyCapRadius.GetFloat( );
	fBodyCapRadiusSqr *= fBodyCapRadiusSqr;
	int nBodyCapRadiusCount = ( int )s_vtBodyCapRadiusCount.GetFloat( );
	nBodyCapRadiusCount = LTMAX( 0, nBodyCapRadiusCount );
	int nBodyCapTotalCount = ( int )s_vtBodyCapTotalCount.GetFloat( );
	nBodyCapTotalCount = LTMAX( 0, nBodyCapTotalCount );

	// Iterate through the list of bodies and put them into 2
	// priority queues.  Sorted by farthest at top of queue.
	for( CharacterList::iterator iter = m_lstBodies.begin( ); iter != m_lstBodies.end( ); iter++ )
	{
		bodyDistance.m_pBody = *iter;
		if( !bodyDistance.m_pBody )
			continue;

		// Don't remove bodies that are permanent.
		if( bodyDistance.m_pBody->m_bPermanentBody )
			continue;

		// Ignore characters who are already fading out.
		if ( bodyDistance.m_pBody->m_tmrBodyFade.IsStarted() )
			continue;

		// Get the distance from the new body.
		g_pLTServer->GetObjectPos( bodyDistance.m_pBody->m_hObject, &vOtherPos );
		bodyDistance.m_fDistanceSqr = vPos.DistSqr( vOtherPos );

		// See if it's in range to check for.
		if( bodyDistance.m_fDistanceSqr < fBodyCapRadiusSqr )
		{
			queRadius.push( bodyDistance );
		}

		// Put in complete list of bodies.
		queTotal.push( bodyDistance );
	}

	// Get the number of bodies that will be removed by the radius check.
	int nNumCapByRadius;
	if( nBodyCapRadiusCount >= 0 )
	{
		nNumCapByRadius = queRadius.size( ) - nBodyCapRadiusCount;
	}
	else
	{
		nNumCapByRadius = 0;
	}
	nNumCapByRadius = LTMAX( 0, nNumCapByRadius );

	// Remove all the farthest bodies until the density is good.
	int nRadiusCount = nNumCapByRadius;
	while( nRadiusCount > 0 )
	{
		// Get the farthest body in radius.
		BodyDistance const& body = queRadius.top( );

		// Fade the body away.
		body.m_pBody->StartFade();

		// Go to the next farthest body.
		queRadius.pop( );

		// Count off this body.
		nRadiusCount--;
	}

	// Keep the whole level capped by removing the very farthest bodies.  Consider
	// bodies removed by radius as not part of the total count.
	if( nBodyCapTotalCount >= 0 )
	{
	int nTotalCount = queTotal.size( ) - nNumCapByRadius - nBodyCapTotalCount;
	while( nTotalCount > 0 )
	{
		// Get the farthest body.
		BodyDistance const& body = queTotal.top( );

		// Fade the body away.
		body.m_pBody->StartFade();

		// Go to the next farthest body.
		queTotal.pop( );

		// Count off this body.
		nTotalCount--;
	}
}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::SetPhysicsWeightSet()
//
//	PURPOSE:	Set the physics weight set for the body and update the
//				hit box
//
// ----------------------------------------------------------------------- //

void CCharacter::SetPhysicsWeightSet(const char* pWeightSet)
{
	// Validate parameter...
	if (!pWeightSet) return;

	// Set the weight set on our object, but only have it do a client only 
	// simulation if we are in multiplayer
	bool bClientOnly = IsMultiplayerGameServer();
	PhysicsUtilities::EnumRigidBodyState eRigidBodyState = PhysicsUtilities::SetPhysicsWeightSet(m_hObject, pWeightSet, bClientOnly);
	m_bIsRagdolling = (eRigidBodyState == PhysicsUtilities::RIGIDBODY_Full);

	// Add/remove ragdoll notifications.
	if (m_bIsRagdolling)
	{
		AddRagdollCollisionNotifications();
	}
	else
	{
		RemoveRagdollCollisionNotifications();
	}

	// If we're using 100% rigid body make sure our hit box correctly follows us and that we
	// don't eat bandwidth in multiplayer...
	//
	if (m_bIsRagdolling && !bClientOnly)
	{
		// Make sure the character's hit box follows the model's visibility node...

		if (m_hHitBox)
		{
			CCharacterHitBox* pHitBox = CCharacterHitBox::DynamicCast(m_hHitBox);
			if ( pHitBox )
			{
				pHitBox->FollowVisNode(true);
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::ForceGib
//
//	PURPOSE:	Force the body to gib
//
// ----------------------------------------------------------------------- //

void CCharacter::ForceGib()
{
	HandleGib();
	g_pLTServer->RemoveObject( m_hObject );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleGib
//
//	PURPOSE:	Handle gibbing the body...
//
// ----------------------------------------------------------------------- //

void CCharacter::HandleGib()
{
	if (!m_bStartedDeath) return;

	//if we're permanent we don't want to be destroyed, but we shouldn't be seen either...
	if (m_bPermanentBody || IsPlayer( m_hObject ))
	{
		HideCharacter(true);
	}
	else
	{
		m_bMakeBody = false;
	}


	//if the model always gibs it shouldn't count against the limit
	if (!g_pModelsDB->AlwaysGib(m_hModel))
	{
		//set frequency cap so we don't get another right away...
		sm_nGibCounter = g_pServerDB->GetGibFrequencyCap();
	}
	

	// no cap for testing...
	if (g_vtBodyGibTest.GetFloat() > 0.0f)
	{
		sm_nGibCounter = 0;
	}


	//fire the GibFX off...
	if (!LTStrEmpty(g_pModelsDB->GetGibFX(m_hModel)))
	{
		LTVector vPos;
		LTRotation rRot;
		g_pLTServer->GetObjectPos(m_hObject,&vPos);
		g_pLTServer->GetObjectRotation(m_hObject,&rRot);


		LTVector vDir = m_damage.GetDeathDir();
		//make sure we have a valid rotation for the death effect...
		if ( LTAbs(vDir.y) > 0.98f)
		{
			vDir.x = 0.0f;
			vDir.Normalize();
			rRot = LTRotation(vDir,rRot.Forward());
		}
		else
		{
			vDir.y = 0.0f;
			vDir.Normalize();
			rRot = LTRotation(vDir,rRot.Up());
		}

		CAutoMessage cMsg;
		cMsg.Writeuint8(SFX_CLIENTFXGROUPINSTANT);
		cMsg.WriteString(g_pModelsDB->GetGibFX(m_hModel));
		cMsg.Writebool( false ); //not looping
		cMsg.Writebool( false ); //smooth shutdown

		// No special parent.
		cMsg.Writebool( false );

		cMsg.WriteLTVector(vPos);
		cMsg.WriteCompLTRotation(rRot);

		// no target info...
		cMsg.Writebool( false );

		g_pLTServer->SendSFXMessage(cMsg.Read(), 0);

	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::PrepareToSever
//
//	PURPOSE:	Switch to the sever body...
//
// ----------------------------------------------------------------------- //
void CCharacter::PrepareToSever()
{
	if (!m_bStartedDeath) return;

	ModelsDB::HSEVERBODY hBody = g_pModelsDB->GetSeverBodyRecord(m_hModel);
	if (!hBody) return;


	ObjectCreateStruct createstruct;
	const char* pFilename = g_pModelsDB->GetBodyModelFile(hBody);
	if( !pFilename )
	{
		LTERROR( "CCharacter::PrepareToSever: Invalid model." );
		return;
	}

	createstruct.SetFileName(pFilename);
	g_pModelsDB->CopyBodyMaterialFilenames(hBody, createstruct.m_Materials[0], LTARRAYSIZE( createstruct.m_Materials ), 
		LTARRAYSIZE( createstruct.m_Materials[0] ));

	g_pCommonLT->SetObjectFilenames(m_hObject, &createstruct);

	if( m_pAttachments )
	{
		m_pAttachments->RemoveAndRecreateAttachments();
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleSever
//
//	PURPOSE:	Handle the severing a specific part
//
// ----------------------------------------------------------------------- //

void CCharacter::HandleSever(ModelsDB::HSEVERPIECE hPiece)
{
	if (!hPiece) return;

	if (g_pModelsDB->GetSPLocation(hPiece) == HL_HEAD)
	{
		m_bDecapitated = true;
	}

	//hide pieces of the body model
	uint32 nPieces = g_pModelsDB->GetSPNumPieces(hPiece);
	for (uint32 n = 0; n < nPieces; ++n)
	{
		HMODELPIECE hHidePiece = (HMODELPIECE)NULL;	
		if( LT_OK == g_pModelLT->GetPiece( m_hObject, g_pModelsDB->GetSPPiece(hPiece,n), hHidePiece ) )
			{
			// hide it
			LTRESULT ltResult;
			ltResult = g_pModelLT->SetPieceHideStatus( m_hObject, hHidePiece, true );
			LTASSERT( ( LT_OK == ltResult) || ( LT_NOCHANGE == ltResult ),  "CCharacter::HandleSever : Failed to set piece status." );
		}
	}

	ILTPhysicsSim* pPhysicsSim = g_pLTServer->PhysicsSim();

	//make the rigid body associated with the node non-solid...
	uint32 nNodes = g_pModelsDB->GetSPNumNodes(hPiece);
	for (uint32 n = 0; n < nNodes; ++n)
	{
		ModelsDB::HNODE hNode = g_pModelsDB->GetSPNode(hPiece,n);
		if (hNode)
		{
			const char* szNodeName = g_pModelsDB->GetNodeName( hNode );
			if( szNodeName )
			{
				HMODELNODE hModelNode;
				if ( LT_OK == g_pModelLT->GetNode(m_hObject, szNodeName, hModelNode))
				{
					HPHYSICSRIGIDBODY hRigidBody = NULL;
					if (LT_OK == pPhysicsSim->GetModelNodeRigidBody(m_hObject, hModelNode, hRigidBody)) 
					{
						//need to make sure the polygons are not weighted across sever points or they will be stretched
						pPhysicsSim->RemoveRigidBodyFromSimulation(hRigidBody);

						pPhysicsSim->ReleaseRigidBody(hRigidBody);
					}
				}

			}
		}
	}

	float fForceMod = LTCLAMP( (1.0f - g_pModelsDB->GetSPResistance(hPiece)), 0.0f, 1.0f);


	//have the client spawn a severed piece and play the client FX on it
	CAutoMessage cMsg;
	cMsg.Writeuint8( MID_SFX_MESSAGE );
	cMsg.Writeuint8( SFX_CHARACTER_ID );
	cMsg.WriteObject( m_hObject );
	cMsg.WriteBits(CFX_SEVER, FNumBitsExclusive<CFX_COUNT>::k_nValue );
	cMsg.Writeuint8(g_pModelsDB->GetSPLocation(hPiece));
	cMsg.WriteCompLTPolarCoord(LTPolarCoord(m_damage.GetDeathDir()));
	cMsg.Writefloat(m_damage.GetDeathImpulseForce() * fForceMod);
	if (m_cs.bIsPlayer)
	{
		((CPlayerObj*)this)->WriteAnimInfo(cMsg);
	}
	g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);


	//remove anything attached to the severed piece
	uint32 nSockets = g_pModelsDB->GetSPNumSockets(hPiece);
	for (uint32 n = 0; n < nSockets; ++n)
	{
		m_pAttachments->HandleSever(g_pModelsDB->GetSPSocket(hPiece,n),m_damage.GetDeathDir());
	}
}

void CCharacter::ResetModel( )
{
	UpdateSurfaceFlags( );
}

void CCharacter::UpdateSurfaceFlags( )
{
	// Set up userflags.
	uint32 nFlags = GetUserFlagSurfaceType() | CollisionPropertyRecordToUserFlag( GetCollisionPropertyRecord( ));
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, nFlags, USRFLG_SURFACEMASK | USRFLG_COLLISIONPROPMASK );
}




// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCharacter::AddRemoteCharge
//
//  PURPOSE:	We set a remote charge and need to keep track of it...
//
// ----------------------------------------------------------------------- //
void CCharacter::AddRemoteCharge(HOBJECT hRemote)
{
	LTObjRefNotifier ref( *this );
	ref = hRemote;
	m_ActiveRemoteCharges.push_back(ref);
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CCharacter::AttachRemoteCharge
//
//  PURPOSE:	A remote charge hit us, does it stick?
//
// ----------------------------------------------------------------------- //
bool CCharacter::AttachRemoteCharge(HOBJECT hRemote, ModelsDB::HNODE hModelNode)
{
	LTRigidTransform tRemoteTrans;
	g_pLTServer->GetObjectTransform(hRemote,&tRemoteTrans);

	//if we're dead, attach anywhere, otherwise see if the node will let us...
	if( IsAlive( ))
	{
//		if( !g_pModelsDB->GetNodeAttachSpears( hModelNode ))
//		{
//			// We can't attach the remote to the node so bail out...
//			return false;
//		}
	}

	char* szNode = (char *)g_pModelsDB->GetNodeName( hModelNode );

	// Get the node transform because we need to make rotation relative
	HMODELNODE hNode;
	if ( szNode && LT_OK == g_pModelLT->GetNode(m_hObject, szNode, hNode) )
	{

		if ( g_vtDebugRemote.GetFloat(0.0f) == 1.0f )
		{
			DebugCPrint(0,"attaching to %s",szNode);
		}
		LTTransform transform;
		if ( LT_OK == g_pModelLT->GetNodeTransform(m_hObject, hNode, transform, true) )
		{
			//get a rigid transform from the node transform
			LTRigidTransform tAttachTransform(transform.m_vPos,transform.m_rRot);

			//invert it
			tAttachTransform.Inverse();

			//multiply by the remote's transform to the get the remote's transform relative to the node
			tAttachTransform *= tRemoteTrans;

			if ( g_vtDebugRemote.GetFloat(0.0f) == 1.0f )
			{
				DebugCPrint(0," -   node pos:  %0.2f,%0.2f,%0.2f",transform.m_vPos.x,transform.m_vPos.y,transform.m_vPos.z);
			}

			HATTACHMENT hAttachment;
			if ( LT_OK == g_pLTServer->CreateAttachment(m_hObject, hRemote, szNode, &tAttachTransform.m_vPos, &tAttachTransform.m_rRot, &hAttachment) )
			{
				LTObjRefNotifier ref( *this );
				ref = hRemote;
				m_AttachedRemoteCharges.push_back(ref);

				//tell everybody we've got a new Remote,so they can decide whether to draw it
				CAutoMessage cMsg;
				cMsg.Writeuint8(MID_SFX_MESSAGE);
				cMsg.Writeuint8(SFX_CHARACTER_ID);
				cMsg.WriteObject(m_hObject);
				cMsg.WriteBits(CFX_UPDATE_ATTACHMENTS, FNumBitsExclusive<CFX_COUNT>::k_nValue );
				g_pLTServer->SendToClient(cMsg.Read(), NULL, MESSAGE_GUARANTEED);

				return true;
			}

		}
	}

	return false;
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleDetonatorMsg()
//
//	PURPOSE:	Handle a DETONATOR message...
//
// --------------------------------------------------------------------------- //

void CCharacter::HandleDetonatorMsg( HOBJECT hSender, const CParsedMsg &crParsedMsg )
{
	if (hSender == m_hObject)
	{
		DetonateRemoteCharges();
	}
}


// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::DetontateRemoteCharges()
//
//	PURPOSE:	Detonate my remotes...
//
// --------------------------------------------------------------------------- //

void CCharacter::DetonateRemoteCharges()
{
	ObjRefNotifierList::iterator iter = m_ActiveRemoteCharges.begin();
	while (iter != m_ActiveRemoteCharges.end()) 
	{
		HOBJECT hObj = *iter;
		if( hObj )
		{
			CRemoteCharge* pRC = dynamic_cast< CRemoteCharge* >( g_pLTServer->HandleToObject( hObj ) );
			if (pRC)
			{
				pRC->RemoteDetonate();
			}
		}
		++iter;
	}
	m_ActiveRemoteCharges.clear();
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::RemoveRemoteCharges()
//
//	PURPOSE:	Remove my remotes from the game...
//
// --------------------------------------------------------------------------- //

void CCharacter::RemoveRemoteCharges()
{
	ObjRefNotifierList::iterator iter = m_ActiveRemoteCharges.begin();
	while (iter != m_ActiveRemoteCharges.end()) 
	{
		HOBJECT hObj = *iter;
		if( hObj )
		{
			g_pLTServer->RemoveObject(hObj);
		}
		++iter;
	}
	m_ActiveRemoteCharges.clear();
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::GetEnemyVisibleStimulusID()
//
//	PURPOSE:	Returns a characters visibility stimulus.  This can be
//			used to retrieve their stimulus record from the 
//			StimulusMgr.
//
// --------------------------------------------------------------------------- //

EnumAIStimulusID CCharacter::GetEnemyVisibleStimulusID()
{
	return m_eEnemyVisibleStimID;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleMeleeBlocked()
//
//	PURPOSE:	Handles responding to this character being blocked.  
//			Clears the blocked windows to insure no one querying 
//			for information receives out of date info.
//
// --------------------------------------------------------------------------- //

void CCharacter::HandleMeleeBlocked()
{
	m_flBlockWindowEndTime = 0.0f;
	m_flDodgeWindowEndTime = 0.0f;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleMeleeBlockedAttacker()
//
//	PURPOSE:	Handles responding to this character successfully 
//			blocking.
//
// --------------------------------------------------------------------------- //

void CCharacter::HandleMeleeBlockedAttacker()
{
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CCharacter::HandleDeathFX
//
//	PURPOSE:	Handle the character death FX
//
// ----------------------------------------------------------------------- //
bool CCharacter::HandleDeathFX()
{
	if (!m_bStartedDeath) 
		return false;

	uint32 nNumDFX = g_pModelsDB->GetNumDeathFX(m_hModel);
	HRECORD hDFX = NULL;
	//step through our death FX until we get a match or we run out
	for (uint32 nDFX = 0; hDFX == NULL && nDFX < nNumDFX; ++nDFX )
	{
		hDFX = g_pModelsDB->GetDeathFXRecord(m_hModel,nDFX);
		//if the damage type doesn't match the kind that killed us, clear the record so we keep looking
		if ( hDFX && g_pModelsDB->GetDeathFXDamageType(hDFX) != m_damage.GetDeathType() )
		{
			hDFX = NULL;
		}
	}

	if (!hDFX)
	{
		return false;
	}

	bool bApply = false;


	ObjectCreateStruct createstruct;
	const char* pFilename = g_pModelsDB->GetDeathFXModelFilename(hDFX);
	if( pFilename )
	{
		createstruct.SetFileName(pFilename);
		bApply = true;
	}

	
	if( g_pModelsDB->GetDeathFXNumMaterials(hDFX) )
	{
		g_pModelsDB->CopyDeathFXMaterialFilenames(hDFX, createstruct.m_Materials[0], LTARRAYSIZE( createstruct.m_Materials ), 
			LTARRAYSIZE( createstruct.m_Materials[0] ));
		bApply = true;
	}
	
	if (bApply)
	{
		// AI need to set the filename on the server since the deathfx animations may not match the AI
		// models.  Players DeathFX model must match the player model.
		if( IsAI( m_hObject ))
		{
			g_pCommonLT->SetObjectFilenames(m_hObject, &createstruct);
		}

		m_pAttachments->DeleteAllAttachmentPositions();

	}

	return bApply;
}

// EOF
