// ----------------------------------------------------------------------- //
//
// MODULE  : AI.cpp
//
// PURPOSE : Base AI class
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"

#include "AI.h"
#include "AIDB.h"
#include "CharacterDB.h"
#include "AIState.h"
#include "AITarget.h"
#include "AINodeMgr.h"
#include "AIGoalMgr.h"
#include "AIBrain.h"
#include "AIStimulusMgr.h"
#include "AISoundDB.h"
#include "AISoundMgr.h"
#include "AIPathKnowledgeMgr.h"
#include "AICoordinator.h"
#include "AIWorkingMemoryCentral.h"

#include "ObjectMsgs.h"
#include "VolumeBrushTypes.h"
#include "TeleportPoint.h"
#include "Camera.h"
#include "ServerSoundMgr.h"
#include "Attachments.h"
#include "AINodeTrackerContext.h"
#include "CharacterHitBox.h"
#include "SharedFXStructs.h"
#include "DEditColors.h"

#include "AIBlackBoard.h"
#include "AINavMesh.h"
#include "AINavMeshLinkDoor.h"
#include "AICommandMgr.h"
#include "AIPlanner.h"
#include "SurfaceFunctions.h"
#include "PhysicsUtilities.h"
#include "Weapon.h"

#include "Spawner.h"
#include "GearItems.h"
#include "WeaponItems.h"
#include "iperformancemonitor.h"

LINKFROM_MODULE( AI );

// Enables debug display of AIInfo.
static VarTrack g_AIInfoTrack;

// Insures the AI do not initially all update their senses the same frame.
static float s_fSenseUpdateBasis = 0.0f;

#define FIND_FLOOR_THRESH_SQR		4.f

#define	BODY_MATERIAL_INDEX			0
#define HEAD_MATERIAL_INDEX			1
#define MATERIAL_EXT_RANDOM			"RANDOM"

#define MAX_DEDIT_CHILD_MODELS		8

// Performance monitoring.
CTimedSystem g_tsAI("AI", NULL);
CTimedSystem g_tsAIMovement("AIMovement", "AI");
CTimedSystem g_tsAIPosition("AIPosition", "AI");
CTimedSystem g_tsAITarget("AITarget", "AI");
CTimedSystem g_tsAIOnGround("AIOnGround", "AI");

#define TIME(x) x;
//#define TIME(x) StartTimingCounterServer(); x; EndTimingCounterServer(#x);

// Define our properties (what is available in WorldEdit)...
BEGIN_CLASS(CAI)

	ADD_VECTORPROP_VAL_FLAG(Dims, 24.0f, 90.0f, 24.0f, PF_DIMS|PF_HIDDEN, "Used to dispaly the diminsions of the AI in the editor.")
	ADD_DEDIT_COLOR( CAI )

	// New properties

	ADD_BOOLPROP_FLAG(UseDefaultAttachments, true, PF_GROUP(2), "If true the default attachments that are specified in the GDBEdit model record will be used.")
	ADD_ATTACHMENTS_AGGREGATE( PF_GROUP(2) )
	
	ADD_BOOLPROP_FLAG(UseDefaultWeapons, true, PF_GROUP(3), "If true the default weapons that are specified in the GDBEdit model record will be used.")
	ADD_BOOLPROP_FLAG(HolsterWeapons, false, PF_GROUP(3), "If true the weapons will be holstered when the AI spawns.")
	ADD_STRINGPROP_FLAG(AmmoLoad, "", PF_GROUP(3)|PF_STATICLIST, ".")
	ADD_ARSENAL_AGGREGATE( PF_GROUP(3) )
	
	ADD_AICONFIG_AGGREGATE( PF_GROUP(6) )

	ADD_STRINGPROP_FLAG(ModelTemplate, "Default", PF_STATICLIST, "The name of the model template in modelbutes.txt.")
	ADD_STRINGPROP_FLAG(Brain, "Default", PF_STATICLIST, "The name of a the brain template that will be used by the AI. (Defined in AIButes.txt)")
	ADD_STRINGPROP_FLAG(GoalSet, "None", PF_STATICLIST, "The name of the goalset in aigoals.txt.")

	
	// Overrides

	ADD_STRINGPROP_FLAG(SpawnItem, "", PF_HIDDEN, "Spawn string to run after death.")
	ADD_REALPROP_FLAG(HitPoints, -1.0f, PF_HIDDEN, "Number of hit points before death.")
	ADD_REALPROP_FLAG(Armor, -1.0f, PF_HIDDEN, "Number of armor points before death")
	ADD_STRINGPROP(BodyMaterialExtension, "", "You can specify a different body skin to use for this Character.  Entering a number 1-3 will look for a body skin that is named the default skin with the number added to it (ex. DefaultBodySkin2.dtx).  If RANDOM is entered one of the AltHeadSkin# specified for the model in modelbutes.txt will be randomly picked.")
	ADD_STRINGPROP(HeadExtension, "", "You can specify a different head skin to use for this Character.  Entering a number 1-3 will look for a head skin that is named the default skin with the number added to it (ex. DefaultHeadSkin2.dtx).  If RANDOM is entered one of the AltHeadSkin# specified for the model in modelbutes.txt will be randomly picked.")

	PROP_DEFINEGROUP(AddChildModel, (PF_GROUP(7)) | (0), "This is a group of properites that allows you to specify child models that should be added to the Character model.") 
		ADD_STRINGPROP_FLAG(ChildModel_1, "", (PF_GROUP(7)) | (PF_FILENAME), "Child model to load.") 
		ADD_STRINGPROP_FLAG(ChildModel_2, "", (PF_GROUP(7)) | (PF_FILENAME), "Child model to load.") 
		ADD_STRINGPROP_FLAG(ChildModel_3, "", (PF_GROUP(7)) | (PF_FILENAME), "Child model to load.")
		ADD_STRINGPROP_FLAG(ChildModel_4, "", (PF_GROUP(7)) | (PF_FILENAME), "Child model to load.") 
		ADD_STRINGPROP_FLAG(ChildModel_5, "", (PF_GROUP(7)) | (PF_FILENAME), "Child model to load.") 
		ADD_STRINGPROP_FLAG(ChildModel_6, "", (PF_GROUP(7)) | (PF_FILENAME), "Child model to load.") 
		ADD_STRINGPROP_FLAG(ChildModel_7, "", (PF_GROUP(7)) | (PF_FILENAME), "Child model to load.") 
		ADD_STRINGPROP_FLAG(ChildModel_8, "", (PF_GROUP(7)) | (PF_FILENAME), "Child model to load.") 


	// New properties

	ADD_BOOLPROP_FLAG(IsCinematicAI, false, 0, "If the AI is only going to be used in a cinematic scene then set this flag to true.")
	ADD_BOOLPROP_FLAG(CanTalk, true, 0, "If the AI is going to talk then set this flag to true.")

	PROP_DEFINEGROUP(AttributeOverrides, PF_GROUP(4), "Group of properties that allow for specific AI attributes to be override." )

		// Basic attributes

		ADD_STRINGPROP_FLAG(OV_SoundRadius,	"", PF_GROUP(4)|PF_RADIUS, "The radius of any sound that AI the plays. Outside of this radius, the sound will not be audible. [WorldEdit units]")
		ADD_STRINGPROP_FLAG(OV_SoundInnerRadius, "", PF_GROUP(4)|PF_RADIUS, "The inner radius of any sound that AI the plays. Inside of this radius, the sound will be at full volume. [WorldEdit units]")

		ADD_STRINGPROP_FLAG(OV_JumpSpeed,		"", PF_GROUP(4), "The speed at which the AI jumps. [WorldEdit units/second]")
		ADD_STRINGPROP_FLAG(OV_WalkSpeed,		"", PF_GROUP(4), "The speed at which the AI walks. [WorldEdit units/second]")
		ADD_STRINGPROP_FLAG(OV_SwimSpeed,		"", PF_GROUP(4), "The speed at which the AI swims. [WorldEdit units/second]")
		ADD_STRINGPROP_FLAG(OV_RunSpeed,		"", PF_GROUP(4), "The speed at which the AI runs. [WorldEdit units/second]")

		ADD_STRINGPROP_FLAG(OV_Accuracy,					"", PF_GROUP(4), "Percent chance a shot will hit AI’s target.")
		ADD_STRINGPROP_FLAG(OV_FullAccuracyRadius,			"", PF_GROUP(4), "Radius from AI where AI has perfect accuracy.")
		ADD_STRINGPROP_FLAG(OV_AccuracyMissPerturb,			"", PF_GROUP(4), "Amount of perturb on bullets that miss their target.")
		ADD_STRINGPROP_FLAG(OV_MaxMovementAccuracyPerturb,	"", PF_GROUP(4), "Maximum amount of perturb caused by aiming at a moving target.")
		ADD_STRINGPROP_FLAG(OV_MovementAccuracyPerturbTime,	"", PF_GROUP(4), "Amount of time to catch up when aiming at a moving target.")

		ADD_STRINGPROP_FLAG(OV_WeaponUpgradeDesire,			"", PF_GROUP(4), "Amount of desire the AI has for trying to find a better weapon.")

		ADD_STRINGPROP_FLAG(OV_Awareness,		"", PF_GROUP(4), "A modifier that affects the rate at which AI's react to stimuli. 0.0 to 1.0 will double to halve their reaction time.")

		// OT_Senses is now handled by the AIConfig class.  Unfortunately, 
		// OV_Senses is heavily used; moving it into a different group would be 
		// confusing for content.  In addition, if it was moved, the name should 
		// be changed.  We can't change the name as we would lose values, so for now,
		// the property is declared in AI.cpp and in AISpawner.cpp, with the intent 
		// of one day moving it fully into AIConfig.

		ADD_BOOLPROP_FLAG(OV_Senses,			true, PF_GROUP(4), "Should the AI uses it's senses to sense things in the world.")

		ADD_STRINGPROP_FLAG(OV_InstantDeath,			"", PF_GROUP(4), "If set to 1, this AI will die immediately after taking damage; hitpoints will be ignored.  If empty or any other value, default behavior for this AI will be applied.")

	PROP_DEFINEGROUP(Commands, PF_GROUP(5), "This is a subset of properties that are command strings that will be sent on certain conditions.")

		ADD_COMMANDPROP_FLAG(Initial,				"", PF_GROUP(5) | PF_NOTIFYCHANGE, "A command that is sent when the level starts.")
		ADD_COMMANDPROP_FLAG(ActivateOn,				"", PF_GROUP(5) | PF_NOTIFYCHANGE, "A command that is sent when the AI's activation is toggled on.")
		ADD_COMMANDPROP_FLAG(ActivateOff,			"", PF_GROUP(5) | PF_NOTIFYCHANGE, "A command that is sent when the AI's activation is toggled off.")

	ADD_PREFETCH_RESOURCE_PROPS()

END_CLASS_FLAGS_PLUGIN_PREFETCH(CAI, CCharacter, 0, CAIPlugin, DefaultPrefetch<CAI>, "This is the main AI object that allows specification of which model template to use as well as any specific brain an goals." )



// static functions
static void SendInfoString( HOBJECT hObject, const char * szInfo )
{
	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_SFX_MESSAGE);
	cMsg.Writeuint8(SFX_CHARACTER_ID);
	cMsg.WriteObject(hObject);
	cMsg.WriteBits(CFX_INFO_STRING, FNumBitsExclusive<CFX_COUNT>::k_nValue );
	cMsg.WriteString(szInfo);
	g_pLTServer->SendToClient(cMsg.Read(), NULL, 0);
}


// Filter functions

HOBJECT s_hFilterAI = NULL;


bool CAI::DefaultFilterFn(HOBJECT hObj, void *pUserData)
{
    if ( !hObj ) return false;
    if ( hObj == s_hFilterAI ) return false;

    HCLASS hBody = g_pLTServer->GetClass("Body");

    if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hBody))
	{
        return false;
	}

	// See thru dead AI.

	if( IsDeadAI( hObj ) )
	{
		return false;
	}

    HCLASS hHitbox = g_pLTServer->GetClass("CCharacterHitBox");

    if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hHitbox))
	{
        return false;
	}

	HCLASS hWeaponItem = g_pLTServer->GetClass("WeaponItem");

	if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hWeaponItem))
	{
		return false;
	}


	HCLASS hProjectile = g_pLTServer->GetClass("CProjectile");

	if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hProjectile))
	{
		return false;
	}

	uint32 dwType;
	if( (g_pCommonLT->GetObjectType( hObj, &dwType ) == LT_OK) && (dwType == OT_CONTAINER) )
	{
		// Honor the "CanSeeThrough" attribute of the objects surface...
		HSURFACE hSurface = g_pSurfaceDB->GetSurface( GetSurfaceType( hObj ));
		if( hSurface && g_pSurfaceDB->GetBool( hSurface, SrfDB_Srf_bCanSeeThrough ))
		{
			return false;
		}
	}

	return true;
}

bool CAI::ShootThroughFilterFn(HOBJECT hObj, void *pUserData)
{
    if ( !hObj ) return false;
    if ( hObj == s_hFilterAI ) return false;

	if ( IsMainWorld(hObj) )
	{
        return true;
	}

    HCLASS hBody = g_pLTServer->GetClass("Body");

    if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hBody))
	{
        return false;
	}

	// See thru dead AI.

	if( IsDeadAI( hObj ) )
	{
		return false;
	}

    HCLASS hHitbox = g_pLTServer->GetClass("CCharacterHitBox");

    if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hHitbox))
	{
        return false;
	}

	HCLASS hWeaponItem = g_pLTServer->GetClass("WeaponItem");

	if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hWeaponItem))
	{
		return false;
	}

	HCLASS hProjectile = g_pLTServer->GetClass("CProjectile");

	if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hProjectile))
	{
		return false;
	}

	uint32 dwType;
	if( (g_pCommonLT->GetObjectType( hObj, &dwType ) == LT_OK) && (dwType == OT_CONTAINER) )
	{
		// Honor the "CanShootThrough" attribute of the objects surface...
		HSURFACE hSurface = g_pSurfaceDB->GetSurface( GetSurfaceType( hObj ));
		if( hSurface && g_pSurfaceDB->GetBool( hSurface, SrfDB_Srf_bCanShootThrough ))
		{
			return false;
		}
	}

	return true;
}

bool CAI::ShootThroughPolyFilterFn(HPOLY hPoly, void *pUserData, const LTVector& vIntersectPoint)
{
    if ( INVALID_HPOLY == hPoly ) return false;

	HSURFACE hSurf = g_pSurfaceDB->GetSurface(GetSurfaceType(hPoly));
	if ( !hSurf )
	{
		g_pLTServer->CPrint("Warning, HPOLY (%d, %d) had no associated surface!", hPoly.m_nPolyIndex, hPoly.m_nWorldIndex);
		return false;
	}

    if ( g_pSurfaceDB->GetBool(hSurf,SrfDB_Srf_bCanShootThrough))
	{
		return false;
	}

    return true;
}

bool CAI::SeeThroughFilterFn(HOBJECT hObj, void *pUserData)
{
    if ( !hObj ) return false;
    if ( hObj == s_hFilterAI ) return false;

	if ( IsMainWorld(hObj) )
	{
        return true;
	}

    HCLASS hBody = g_pLTServer->GetClass("Body");

    if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hBody))
	{
        return false;
	}

	// See thru dead AI.

	if( IsDeadAI( hObj ) )
	{
		return false;
	}

    HCLASS hHitbox = g_pLTServer->GetClass("CCharacterHitBox");

    if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hHitbox))
	{
        return false;
	}

	HCLASS hWeaponItem = g_pLTServer->GetClass("WeaponItem");

	if (g_pLTServer->IsKindOf(g_pLTServer->GetObjectClass(hObj), hWeaponItem))
	{
		return false;
	}

	uint32 dwType;
	if( (g_pCommonLT->GetObjectType( hObj, &dwType ) == LT_OK) && (dwType == OT_CONTAINER) )
	{
		// Honor the "CanSeeThrough" attribute of the objects surface...
		HSURFACE hSurface = g_pSurfaceDB->GetSurface( GetSurfaceType( hObj ));
		if( hSurface && g_pSurfaceDB->GetBool( hSurface, SrfDB_Srf_bCanSeeThrough ))
		{
			return false;
		}
	}

	return true;
}

bool CAI::SeeThroughPolyFilterFn(HPOLY hPoly, void *pUserData, const LTVector& vIntersectPoint)
{
    if ( INVALID_HPOLY == hPoly ) return false;

	HSURFACE hSurf = g_pSurfaceDB->GetSurface(GetSurfaceType(hPoly));
	if ( !hSurf )
	{
		g_pLTServer->CPrint("Warning, HPOLY (%d, %d) had no associated surface!", hPoly.m_nPolyIndex, hPoly.m_nWorldIndex);
		return false;
	}

	if ( g_pSurfaceDB->GetBool(hSurf,SrfDB_Srf_bCanSeeThrough))
	{
		return false;
	}

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::CAI()
//
//	PURPOSE:	Constructor - Initialize
//
// ----------------------------------------------------------------------- //

CAI::AIList CAI::m_lstAIs;
uint8 CAI::m_nDropIndex = 0;

CAI::CAI() : CCharacter()
{
	AddAggregate( &m_AIConfig );
	m_bConfigured = false;

	m_pTarget = AI_FACTORY_NEW(CAITarget);

    m_bFirstUpdate = true;
	m_fNextUpdateRate = 0.f;
	m_bUpdateAI = true;

	m_vPos.Init();

	m_vEyePos.Init();
	m_vEyeForward.Init();

	m_vWeaponPos.Init();
	m_vWeaponForward.Init();

	m_vDims.Init();
	m_fRadius = 1.0f;

	m_bUpdateNodes = true;
	m_hHeadNode = INVALID_MODEL_NODE;

	m_bShortRecoil		= true;

	m_pState = NULL;

	m_pAnimationContext = NULL;

	m_bCheapMovement = true;
	m_vMovePos.Init();
	m_bMove = false;
	m_fJumpVel		= 0.0f;
	m_fJumpOverVel	= 0.0f;
	m_fFallVel		= 0.0f;
	m_fWalkVel		= 0.0f;
	m_fSwimVel		= 0.0f;
	m_fRunVel		= 0.0f;
	
	m_pAIMovement = debug_new( CAIMovement );
	m_bForceGround = false;
	m_bPosDirty = true;
	m_vLastFindFloorPos = LTVector( 99999999.f, 99999999.f, 99999999.f );
	m_fNextFindFloorTime = 0.f;
	m_bSyncPosition = false;

	m_nDamagedPlayerNumCount = 0;
	m_nDamagedPlayerActivationCount = 0;

	m_pPathKnowledgeMgr = AI_FACTORY_NEW( CAIPathKnowledgeMgr );
	m_pPathKnowledgeMgr->Init(this);

	m_pGoalMgr = AI_FACTORY_NEW(CAIGoalMgr);

	m_fAccuracy = 0.0f;
	m_fAccuracyIncreaseRate = 0.0f;
	m_fFullAccuracyRadiusSqr = 256.f;
	m_fAccuracyMissPerturb = 64.f;
	m_fMaxMovementAccuracyPerturb = 10.f;
	m_fMovementAccuracyPerturbDecay = 3.f;

	m_eLastAwareness = kAware_Relaxed;
	m_nAlarmLevel = 0;
	m_eAlarmStimID = kStimID_Unset;

    m_bSeeThrough = true;
    m_bShootThrough = true;

    m_bActivated = false;
	m_bCanTalk = true;

	m_bInvulnerable = false;

	m_bUnconscious = false;

	m_bPreserveActiveCmds = false;

	m_fSenseUpdateRate	= 0.0f;
	m_fNextSenseUpdate	= 0.0f;
	m_flagsCurSenses	= kSense_All;

	m_rngSightGridX.Set(-2, 2);
	m_rngSightGridY.Set(-2, 2);

	m_pNodeTrackerContext = debug_new(CAINodeTrackerContext);

	m_pBrain = NULL;

	m_vPathingPosition = LTVector(0,0,0);

	m_bIsCinematicAI = false;

	m_bUseDefaultAttachments = true;
	m_bUseDefaultWeapons = true;
	m_eAmmoLoad = kAIAmmoLoadRecordID_Invalid;
	m_hAnimObject = NULL;

	// Add this instance to a list of all AI's.
	m_lstAIs.push_back( this );

	m_pAIWorkingMemory = debug_new( CAIWorkingMemory );
	m_pAIBlackBoard = AI_FACTORY_NEW( CAIBlackBoard );
	m_pAISensorMgr = AI_FACTORY_NEW( CAISensorMgr );
	m_pAIWorldState = AI_FACTORY_NEW( CAIWorldState );
	m_pAINavigationMgr = debug_new( CAINavigationMgr );
	m_pAIWeaponMgr = debug_new1( CAIWeaponMgr, this );
	m_pAICommandMgr = debug_new( CAICommandMgr );
	m_pAIPlan = NULL;

	// Not part of a team.
	m_nTeamId = INVALID_TEAM;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::~CAI()
//
//	PURPOSE:	Deallocate data
//
// ----------------------------------------------------------------------- //

CAI::~CAI()
{
    if (!g_pLTServer) return;

	// Remove the plan FIRST, as the plans deactivation makes use of 
	// subsystems such as AIMovement and the blackboard.
	if( m_pAIPlan )
	{
		AI_FACTORY_DELETE( m_pAIPlan );
		m_pAIPlan = NULL;
	}

	// Take us out of the stimulusmgr
	g_pAIStimulusMgr->RemoveSensingObject( this );

	if ( m_pState )
	{
		AI_FACTORY_DELETE(m_pState);
		m_pState = NULL;
	}

	if ( m_pAnimationContext )
	{
		debug_delete( m_pAnimationContext );
		m_pAnimationContext = NULL;
	}

	if( m_pPathKnowledgeMgr )
	{
		AI_FACTORY_DELETE( m_pPathKnowledgeMgr );
		m_pPathKnowledgeMgr = NULL;
	}

	if( m_pGoalMgr)
	{
		AI_FACTORY_DELETE(m_pGoalMgr);
		m_pGoalMgr = NULL;
	}

	if ( m_pAIMovement )
	{
		debug_delete( m_pAIMovement );
		m_pAIMovement = NULL;
	}

	m_hDialogueObject = NULL;

	AI_FACTORY_DELETE(m_pTarget);
	m_pTarget = NULL;

	if ( m_pBrain )
	{
		AI_FACTORY_DELETE(m_pBrain);
		m_pBrain = NULL;
	}

	debug_delete( m_pNodeTrackerContext );
	m_pNodeTrackerContext = NULL;

	// Erase this instance from the list of all PlayerObj's.
	AIList::iterator itAI = m_lstAIs.begin( );
	while( itAI != m_lstAIs.end( ))
	{
		if( *itAI == this )
		{
			m_lstAIs.erase( itAI );
			break;
		}

		itAI++;
	}

	// Delete the sensor manager before WorkingMemory, as some sensors do 
	// cleanup/verification in their destructor.  These goals use
	// WorkingMemory as a service which is always present.

	AI_FACTORY_DELETE( m_pAISensorMgr );
	m_pAISensorMgr = NULL;

	debug_delete( m_pAIWorkingMemory );
	m_pAIWorkingMemory = NULL;

	AI_FACTORY_DELETE( m_pAIBlackBoard );
	m_pAIBlackBoard = NULL;

	AI_FACTORY_DELETE( m_pAIWorldState );
	m_pAIWorldState = NULL;

	debug_delete( m_pAINavigationMgr );
	m_pAINavigationMgr = NULL;

	if (m_pAIWeaponMgr)
	{
		debug_delete( m_pAIWeaponMgr );
		m_pAIWeaponMgr = NULL;
	}

	debug_delete( m_pAICommandMgr );		
	m_pAICommandMgr = NULL;

	if ( m_eAlarmStimID != kStimID_Unset )
	{
		g_pAIStimulusMgr->RemoveStimulus( m_eAlarmStimID );
		m_eAlarmStimID = kStimID_Unset;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::SetNextServerUpdate
//
//	PURPOSE:	Set next update delta, and record the delta.
//
// ----------------------------------------------------------------------- //

void CAI::SetNextServerUpdate( float fDelta )
{
	g_pLTServer->SetNextUpdate( m_hObject, fDelta );
	m_fNextUpdateRate = fDelta;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::EngineMessageFn
//
//	PURPOSE:	Handle engine messages
//
// ----------------------------------------------------------------------- //

uint32 CAI::EngineMessageFn(uint32 messageID, void *pData, float fData)
{
	switch(messageID)
	{
		case MID_UPDATE:
		{
			SetNextServerUpdate( c_fUpdateDelta );

			if( m_bUpdateAI )
			{
				PreUpdate();
				Update();
			}
		}
		break;

		case MID_TOUCHNOTIFY:
		{
			HandleTouch( (HOBJECT)pData );
		}
		break;

		case MID_MODELSTRINGKEY:
		{
			HandleModelString((ArgList*)pData, (ANIMTRACKERID)fData);
		}
		break;

		case MID_ACTIVATING:
		{
			g_pAIStimulusMgr->AddSensingObject( this );
		}
		break;

		case MID_DEACTIVATING:
		{
			g_pAIStimulusMgr->RemoveSensingObject( this );
		}
		break;

		case MID_INITIALUPDATE:
		{
			m_pAIMovement->InitAIMovement(this);
			m_pAINavigationMgr->InitNavigationMgr( this );
			if (m_pAIWeaponMgr)
			{
				m_pAIWeaponMgr->InitAIWeaponMgr( this );
			}
			m_pAICommandMgr->InitAICommandMgr( this );

			SetNextServerUpdate( c_fUpdateDelta );

			g_pLTServer->SetNetFlags(m_hObject, NETFLAG_POSUNGUARANTEED|NETFLAG_ROTUNGUARANTEED|NETFLAG_ANIMUNGUARANTEED);

			m_pTarget->InitTarget(this);

			int nInfo = (int)fData;
			if (nInfo != INITIALUPDATE_SAVEGAME)
			{
				InitialUpdate();
			}

			// Set up movement encoding so that it will come from the main tracker's animations
			g_pModelLT->SetMovementEncodingTracker(m_hObject, MAIN_TRACKER);

			uint32 dwRet = CCharacter::EngineMessageFn(messageID, pData, fData);
			
			if (nInfo != INITIALUPDATE_SAVEGAME)
			{
				// Update our position, rotation, etc.  Need to do this
				// after the character does the initialupdate, because character
				// sets up our dims.
				UpdatePosition();

				// Update our can activate flags.  Need to do this after the character does the 
				// initialupdate because character creates our hitbox.
				UpdateUserFlagCanActivate();

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
			}

			return dwRet;
		}
		break;

		case MID_ALLOBJECTSCREATED:
		{
			// Run the AIConfig.

			if(!m_bConfigured)
			{
				m_AIConfig.ConfigureAI( this );
				m_bConfigured = true;
			}

			// Send the initial message to ourselves if we have one

			if ( !m_strCmdInitial.empty() )
			{
				g_pCmdMgr->QueueCommand( m_strCmdInitial.c_str(), this, this );
				m_strCmdInitial.clear();
			}
		}
		break;

		case MID_PRECREATE:
		{
			int nInfo = (int)fData;

            uint32 dwRet = CCharacter::EngineMessageFn(messageID, pData, fData);

			ObjectCreateStruct *pStruct = (ObjectCreateStruct*)pData;
			
			if (nInfo == PRECREATE_WORLDFILE || nInfo == PRECREATE_STRINGPROP)
			{
				// If we failed to create the AI, don't create the object.  
				// If we do, we end up with a partially initialized AI which 
				// causes many errors being reported which are fairly 
				// misleading (ie many subsystems may not be initialized at all).

				if ( !ReadProp(&pStruct->m_cProperties) )
				{
					return false;
				}

				PostReadProp(pStruct);
			}

			// Disable AIConfig if this AI is spawned from a template.
			// If an AI is spawned from an AISpawner, the AIConfig
			// on the AI template is disabled, and only the AIConfig
			// on the AISpawner should run.

			if( nInfo == PRECREATE_STRINGPROP )
			{
				m_AIConfig.EnableAIConfig( false );
			}

			if( nInfo != PRECREATE_SAVEGAME )
			{
				// Set our physics group to AI.
				pStruct->m_eGroup = PhysicsUtilities::ePhysicsGroup_UserAI;
			}

			return dwRet;
		}
		break;

		case MID_SAVEOBJECT:
		{
            uint32 dwRet = CCharacter::EngineMessageFn(messageID, pData, fData);
            Save((ILTMessage_Write*)pData, (uint32)fData);
			return dwRet;
		}
		break;

		case MID_LOADOBJECT:
		{
			// Need to load the base character first because some AI loading depends on data loaded from character...
			uint32 dwRet = CCharacter::EngineMessageFn(messageID, pData, fData);

            Load((ILTMessage_Read*)pData, (uint32)fData);
			return dwRet;
		}
		break;

		default:
		{
		}
		break;
	}

	return CCharacter::EngineMessageFn(messageID, pData, fData);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::ObjectMessageFn
//
//	PURPOSE:	Handle object-to-object messages
//
// ----------------------------------------------------------------------- //

uint32 CAI::ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg)
{
	int nResult = CCharacter::ObjectMessageFn(hSender, pMsg);

	pMsg->SeekTo(0);
	uint32 messageID = pMsg->Readuint32();

	switch(messageID)
	{
		case MID_DAMAGE:
		{
			if ( !Camera::IsActive() )
			{
				DamageStruct damage;
				damage.InitFromMessage(pMsg);

				HandleDamage(damage);

				// Register AllyPainSound stimulus.
				if( ( m_fLastPainVolume > 0.1f ) &&
					( m_fSoundOuterRadius > 0.f ) )
				{
					LTVector vPainPos;
					g_pLTServer->GetObjectPos(m_hObject, &vPainPos);

					StimulusRecordCreateStruct scs(kStim_PainSound, GetAlignment(), vPainPos, m_hObject);
					scs.m_flRadiusScalar = m_fLastPainVolume;
					g_pAIStimulusMgr->RegisterStimulus( scs );
				}
			}
		}
		break;

		case MID_ADDWEAPON:
		{
			m_pAIWeaponMgr->AddWeapon(hSender,pMsg, FAILURE_IS_ERROR);
			HandleArsenalChange();
		}

		default : break;
	}

	return nResult;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::ReadProp
//
//	PURPOSE:	Sets up the CAI instance with properties from the property
//				list.  Returns true if the AI was successfully created, 
//				false if he was not.
//
// ----------------------------------------------------------------------- //

bool CAI::ReadProp(const GenericPropList *pProps)
{
	// Insure all of the systems this function depends on exist.

	AIASSERT( g_pModelsDB, m_hObject, "CAI::ReadProp: No g_pModelsDB");
	AIASSERT( g_pLTServer, m_hObject, "CAI::ReadProp: No g_pLTServer." );
	AIASSERT( pProps, m_hObject, "CAI::ReadProp: No pProps." );
    if (!g_pLTServer || !pProps || !g_pModelsDB)
	{
		return false;
	}

	const char* pszName = pProps->GetString( "Name", "" );

	// Initialize the model, skeleton and animation system.

	const char* pszModelTemplate = pProps->GetString( "ModelTemplate", "" );
	m_hModel = g_pModelsDB->GetModelByRecordName( pszModelTemplate );
	if( !m_hModel )
	{
		AIASSERT2(m_hModel , m_hObject, "Failed to create AI '%s' : Invalid model specified.  %s is not a valid model name.  This AI is not properly configured and will not appear in game.", 
			pProps->GetString( "Name", "<null>" ),
			pszModelTemplate ? pszModelTemplate : "<null>" );
		return false;
	}

	m_hModelSkeleton = g_pModelsDB->GetModelSkeleton(m_hModel);
	AIASSERT1(m_hModelSkeleton, m_hObject, "%s : Invalid model skeleton specified.", pProps->GetString( "Name", "<null>" ));

	// Set the AIs template and brain, then apply template settings to the AI.

	SetBrain( pProps->GetString( "Brain", "" ) );
	if( !SetAIAttributes( g_pModelsDB->GetModelAIName( m_hModel ) ) )
	{
		AIASSERT1( 0, m_hObject, "Error setting up AIAttributes for model record: '%s'", pszModelTemplate );
	}

	// Apply template based settings to the AI.

	AIASSERT1(m_pAIAttributes, m_hObject, "%s : Failed to get current AIAttributes.", pProps->GetString( "Name", "<null>" ));
	if (m_pAIAttributes)
	{
		m_fJumpVel		= m_pAIAttributes->fJumpSpeed;
		m_fJumpOverVel	= m_pAIAttributes->fJumpOverSpeed;
		m_fFallVel		= m_pAIAttributes->fFallSpeed;
		m_fWalkVel		= m_pAIAttributes->fWalkSpeed;
		m_fSwimVel		= m_pAIAttributes->fSwimSpeed;
		m_fRunVel		= m_pAIAttributes->fRunSpeed;

		m_fAccuracy		= m_pAIAttributes->fAccuracy;
		m_fAccuracyIncreaseRate		= m_pAIAttributes->fAccuracyIncreaseRate;
		m_fFullAccuracyRadiusSqr	= m_pAIAttributes->fFullAccuracyRadiusSqr;
		m_fAccuracyMissPerturb		= m_pAIAttributes->fAccuracyMissPerturb;
		m_fMaxMovementAccuracyPerturb	= m_pAIAttributes->fMaxMovementAccuracyPerturb;
		m_fMovementAccuracyPerturbDecay = m_pAIAttributes->fMovementAccuracyPerturbDecay;
		m_fSoundOuterRadius				= m_pAIAttributes->fSoundOuterRadius <= 0.0 ? m_fSoundOuterRadius : m_pAIAttributes->fSoundOuterRadius;
		m_fSoundInnerRadius				= m_pAIAttributes->fSoundInnerRadius <= 0.0 ? m_fSoundInnerRadius : m_pAIAttributes->fSoundInnerRadius;

		// Distance up and down an AI checks for volumes
		m_flVerticalThreshold = m_pAIAttributes->fVerticalThreshold;

		// Set the Update Rate in the Attributes now, as this information
		// is not related to the characters alignment in any way.  Default
		// value should
		m_fSenseUpdateRate = m_pAIAttributes->fUpdateRate;

		// Set the AIs action set.
		ENUM_AIActionSet eAIActionSet = 
			g_pAIDB->GetAIActionSetRecordID(m_pAIAttributes->strAIActionSet.c_str());
		GetAIBlackBoard()->SetBBAIActionSet(eAIActionSet);


		// Set the AIs target selection set.
		ENUM_AITargetSelectSet eTargetSelectSet = 
			g_pAIDB->GetAITargetSelectSetRecordID(m_pAIAttributes->strTargetSelectSet.c_str());
		GetAIBlackBoard()->SetBBAITargetSelectSet(eTargetSelectSet);

		// Set the AIs activity set.
		ENUM_AIActivitySet eAIActivitySet = 
			g_pAIDB->GetAIActivitySetRecordID(m_pAIAttributes->strAIActivitySet.c_str());
		GetAIBlackBoard()->SetBBAIActivitySet(eAIActivitySet);
	
		// Set the AIs movement set.
		ENUM_AIMovementSetID eAIMovementSet =
			g_pAIDB->GetAIMovementSetRecordID(m_pAIAttributes->strAIMovementSet.c_str());
		GetAIBlackBoard()->SetBBAIMovementSet(eAIMovementSet);

		// Set the AIs WeaponOverride set.
		ENUM_AIWeaponOverrideSetID eAIWeaponOverrideSet =
			g_pAIDB->GetAIWeaponOverrideSetRecordID(m_pAIAttributes->strAIWeaponOverrideSet.c_str());
		GetAIBlackBoard()->SetBBAIWeaponOverrideSet(eAIWeaponOverrideSet);

		// Store the AIs alignment.
		m_eAlignment = g_pCharacterDB->String2Alignment( m_pAIAttributes->strAlignment.c_str() );

		// Configure the blackboard with template based information.
		m_pAIBlackBoard->SetBBSeeDistance( m_pAIAttributes->fSeeDistance );
		m_pAIBlackBoard->SetBBHearDistance( m_pAIAttributes->fHearDistance );

		// Store the distance an AI must be facing a direction to apply a 
		// directional change anim.
		m_pAIBlackBoard->SetBBMinDirectionalRunChangeDistanceSqr( m_pAIAttributes->fMinDirectionalRunChangeDistanceSqr );

		// Store the AIs instant death setting.
		m_pAIBlackBoard->SetBBInstantDeath( m_pAIAttributes->bInstantDeath );
	}

	// Set the goal system
	const char* pszGoalSet = pProps->GetString( "GoalSet", "" );
	if( pszGoalSet[0] )
	{
		m_pAISensorMgr->InitSensorMgr( this );
		m_pGoalMgr->Init(this);		
		m_pGoalMgr->SetGoalSet( pszGoalSet, pszName, false );
	}

	// Default attachments.

	m_bUseDefaultAttachments = pProps->GetBool( "UseDefaultAttachments", m_bUseDefaultAttachments );
	m_bUseDefaultWeapons = pProps->GetBool( "UseDefaultWeapons", m_bUseDefaultWeapons );

	// Read the AmmoLoad.

	m_eAmmoLoad = g_pAIDB->GetAIAmmoLoadRecordID(pProps->GetString( "AmmoLoad", "" ));

	// Create holster string.

	char	szHolster[128] = {0};
	uint32	dwWeapon = 0;
	bool	bReadWeapon = true;

	while( bReadWeapon )
	{
		char	szPropName[32] = {0};
		LTSNPrintF( szPropName, ARRAY_LEN( szPropName ), "Weapon%i", dwWeapon );
		const char* const pszWeaponName = pProps->GetString( szPropName, "" );
		if( pszWeaponName[0] )
		{
			// Don't read a weapon that isn't a valid selection...

			if( pszWeaponName[0] && !LTStrIEquals( SELECTION_NONE, pszWeaponName ) )
			{
				AIASSERT( 1 + LTStrLen( szHolster ) + LTStrLen( pszWeaponName ) < 128, m_hObject, "CAI::ReadProp: Holster string exceeds 128 chars." );
				LTStrCat( szHolster, pszWeaponName, LTARRAYSIZE(szHolster));
				LTStrCat( szHolster, ";", LTARRAYSIZE(szHolster) );
			}
		}
		else
		{
			// No more weapon properties to read in...

			bReadWeapon = false;
		}

		++dwWeapon;
	}

	// Holster weapons.

	bool bHolsterWeapons = pProps->GetBool( "HolsterWeapons", false );
	if( bHolsterWeapons && szHolster[0] )
	{
		char szHolsterCopy[129];
		LTStrCpy( szHolsterCopy, "-", LTARRAYSIZE(szHolsterCopy));
		LTStrCat( szHolsterCopy, szHolster, LTARRAYSIZE(szHolsterCopy) );

		if( m_pAIWeaponMgr && szHolsterCopy[0] )
		{
			m_pAIWeaponMgr->SetHolsterString(szHolsterCopy);
		}
	}

	// Set next sense update.
	m_fNextSenseUpdate = g_pLTServer->GetTime() + s_fSenseUpdateBasis;
	s_fSenseUpdateBasis += .02f;
	if ( s_fSenseUpdateBasis > 0.5f )
	{
		s_fSenseUpdateBasis = 0.0f;
	}

	m_bIsCinematicAI	= pProps->GetBool( "IsCinematicAI", m_bIsCinematicAI );
	m_bCanTalk			= pProps->GetBool( "CanTalk", m_bCanTalk );

	// Commands

	const char* const pszInitialCommand = pProps->GetCommand( "Initial", "" );
	m_strCmdInitial = pszInitialCommand ? pszInitialCommand : "";

	const char* const pszActivationOn = pProps->GetCommand( "ActivateOn", "" );
	m_strCmdActivateOn = pszActivationOn ? pszActivationOn : "";

	const char* const pszActivationOff = pProps->GetCommand( "ActivateOff", "" );
	m_strCmdActivateOff =  pszActivationOff ? pszActivationOff : "";
	
	// Overrides last.

	const char* const pszInstantDeath = pProps->GetString( "OV_InstantDeath", "" );
	if ( pszInstantDeath[0] )
	{
		m_pAIBlackBoard->SetBBInstantDeath( IsTrueChar( pszInstantDeath[0] ) );
	}

	#define READ_OVERRIDE_PROP( member, label ) \
		const char* const psz##label = pProps->GetString( #label, "" ); \
		if( psz##label[0] ) { member = (float)atof( psz##label ); }

	READ_OVERRIDE_PROP( m_fSoundOuterRadius, OV_SoundRadius );
	READ_OVERRIDE_PROP( m_fSoundInnerRadius, OV_SoundInnerRadius );

	READ_OVERRIDE_PROP( m_fJumpVel, OV_JumpSpeed );
	READ_OVERRIDE_PROP( m_fWalkVel, OV_WalkSpeed );
	READ_OVERRIDE_PROP( m_fSwimVel, OV_SwimSpeed );
	READ_OVERRIDE_PROP( m_fRunVel, OV_RunSpeed );

	READ_OVERRIDE_PROP( m_fAccuracy, OV_Accuracy );
	READ_OVERRIDE_PROP( m_fAccuracyIncreaseRate, OV_AccuracyIncreaseRate );
	READ_OVERRIDE_PROP( m_fAccuracyMissPerturb, OV_AccuracyMissPerturb );
	READ_OVERRIDE_PROP( m_fMaxMovementAccuracyPerturb, OV_MaxMovementAccuracyPerturb );

	const char* const pszAccuracyRadius = pProps->GetString( "OV_FullAccuracyRadius", "" );
	if( pszAccuracyRadius[0] )
	{
		m_fFullAccuracyRadiusSqr = (float)atof( pszAccuracyRadius );
		m_fFullAccuracyRadiusSqr *= m_fFullAccuracyRadiusSqr;
	}

	const char* const pszAccuracyPerturbTime = pProps->GetString( "OV_MovementAccuracyPerturbTime", "" );
	if( pszAccuracyPerturbTime[0] )
	{
		m_fMovementAccuracyPerturbDecay = (float)atof( pszAccuracyPerturbTime );
		m_fMovementAccuracyPerturbDecay = m_fMaxMovementAccuracyPerturb / m_fMovementAccuracyPerturbDecay;
	}

	float flWeaponUpgradeDesire = 1.f;
	READ_OVERRIDE_PROP( flWeaponUpgradeDesire, OV_WeaponUpgradeDesire );
	flWeaponUpgradeDesire = Clamp(flWeaponUpgradeDesire, 0.f, 1.0f );

	CAIWMFact* pFact = m_pAIWorkingMemory->CreateWMFact( kFact_Desire );
	if ( pFact )
	{
		pFact->SetDesireType( kDesire_ObtainBetterWeapon, flWeaponUpgradeDesire);
	}

    return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::PostReadProp
//
//	PURPOSE:	Update the ObjectCreateStruct when creating the object
//
// ----------------------------------------------------------------------- //

void CAI::PostReadProp(ObjectCreateStruct *pStruct)
{
	const GenericPropList *pProps = &pStruct->m_cProperties;
	
	// Set up the model and materials.

	const char* pFilename = g_pModelsDB->GetModelFilename(m_hModel);
	if (pFilename && pFilename[0])
	{
		pStruct->SetFileName(pFilename);
	}

	g_pModelsDB->CopyMaterialFilenames(m_hModel, pStruct->m_Materials[0], LTARRAYSIZE( pStruct->m_Materials ), 
		LTARRAYSIZE( pStruct->m_Materials[0] ));

	// Check for random or overriding body textures.

	const char* const	pszBodyMaterialExtension = pProps->GetString( "BodyMaterialExtension", "" );

	if( pszBodyMaterialExtension 
		&& !LTStrEmpty(pszBodyMaterialExtension) 
		&& !LTStrEmpty(pStruct->m_Materials[BODY_MATERIAL_INDEX]) )
	{
		// insert the value into into pData.
		// get the head ext value

		if( LTStrIEquals( pszBodyMaterialExtension, MATERIAL_EXT_RANDOM ))
		{
			// Randomly use one of the 'AltBodyMaterial#' for the model...

			uint8 nMaterials = g_pModelsDB->GetNumAltBodyMaterials( m_hModel );
			if( nMaterials > 0 )
			{
				int iRandomMaterialIndex = GetRandom( 0, nMaterials - 1 );
				const char *pMaterial = g_pModelsDB->GetAltBodyMaterial( m_hModel, iRandomMaterialIndex );
				if( pMaterial )
				{
					pStruct->SetMaterial(BODY_MATERIAL_INDEX, pMaterial );
				}
			}
		}
		else
		{
			//This is horribly unsafe and doesn't perform any of the appropriate error checking.
			//MUCH cleaner approaches could be taken...

			// back search for the extension indicator
			const char *szExt   = strrchr(pStruct->m_Materials[BODY_MATERIAL_INDEX],'.');
			// get the position of the extension
			uint32 pos = szExt - (char*)(pStruct->m_Materials[BODY_MATERIAL_INDEX]) + 1 ;

			// add the headextension.
			LTStrCpy( pStruct->m_Materials[BODY_MATERIAL_INDEX] + pos-1, pszBodyMaterialExtension, LTARRAYSIZE(pStruct->m_Materials[BODY_MATERIAL_INDEX]) - (pos - 1));

			// put back the file extension.
			LTStrCpy( (pStruct->m_Materials[BODY_MATERIAL_INDEX]) + pos + (LTStrLen(pszBodyMaterialExtension)-1), "."RESEXT_MATERIAL, LTARRAYSIZE(pStruct->m_Materials[BODY_MATERIAL_INDEX]) - (pos + (LTStrLen(pszBodyMaterialExtension)-1)));
		}
	}

	const char* const pszHeadExtension = pProps->GetString( "HeadExtension", "" );

	// fix the material names if head extension or body extension have values.
	if( pszHeadExtension 
		&& !LTStrEmpty(pszHeadExtension) 
		&& !LTStrEmpty( pStruct->m_Materials[HEAD_MATERIAL_INDEX] ))
	{
		// insert the value into into pStruct.
		// get the head ext value

		if( LTStrIEquals(pszHeadExtension, MATERIAL_EXT_RANDOM ))
		{
			// Randomly use one of the 'AltHeadMaterial#' for the model...

			uint8 nMaterials = g_pModelsDB->GetNumAltHeadMaterials( m_hModel );
			if( nMaterials > 0 )
			{
				int iRandomMaterialIndex = GetRandom( 0, nMaterials - 1 );
				const char *pMaterial = g_pModelsDB->GetAltHeadMaterial( m_hModel, iRandomMaterialIndex);
				if( pMaterial )
				{
					pStruct->SetMaterial(HEAD_MATERIAL_INDEX, pMaterial);
				}
			}
		}
		else
		{
			//This is horribly unsafe and doesn't perform any of the appropriate error checking.
			//MUCH cleaner approaches could be taken...

			// back search for the extention indicator
			const char *szExt   = strrchr(pStruct->m_Materials[HEAD_MATERIAL_INDEX],'.');
			// get the position of the extention
			uint32 pos = szExt - (char*)(pStruct->m_Materials[HEAD_MATERIAL_INDEX]) + 1 ;

			// add the headextension.
			LTStrCpy( pStruct->m_Materials[HEAD_MATERIAL_INDEX] +pos -1, pszHeadExtension, LTARRAYSIZE(pStruct->m_Materials[BODY_MATERIAL_INDEX]) - (pos - 1));
			// put back the file extension.
			LTStrCpy( (pStruct->m_Materials[HEAD_MATERIAL_INDEX]) + pos + (LTStrLen(pszHeadExtension)-1), "."RESEXT_MATERIAL, LTARRAYSIZE(pStruct->m_Materials[BODY_MATERIAL_INDEX]) - (pos + (LTStrLen(pszHeadExtension)-1)));
		}
	}

	// Read the specified child models...
	// add child models but since we are not guarenteed that the client side object
	// is created yet, we're going to put the new child models on the objectcreatestruct.
	// also we are going to cache our new file...

	for( uint32 nModel = 0 ; nModel < MAX_DEDIT_CHILD_MODELS ; ++nModel )
	{
		char szPropValue[256];
		LTSNPrintF( szPropValue,LTARRAYSIZE(szPropValue), "ChildModel_%d", nModel + 1 );

		const char *pszChildModel = pProps->GetString( szPropValue, "" );

		if(!LTStrEmpty(pszChildModel))
		{
			// cache the requested child model and get a ref to it
			LTStrCpy( pStruct->m_ChildModels[nModel], pszChildModel, LTARRAYSIZE( pStruct->m_ChildModels[nModel] ));
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::GetName()
//
//	PURPOSE:	Get the character's name.
//
// ----------------------------------------------------------------------- //

const char* CAI::GetName() const 
{
	return ::ToString( m_hObject );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::InitialUpdate()
//
//	PURPOSE:	Initialize the AI routines
//
// ----------------------------------------------------------------------- //
void CAI::InitialUpdate()
{
    if( !m_hObject )
		return;

	m_pAnimationContext = debug_new( CAnimationContext );	
	AIASSERT( m_pAnimationContext, m_hObject, "Failed to create AnimationContext." );

	m_pAnimationContext->Init( m_hObject, m_hModel );

	// Add the trees and insure animation is set to the true base (ie no 
	// properties set)  This is required as the index of the base animation
	// isn't known until after the trees have been added.  If this isn't 
	// done, the AIs animation the first frame and its props may be out of 
	// sync, which results in various warning messages if the value of a group
	//  is queried (ie GetCurrentAnimationProps asserts due to the props not 
	//  matching the animation.

	const char* pszAnimTree;
	uint32 cAnimTrees = g_pModelsDB->GetNumModelAnimationTrees( m_hModel );
	for( uint32 iAnimTree=0; iAnimTree < cAnimTrees; ++iAnimTree )
	{
		pszAnimTree = g_pModelsDB->GetModelAnimationTree( m_hModel, iAnimTree );
		if( pszAnimTree[0] )
		{
			m_pAnimationContext->AddAnimTreePacked( pszAnimTree );
		}
	}
	m_pAnimationContext->SetSpecial( CAnimationProps() );

	// Make sure the packed trees were actullay added...
	if( m_pAnimationContext->GetNumPackedTrees() == 0 )
	{
		debug_delete( m_pAnimationContext );
		m_pAnimationContext = NULL;
	}

	// Add the recoil tracker after loading the animation context...
	// This needs to be done after the other animation trackers because
	// recoils are additively blended so the recoil tracker should be
	// the last tracker added...
	AddRecoilTracker( );
	
	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_AI_CLIENT_SOLID, USRFLG_AI_CLIENT_SOLID );

	UpdateUserFlagCanActivate();

	if ( m_bCheapMovement )
	{
		m_dwFlags &= ~FLAG_GRAVITY;
		m_dwFlags |= FLAG_GOTHRUWORLD;
		m_dwFlags &= ~FLAG_SOLID;
	}

	// If the AI is made invisible during his initial update, he may not be 
	// sent to the client.  This defeats rigidbody network transmission, 
	// which sets FLAG_FORCECLIENTUPDATE flag internally to force 
	// transmission.  If we don't set this here, FLAG_FORCECLIENTUPDATE 
	// will be cleared when m_dwFlags is applied to the model in character.cpp.

	m_dwFlags |= FLAG_FORCECLIENTUPDATE;

	// Set the model's default physics weight set...

	const char* pPhysicsWeightSetName = g_pModelsDB->GetDefaultPhysicsWeightSet(m_hModel);
	if (pPhysicsWeightSetName)
	{
		SetPhysicsWeightSet(pPhysicsWeightSetName);
	}


	// Default WorldState values.

	m_pAIWorldState->SetWSProp( kWSK_AnimLooped, m_hObject, kWST_int, (int)INVALID_MODEL_ANIM );
	m_pAIWorldState->SetWSProp( kWSK_AnimPlayed, m_hObject, kWST_int, (int)INVALID_MODEL_ANIM );
	m_pAIWorldState->SetWSProp( kWSK_AtNode, m_hObject, kWST_HOBJECT, 0 );
	m_pAIWorldState->SetWSProp( kWSK_AtNodeType, m_hObject, kWST_EnumAINodeType, kNode_InvalidType );
	m_pAIWorldState->SetWSProp( kWSK_AtTargetPos, m_hObject, kWST_bool, false );
	m_pAIWorldState->SetWSProp( kWSK_CoverStatus, m_hObject, kWST_EnumAnimProp, kAP_None );
	m_pAIWorldState->SetWSProp( kWSK_DisturbanceExists, m_hObject, kWST_bool, false );
	m_pAIWorldState->SetWSProp( kWSK_Idling, m_hObject, kWST_bool, false );
	m_pAIWorldState->SetWSProp( kWSK_MountedObject, m_hObject, kWST_HOBJECT, 0 );
	m_pAIWorldState->SetWSProp( kWSK_PositionIsValid, m_hObject, kWST_bool, true );
	m_pAIWorldState->SetWSProp( kWSK_ReactedToWorldStateEvent, m_hObject, kWST_ENUM_AIWorldStateEvent, kWSE_Invalid );
	m_pAIWorldState->SetWSProp( kWSK_RidingVehicle, m_hObject, kWST_EnumAnimProp, kAP_None );
	m_pAIWorldState->SetWSProp( kWSK_SurveyedArea, m_hObject, kWST_bool, false );
	m_pAIWorldState->SetWSProp( kWSK_TargetIsAimingAtMe, m_hObject, kWST_bool, false );
	m_pAIWorldState->SetWSProp( kWSK_TargetIsDead, m_hObject, kWST_bool, false );
	m_pAIWorldState->SetWSProp( kWSK_TargetIsFlushedOut, m_hObject, kWST_bool, false );
	m_pAIWorldState->SetWSProp( kWSK_TargetIsSuppressed, m_hObject, kWST_bool, false );
	m_pAIWorldState->SetWSProp( kWSK_TraversedLink, m_hObject, kWST_ENUM_NMLinkID, kNMLink_Invalid );
	m_pAIWorldState->SetWSProp( kWSK_UsingObject, m_hObject, kWST_HOBJECT, 0 );
	m_pAIWorldState->SetWSProp( kWSK_WeaponArmed, m_hObject, kWST_bool, false );
	m_pAIWorldState->SetWSProp( kWSK_WeaponLoaded, m_hObject, kWST_bool, true );

	// Initialize the client.

	m_pAIBlackBoard->SetBBUpdateClient( true );

	// Do not update if AI was not successfully setup.

	if( !m_pAnimationContext )
	{
		SetNextServerUpdate( UPDATE_NEVER );
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleCharacterRemoval()
//
//	PURPOSE:	Handle removing a character.
//
// --------------------------------------------------------------------------- //

void CAI::HandleCharacterRemoval()
{
	super::HandleCharacterRemoval();
	RemoveFromAIRegions();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleNavMeshPolyEnter()
//
//	PURPOSE:	Handle entering a new NavMesh poly.
//
// ----------------------------------------------------------------------- //

void CAI::HandleNavMeshPolyEnter( ENUM_NMPolyID ePoly )
{
	super::HandleNavMeshPolyEnter( ePoly );

	// If AI is re-entering the NavMesh, or entering the
	// NavMesh for the first time, clear any cached pathfinding data.

	if( m_eLastNavMeshPoly == kNMPoly_Invalid )
	{
		m_pPathKnowledgeMgr->ClearPathKnowledge();
	}

	// Get the link entered, if poly has a link.

	if( HasCurrentNavMeshLink() )
	{
		// Get the link.

		EnumAINavMeshLinkType eLinkType = kLink_InvalidType;
		AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( GetCurrentNavMeshLink() );
		if( pLink )
		{
			eLinkType = pLink->GetNMLinkType();

			// The Link may do something upon entry.

			pLink->HandleNavMeshLinkEnter( this );
		}

		// Trace the poly and type of link entered.
		// Re-evaluate goals immediately.

		if( pLink && ( eLinkType != kLink_InvalidType ) )
		{
			AITRACE( AIShowPaths, ( m_hObject, "Entered NavMeshLink%s: %s %d", s_aszNavMeshLinkTypes[eLinkType], pLink->GetName(), ePoly ) );
			m_pAIBlackBoard->SetBBSelectAction( true );
		}
		else {
			AITRACE( AIShowPaths, ( m_hObject, "Entered Invalid NavMeshLink: %d", ePoly ) );
		}
	}

	// Trace the poly entered.

	else {
		AITRACE( AIShowPaths, ( m_hObject, "Entered NavMeshPoly: %d", ePoly ) );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleNavMeshPolyExit()
//
//	PURPOSE:	Handle exiting a NavMesh poly.
//
// ----------------------------------------------------------------------- //

void CAI::HandleNavMeshPolyExit( ENUM_NMPolyID ePoly )
{
	super::HandleNavMeshPolyExit( ePoly );

	// Exited poly may have an associated NavMeshLink.

	CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( ePoly );
	if( pPoly && ( pPoly->GetNMLinkID() != kNMLink_Invalid ) )
	{
		AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( pPoly->GetNMLinkID() );
		if( pLink )
		{
			// The Link may do something upon exit.

			pLink->HandleNavMeshLinkExit( this );
			m_pAIBlackBoard->SetBBTraversingNMLink( kNMLink_Invalid );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::InitAnimation()
//
//	PURPOSE:	Initializes our animation
//
// ----------------------------------------------------------------------- //

void CAI::InitAnimation()
{
	CCharacter::InitAnimation();

	// The Blend Tracker may change whenever this function is called.  Make 
	// sure this change is reflected in the animation context.  This may be 
	// an issue if the CAIs model is ever changed.

	if ( m_pAnimationContext )
	{
		m_pAnimationContext->SetNullWeightset( m_hNullWeightset );
		m_pAnimationContext->SetBlendTracker( m_BlendAnimTracker );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleAIRegionEnter()
//
//	PURPOSE:	Handle entering a new AIRegion.
//
// ----------------------------------------------------------------------- //

void CAI::RemoveFromAIRegions()
{
	// Bail if AI was not in NavMesh.

	CAINavMeshPoly* pPoly = g_pAINavMesh->GetNMPoly( m_eLastNavMeshPoly );
	if( !pPoly )
	{
		return;
	}

	// Remove AI from each AIRegion.

	ENUM_AIRegionID eAIRegion;
	for( int iAIRegion=0; iAIRegion < pPoly->GetNumAIRegions(); ++iAIRegion )
	{
		eAIRegion = pPoly->GetAIRegion( iAIRegion );
		HandleAIRegionExit( eAIRegion );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleAIRegionEnter()
//
//	PURPOSE:	Handle entering a new AIRegion.
//
// ----------------------------------------------------------------------- //

void CAI::HandleAIRegionEnter( ENUM_AIRegionID eAIRegion )
{
	super::HandleAIRegionEnter( eAIRegion );

	// Bail if AIRegion does not exist.

	AIRegion* pAIRegion = g_pAINavMesh->GetAIRegion( eAIRegion );
	if( !pAIRegion )
	{
		return;
	}

	// AIRegion does not accept this type of AI.

	if( !( pAIRegion->GetCharTypeMask() & GetCharTypeMask() ) )
	{
		return;
	}

	// Enter the AIRegion.

	pAIRegion->EnterAIRegion( this );

	// Trace the AIRegion entered.

	AITRACE( AIShowPaths, ( m_hObject, "Entered AIRegion: %s", pAIRegion->GetName() ) );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleAIRegionExit()
//
//	PURPOSE:	Handle exiting an AIRegion.
//
// ----------------------------------------------------------------------- //

void CAI::HandleAIRegionExit( ENUM_AIRegionID eAIRegion )
{
	super::HandleAIRegionExit( eAIRegion );

	// Bail if AIRegion does not exist.

	AIRegion* pAIRegion = g_pAINavMesh->GetAIRegion( eAIRegion );
	if( !pAIRegion )
	{
		return;
	}

	// AIRegion does not accept this type of AI.

	if( !( pAIRegion->GetCharTypeMask() & GetCharTypeMask() ) )
	{
		return;
	}

	// Exit the AIRegion.

	pAIRegion->ExitAIRegion( this );

	// Trace the AIRegion exited.

	AITRACE( AIShowPaths, ( m_hObject, "Exited AIRegion: %s", pAIRegion->GetName() ) );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleMeleeBlocked
//
//	PURPOSE:	Handle being blocked.  Translate this event into a fact,
//				and store it in working memory.
//
// ----------------------------------------------------------------------- //

void CAI::HandleMeleeBlocked()
{
	super::HandleMeleeBlocked();

	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Knowledge );
	queryFact.SetKnowledgeType( kKnowledge_MeleeBlocked );
	
	CAIWMFact* pFact = m_pAIWorkingMemory->FindWMFact( queryFact );
	if ( !pFact )
	{
		pFact = m_pAIWorkingMemory->CreateWMFact( kFact_Knowledge );
		if ( pFact )
		{
			pFact->SetKnowledgeType( kKnowledge_MeleeBlocked );
		}
	}

	if ( pFact )
	{
		pFact->SetTime( g_pLTServer->GetTime() );
	}

	// Request a behavior update.

	m_pAIBlackBoard->SetBBSelectAction( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleMeleeBlocked
//
//	PURPOSE:	Handle successfully blocking.  Translate this event into 
//				a fact, and store it in working memory.
//
// ----------------------------------------------------------------------- //

void CAI::HandleMeleeBlockedAttacker()
{
	super::HandleMeleeBlockedAttacker();

	// Clear the desire to counter the melee (to insure we don't repeat 
	// this)

	CAIWMFact queryFactCounterMeleeDesire;
	queryFactCounterMeleeDesire.SetFactType( kFact_Desire );
	queryFactCounterMeleeDesire.SetDesireType( kDesire_CounterMelee );
	m_pAIWorkingMemory->ClearWMFacts( queryFactCounterMeleeDesire );

	// Notify the AI that the blocking was successful.

	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Knowledge );
	queryFact.SetKnowledgeType( kKnowledge_MeleeBlockedAttacker );
	
	CAIWMFact* pFact = m_pAIWorkingMemory->FindWMFact( queryFact );
	if ( !pFact )
	{
		pFact = m_pAIWorkingMemory->CreateWMFact( kFact_Knowledge );
		if ( pFact )
		{
			pFact->SetKnowledgeType( kKnowledge_MeleeBlockedAttacker );
		}
	}

	if ( pFact )
	{
		pFact->SetTime( g_pLTServer->GetTime() );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleBerserkerKicked
//
//	PURPOSE:	Handle responding to being kicked by the player during a 
//				berserker attack.
//
// ----------------------------------------------------------------------- //

void CAI::HandleBerserkerKicked()
{
	CAIWMFact queryFact;
	queryFact.SetFactType( kFact_Knowledge );
	queryFact.SetKnowledgeType( kKnowledge_BerserkerKicked );
	CAIWMFact* pFact = m_pAIWorkingMemory->FindWMFact( queryFact );
	if ( pFact )
	{
		AITRACE( AIShowBerserker, ( GetHOBJECT(), "Kicked while still responding to a kick." ) );
		return;
	}

	AITRACE( AIShowBerserker, ( GetHOBJECT(), "Kicked." ) );
	pFact = m_pAIWorkingMemory->CreateWMFact( kFact_Knowledge );
	if ( pFact )
	{
		pFact->SetKnowledgeType( kKnowledge_BerserkerKicked );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleFinishingMove
//
//	PURPOSE:	Respond to the player performing a finishing move on us.
//
// ----------------------------------------------------------------------- //

void CAI::HandleFinishingMove( HRECORD hSyncAction )
{
	if ( m_pAIBlackBoard->GetBBFinishingSyncAction() )
	{
		AITRACE( AIShowFinishingMove, ( GetHOBJECT(), "Trying to start a finishing move while already performing one." ) );
		return;
	}

	AITRACE( AIShowFinishingMove, ( GetHOBJECT(), "Begin finishing move." ) );

	// Set the finishing action and force a action selection.

	m_pAIBlackBoard->SetBBFinishingSyncAction( hSyncAction );
	m_pAIBlackBoard->SetBBSelectAction( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleDamagedPlayer
//
//	PURPOSE:	Handles responding to the 'damaged player' event.
//
// ----------------------------------------------------------------------- //

void CAI::HandleDamagedPlayer( HOBJECT hPlayer )
{
	// If the DamagedPlayer activation count is 0 (infinite) of if the number 
	// of activations is less than the count, activate.

	if ( 0 == m_nDamagedPlayerActivationCount 
		|| m_nDamagedPlayerNumCount < m_nDamagedPlayerActivationCount )
	{
		if ( !m_strCmdDamagedPlayer.empty() )
		{
			g_pCmdMgr->QueueCommand( m_strCmdDamagedPlayer.c_str(), this, this );
		}

		++m_nDamagedPlayerNumCount;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::FindFloorHeight
//
//	PURPOSE:	Find height of the floor under AI at some position.
//
// ----------------------------------------------------------------------- //

bool CAI::FindFloorHeight(const LTVector& vPos, float* pfFloorHeight)
{
	AIASSERT( pfFloorHeight, m_hObject, "CAI::FindFloorHeight: fFloorHeight is NULL" );

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_From = LTVector(vPos.x, vPos.y + m_vDims.y, vPos.z);
	IQuery.m_To = LTVector(vPos.x, vPos.y - m_vDims.y*10.0f, vPos.z);

	IQuery.m_Flags = INTERSECT_OBJECTS | IGNORE_NONSOLID | INTERSECT_HPOLY;
	IQuery.m_FilterFn = GroundFilterFn;

	g_cIntersectSegmentCalls++;
    if (g_pLTServer->IntersectSegment(IQuery, &IInfo) && (IsMainWorld(IInfo.m_hObject) || (OT_WORLDMODEL == GetObjectType(IInfo.m_hObject))))
	{
		*pfFloorHeight = IInfo.m_Point.y + m_vDims.y;

		// [KLS 8/8/02] Set the standing on surface based on whatever we're standing on.
		// This is used in CCharacter::HandleModelString() to determine the stimuli
		// to register for AI walking around...

		m_eStandingOnSurface = GetSurfaceType(IInfo);
		
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::*ProcessingStimulus()
//
//	PURPOSE:	Handle stimuli
//
// ----------------------------------------------------------------------- //

bool CAI::GetDoneProcessingStimuli() const
{
	if( m_pAISensorMgr )
	{
		return m_pAISensorMgr->GetDoneProcessingStimuli();
	}

	return false;
}

void CAI::SetDoneProcessingStimuli( bool bDone )
{
	if( m_pAISensorMgr )
	{
		m_pAISensorMgr->SetDoneProcessingStimuli( bDone );
	}
}

void CAI::ClearProcessedStimuli()
{
	if( m_pAISensorMgr )
	{
		m_pAISensorMgr->ClearProcessedStimuli();
	}
}

bool CAI::ProcessStimulus(CAIStimulusRecord* pRecord)
{
	if( m_pAISensorMgr )
	{
		return m_pAISensorMgr->ProcessStimulus( pRecord );
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::*IntersectSegmentCount()
//
//	PURPOSE:	Handle IntersectSegment Counting.
//
// ----------------------------------------------------------------------- //

int	CAI::GetIntersectSegmentCount() const
{
	if( m_pAISensorMgr )
	{
		return m_pAISensorMgr->GetIntersectSegmentCount();
	}

	return 0;
}

void CAI::ClearIntersectSegmentCount()
{
	if( m_pAISensorMgr )
	{
		m_pAISensorMgr->ClearIntersectSegmentCount();
	}
}

void CAI::IncrementIntersectSegmentCount()
{
	if( m_pAISensorMgr )
	{
		m_pAISensorMgr->IncrementIntersectSegmentCount();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleSense*()
//
//	PURPOSE:	Handle senses
//
// ----------------------------------------------------------------------- //

bool CAI::HandleSenseRecord(CAIStimulusRecord* pStimulusRecord, uint32 nCycle)
{
	return m_pAISensorMgr->StimulateSensors( pStimulusRecord );
}

void CAI::HandleSenses(uint32 nCycle)
{
	if( m_pAISensorMgr )
	{
		m_pAISensorMgr->HandleSenses( nCycle );
	}
}

uint32 CAI::GetCurSenseFlags() const
{
	return ( m_pAIBlackBoard->GetBBSensesOn() ) ? m_flagsCurSenses : kSense_None | kSense_Tactile; 
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::Activate
//
//	PURPOSE:	Play a cineractive animation.
//
// ----------------------------------------------------------------------- //

void CAI::Activate()
{
	if (m_bCanTalk )
	{
		m_bActivated = !m_bActivated;

		if ( m_bActivated && !m_strCmdActivateOn.empty() )
		{
          g_pCmdMgr->QueueCommand( m_strCmdActivateOn.c_str(), this, this );

			if ( !m_bPreserveActiveCmds )
			{
				m_strCmdActivateOn.clear();
			}

			UpdateUserFlagCanActivate();
		}
		else if ( !m_bActivated && !m_strCmdActivateOff.empty() )
		{
			g_pCmdMgr->QueueCommand( m_strCmdActivateOff.c_str(), this, this );

			if ( !m_bPreserveActiveCmds )
			{
				m_strCmdActivateOff.clear();
			}

			UpdateUserFlagCanActivate();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::Cineract
//
//	PURPOSE:	Play a cineractive animation.
//
// ----------------------------------------------------------------------- //

void CAI::Cineract(const char* szAnim, bool bLoop)
{
	if( !m_pAnimationContext )
		return;
	
	m_pAnimationContext->SetSpecial( szAnim );

	if( bLoop )
	{
		m_pAnimationContext->LoopSpecial();
	}
	else 
	{
		m_pAnimationContext->PlaySpecial();
	}

	m_pAnimationContext->Update();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HideCharacter
//
//	PURPOSE:	Hide/Show character.
//
// ----------------------------------------------------------------------- //

void CAI::HideCharacter(bool bHide)
{
	super::HideCharacter( bHide );

	if( bHide )
	{
		SetClientSolid( false );
	}
	else 
	{
		// While visible, we should be solid
		// if we are alive but non-solid when
		// we are dead.
		SetClientSolid( !m_damage.IsDead() );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::SetDamagedPlayerCommand
//
//	PURPOSE:	Handle configuring the AI with a DamagedPlayer command.
//
// ----------------------------------------------------------------------- //

void CAI::SetDamagedPlayerCommand( const char* pszCmdDamagedPlayer, int nDamagedPlayerActivationCount )
{
	// Make sure the pointer is to a valid string.
	const char* pszCmdDamagedPlayerValue = pszCmdDamagedPlayer ? pszCmdDamagedPlayer : "";

	m_strCmdDamagedPlayer = pszCmdDamagedPlayerValue;
	m_nDamagedPlayerActivationCount = nDamagedPlayerActivationCount;

	// Reset the activation count (to minimize potential surprise)
	m_nDamagedPlayerNumCount = 0;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::MakeInvulnerable
//
//	PURPOSE:	Make AI invulnerable, or vulnerable.
//
// ----------------------------------------------------------------------- //

void CAI::MakeInvulnerable( bool bMakeInvulerable )
{
	// Make invulnerable.

	if( bMakeInvulerable )
	{
		m_bInvulnerable = true;
		m_damage.SetCanDamage( false );

		g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_IGNORE_PROJECTILES, USRFLG_IGNORE_PROJECTILES);

		SetClientSolid( false );

		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, 0, FLAG_RAYHIT );
		g_pCommonLT->SetObjectFlags(m_hHitBox, OFT_Flags, 0, FLAG_RAYHIT );
	}

	// Make vulnerable.

	else 
	{
		m_bInvulnerable = false;
		m_damage.SetCanDamage( true );

		g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_IGNORE_PROJECTILES);

		SetClientSolid( true );

		g_pCommonLT->SetObjectFlags(m_hObject, OFT_Flags, FLAG_RAYHIT, FLAG_RAYHIT );
		g_pCommonLT->SetObjectFlags(m_hHitBox, OFT_Flags, FLAG_RAYHIT, FLAG_RAYHIT );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::UpdateUserFlagCanActivate()
//
//	PURPOSE:	Updates our CAN_ACTIVATE user flag.
//
// ----------------------------------------------------------------------- //

void CAI::UpdateUserFlagCanActivate()
{
	if (m_bCanTalk && !m_bUnconscious && (!m_strCmdActivateOff.empty() || !m_strCmdActivateOn.empty() ))
	{
		g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, USRFLG_CAN_ACTIVATE, USRFLG_CAN_ACTIVATE);
		return;
	}

	g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_CAN_ACTIVATE);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::SetUnconscious()
//
//	PURPOSE:	Updates flags for sleeping/waking AI
//
// ----------------------------------------------------------------------- //

void CAI::SetUnconscious(bool bUnconscious)
{
	if (bUnconscious == m_bUnconscious) return;

	m_bUnconscious = bUnconscious;

	if (bUnconscious)
	{
		if (m_pAIWeaponMgr)
		{
			m_pAIWeaponMgr->SpawnHolsteredItems();
		}

		if (m_hHitBox)
		{
			CCharacterHitBox* pHitBox = (CCharacterHitBox*) g_pLTServer->HandleToObject(m_hHitBox);
			if ( pHitBox )
			{
				LTVector vNewDims,vDims;
				pHitBox->GetDefaultModelDims( vDims );

				vNewDims = vDims;
				if (GetAnimationContext()->IsPropSet( kAPG_Posture, kAP_POS_Stand ) )
				{
					//if we were standing, adjust our hitbox to cover us lying down
					
					vNewDims.x *= 2.0f;
					vNewDims.z *= 2.0f;
					vNewDims.y = 15.0f;
					g_pPhysicsLT->SetObjectDims(m_hHitBox, &vNewDims, 0);
				}

				LTVector vOffset(0, vNewDims.y - vDims.y, 0);
				pHitBox->SetOffset(vOffset);
				pHitBox->Update();
			}

			UpdateClientHitBox();
		}

		//spawn pickups...
		if ( m_pAttachments )
		{
			if (m_pAIWeaponMgr)
			{
				m_pAIWeaponMgr->SpawnPickupItems();
			}
	
			g_pCommonLT->SetObjectFlags(m_hObject, OFT_User, 0, USRFLG_CAN_ACTIVATE);
		}
		
	}
	else
	{
		if (m_hHitBox)
		{
			CCharacterHitBox* pHitBox = (CCharacterHitBox*) g_pLTServer->HandleToObject(m_hHitBox);
			if ( pHitBox )
			{
				LTVector vDims;
				pHitBox->GetDefaultModelDims( vDims );
				
				LTVector vHitDims = vDims;
				vHitDims *= 1.2f;
				g_pPhysicsLT->SetObjectDims(m_hHitBox, &vHitDims, 0);
				
				pHitBox->SetOffset( LTVector( 0.0f, 0.0f, 0.0f ) );
				pHitBox->Update();
			}

			UpdateClientHitBox();
		}

		UpdateUserFlagCanActivate();

		if (m_pAIWeaponMgr)
		{
			m_pAIWeaponMgr->DestroyDroppedWeapons();
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleTouch()
//
//	PURPOSE:	Notification that we have been touched by something
//
// ----------------------------------------------------------------------- //

void CAI::HandleTouch( HOBJECT hTouched )
{
	// Bail if touch handling is not turned on.

	if ( m_pAIBlackBoard->GetBBHandleTouch() == kTouch_None )
	{
		return;
	}

	// Touch the character inside the hitbox.

	if( IsCharacterHitBox( hTouched ) )
	{
		CCharacterHitBox* pHitBox = (CCharacterHitBox*)g_pLTServer->HandleToObject( hTouched );
		hTouched = pHitBox->GetModelObject();
	}

	// Bail if touching myself!

	if( hTouched == m_hObject )
	{
		return;
	}

	// Update a fact about this event, or create one if there is no such fact.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Knowledge );
	factQuery.SetKnowledgeType( kKnowledge_Touch );
	factQuery.SetSourceObject( hTouched );

	CAIWMFact* pFact = m_pAIWorkingMemory->FindWMFact( factQuery );
	if ( !pFact )
	{
		pFact = m_pAIWorkingMemory->CreateWMFact( kFact_Knowledge );
		if ( pFact )
		{
			pFact->SetKnowledgeType( kKnowledge_Touch, 1.f );
			pFact->SetSourceObject( hTouched, 1.f );
		}
	}

	// Insure the time is always updated.

	if ( pFact )
	{
		pFact->SetTime( g_pLTServer->GetTime() );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleDamage()
//
//	PURPOSE:	Notification that we are hit by something
//
// ----------------------------------------------------------------------- //

void CAI::HandleDamage(const DamageStruct& damage)
{
	if ( (DT_MELEE == damage.eType) && m_damage.IsCantDamageType(damage.eType) )
	{
		bool bIsAlert = ( m_pAIBlackBoard->GetBBAwareness() == kAware_Alert ) ||
						( m_pAIBlackBoard->GetBBAwareness() == kAware_Suspicious );

		if ( !bIsAlert && (damage.fDamage > 100.0f) )
		{
			m_fLastPainVolume = 0.1f;
			m_damage.SetHitPoints(0.0f);
			m_damage.HandleDestruction(damage.hDamager);
			return;
		}
		else
		{
			return;
		}
	}

	// HACK: This hack for FEAR is necessary, because if an AI is killed with
	// a rifle butt, we never get a chance to update sensors to detect an 
	// instant death.

	if( ( ( DT_RIFLE_BUTT == damage.eType ) || ( DT_MELEE == damage.eType ) ) &&
		( m_pAIBlackBoard->GetBBTargetType() == kTarget_None ) && 
		m_damage.IsDead() )
	{
		m_fLastPainVolume = 0.1f;
	}

	// END HACK

	if ( DT_WORLDONLY == damage.eType )
	{
		m_fLastPainVolume = 0.0f;
		return;
	}

	// AI died!

	if( m_damage.IsDead() )
	{
		RemoveFromAIRegions();
		m_pAINavigationMgr->HandleAIDeath();
		m_pAIBlackBoard->SetBBSelectAction( true );
		return;
	}

	// AI did not die.

	// Register a stimulus associated with the damage type.

	const char* pszStimulus = GetDamageStimulusName(damage.eType);
	if( pszStimulus[0] )
	{
		// Register stimulus for taking damage.

		EnumAIStimulusType eStimulus = CAIStimulusMgr::StimulusFromString( pszStimulus );
		StimulusRecordCreateStruct scs(eStimulus, GetAlignment(), GetPosition(), m_hObject);
		scs.m_hStimulusTarget = damage.hDamager;
		scs.m_eDamageType = damage.eType;
		scs.m_fDamageAmount = damage.fDamage;
		scs.m_vDamageDir = damage.GetDamageDir();
		
		g_pAIStimulusMgr->RegisterStimulus( scs );

		// Process damage immediately.

		SetNextSenseUpdate( 0.f );
	}

	if ( m_damage.IsCantDamageType(damage.eType) || !m_damage.GetCanDamage() )
	{
		m_fLastPainVolume = 0.0f;
		return;
	}

	// Cache the damage struct to allow reconstruction later on.

	m_pAIBlackBoard->SetBBLastDamage( damage );

	if ( !m_damage.GetCanDamage() ) return;

	// Do not play pain sounds if affected by special damage.

	if( ( damage.fDamage > 0.0f ) &&
		( !GetDamageFlags() ) )
	{
		g_pAISoundMgr->RequestAISound( m_hObject, kAIS_Pain, kAISndCat_Interrupt, NULL, 0.f );
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::GetUserFlagSurfaceType()
//
//	PURPOSE:	Return our surface type as a user flag
//
// --------------------------------------------------------------------------- //

uint32 CAI::GetUserFlagSurfaceType() const
{
	if( m_pAIBlackBoard->GetBBSurfaceOverride() != ST_UNKNOWN )
	{
		return SurfaceToUserFlag( m_pAIBlackBoard->GetBBSurfaceOverride() );
	}

	return super::GetUserFlagSurfaceType();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::AttachToObject()
//
//	PURPOSE:	Attach the AI to an object.
//
// ----------------------------------------------------------------------- //

void CAI::AttachToObject( HOBJECT hObject )
{
	char szName[64];
	if( hObject )
	{
		g_pLTServer->GetObjectName( hObject, szName, LTARRAYSIZE(szName) );
	}
	else {
		LTStrCpy( szName, "(null)", LTARRAYSIZE(szName) );
	}
	AITRACE( AIShowPaths, ( m_hObject, "Attaching to object: %s", szName ) );

	m_pAIBlackBoard->SetBBAttachedTo( hObject );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::DetachFromObject()
//
//	PURPOSE:	Detach the AI from an object.
//
// ----------------------------------------------------------------------- //

void CAI::DetachFromObject( HOBJECT hObject )
{
	if( m_pAIBlackBoard->GetBBAttachedTo() == hObject )
	{
		char szName[64];
		if( hObject )
		{
			g_pLTServer->GetObjectName( hObject, szName, LTARRAYSIZE(szName) );
		}
		else {
			LTStrCpy( szName, "(null)", LTARRAYSIZE(szName) );
		}
		AITRACE( AIShowPaths, ( m_hObject, "Detaching from object: %s", szName ) );

		m_pAIBlackBoard->SetBBAttachedTo( NULL );
	}

	// Clear any cached pathfinding data, because the AI may have
	// travelled somewhere completely new.

	m_pPathKnowledgeMgr->ClearPathKnowledge();
	m_eLastNavMeshPoly = kNMPoly_Invalid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleTeleport()
//
//	PURPOSE:	Teleport the ai to the specified position
//
// ----------------------------------------------------------------------- //

void CAI::HandleTeleport(const LTVector& vPos)
{
	g_pLTServer->SetObjectPos(m_hObject, vPos);
	m_vMovePos = vPos;
	SetPosition( vPos, true );

	LTVector vDeditPos = vPos;
	vDeditPos = ConvertToDEditPos( vDeditPos );

	m_bPosDirty = true;

	// Clear any cached pathfinding data, because the AI may have
	// travelled somewhere completely new.

	m_pPathKnowledgeMgr->ClearPathKnowledge();
	m_eLastNavMeshPoly = kNMPoly_Invalid;

	// Re-evaluate goals after teleporting to a new position.

	m_pAIBlackBoard->SetBBSelectAction( true );

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleTeleport()
//
//	PURPOSE:	Teleport the ai to the specified point
//
// ----------------------------------------------------------------------- //

void CAI::HandleTeleport(TeleportPoint* pTeleportPoint)
{
	// Set our starting values...

    LTVector vPos;
	g_pLTServer->GetObjectPos(pTeleportPoint->m_hObject, &vPos);

    LTRotation rRot;
	g_pLTServer->GetObjectRotation(pTeleportPoint->m_hObject, &rRot);

	HandleTeleport( vPos );
	g_pLTServer->SetObjectRotation(m_hObject, rRot);

	if ( pTeleportPoint->IsMoveToFloor() )
	{
		// [KLS 8/8/02] Make sure we get moved to the floor right now...
		m_bPosDirty = true;
		m_bForceGround = m_bMoveToFloor;
		UpdateMovement();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::Move()
//
//	PURPOSE:	Sets our new position for this frame
//
// ----------------------------------------------------------------------- //

void CAI::Move(const LTVector& vPos)
{
	m_vMovePos = vPos;
	m_bMove = true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::PreUpdate
//
//	PURPOSE:	Does our Preupdate
//
// ----------------------------------------------------------------------- //

void CAI::PreUpdate()
{
	if( m_bFirstUpdate )
	{
#ifndef _FINAL
		// FindNamedObject will fail if there is already an AI existing with my name.
		// Having two AI (or more) in a level with the same name can cause bugs that
		// are difficult to debug.  
		// So, complain and make the problem very obvious by shutting
		// down all existing AI.

		HOBJECT hObject;
		if( LT_OK != FindNamedObject( GetName(), hObject ) )
		{
			AIASSERT1( 0, m_hObject, "Shutting down all AI!! Found more than one AI named: %s", GetName() );

			// Shut all existing AI down.

			CCharacter* pChar;
			AIList::iterator itAI;
			for( itAI = m_lstAIs.begin(); itAI != m_lstAIs.end(); ++itAI )
			{
				pChar = *itAI;
				g_pCmdMgr->QueueMessage( pChar, pChar, "GOALSET NONE" );
				g_pCmdMgr->QueueMessage( pChar, pChar, "SENSES 0" );
			}
		}
#endif

		// Set up tracking nodes.

		m_pNodeTrackerContext->Init( m_hObject, m_hModelSkeleton );

		if (m_pAIWeaponMgr)
		{
			// Add all of the weapons the arsenal cached in the ReadProp, then 
			// clear out the list.
			
			for ( int i = 0; i < m_Arsenal.GetWeaponAttachmentPropertiesCount(); ++i )
			{
				HWEAPON hWeapon = NULL;
				HAMMO hAmmo = NULL;
				const char* pszSocketName = NULL;
				if ( m_Arsenal.GetWeaponAttachmentProperties( i, hWeapon, hAmmo, &pszSocketName ) )
				{
					m_pAIWeaponMgr->AddWeapon( hWeapon, pszSocketName, FAILURE_IS_ERROR );
				}
			}
			m_Arsenal.ClearWeaponAttachmentProperties();

			m_pAIWeaponMgr->InitPrimaryWeapon();
		}

		// Give the AI its ammo

		AIDB_AIAmmoLoadRecord* pLoad = g_pAIDB->GetAIAmmoLoadRecord(m_eAmmoLoad);
		if (!pLoad)
		{
			// No AIDB_AIAmmoLoadRecord is specified, so give the AI infinite 
			// ammo.  This is to preserve the original behavior of the system,
			// where all AI had 'infinite' ammo.

			m_Arsenal.GetInfiniteAmmo();
		}
		else
		{
			// For each ammo type listed, give the AI some number of ammo of that
			// type.

			int nAmmoTypes = pLoad->m_AmmoAmountList.size();
			for (int i = 0; i < nAmmoTypes; ++i)
			{
				m_Arsenal.AddAmmo(pLoad->m_AmmoAmountList[i].m_hAmmo, pLoad->m_AmmoAmountList[i].m_nRounds);
			}
		}

		// Weapons initially start empty, so relaod them all.

		m_pAIWeaponMgr->ReloadAllWeapons();
	}

	// AllyDisturbance stimulus.

	EnumAIAwareness eAwareness = m_pAIBlackBoard->GetBBAwareness();
	if( ( eAwareness != kAware_Relaxed ) && (m_eAlarmStimID == kStimID_Unset) )
	{
		StimulusRecordCreateStruct scs(kStim_DisturbanceVisible, GetAlignment(), GetPosition(), m_hObject );
		scs.m_dwDynamicPosFlags |= CAIStimulusRecord::kDynamicPos_TrackSource;
		m_eAlarmStimID = g_pAIStimulusMgr->RegisterStimulus( scs );
	}
	else if( ( eAwareness == kAware_Relaxed ) && ( m_eAlarmStimID != kStimID_Unset ) )
	{
		g_pAIStimulusMgr->RemoveStimulus( m_eAlarmStimID );
		m_eAlarmStimID = kStimID_Unset;

		// Clear target when the AI returns to a relaxed state.

		m_pTarget->ClearTarget( this );
	}


	if ( m_bFirstUpdate )
	{
		if ( !m_bMoveToFloor )
		{
			m_bPosDirty = false;
		}

		// Add default attachments.

		if( m_bUseDefaultAttachments )
		{
			const char *pszAttachment;
			char szMsg[128];

			uint32 cAttachments = g_pModelsDB->GetNumDefaultAttachments( m_hModel );
			for( uint32 iAttachment = 0; iAttachment < cAttachments; ++iAttachment )
			{
				pszAttachment = g_pModelsDB->GetDefaultAttachment( m_hModel, iAttachment );

				if( pszAttachment && pszAttachment[0] )
				{
					LTSNPrintF( szMsg, ARRAY_LEN(szMsg), "%s %s", KEY_ATTACH, pszAttachment );
					g_pCmdMgr->QueueMessage( this, this, szMsg );
				}
			}
		}

		// Add default weapons.

		const char* pszSocketName;
		uint32 cWeapons;
		HWEAPON hWeapon;
		if( m_bUseDefaultWeapons )
		{
			cWeapons = g_pModelsDB->GetNumDefaultWeapons( m_hModel );
			for( uint32 iWeapon = 0; iWeapon < cWeapons; ++iWeapon )
			{
				// Skip default weapons that are at the same socket as weapons
				// specified in WorldEdit.

				g_pModelsDB->GetDefaultWeapon( m_hModel, iWeapon, pszSocketName, hWeapon );
				if( hWeapon )
				{
					AddWeaponToAI( hWeapon, pszSocketName );
				}
			}

			HandleArsenalChange();
		}

		// Add required weapons.

		cWeapons = g_pModelsDB->GetNumRequiredWeapons( m_hModel );
		if( cWeapons > 0 )
		{
			for( uint32 iWeapon = 0; iWeapon < cWeapons; ++iWeapon )
			{
				// Skip required weapons that are at the same socket as weapons
				// specified in WorldEdit.

				g_pModelsDB->GetRequiredWeapon( m_hModel, iWeapon, pszSocketName, hWeapon );
				if( hWeapon )
				{
					AddWeaponToAI( hWeapon, pszSocketName );
				}
			}
	
			HandleArsenalChange();
		}

		// If AI has a holster string that starts with a minus,
		// then flag was set in WorldEdit to holster weapons.
		// So remove hand attachments, and recreate holster string without a minus.

		if( m_pAIWeaponMgr && m_pAIWeaponMgr->GetStartHolstered() )
		{
			m_pAIWeaponMgr->Holster();
		}

		// We need to call HideCharacter( false ) to setup the correct shadow lod.  Since
		// the ai could have been hidden with an Initial message, we'll set the 
		// ai back to hidden if they were originally invisible.
		uint32 nFlags = 0;
		g_pCommonLT->GetObjectFlags(m_hObject, OFT_Flags, nFlags );
		HideCharacter( false );
		if( !( nFlags & FLAG_VISIBLE ))
		{
			HideCharacter( true );
		}
	}

	if ( m_bSyncPosition )
	{
		LTVector vNewPosition(m_vPos.x, m_vPos.y, m_vPos.z);
		g_pLTServer->SetObjectPos(m_hObject, vNewPosition);

		m_bPosDirty = true;
		m_bSyncPosition = false;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleInitWeapon()
//
//	PURPOSE:	Allow the AI to handle weapon initialization
//
// ----------------------------------------------------------------------- //

void CAI::HandleInitWeapon()
{
	m_bUpdateNodes = true;
	
	// AI is armed if it has a ranged or melee weapon.  The Kick weapon 
	// does NOT count as being armed, as the AI cannot use it without a 
	// ranged weapon.  Without this exception, AIs will attempt to drop 
	// their Kick weapon when they try to pick up a ranged or melee weapon.

	bool bArmed = 
		m_pAIWeaponMgr->GetWeaponOfType( kAIWeaponType_Ranged ) 
		|| m_pAIWeaponMgr->GetWeaponOfType( kAIWeaponType_Melee );

	m_pAIWorldState->SetWSProp( kWSK_WeaponArmed, m_hObject, kWST_bool, bArmed );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::AddWeaponToAI
//
//	PURPOSE:	Add a weapon to an AI, if there isn't already a 
//              weapon at that socket.
//
// ----------------------------------------------------------------------- //

void CAI::AddWeaponToAI( HRECORD hWeapon, const char* pszSocketName )
{
	// Sanity check.

	if( !hWeapon || !pszSocketName )
	{
		return;
	}

	// Add the weapon to the arsenal.

	m_pAIWeaponMgr->AddWeapon( hWeapon, pszSocketName, !FAILURE_IS_ERROR );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::RemoveWeapon
//
//	PURPOSE:	Handle removing the weapon from the AI through the
//			AIs weapon system.
//
// ----------------------------------------------------------------------- //

void CAI::RemoveWeapon( CActiveWeapon* pActiveWeapon )
{
	if ( !pActiveWeapon || !m_pAIWeaponMgr )
	{
		return;
	}

	m_pAIWeaponMgr->RemoveWeapon( pActiveWeapon->GetWeaponRecord() );
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::CreateAttachments
//
//	PURPOSE:	Creates our attachments aggregate
//
// ----------------------------------------------------------------------- //

void CAI::CreateAttachments()
{
	if ( !m_pAttachments )
	{
		m_pAttachments = static_cast<CAttachments*>(CAttachments::Create());
	}
}

// ----------------------------------------------------------------------- //

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::UpdateSensingMembers()
//
//	PURPOSE:	Update the AI's variables for sensing.
//
// ----------------------------------------------------------------------- //

void CAI::UpdateSensingMembers()
{
	// Update the position of important nodes (eye, torso, etc)
	// Nodes only need to update before each sensing iteration.
	// Nodes are only updated on-demand - if someone requests their pos or rot.

	m_bUpdateNodes = true;

	// It is OK to update the target's visibility if new sense info has come in.

	m_pTarget->SetCanUpdateVisibility( true );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::Update()
//
//	PURPOSE:	Update the AI
//
// ----------------------------------------------------------------------- //

// The following Macro is very useful for debugging one AI at a time.
#define ONLYONE(x) if( LTStrICmp( GetName(), #x ) ) { PostUpdate(); return; }

void CAI::Update()
{
	//track our performance
	CTimedSystemBlock TimingBlock(g_tsAI);

	// USE THE FOLLOWING BLOCK FOR DEBUGGING 1 AI:
//	ONLYONE(partner);

	// Record changes in awareness.

	TIME(UpdateAwareness());

	// Let the client know of anything a player may be interested in.

	TIME(UpdateCharacterFXBodyState());

	// Perform any movement found in the animation.

	TIME(UpdateMovement());

	// Update our position, rotation, etc

	TIME(UpdatePosition());

/*
	// Update our debug iseg junk

	static int s_cTotalCalls = 0;
	static int s_cTotalUpdates = 0;
	s_cTotalCalls += g_cIntersectSegmentCalls;
	s_cTotalUpdates++;
    g_pLTServer->CPrint("iseg calls = %3.3d avg = %f", g_cIntersectSegmentCalls, (float)s_cTotalCalls/(float)s_cTotalUpdates);
	g_cIntersectSegmentCalls = 0;
*/
	
	// Moved sensor updating to AIMgr::update(), so that we can distribute
	// timing of all AI sensor updates from a centralized location.
	//	TIME(m_pAISensorMgr->UpdateSensors( true ));

	TIME(UpdateTarget());

	TIME( m_pNodeTrackerContext->UpdateNodeTrackers( this ) );

	// Update the AIWeaponMgrs range status based on the updated target.
	// This must happen AFTER UpdateTarget().

	if (m_pAIWeaponMgr)
	{
		m_pAIWeaponMgr->UpdateRangeStatus();
	}

	// Update the state if we have one

	if ( m_pState )
	{
		// Update our state.

		if ( g_pAINodeMgr->IsInitialized() )
		{
			// Update NavigationMgr BEFORE updating the state,
			// since the state checks the BBDestStatus.

			TIME(m_pState->Update());
		}
	}

	// Update the goal AFTER the state, since the state may have
	// just completed after finishing an animation or path.

	TIME(m_pGoalMgr->UpdateGoal());

	// Update pathfinding BEFORE selecting a new goal.
	// Arriving at a path waypoint may trigger the AI to search
	// for a new goal (eg TraverseLink).

	TIME(m_pAINavigationMgr->UpdateNavigation());

	TIME(m_pGoalMgr->SelectRelevantGoal());

	// State is responsible for always being able to play an animation.

	if( m_pState )
	{
		m_pState->UpdateAnimation();
	}

	// HACK -- Extra call to UpdateNavigation added to find a path prior
	// to selecting movement animation.  If we have a destination, but do not 
	// yet have a path, AI may choose an incorrect movement animation. leading
	// to an incorrect transition animation.

	if( ( m_pAIBlackBoard->GetBBDestStatus() == kNav_Set ) &&
		( !m_pAIMovement->IsSet() ) )
	{
		m_pAINavigationMgr->UpdateNavigation();
	}

	// Update the NavMgr's animation AFTER the state, so that
	// it can stomp on movement-related animations.

	TIME(m_pAINavigationMgr->UpdateNavigationAnimation());

	if (m_pAIWeaponMgr)
	{
		TIME(m_pAIWeaponMgr->UpdateAIWeapons());
	}

	// Update our ground info

	TIME(UpdateOnGround());

	// Update debug info

	TIME(UpdateInfo());

	// Post update

	TIME(PostUpdate());
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::UpdateDead
//
//	PURPOSE:	Insure that, when an AI dies, his plan is 
//			immediately destroyed.  This is required to 
//			free any resources the current was providing/using
//
// ----------------------------------------------------------------------- //

void CAI::UpdateDead( bool bCanBeRemoved )
{
	if (!m_damage.IsDead())
		return;

	if ( m_pAIPlan 
		&& ( false == m_pAIBlackBoard->GetBBAIHandlingDeath() ) )
	{
		SetAIPlan( NULL );
	}

	super::UpdateDead( bCanBeRemoved );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::PostUpdate
//
//	PURPOSE:	Does our postupdate
//
// ----------------------------------------------------------------------- //

void CAI::PostUpdate()
{
	// Set first update flag

	m_bFirstUpdate = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::UpdateAnimation
//
//	PURPOSE:	Handles any pending animator changes
//
// ----------------------------------------------------------------------- //

void CAI::UpdateAnimation()
{
	if( m_fNextUpdateRate == UPDATE_NEVER )
	{
		return;
	}

	// AI may choose to circumvent the default CCharacter
	// death codepath.

	if( !m_pAIBlackBoard->GetBBAIHandlingDeath() )
	{
		CCharacter::UpdateAnimation();

		if(m_damage.IsDead())
		{
			return;
		}
	}

	// If the Weapon is not set to None, the state code already set it.

	if( GetAnimationContext()->GetProp( kAPG_Weapon ) == kAP_None )
	{
		GetAnimationContext()->SetProp( kAPG_Weapon, 
			m_pAIBlackBoard->GetBBPrimaryWeaponProp() );
	}

	// Record the AI's current Action before potentially changing animations.

	EnumAnimProp eLastAction = GetAnimationContext()->GetCurrentProp( kAPG_Action );

	// Don't interpolate into a new anim if the dims are changing!

	if( m_pAIBlackBoard->GetBBUpdateDims() )
	{
		GetAnimationContext()->DoInterpolation( false );
	}

	bool bAnimChanged = GetAnimationContext()->Update();

	// Update dims on request, but only when the animation changes.

	if( bAnimChanged && m_pAIBlackBoard->GetBBUpdateDims() )
	{
		g_pModelLT->GetModelAnimUserDims( m_hObject, g_pLTServer->GetModelAnimation(m_hObject), &m_vDims );
		g_pPhysicsLT->SetObjectDims( m_hObject, &m_vDims, 0 );
		
		// Movement updates before animation, so if the dims change
		// we need to update movement again.  Otherwise the AI may pop
		// up or down for a frame.

		m_bForceGround = true;
		UpdateMovement();

		// Continue updating dims until the transition out of the animation completes.

		if( !GetAnimationContext()->IsTransitioning() )
		{
			m_pAIBlackBoard->SetBBUpdateDims( false );
		}

		// HACK:  Disable torso tracking while transitioning.
		//        This fixes weirdness when transitioning out of crawling.

		else {
			m_pAIBlackBoard->SetBBTargetTrackerFlags( kTrackerFlag_None );
		}
	}

	// Give the AIWeaponMgr a chance to sync to the AIs animation as soon as 
	// the animation changes.  This must be done the same frame to keep the 
	// weapon animation in sync with the AIs animation.

	if ( m_pAIWeaponMgr )
	{
		m_pAIWeaponMgr->UpdateAnimation();
	}

	// Update the client if the AI started or finished a LongRecoil or 
	// DefeatedRecoil. This allows the player to know when to play a 
	// "finishing move" to instantly kill AI.

	EnumAnimProp eCurAction = GetAnimationContext()->GetCurrentProp( kAPG_Action );
	if ( ( eLastAction != eCurAction ) 
		&& ( 
			( eLastAction == kAP_ACT_LongRecoil ) 
			|| ( eCurAction == kAP_ACT_LongRecoil ) 
			|| ( eLastAction == kAP_ACT_AttackBerserker ) 
			|| ( eCurAction == kAP_ACT_AttackBerserker ) 
			) 
		  )
	{
		m_pAIBlackBoard->SetBBUpdateClient( true );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::UpdateOnGround
//
//	PURPOSE:	Update AI on ground
//
// ----------------------------------------------------------------------- //

void CAI::UpdateOnGround()
{
	//track our performance
	CTimedSystemBlock TimingBlock(g_tsAIOnGround);

	CCharacter::UpdateOnGround();

	// Lets see if we are in the ground or in the air.

	CollisionInfo Info;
    g_pLTServer->Physics()->GetStandingOn(m_hObject, &Info);

	// Reset standing on info

	m_eStandingOnSurface = ST_UNKNOWN;
    m_bOnGround = true;

	if (Info.m_hObject)
	{
		if (Info.m_Plane.m_Normal.y < 0.76)
		{
			// Get rid of our XZ velocities

            LTVector vVel;
            g_pPhysicsLT->GetVelocity(m_hObject, &vVel);

			vVel.x = vVel.z = 0.0f;
            g_pPhysicsLT->SetVelocity(m_hObject, vVel);
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::UpdateInfo
//
//	PURPOSE:	Updates info sent down for debugging.
//
// ----------------------------------------------------------------------- //

void CAI::UpdateInfo()
{
	if (!g_AIInfoTrack.IsInitted())
	{
		g_AIInfoTrack.Init(g_pLTServer, "AIInfo", NULL, 0.0f);
	}

	// Update our info string.
	if( g_AIInfoTrack.GetFloat() > 0.0f )
	{
		char szTemp[128];

		std::string info = GetName(); 
		info += "\n";

		if( g_AIInfoTrack.GetFloat() == 1.0f )
		{
			sprintf( szTemp, "HitPoints : %.0f\nArmor : %.0f\n", m_damage.GetHitPoints( ),
				m_damage.GetArmorPoints( ));
			info += szTemp;

			// Show Senses 0 or 1.

			if( m_pAIBlackBoard->GetBBSensesOn() )
			{
				info += "Senses : 1\n";
			} else {
				info += "Senses : 0\n";
			}

			ENUM_AI_SQUAD_ID eSquad = g_pAICoordinator->GetSquadID( m_hObject );

			// Show alignment.
			info += "Algnmnt : ";
			info += g_pCharacterDB->Alignment2String( m_eAlignment );
			if( eSquad != kSquad_Invalid )
			{
				char szSquadID[32];
				LTSNPrintF( szSquadID, ARRAY_LEN( szSquadID ), " (Sq: %d)", eSquad );
				info += szSquadID;
			}
			info += "\n";

			// Show team ID
			sprintf( szTemp, "Team : %d\n", int(GetTeamID()) );
			info += szTemp;

			// Show goalset, goal and state.
			info += "GS : ";
			if( m_pGoalMgr && ( m_pGoalMgr->GetGoalSetIndex() != kAIGoalSetID_Invalid ) )
			{
				info += g_pAIDB->GetAIGoalSetRecord( m_pGoalMgr->GetGoalSetIndex() )->strName.c_str();
			}
			else
			{
				info += "none";
			}
			
			info += "\n";

			info += "G : ";
			if( m_pGoalMgr && m_pGoalMgr->GetCurrentGoal() )
			{
				info += CAIGoalAbstract::GetGoalTypeName(m_pGoalMgr->GetCurrentGoal()->GetGoalType());
			}
			else
			{
				info += "none";
			}

			info += "\n";

			info += "A : ";
			if( m_pGoalMgr && m_pGoalMgr->GetCurrentGoal() )
			{
				CAIActionAbstract* pAction = m_pGoalMgr->GetCurrentGoal()->GetCurrentAction();
				if( pAction )
				{
					info += s_aszActionTypes[pAction->GetActionRecord()->eActionType];
				}
				else {
					info += "none";
				}
			}
			else
			{
				info += "none";
			}
			info += "\n";

			if( m_pState )
			{
				info += "S : ";
				info += s_aszStateTypes[m_pState->GetStateClassType()];
			}
			else {
				info += "S : none";
			}
			info += "\n";

			if (m_pTarget)
			{
				m_pTarget->GetDebugInfoString(info);
			}
		}
		else if ( g_AIInfoTrack.GetFloat() == 2.0f )
		{
			m_pAnimationContext->GetDebugInfoString(info);
		}
		else if ( g_AIInfoTrack.GetFloat() == 3.0f )
		{
			m_pAIWorldState->GetDebugInfoString(info);
		}

		// Update the string if it changed.

		if( info != m_cstrCurrentInfo )
		{
			m_cstrCurrentInfo = info;
			SendInfoString( m_hObject, m_cstrCurrentInfo.c_str() );
		}
	}
	else
	{
		// Clear the string.

		if( "" != m_cstrCurrentInfo )
		{
			m_cstrCurrentInfo = "";
			SendInfoString( m_hObject, m_cstrCurrentInfo.c_str() );
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::GetWeaponPosition()
//
//	PURPOSE:	returns the position of our "Weapon" (could be anything)
//
// ----------------------------------------------------------------------- //
LTVector CAI::GetWeaponPosition( const CWeapon* pWeapon, bool bForceCalculation)
{
	// TODO: This function should have better error handling for this case.
	// it ought to return the position by param and return a bool for if 
	// the position is valid or not.
	if (!m_pAIWeaponMgr)
	{
		AIASSERT(0, m_hObject, "CAI::GetWeaponPosition: No weapon position without a AIWeaponMgr" );
		return LTVector(0,0,0);
	}

	if ( ( m_pAIWeaponMgr->GetCurrentWeapon() == pWeapon ) &&
		 ( !bForceCalculation ) )
	{
		// If the weapon requested is the current weapon, then we already 
		// should have the location in cache, and shouldn't need to do any
		// expensive lookups/node evaluating.
		UpdateNodes();
		return m_vWeaponPos; 
	}
	else
	{
		LTVector vPos;
		LTRotation rRot;
		m_pAIWeaponMgr->GetWeaponOrientation(pWeapon, vPos, rRot);

		return vPos;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::GetWeaponForward()
//
//	PURPOSE:	returns the forward vector of our "Weapon" (could be anything)
//
// ----------------------------------------------------------------------- //

LTVector CAI::GetWeaponForward( const CWeapon* pWeapon)
{
  	// TODO: This function should have better error handling for this case.
  	// it ought to return the position by param and return a bool for if 
  	// the position is valid or not.
  	if (!m_pAIWeaponMgr)
  	{
  		AIASSERT( 0, m_hObject, "CAI::GetWeaponForward: No weapon position without a AIWeaponMgr" );
  		return LTVector(0,0,0);
  	}
  
  	if (m_pAIWeaponMgr->GetCurrentWeapon() == pWeapon)
  	{
  		// If the weapon requested is the current weapon, then we already 
  		// should have the location in cache, and shouldn't need to do any
  		// expensive lookups/node evaluating.
  		UpdateNodes();
  		return m_vWeaponForward; 
  	}
  	else
  	{
  		LTVector vPos;
  		LTRotation rRot;
  		m_pAIWeaponMgr->GetWeaponOrientation(pWeapon, vPos, rRot);					

  		return rRot.Forward();
  	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::GetEye*()
//
//	PURPOSE:	Get eye node position and forward.
//
// ----------------------------------------------------------------------- //

const LTVector& CAI::GetEyePosition()
{
	// Nodes will only really update once per sense update.
	UpdateNodes();
	return m_vEyePos; 
}

const LTVector& CAI::GetEyeForward()
{
	// Nodes will only really update once per sense update.
	UpdateNodes();
	return m_vEyeForward; 
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::GetTorso*()
//
//	PURPOSE:	Get torso vectors.
//
// ----------------------------------------------------------------------- //

const LTVector& CAI::GetTorsoRight()
{
	// Nodes will only really update once per sense update.
	UpdateNodes();
	return m_vTorsoRight; 
}

const LTVector& CAI::GetTorsoForward()
{
	// Nodes will only really update once per sense update.
	UpdateNodes();
	return m_vTorsoForward; 
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::IsInsideFOV()
//
//	PURPOSE:	Given a distance and a direction, determine if the 
//				position is considered inside the AIs FOV
//
// ----------------------------------------------------------------------- //

bool CAI::IsInsideFOV(float fDistanceSqr, const LTVector& vDirNorm)
{
	// First check horizontal FOV

	float fHorizontalDp = vDirNorm.Dot( GetEyeForward() );

	// Base the FOV limits on the AI's awareness.

	float fFOV = c_fFOV180;
	if( fDistanceSqr >= g_pAIDB->GetAIConstantsRecord()->fNoFOVDistanceSqr )
	{
		switch( m_pAIBlackBoard->GetBBAwareness() )
		{
			case kAware_Alert:
				if( m_pAIBlackBoard->GetBBAwarenessMod() == kAwarenessMod_ImmediateThreat )
				{
					fFOV = g_pAIDB->GetAIConstantsRecord()->fFOVAlertImmediateThreat;
				}
				else 
				{
					fFOV = g_pAIDB->GetAIConstantsRecord()->fFOVAlert;
				}
				break;
			case kAware_Suspicious:
				fFOV = g_pAIDB->GetAIConstantsRecord()->fFOVSuspicious;
				break;
			case kAware_Relaxed:
				fFOV = g_pAIDB->GetAIConstantsRecord()->fFOVRelaxed;
				break;
		}
	}

	// Target direction is outside of our horizontal field of view.

	if( fHorizontalDp <= fFOV )
	{
		return false;
	}

	// Now check vertical FOV

	// We already know that the object is in front of us at this point, so no need
	// to test if object is within 180 vertical fov if we're not alert

	if ( fFOV != c_fFOV180 )
	{
		float fVerticalDp = vDirNorm.Dot(GetUpVector());

		// FOV is 120' if we're not alert

        if (fVerticalDp >= c_fFOV120)
		{
			return false;
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::IsObjectPositionVisible()
//
//	PURPOSE:	Is a given position on the test object visible
//
// ----------------------------------------------------------------------- //

bool CAI::IsObjectPositionVisible(ObjectFilterFn ofn, PolyFilterFn pfn, const LTVector& vSourcePosition, HOBJECT hObj, const LTVector& vObjectPosition, float fVisibleDistanceSqr, bool bFOV, bool bDoVolumeCheck, HOBJECT* phBlockingObject /*= NULL*/, float* pfDistanceSqr /*= NULL*/)
{
    if (!g_pLTServer || !m_hObject) return false;

	if ( hObj != NULL )
	{
		LTVector vObjectDims;	
		g_pPhysicsLT->GetObjectDims(hObj, &vObjectDims);

		// See if the position is inside the object...

		if ( vSourcePosition.x > vObjectPosition.x - vObjectDims.x && vSourcePosition.x < vObjectPosition.x + vObjectDims.x &&
			 vSourcePosition.y > vObjectPosition.y - vObjectDims.y && vSourcePosition.y < vObjectPosition.y + vObjectDims.y &&
			 vSourcePosition.z > vObjectPosition.z - vObjectDims.z && vSourcePosition.z < vObjectPosition.z + vObjectDims.z)
		{
			// Gotta fudge some of these values

			if ( pfDistanceSqr ) *pfDistanceSqr = MATH_EPSILON;

			return true;
		}
	}

    LTVector vDir = vObjectPosition - vSourcePosition;
    float fDistanceSqr = vDir.MagSqr();

	// Record the dist.

	if ( pfDistanceSqr ) *pfDistanceSqr = fDistanceSqr;

	// Make sure it is close enough if we aren't alert

	if( fDistanceSqr >= fVisibleDistanceSqr)
	{
        return false;
	}


    LTVector vDirNorm = vDir.GetUnit();

	// Make sure it is in our FOV

	if ( bFOV )
	{
		if (!IsInsideFOV(fDistanceSqr, vDirNorm))
		{
			return false;
		}
	}

	IntersectQuery IQuery;
	IntersectInfo IInfo;

	IQuery.m_From = vSourcePosition;
	IQuery.m_To = vObjectPosition;

	IQuery.m_Flags	  = INTERSECT_OBJECTS | IGNORE_NONSOLID | (pfn ? INTERSECT_HPOLY : 0);
	IQuery.m_FilterFn = ofn;
	IQuery.m_PolyFilterFn = pfn;

	s_hFilterAI = m_hObject;

	g_cIntersectSegmentCalls++;

	bool bIntersected = g_pLTServer->IntersectSegment(IQuery, &IInfo);

	if (!hObj)
	{
		if (false == bIntersected)
		{
			return true;
		}
	}
	else
	{
		LTASSERT(!IsCharacterHitBox(IInfo.m_hObject), "Intersection does not collide with character hitboxes");
		LTASSERT(!IsCharacterHitBox(hObj), "Intersection does not collide with character hitboxes");

		if ( hObj == IInfo.m_hObject )
		{
			return true;
		}
	}

	if( phBlockingObject )
	{
		*phBlockingObject = IInfo.m_hObject;
	}

    return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::CanSearch()
//
//	PURPOSE:	Returns TRUE if AI can search from where it is.
//
// ----------------------------------------------------------------------- //

bool CAI::CanSearch()
{
	// Can't search of no search nodes exist.

	AINODE_LIST* pNodeList = g_pAINodeMgr->GetNodeList( kNode_Search );
	if( !( pNodeList && pNodeList->size() > 0 ) )
	{
		return false;
	}

	// We can only search if we know of at least one reachable search node.

	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Node );
	factQuery.SetNodeType( kNode_Search );
	CAIWMFact* pFact = m_pAIWorkingMemory->FindWMFact( factQuery );
	return !!pFact;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::RequestCrouch()
//
//	PURPOSE:	Returns TRUE if ally AI agrees to crouch.
//
// ----------------------------------------------------------------------- //

bool CAI::RequestCrouch(HOBJECT hRequestingAI)
{
//	if( !( m_pState && m_pState->GetStateType() == kState_HumanAttack ) )
	{
		return false;
	}

//	CAIHumanStateAttack* pStateAttack = (CAIHumanStateAttack*)m_pState;
//	return pStateAttack->RequestCrouch( hRequestingAI );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::RequestDodge()
//
//	PURPOSE:	Returns TRUE if ally AI agrees to dodge.
//
// ----------------------------------------------------------------------- //

bool CAI::RequestDodge(HOBJECT hRequestingAI)
{
//	if( !( m_pState && m_pState->GetStateType() == kState_HumanAttack ) )
	{
		return false;
	}

//	CAIHumanStateAttack* pStateAttack = (CAIHumanStateAttack*)m_pState;
//	return pStateAttack->RequestDodge( hRequestingAI );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::SetAIPlan
//
//	PURPOSE:	Set a new AIPlan.
//
// ----------------------------------------------------------------------- //

void CAI::SetAIPlan( CAIPlan* pAIPlan )
{
	if( m_pAIPlan )
	{
		AI_FACTORY_DELETE( m_pAIPlan );
	}

	m_pAIPlan = pAIPlan;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::AI_FACTORY_NEW_State
//
//	PURPOSE:	Create a state
//
// ----------------------------------------------------------------------- //

CAIState* CAI::AI_FACTORY_NEW_State(EnumAIStateType eStateType)
{
	// Call AI_FACTORY_NEW for the requested type of state.

	switch( eStateType )
	{
		#define STATE_TYPE_AS_SWITCH_HUMAN 1
		#include "AIEnumStateTypeValues.h"
		#undef STATE_TYPE_AS_SWITCH_HUMAN

		default: AIASSERT( 0, m_hObject, "CAI::AI_FACTORY_NEW_State: Unrecognized state type.");
	}

	return NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::ClearState
//
//	PURPOSE:	Clears our current state
//
// ----------------------------------------------------------------------- //

void CAI::ClearState()
{
	if( !m_pState )
	{
		return;
	}

	// Delete the old state.

	AI_FACTORY_DELETE(m_pState);
	m_pState = NULL;
	GetAnimationContext()->ClearLock();
	GetAnimationContext()->ClearProps();

	// Record when a state change occured.

	m_pAIBlackBoard->SetBBStateChangeTime( g_pLTServer->GetTime() );

	// Update our user flags

	UpdateUserFlagCanActivate();

	AITRACE( AIShowStates, ( m_hObject, "Clearing State\n" ) );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::SetState
//
//	PURPOSE:	Changes our current state
//
// ----------------------------------------------------------------------- //

void CAI::SetState(EnumAIStateType eState, bool bUnlockAnimation /* = true */)
{
	// Delete the old state.

	if ( m_pState )
	{
		AI_FACTORY_DELETE(m_pState);
		m_pState = NULL;
		GetAnimationContext()->ClearLock();
	}

	m_pState = AI_FACTORY_NEW_State( eState );
	AIASSERT( m_pState, m_hObject, "CAI::SetState: Could not create state.");

	if ( !m_pState )
	{
		return;
	}

	// Unlock the animation context so we don't get stuck playing some dippy animation

	if ( bUnlockAnimation && GetAnimationContext() )
	{
		GetAnimationContext()->Unlock();
	}

	// Init the state

	if ( !m_pState->Init(this) )
	{
		AIASSERT(0, m_hObject, "Failed to Init state" );
		AI_FACTORY_DELETE(m_pState);
		m_pState = NULL;
		return;
	}

	// Record when a state change occured.
	  
	m_pAIBlackBoard->SetBBStateChangeTime( g_pLTServer->GetTime() );

	// Update our user flags

	UpdateUserFlagCanActivate();

	AITRACE(AIShowStates, ( m_hObject, "Setting State %s\n", s_aszStateTypes[m_pState->GetStateClassType()] ) );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleModelString()
//
//	PURPOSE:	Handles model keyframe strings
//
// ----------------------------------------------------------------------- //

void CAI::HandleModelString(ArgList* pArgList, ANIMTRACKERID nTrackerId)
{
    if (!g_pLTServer || !pArgList || !pArgList->argv || pArgList->argc == 0) return;

	// For now, ignore any modelstrings coming from the blend tracker.  In 
	// the future, we may want some strings to be allowed.  If so, 
	// selectively enable strings.

	if ( nTrackerId == kAD_TRK_Blend )
	{
		return;
	}

	// Set up the parsed message.

	CParsedMsg cParsedMsg(pArgList->argc, pArgList->argv);

	static CParsedMsg::CToken s_cTok_BodySlump( "NOISE" );
	static CParsedMsg::CToken s_cTok_On( "ON" );
	static CParsedMsg::CToken s_cTok_Off( "OFF" );
	static CParsedMsg::CToken s_cTok_Activate( "ACTIVATE" );
	static CParsedMsg::CToken s_cTok_Door( "DOOR" );
	static CParsedMsg::CToken s_cTok_KickDoor( "KICKDOOR" );
	static CParsedMsg::CToken s_cTok_Material( "MATERIAL_GROUP" );
	static CParsedMsg::CToken s_cTok_HidePiece( "HIDEPIECE" );
	static CParsedMsg::CToken s_cTok_Desire( "DESIRE" );
	static CParsedMsg::CToken s_cTok_TrackLookAtTarget( "TRACKLOOKATTARGET" );
	static CParsedMsg::CToken s_cTok_TrackAimAtTarget( "TRACKAIMATTARGET" );
	static CParsedMsg::CToken s_cTok_Shove( "SHOVE" );
	static CParsedMsg::CToken s_cTok_Damage( "DAMAGE" );
	static CParsedMsg::CToken s_cTok_NodeImpulse( "NODEIMPULSE" );
	static CParsedMsg::CToken s_cTok_BodyState( "BODYSTATE" );
	static CParsedMsg::CToken s_cTok_AISound( "AISOUND" );

	if( cParsedMsg.GetArg(0) == s_cTok_BodySlump )
	{
		// Hitting the ground noise

		SurfaceType eSurface = ST_UNKNOWN;

		CollisionInfo Info;
        g_pLTServer->Physics()->GetStandingOn(m_hObject, &Info);

		if (Info.m_hPoly != INVALID_HPOLY)
		{
			eSurface = (SurfaceType)GetSurfaceType(Info.m_hPoly);
		}
		else if (Info.m_hObject) // Get the texture flags from the object...
		{
			eSurface = (SurfaceType)GetSurfaceType(Info.m_hObject);
		}

		// Play the noise

        LTVector vPos;
        g_pLTServer->GetObjectPos(m_hObject, &vPos);

		HSURFACE hSurf = g_pSurfaceDB->GetSurface(eSurface);
		if (hSurf)
		{
			HRECORD hFallSnd = g_pSurfaceDB->GetRecordLink(hSurf,SrfDB_Srf_rBodyFallSnd);
			if (hFallSnd)
			{
				g_pServerSoundMgr->PlayDBSoundFromPos(vPos, hFallSnd, SMGR_INVALID_RADIUS, SOUNDPRIORITY_MISC_LOW,
					0, SMGR_INVALID_VOLUME, 1.0f, SMGR_INVALID_RADIUS, 
					DEFAULT_SOUND_CLASS, PLAYSOUND_MIX_WEAPONS_NONPLAYER);
			}
		}
	}
	else if( cParsedMsg.GetArg(0) == s_cTok_Off )
	{
		g_pCmdMgr->QueueMessage( this, g_pLTServer->HandleToObject(m_hAnimObject), "OFF" );
	}
	else if( cParsedMsg.GetArg(0) == s_cTok_On )
	{
		g_pCmdMgr->QueueMessage( this, g_pLTServer->HandleToObject(m_hAnimObject), "ON" );
	}
	else if( cParsedMsg.GetArg(0) == s_cTok_Activate )
	{
		g_pCmdMgr->QueueMessage( this, g_pLTServer->HandleToObject(m_hAnimObject), "ACTIVATE" );
	}
	else if( cParsedMsg.GetArg(0) == s_cTok_Door )
	{
		OpenDoor();
	}
	else if (cParsedMsg.GetArg(0) == s_cTok_KickDoor)
	{
		KickDoor();
	}
	else if (cParsedMsg.GetArg(0) == s_cTok_Material)
	{
		if( cParsedMsg.GetArgCount() >= 2 )
		{
			// Get the material index to change, and the group to change it to.
			int iMaterialIndex = atoi(cParsedMsg.GetArg(1));
			int iGroup = atoi(cParsedMsg.GetArg(2));

			// Get the filename in the animation group at the requested index.

			int iGroupStride = GetObjectMaterialCount(m_hObject);
			uint32 iButeMaterialListIndex = iGroup*iGroupStride + iMaterialIndex;

			// Set the selected material.

			if (iButeMaterialListIndex >= 0 && 
				iButeMaterialListIndex < g_pModelsDB->GetNumMaterials(GetModel()))
			{
				const char* pszFileName = g_pModelsDB->GetMaterialFilename(GetModel(), iButeMaterialListIndex);
				g_pModelLT->SetMaterialFilename(m_hObject, iMaterialIndex, pszFileName);
			}
			else
			{
				ObjectCPrint(m_hObject, "CAICommandMgr::HandleMaterialMsg : Material Index out of bounds: %d", iButeMaterialListIndex);
			}
		}
	}
	else if (cParsedMsg.GetArg(0) == s_cTok_HidePiece)
	{
		HMODELPIECE hPiece = (HMODELPIECE)NULL;	
		if( LT_OK == g_pModelLT->GetPiece( m_hObject, cParsedMsg.GetArg(1), hPiece ) )
		{
			// hide it
			LTRESULT ltResult;
			bool bHiddenStatus = !!atoi(cParsedMsg.GetArg(2));
			ltResult = g_pModelLT->SetPieceHideStatus( m_hObject, hPiece, bHiddenStatus );
			AIASSERT( ( LT_OK == ltResult) || ( LT_NOCHANGE == ltResult ), m_hObject, "CAICommandMgr::HandlePieceMsg : Failed to set piece status." );
		}
		else
		{
			ObjectCPrint( m_hObject, "CAICommandMgr::HandlePieceMsg : No such piece on model: %s", cParsedMsg.GetArg(1).c_str() );
		}
	}
	else if( cParsedMsg.GetArg(0) == s_cTok_Desire )
	{
		if( cParsedMsg.GetArgCount() > 2 )
		{
			// Desire type.

			ENUM_AIWMDESIRE_TYPE eDesire = kDesire_InvalidType;
			if( LTStrIEquals( cParsedMsg.GetArg(1), "UNCLOAK" ) )
			{
				eDesire = kDesire_Uncloak;
			}

			// Desire is valid.

			if( eDesire != kDesire_InvalidType )
			{
				// Last parameter is the confidence.

				float fConfidence = (float)atof( cParsedMsg.GetArg(2) );

				CAIWMFact factQuery;
				factQuery.SetFactType( kFact_Desire );
				factQuery.SetDesireType( eDesire );

				// Clear the desire.

				if( fConfidence <= 0.f )
				{
					m_pAIWorkingMemory->ClearWMFacts( factQuery );
				}

				// Create or update the desire.

				else {
					CAIWMFact* pFact = m_pAIWorkingMemory->FindWMFact( factQuery );
					if( !pFact )
					{
						pFact = m_pAIWorkingMemory->CreateWMFact( kFact_Desire );
					}
					if( pFact )
					{
						pFact->SetDesireType( eDesire, fConfidence );
					}
				}
			}
		}
	}
	else if ( cParsedMsg.GetArg(0) == s_cTok_TrackLookAtTarget )
	{
		m_pAIBlackBoard->SetBBTargetTrackerFlags( kTrackerFlag_LookAt );
	}
	else if ( cParsedMsg.GetArg(0) == s_cTok_TrackAimAtTarget )
	{
		m_pAIBlackBoard->SetBBTargetTrackerFlags( kTrackerFlag_AimAt );
	}
	else if ( cParsedMsg.GetArg(0) == s_cTok_Shove )
	{
		CAI* pShovedAI = CAI::DynamicCast( m_hAnimObject );
		if ( pShovedAI )
		{
			StimulusRecordCreateStruct scs( kStim_Shoved, pShovedAI->GetAlignment(), pShovedAI->GetPosition(), pShovedAI->GetHOBJECT() ); 
			scs.m_hStimulusTarget = m_hObject;
			g_pAIStimulusMgr->RegisterStimulus( scs );
		}
	}
	else if ( cParsedMsg.GetArg(0) == s_cTok_Damage )
	{
		AIASSERT( cParsedMsg.GetArgCount() == 3, GetHOBJECT(), "CAI::HandleModelString: 'DAMAGE' model string requires an ammo name: DAMAGE <amount> <ammo name>" );
		if ( m_hAnimObject )
		{
			int nDamage = atoi( cParsedMsg.GetArg(2).c_str() );
			const char* const pszAmmoName = cParsedMsg.GetArg(1).c_str();
			AIASSERT( cParsedMsg.GetArgCount() == 2, GetHOBJECT(), "CAI::HandleModelString: 'DAMAGE' model string requires an ammo name: DAMAGE <amount> <ammo name>" );
			HAMMO hAmmo = g_pWeaponDB->GetRecord( g_pWeaponDB->GetAmmoCategory(), pszAmmoName );
			AIASSERT1( hAmmo, GetHOBJECT(), "CAI::HandleModelString: 'DAMAGE' model string specifies an invalid ammo name: %s", pszAmmoName );
			if ( hAmmo )
			{
				DamageStruct damage;
				damage.fDamage			= (float)nDamage;
				damage.hDamager			= GetHOBJECT();
				damage.hAmmo			= hAmmo;
				damage.DoDamage( GetHOBJECT(), m_hAnimObject );
			}
		}
	}
	else if ( cParsedMsg.GetArg(0) == s_cTok_NodeImpulse )
	{
		// From the node get the rigidbody.
		// Apply an impulse to the rigidbody.

		const char* const pszNodeName = cParsedMsg.GetArg( 1 );
		float flImpulse = (float)atoi( cParsedMsg.GetArg( 2 ) );

		if ( pszNodeName == NULL 
			|| '\0' == pszNodeName[0] )
		{
			ObjectCPrint( m_hObject, "CAICommandMgr::HandlePieceMsg : Node name not specified.  Syntax: '%s <node name> <impulse>'" );
		}

		if ( 0 == flImpulse )
		{
			ObjectCPrint( m_hObject, "CAICommandMgr::HandlePieceMsg : Impulse Node name not specified.  Syntax: '%s <node name> <impulse>'" );
		}

		if ( pszNodeName 
			&& '\0' != pszNodeName[0]
			&& 0 != flImpulse  )
		{
			HMODELNODE hNode = INVALID_MODEL_NODE;
			if ( LT_OK == g_pModelLT->GetNode( GetHOBJECT(), pszNodeName, hNode ) )
			{
				ILTPhysicsSim* pILTPhysicsSim = g_pLTBase->PhysicsSim();
				HPHYSICSRIGIDBODY hRigidBody = PhysicsUtilities::GetNodeParentRigidBody(GetHOBJECT(), hNode);
				if ( hRigidBody != INVALID_PHYSICS_RIGID_BODY )
				{
					LTVector vCenterOfMass;
					g_pLTServer->PhysicsSim( )->GetRigidBodyCenterOfMassInWorld( hRigidBody, vCenterOfMass );
					if( LTIsNaN( flImpulse ) || flImpulse > 1000000.0f )
					{
						LTERROR( "Invalid impulse detected." );
						flImpulse = 10.0f;
					}
					g_pLTServer->PhysicsSim( )->ApplyRigidBodyImpulseWorldSpace( hRigidBody, vCenterOfMass, LTVector( 0, flImpulse, 0 ) );
					g_pLTServer->PhysicsSim( )->ReleaseRigidBody(hRigidBody);
				}
				else
				{
					ObjectCPrint( m_hObject, "CAICommandMgr::HandlePieceMsg : Node %s does not have a rigidbody", pszNodeName );
				}
			}
			else
			{
				ObjectCPrint( m_hObject, "CAICommandMgr::HandlePieceMsg : No such model node on model: %s", pszNodeName );
			}
		}
	}
	else if ( cParsedMsg.GetArg(0) == s_cTok_BodyState )
	{
		//
		// Handle setting a scripted body state.  This is ONLY valid when the
		// current action is going to reset the body state on exit.  We only 
		// have a single action which uses this functionality at this time.  
		// We may want to protect this more/generalize it if we see more 
		// common use.
		//

		if ( cParsedMsg.GetArgCount() < 2 )
		{
			ObjectCPrint( m_hObject, "CAICommandMgr::HandlePieceMsg : %s an argument specifying the body state.", s_cTok_BodyState.c_str() );
		}
		else
		{
			const char* pszBodyState = cParsedMsg.GetArg(1);

			BodyState eScriptedBodyState = StringToBodyState( pszBodyState );
			if ( eBodyStateInvalid == eScriptedBodyState )
			{
				ObjectCPrint( m_hObject, "CAICommandMgr::HandlePieceMsg : BodyState %s is not valid.", pszBodyState );
			}
			else
			{
				// Set the scripted body state and request an update.

				m_pAIBlackBoard->SetBBScriptedBodyState( eScriptedBodyState );
				m_pAIBlackBoard->SetBBUpdateClient( true );

				// Special case for finishing moves...

				if (eScriptedBodyState == eBodyStateDefeated)
				{
					PlayerFinishingMoveSetup();
				}
				else
				{
					PlayerFinishingMoveCleanup();
				}
			}
		}
	}
	else if ( cParsedMsg.GetArg(0) == s_cTok_AISound )
	{
		if (cParsedMsg.GetArgCount() != 2 )
		{
			CONTENT_WARNING(
				HMODELANIM hAnim = INVALID_MODEL_ANIM;
				g_pModelLT->GetCurAnim( m_hObject, nTrackerId, hAnim );

				char szAnimName[64];
				szAnimName[0] = '\0';
				g_pModelLT->GetAnimName( m_hObject,  hAnim, szAnimName, LTARRAYSIZE(szAnimName) );

				char szErrorMsg[1024];
				LTSNPrintF( szErrorMsg, LTARRAYSIZE(szErrorMsg), 
					"Failed to play AISOUND from animation '%s'; incorrect number of arguments. Correct syntax is 'AISOUND <name of sound type in the AISoundTemplate>'.", 
					szAnimName
					);
				AIASSERT( 0, m_hObject, szErrorMsg );
			)
		}
		else
		{
			const char* pszAISoundTypeName = cParsedMsg.GetArg(1);

			EnumAISoundType eSoundType = AISoundUtils::Enum( pszAISoundTypeName );
			if ( eSoundType == kAIS_InvalidType )
			{
				CONTENT_WARNING(
					HMODELANIM hAnim = INVALID_MODEL_ANIM;
					g_pModelLT->GetCurAnim( m_hObject, nTrackerId, hAnim );

					char szAnimName[64];
					szAnimName[0] = '\0';
					g_pModelLT->GetAnimName( m_hObject,  hAnim, szAnimName, LTARRAYSIZE(szAnimName) );

					char szErrorMsg[1024];
					LTSNPrintF( szErrorMsg, LTARRAYSIZE(szErrorMsg), 
						"Failed to play AISOUND '%s' from animation '%s'. '%s' is not listed in the AISoundTemplate.", 
						pszAISoundTypeName, szAnimName, pszAISoundTypeName );
					AIASSERT( 0, m_hObject, szErrorMsg );
				)
			}
			else
			{
				g_pAISoundMgr->RequestAISound( GetHOBJECT(), eSoundType, kAISndCat_ModelKey, NULL, 0.0f );
			}
		}
	}

	// Let AIWeaponMgr handle it.

	if (m_pAIWeaponMgr)
	{
		m_pAIWeaponMgr->HandleModelString( cParsedMsg );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::PlayerFinishingMoveSetup
//
//	PURPOSE:	Setup a finishing move object for the player
//
// ----------------------------------------------------------------------- //

void CAI::PlayerFinishingMoveSetup()
{
	PlayerFinishingMoveCleanup();

	CAIWMFact* pFact = m_pAIWorkingMemory->CreateWMFact( kFact_Knowledge );
	if ( pFact )
	{
		pFact->SetKnowledgeType( kKnowledge_PlayerFinishingMove );

		// Create the special move object.
		CAutoMessage cMsg;
		cMsg.Writeuint8(SFX_FINISHINGMOVE_ID);
		cMsg.WriteObject(m_hObject);
		g_pLTServer->SendSFXMessage(cMsg.Read(), MESSAGE_GUARANTEED);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::PlayerFinishingMoveCleanup
//
//	PURPOSE:	Do some cleanup once a player finishing move is no longer wanted
//
// ----------------------------------------------------------------------- //

void CAI::PlayerFinishingMoveCleanup()
{
	CAIWMFact factQuery;
	factQuery.SetFactType( kFact_Knowledge );
	factQuery.SetKnowledgeType( kKnowledge_PlayerFinishingMove );

	CAIWMFact* pFact = m_pAIWorkingMemory->FindWMFact( factQuery );
	if ( pFact )
	{
		m_pAIWorkingMemory->ClearWMFact( factQuery );

		// Destroy object on client.
		CAutoMessage cMsg;
		cMsg.Writeuint8( MID_SFX_MESSAGE );
		cMsg.Writeuint8( SFX_FINISHINGMOVE_ID );
		cMsg.WriteObject( m_hObject );
		cMsg.Writeuint8( SPECIALMOVEFX_DESTROY );
		g_pLTServer->SendToClient( cMsg.Read(), NULL, MESSAGE_GUARANTEED );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::Save
//
//	PURPOSE:	Save the object
//
// ----------------------------------------------------------------------- //

void CAI::Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags)
{
    if (!g_pLTServer || !pMsg || !m_pBrain) return;

	pMsg->WriteString( m_pBrain->GetName() );
	SAVE_bool(((m_pBrain != NULL) ? true : false));
	
	SAVE_BOOL(m_bCheapMovement);
	SAVE_FLOAT(m_flVerticalThreshold);

	SAVE_BOOL(m_bUseDefaultAttachments);
	SAVE_BOOL(m_bUseDefaultWeapons);

	static std::string strInvalid = "Invalid";
	AIDB_AIAmmoLoadRecord* pAmmoLoad = g_pAIDB->GetAIAmmoLoadRecord( m_eAmmoLoad );
	if( pAmmoLoad )
	{
		SAVE_STDSTRING( pAmmoLoad->strName );
	}
	else {
		SAVE_STDSTRING( strInvalid );
	}

	m_pTarget->Save(pMsg);

	SAVE_VECTOR(m_vEyePos);
	SAVE_VECTOR(m_vEyeForward);
	SAVE_VECTOR(m_vTorsoRight);
	SAVE_VECTOR(m_vTorsoForward);
	SAVE_VECTOR(m_vWeaponPos);
	SAVE_VECTOR(m_vWeaponForward);
	SAVE_BOOL(m_bUpdateNodes);

	SAVE_VECTOR(m_vPos);
	SAVE_VECTOR(m_vPathingPosition);
	SAVE_VECTOR(m_vDims);
	SAVE_FLOAT(m_fRadius);

	SAVE_BOOL(m_bSeeThrough);
	SAVE_BOOL(m_bShootThrough);

	SAVE_STDSTRING(m_strCmdInitial);
	SAVE_STDSTRING(m_strCmdActivateOn);
	SAVE_STDSTRING(m_strCmdActivateOff);

	SAVE_BOOL(m_bActivated);
	SAVE_bool(m_bCanTalk);

	SAVE_bool(m_bInvulnerable);

	SAVE_bool(m_bUnconscious);

	SAVE_BOOL(m_bFirstUpdate);
	SAVE_FLOAT(m_fNextUpdateRate);
	SAVE_BOOL(m_bUpdateAI);

	SAVE_DWORD(m_nAlarmLevel);
	SAVE_DWORD(m_eAlarmStimID);

	SAVE_FLOAT(m_fAccuracy);
	SAVE_FLOAT(m_fAccuracyIncreaseRate);
	SAVE_FLOAT(m_fFullAccuracyRadiusSqr);
	SAVE_FLOAT(m_fAccuracyMissPerturb);
	SAVE_FLOAT(m_fMaxMovementAccuracyPerturb);
	SAVE_FLOAT(m_fMovementAccuracyPerturbDecay);

	SAVE_HOBJECT(m_hDialogueObject );

	SAVE_STDSTRING( m_strCmdDamagedPlayer );
	SAVE_INT( m_nDamagedPlayerNumCount );
	SAVE_INT( m_nDamagedPlayerActivationCount );

	SAVE_FLOAT(m_fSenseUpdateRate);
	SAVE_TIME(m_fNextSenseUpdate);
	SAVE_DWORD(m_flagsCurSenses);
	
	SAVE_RANGE(m_rngSightGridX);
	SAVE_RANGE(m_rngSightGridY);

	SAVE_BOOL(m_bPreserveActiveCmds);

	SAVE_BOOL((bool)(!!m_pAnimationContext));

	if ( m_pAnimationContext )
	{
		m_pAnimationContext->Save(pMsg);
	}
	SAVE_HOBJECT(m_hAnimObject);

	m_pGoalMgr->Save(pMsg);

	// Save state...

	uint32 dwState = (uint32)-1;
	if (m_pState)
	{
		dwState = (uint32) m_pState->GetStateClassType();
	}

	SAVE_DWORD(dwState);

	if (m_pState)
	{
		m_pState->Save(pMsg);
	}

	m_pAIMovement->Save(pMsg);
	SAVE_BOOL(m_bForceGround);
	SAVE_BOOL(m_bPosDirty);
	SAVE_VECTOR(m_vLastFindFloorPos);
	SAVE_TIME(m_fNextFindFloorTime);
	SAVE_BOOL(m_bSyncPosition);

	m_pPathKnowledgeMgr->Save( pMsg );

	m_pNodeTrackerContext->Save( pMsg );

	SAVE_VECTOR(m_vMovePos);
	SAVE_BOOL(m_bMove);
	SAVE_FLOAT(m_fJumpOverVel);

	SAVE_bool( m_bIsCinematicAI );

	m_pAIWorkingMemory->Save(pMsg);
	m_pAIBlackBoard->Save(pMsg);
	m_pAISensorMgr->Save(pMsg);
	m_pAIWorldState->Save(pMsg);
	m_pAINavigationMgr->Save(pMsg);

	SAVE_bool(m_pAIWeaponMgr ? true : false);
	if (m_pAIWeaponMgr)
	{
		m_pAIWeaponMgr->Save(pMsg);
	}

	m_pAICommandMgr->Save(pMsg);

	SAVE_bool( m_pAIPlan != NULL );
	if (m_pAIPlan)
	{
		m_pAIPlan->Save(pMsg);
	}

	m_AIConfig.Save( pMsg );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::Load
//
//	PURPOSE:	Load the object
//
// ----------------------------------------------------------------------- //

void CAI::Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags)
{
    if (!g_pLTServer || !pMsg) return;

	char szName[128];
	pMsg->ReadString( szName, 128 );
	bool bBrain = false;
	LOAD_bool(bBrain);
	if ( bBrain )
	{
		SetBrain(szName);
	}

	LOAD_BOOL(m_bCheapMovement);
	LOAD_FLOAT(m_flVerticalThreshold);

	LOAD_BOOL(m_bUseDefaultAttachments);
	LOAD_BOOL(m_bUseDefaultWeapons);

	static std::string strAmmoLoad;
	LOAD_STDSTRING(strAmmoLoad);
	m_eAmmoLoad = g_pAIDB->GetAIAmmoLoadRecordID( strAmmoLoad.c_str() );

	m_pTarget->Load(pMsg);

	LOAD_VECTOR(m_vEyePos);
	LOAD_VECTOR(m_vEyeForward);
	LOAD_VECTOR(m_vTorsoRight);
	LOAD_VECTOR(m_vTorsoForward);
	LOAD_VECTOR(m_vWeaponPos);
	LOAD_VECTOR(m_vWeaponForward);
	LOAD_BOOL(m_bUpdateNodes);

	LOAD_VECTOR(m_vPos);
	LOAD_VECTOR(m_vPathingPosition);
	LOAD_VECTOR(m_vDims);
	LOAD_FLOAT(m_fRadius);

	LOAD_BOOL(m_bSeeThrough);
	LOAD_BOOL(m_bShootThrough);

	LOAD_STDSTRING(m_strCmdInitial);
	LOAD_STDSTRING(m_strCmdActivateOn);
	LOAD_STDSTRING(m_strCmdActivateOff);

	LOAD_BOOL(m_bActivated);
	LOAD_bool(m_bCanTalk);

	LOAD_bool(m_bInvulnerable);
	if( m_bInvulnerable )
	{
		MakeInvulnerable( true );
	}

	LOAD_bool(m_bUnconscious);

	LOAD_BOOL(m_bFirstUpdate);
	LOAD_FLOAT(m_fNextUpdateRate);
	LOAD_BOOL(m_bUpdateAI);

	LOAD_DWORD(m_nAlarmLevel);
	LOAD_DWORD_CAST(m_eAlarmStimID, EnumAIStimulusID);

	LOAD_FLOAT(m_fAccuracy);
	LOAD_FLOAT(m_fAccuracyIncreaseRate);
	LOAD_FLOAT(m_fFullAccuracyRadiusSqr);
	LOAD_FLOAT(m_fAccuracyMissPerturb);
	LOAD_FLOAT(m_fMaxMovementAccuracyPerturb);
	LOAD_FLOAT(m_fMovementAccuracyPerturbDecay);

	LOAD_HOBJECT(m_hDialogueObject);

	LOAD_STDSTRING( m_strCmdDamagedPlayer );
	LOAD_INT( m_nDamagedPlayerNumCount );
	LOAD_INT( m_nDamagedPlayerActivationCount );

	LOAD_FLOAT(m_fSenseUpdateRate);
	LOAD_TIME(m_fNextSenseUpdate);
	LOAD_DWORD(m_flagsCurSenses);

	LOAD_RANGE_CAST(m_rngSightGridX, int);
	LOAD_RANGE_CAST(m_rngSightGridY, int);

	LOAD_BOOL(m_bPreserveActiveCmds);

	bool bAnimationContext = false;
	LOAD_BOOL(bAnimationContext);

	if( m_pAnimationContext )
	{
		debug_delete( m_pAnimationContext );
		m_pAnimationContext = NULL;
	}
	
	if( bAnimationContext )
	{
		m_pAnimationContext = debug_new( CAnimationContext );
        if( m_pAnimationContext )
		{
			m_pAnimationContext->Init( m_hObject, m_hModel );
			m_pAnimationContext->Load( pMsg );
			
			if( m_pAnimationContext->GetNumPackedTrees( ) == 0 )
			{
				debug_delete( m_pAnimationContext );
				m_pAnimationContext = NULL;
			}
		}
	}

	// Add the recoil tracker after loading the animation context...
	// This needs to be done after the other animation trackers because
	// recoils are additively blended so the recoil tracker should be
	// the last tracker added...
	AddRecoilTracker( );

	LOAD_HOBJECT(m_hAnimObject);

	m_pAISensorMgr->InitSensorMgr( this );
	m_pGoalMgr->Init(this);
	m_pGoalMgr->Load(pMsg);

	// Load state...

	uint32 dwState;
	LOAD_DWORD(dwState);

	if (dwState != (DWORD)-1)
	{
		SetState((EnumAIStateType)dwState, false);

		if (m_pState)
		{
			m_pState->Load(pMsg);
		}
	}

	m_pAIMovement->Load(pMsg);
	LOAD_BOOL(m_bForceGround);
	LOAD_BOOL(m_bPosDirty);
	LOAD_VECTOR(m_vLastFindFloorPos);
	LOAD_TIME(m_fNextFindFloorTime);
	LOAD_BOOL(m_bSyncPosition);

	// If we just transitioned, then our position information is invalid.
	if( dwLoadFlags == LOAD_TRANSITION )
	{
		m_vLastFindFloorPos = LTVector( 99999999.f, 99999999.f, 99999999.f );
	}

	m_pPathKnowledgeMgr->Load( pMsg );

	m_pNodeTrackerContext->Init( m_hObject, m_hModelSkeleton );
	m_pNodeTrackerContext->Load( pMsg );

	// Go back to tracking none, since we reset this all the time anyway.

	m_pNodeTrackerContext->SetActiveTrackerGroups( kTrackerFlag_None );

	LOAD_VECTOR(m_vMovePos);
	LOAD_BOOL(m_bMove);
	LOAD_FLOAT(m_fJumpOverVel);

	m_cstrCurrentInfo = "";

	LOAD_bool( m_bIsCinematicAI );

	m_pAIWorkingMemory->Load(pMsg);
	m_pAIBlackBoard->Load(pMsg);
	m_pAISensorMgr->Load(pMsg);
	m_pAIWorldState->Load(pMsg);
	m_pAINavigationMgr->Load(pMsg);

	bool bHasWeaponMgr = false;
	LOAD_bool(bHasWeaponMgr);
	if (bHasWeaponMgr)
	{
		m_pAIWeaponMgr->Load(pMsg);
	}

	m_pAICommandMgr->Load(pMsg);

	bool bHasPlan = false;
	LOAD_bool( bHasPlan );
	if (bHasPlan)
	{
		m_pAIPlan = AI_FACTORY_NEW( CAIPlan );
		m_pAIPlan->Load(pMsg);
	}
	else
	{
		m_pAIPlan = NULL;
	}

	m_AIConfig.Load( pMsg );
	m_bConfigured = true;	// The configuration was done before save.

	// Recreate the specialfx message since we know all of our data is loaded and 
	// it may have gotten created inadvertently with default data by loading other
	// data such as the GoalMgr...

	CreateSpecialFX();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::*Target
//
//	PURPOSE:	Target methods
//
// ----------------------------------------------------------------------- //

CAITarget* CAI::GetTarget()
{
	return m_pTarget;
}

bool CAI::HasTarget( unsigned int dwTargetFlags )
{
	if( GetAIBlackBoard()->GetBBTargetType() & dwTargetFlags )
		return true;

	return false;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::UpdateAwareness
//
//	PURPOSE:	Awareness methods
//
// ----------------------------------------------------------------------- //

void CAI::UpdateAwareness()
{
	// No change.

	EnumAIAwareness eAwareness = m_pAIBlackBoard->GetBBAwareness();
	if( m_eLastAwareness == eAwareness )
	{
		return;
	}

	// If we going into or out of an alert state, clear path knowledge
	// because some volumes may only be useable for pathfinding when alert.

	if( ( m_eLastAwareness == kAware_Alert ) || 
		( eAwareness == kAware_Alert ) )
	{
		if( eAwareness == kAware_Alert )
		{
			AITRACE( AIShowGoals, ( m_hObject, "CAI::SetAwareness: Going ALERT!" ) );
		}

		m_pPathKnowledgeMgr->ClearPathKnowledge();

		// Tell the client, so that the player can detect when AI
		// are unalert, and vulnerable to instant kills from 
		// "finishing moves."

		m_pAIBlackBoard->SetBBUpdateClient( true );
	}

	// Record new awareness.

	m_eLastAwareness = eAwareness;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::Get*Sound
//
//	PURPOSE:	Gets various sounds
//
// ----------------------------------------------------------------------- //

const char* CAI::GetDeathSound()
{
	if ( m_damage.GetLastDamageType() == DT_EXPLODE )
	{
		return ::GetSound(this, kAIS_Explode);
	}
	else if ( m_damage.GetLastDamageType() == DT_CRUSH )
	{
		return ::GetSound(this, kAIS_Crush);
	}

	return ::GetSound(this, kAIS_Death);

}

const char* CAI::GetDeathSilentSound()
{
	return GetSound(this, kAIS_DeathQuiet);
}

const char* CAI::GetDamageSound(DamageType eType)
{
	return ::GetSound(this, kAIS_Pain);
}

bool CAI::CanLipSync()
{
	return ( m_pAIAttributes ) ? m_pAIAttributes->bCanLipSync : false; 
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::UpdateNodes
//
//  PURPOSE:	Update the position of important nodes (eye, torso, etc)
//
// ----------------------------------------------------------------------- //

void CAI::UpdateNodes()
{
	if( m_bUpdateNodes )
	{
		// Do not update nodes again until someone clears this flag.

		m_bUpdateNodes = false;

		if ( INVALID_MODEL_NODE == m_hHeadNode )
		{
			g_pModelLT->GetNode(m_hObject, "Head", m_hHeadNode);

			AIASSERT( INVALID_MODEL_NODE != m_hHeadNode, m_hObject,
				"Unable to find node named Head for this character.  Invalid Eye position" );
		}

/**
		// BUG BUG
		// If we do not call ApplyAnimations() here, the head node's transform
		// never reflects the direction the head is actually facing.
		// In Office_Layout-GP01 the model is facing (0,0,1).
		// If the player fires a weapon, the model turns its head to face the
		// sound at about (1,0,0), but still returns a transform of (0,0,1).
		// If the model is rotated, the head trasform does reflect the rotation,
		// meaning that the base transofrm is being applied.
		// Querying the eye nodes gives the same results.

		g_pModelLT->DirtyAllNodeTransforms( m_hObject );

		// END BUG BUG
**/

		// Set the eye position to be at the head node.
		LTTransform transform;
		g_pModelLT->GetNodeTransform(m_hObject, m_hHeadNode, transform, true);

		m_vEyePos = transform.m_vPos;
		LTRotation rRot = transform.m_rRot;
		m_vEyeForward = rRot.Up();

//AITRACE( AIShowGoals, ( m_hObject, "HEAD: %.2f %.2f %.2f",
//		m_vEyeForward.x, m_vEyeForward.y, m_vEyeForward.z ) );

		// Get the current Torso node basis, which may be different from
		// the AI's if Torso tracking is enabled.

		if( m_pNodeTrackerContext->IsTrackerGroupActive( kTrackerGroup_AimAt ) )
		{
			m_vTorsoRight = -rRot.Forward();
			m_vTorsoRight.y = 0.f;
			m_vTorsoForward = m_vEyeForward;
		}
		else 
		{
			m_vTorsoRight = GetRightVector();
			m_vTorsoForward = GetForwardVector();
		}

		// Get the current weapon pos.

		if (m_pAIWeaponMgr)
		{
			const CWeapon* pWeapon = m_pAIWeaponMgr->GetCurrentWeapon();
			if( pWeapon )
			{
				LTRotation rRot;
				m_pAIWeaponMgr->GetWeaponOrientation(pWeapon, m_vWeaponPos, rRot);
				m_vWeaponForward = rRot.Forward();
			}
		}
	}
}

void CAI::UpdateMovement()
{
	//track our performance
	CTimedSystemBlock TimingBlock(g_tsAIMovement);

	bool bLastCheapMovement = m_bCheapMovement;
	m_pAIMovement->Update();

	// If last update did not use cheap movement,
	// but this update does, force the AI to the ground.

	if( ( !bLastCheapMovement ) && m_bCheapMovement )
	{
		m_bForceGround = m_bMoveToFloor;
	}

	// Force dead AI to the ground, if they were not sitting, crouching, or riding a vehicle.

	if( m_damage.IsDead() && 
		m_pAnimationContext->IsPropSet( kAPG_Posture, kAP_POS_Stand ) &&
		( m_pAIBlackBoard->GetBBAttachedTo() == NULL ) )
	{
		m_bForceGround = m_bMoveToFloor;
	}

	if ( !m_bMove || m_bForceGround || m_bPosDirty)
	{
		if ( !m_bForceGround && !m_bPosDirty )
		{
			return;
		}
		else if ( !m_bMove )
		{
			m_vMovePos = m_vPos;
		}
	}

	if ( m_bCheapMovement || m_bForceGround )
	{
		// Only call FindFloorHeight() (which calls $$ IntersectSegment() $$ ) 
		// if the AI has moved past a threshold distance.

		double fCurTime = g_pLTServer->GetTime();
		float fDiffSqr = m_vLastFindFloorPos.DistSqr( m_vMovePos );
		if( m_bForceGround || 
			( ( fDiffSqr >= FIND_FLOOR_THRESH_SQR ) && 
				( fCurTime > m_fNextFindFloorTime ) ) )
		{
			// Find the floor underneath us

			float fFloorHeight;
			if( FindFloorHeight(m_vMovePos, &fFloorHeight) )
			{
				m_vMovePos.y = fFloorHeight;
			}

			m_fNextFindFloorTime = fCurTime + 0.33f;
			m_vLastFindFloorPos = m_vMovePos;
		}
	}

	g_pLTServer->Physics()->MoveObject(m_hObject, m_vMovePos, 0 );

	// Clear out movement info

	m_bForceGround = false;
	m_bPosDirty = false;
	m_bMove = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::OpenDoor()
//
//	PURPOSE:	Attempts to open a door
//
// ----------------------------------------------------------------------- //

void CAI::OpenDoor()
{
	// Link does not exist, or is not of type AINavMeshLinkDoor.

	AINavMeshLinkAbstract* pLink = g_pAINavMesh->GetNMLink( GetCurrentNavMeshLink() );
	if( ( !pLink ) ||
		( pLink->GetNMLinkType() != kLink_Door ) )
	{
		pLink = g_pAINavMesh->GetNMLink(GetAIBlackBoard()->GetBBEnteringNMLink());
		if( ( !pLink ) ||
			( pLink->GetNMLinkType() != kLink_Door ) )
		{
			return;
		}
	}

	AINavMeshLinkDoor* pLinkDoor = ( AINavMeshLinkDoor* )pLink;
	pLinkDoor->OpenDoor(this);

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::KickDoor()
//
//	PURPOSE:	Attempts to open a door by kicking -- this needs to push 
//				back the player if they are blocking the door.  Once 
//				physics are in, hopefully the door can do the pushing on 
//				its own.
//
// ----------------------------------------------------------------------- //

void CAI::KickDoor()
{
	// Handle requesting the door open
	OpenDoor();

	//
	// TEMP HACK 
	//
	// bjl 12/18/03
	//
	// push the player back until physics handles doors pushing the player 
	// when a kick occurs.  Once physics goes in, the door should handle this 
	// on its own.  See AIActionTraverseBlockedDoor for additional related temp 
	// code.
	//
	if (m_pGoalMgr->GetCurrentGoal() 
		&& m_pGoalMgr->GetCurrentGoal()->GetCurrentAction()
		&& m_pGoalMgr->GetCurrentGoal()->GetCurrentAction()->GetActionRecord()->eActionType == kAct_TraverseBlockedDoor)
	{
		GetTarget()->SetPushSpeed(600.f);
		GetTarget()->SetPushMinDist(200.f);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::UpdateTarget
//
//	PURPOSE:	Updates the target info
//
// ----------------------------------------------------------------------- //

void CAI::UpdateTarget()
{
	//track our performance
	CTimedSystemBlock TimingBlock(g_tsAITarget);

	m_pTarget->UpdateTarget();

	HOBJECT hObject = m_pAIBlackBoard->GetBBTargetObject();
	if (IsCharacter(hObject))
	{
		CCharacter *pCharacter = (CCharacter*)g_pLTServer->HandleToObject( hObject );
		if( pCharacter )
		{
			m_pTarget->UpdatePush( pCharacter );
		}
	}

	// Check visibility from a position dependent on the current weapon.
	// Primary weapon is on the right side, secondary on the left.
	// If no weapon, check from the eyes.

	LTVector vCheckPos;
	if( m_pAIWeaponMgr && m_pAIWeaponMgr->GetCurrentWeapon() )
	{
		vCheckPos = GetWeaponPosition(m_pAIWeaponMgr->GetCurrentWeapon(), false);
	}
	else 
	{
		vCheckPos = GetEyePosition();
	}

	m_pTarget->UpdateVisibility(vCheckPos);
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAI::GetAccuracy()
//              
//	PURPOSE:	returns the heavily modified accuracy modifier
//              
//----------------------------------------------------------------------------
float CAI::GetAccuracy()
{
	float fAccuracy = RAISE_BY_DIFFICULTY(m_fAccuracy);
	if( fAccuracy < 0.f )
	{
		fAccuracy = 0.f;
	}
	else if( fAccuracy > 1.f )
	{
		fAccuracy = 1.f;
	}
	return fAccuracy;
}

float	CAI::GetAccuracyMissPerturb()
{
	return LOWER_BY_DIFFICULTY(m_fAccuracyMissPerturb); 
}

float	CAI::GetMaxMovementAccuracyPerturb()
{
	return LOWER_BY_DIFFICULTY(m_fMaxMovementAccuracyPerturb); 
}

//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAI::SetPosition()
//              
//	PURPOSE:	Function to set the AIs position -- added so that all setting
//				of the position can be trapped at a single point.
//              
//----------------------------------------------------------------------------
void CAI::SetPosition(const LTVector& vPos, bool bFindFloorHeight)
{
	// Set the new position
	m_vPos = vPos;

	LTVector vPathingPos = m_vPos;
	float flHeight;
	if ( bFindFloorHeight && m_bMoveToFloor && ( true == FindFloorHeight( m_vPos, &flHeight ) ) )
	{
		vPathingPos.y = flHeight;
	}
	
	m_vPathingPosition = vPos;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::UpdatePosition
//
//	PURPOSE:	Updates the position, orientation, etc of our object
//
// ----------------------------------------------------------------------- //

void CAI::UpdatePosition()
{
	//track our performance
	CTimedSystemBlock TimingBlock(g_tsAIPosition);

	// Set the base AI's position

	LTVector vPosition;
	g_pLTServer->GetObjectPos(m_hObject, &vPosition);
	SetPosition( vPosition, false );

	// Get our dims+radius

    g_pPhysicsLT->GetObjectDims(m_hObject, &m_vDims);
    m_fRadius = sqrt( ( m_vDims.x*m_vDims.x ) + ( m_vDims.z*m_vDims.z ) );

	// Get rid of any acceleration the server is applying to us

    LTVector vAccel;
	g_pPhysicsLT->GetAcceleration(m_hObject, &vAccel);
	vAccel.x = vAccel.z = 0.0f;
	if (vAccel.y > 0.0f) vAccel.y = 0.0f;
	g_pPhysicsLT->SetAcceleration(m_hObject, vAccel);

    LTVector vVelocity;
	g_pPhysicsLT->GetVelocity(m_hObject, &vVelocity);
	vVelocity.x = vVelocity.z = 0.0f;
	if (vVelocity.y > 0.0f) vVelocity.y = 0.0f;
	g_pPhysicsLT->SetVelocity(m_hObject, vVelocity);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::ComputeDamageModifier
//
//	PURPOSE:	Adjust the amount of damage based on the node hit...
//
// ----------------------------------------------------------------------- //

float CAI::ComputeDamageModifier( ModelsDB::HNODE hModelNode)
{
    float fModifier = CCharacter::ComputeDamageModifier(hModelNode);

	bool bIsAlert = ( m_pAIBlackBoard->GetBBAwareness() == kAware_Alert ) ||
					( m_pAIBlackBoard->GetBBAwareness() == kAware_Suspicious );

	if ( bIsAlert )
	{
        fModifier = LTMIN(2.0f, fModifier);
	}
	else
	{
		fModifier *= g_pModelsDB->GetUnalertDamageFactor( m_hModel );
	}

	return fModifier;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::IncrementAlarmLevel
//
//	PURPOSE:	Raises alarm level. Registers suspicious stimulus if reached threshold.
//
// ----------------------------------------------------------------------- //

void CAI::IncrementAlarmLevel(uint32 nIncr)
{
	m_nAlarmLevel += nIncr;

	if( ( m_pAIBlackBoard->GetBBAwareness() < kAware_Alert ) &&
		( IsMajorlyAlarmed() ) )
	{
		m_pAIBlackBoard->SetBBAwareness( kAware_Alert );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::IsMajorlyAlarmed
//
//	PURPOSE:	Determines if we are majorly alarmed
//
// ----------------------------------------------------------------------- //

bool CAI::IsMajorlyAlarmed()
{
	return ( GetBrain() && ( m_nAlarmLevel >= GetBrain()->GetMajorAlarmThreshold() ) );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::IsImmediatelyAlarmed
//
//	PURPOSE:	Determines if we are immediately alarmed
//
// ----------------------------------------------------------------------- //

bool CAI::IsImmediatelyAlarmed()
{
	return ( GetBrain() && ( m_nAlarmLevel >= GetBrain()->GetImmediateAlarmThreshold() ) );
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::LinkDialogueObject()
//
//	PURPOSE:	Links us to a dialogue object
//
// --------------------------------------------------------------------------- //

void CAI::LinkDialogueObject(HOBJECT hDialogueObject)
{
	if ( m_hDialogueObject )
	{
		// We allow this because the dialogue is messaging us per everytime
		// we're in the list rather than be smart and only doing it once.
		// g_pLTServer->CPrint("AI in simultaneous dialogues!");
	}
	else
	{
		m_hDialogueObject = hDialogueObject;
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::UnlinkDialogueObject()
//
//	PURPOSE:	Unlinks us from a dialogue object
//
// --------------------------------------------------------------------------- //

void CAI::UnlinkDialogueObject(HOBJECT hDialogueObject)
{
	if (!m_hDialogueObject) return;

	if ( hDialogueObject != m_hDialogueObject )
	{
		g_pLTServer->CPrint("AI ''%s'' was unlinked from dialogue object it was not linked to!", GetName());
		return;
	}

	m_hDialogueObject = NULL;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::GetAlternateDeathAnimation()
//
//	PURPOSE:	Gives goals a chance to choose an appropriate death animation.
//
// --------------------------------------------------------------------------- //

HMODELANIM CAI::GetAlternateDeathAnimation()
{
	return INVALID_ANI;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::GetDeathAni()
//
//	PURPOSE:	Get the death animation
//
// ----------------------------------------------------------------------- //

HMODELANIM CAI::GetDeathAni(bool bFront)
{
	HMODELANIM hAni = INVALID_MODEL_ANIM;

	if ( GetAnimationContext()->IsPropSet(kAPG_Movement, kAP_MOV_Run) )
	{
		char* aszDeathRunsFront[] = { "DRun", "DRun2" };
		char* aszDeathRunsBack[]  = { "DRunBack", "DRunBack2" };

		int cDeathRuns = sizeof(aszDeathRunsBack)/sizeof(const char*);
		char** pDeathRuns = (bFront ? aszDeathRunsFront : aszDeathRunsBack);

        if ( INVALID_MODEL_ANIM != (hAni = g_pLTServer->GetAnimIndex(m_hObject, (char*)pDeathRuns[GetRandom(0, cDeathRuns-1)])) )
		{
			return hAni;
		}
	}

	if ( GetAnimationContext()->IsPropSet(kAPG_Posture, kAP_POS_Crouch) )
	{
        if ( INVALID_MODEL_ANIM != (hAni = g_pLTServer->GetAnimIndex(m_hObject, (char*)GetCrouchDeathAni())) )
		{
			return hAni;
		}
	}

	if ( GetAnimationContext()->IsPropSet(kAPG_Posture, kAP_POS_Prone) )
	{
        if ( INVALID_MODEL_ANIM != (hAni = g_pLTServer->GetAnimIndex(m_hObject, (char*)GetProneDeathAni())) )
		{
			return hAni;
		}
	}

	return super::GetDeathAni(bFront);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::GetCrouchDeathAni()
//
//	PURPOSE:	Get a crouching death animation
//
// ----------------------------------------------------------------------- //

const char* CAI::GetCrouchDeathAni()
{
	static const char* aszDeathCrouches[] = { "DCrouch2", "DCrouch3", "DCrouch4", "DCrouch5" };
    static const int cDeathCrouches = sizeof(aszDeathCrouches)/sizeof(const char*);
	return aszDeathCrouches[GetRandom(0, cDeathCrouches-1)];
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::GetProneDeathAni()
//
//	PURPOSE:	Get a Proneing death animation
//
// ----------------------------------------------------------------------- //

const char* CAI::GetProneDeathAni()
{
	static const char* aszDeathPronees[] = { "DProne", };
    static const int cDeathPronees = sizeof(aszDeathPronees)/sizeof(const char*);
	return aszDeathPronees[GetRandom(0, cDeathPronees-1)];
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::PlayDeathSound()
//
//	PURPOSE:	Play the death sound
//
// ----------------------------------------------------------------------- //

void CAI::PlayDeathSound()
{

	if (m_bDecapitated || m_bGibbed)
	{
		CCharacter::PlayDeathSound();
		return;
	}

	const char* szDeathSound = NULL;
	// If we're unconscious make no noise

	if ( GetAnimationContext()->IsPropSet(kAPG_Action, kAP_ACT_Unconscious) || m_bUnconscious )
	{
		return;
	}

	// Check for "silent kill" -- hit in head and not alert

	if ( WasSilentKill() ||
		 GetAnimationContext()->IsPropSet(kAPG_Action, kAP_ACT_Asleep) )
	{
		szDeathSound = GetDeathSilentSound();
	}
	else
	{
		szDeathSound = GetDeathSound();
	}

	CCharacter::PlayDialogSound(szDeathSound, CST_DEATH);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::WasSilentKill()
//
//	PURPOSE:	Determines if we have been killed silently or not
//
// ----------------------------------------------------------------------- //

bool CAI::WasSilentKill()
{
	if( m_fLastPainVolume <= 0.1f )
	{
		return true;
	}

	bool bIsAlert = ( m_pAIBlackBoard->GetBBAwareness() == kAware_Alert ) ||
					( m_pAIBlackBoard->GetBBAwareness() == kAware_Suspicious );

	if ( bIsAlert )
	{
		return false;
	}
	else if ( !m_hModelNodeLastHit )
	{
		return false;
	}
	else
	{
		return NODEFLAG_CRITICALHIT & g_pModelsDB->GetNodeFlags(m_hModelNodeLastHit) ? true : false;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::PrepareToSever
//
//	PURPOSE:	Switch to the sever body...
//
// ----------------------------------------------------------------------- //

void CAI::PrepareToSever()
{
	//save off the old anim
	HMODELANIM hAni = INVALID_MODEL_ANIM;
	uint32 nTime = 0;
	g_pModelLT->GetCurAnim(m_hObject, MAIN_TRACKER, hAni);
	if (hAni != INVALID_MODEL_ANIM)
	{
		g_pModelLT->GetCurAnimTime(m_hObject, MAIN_TRACKER, nTime);
	}

	super::PrepareToSever();

	//apply the old anim to the new model
	if (hAni != INVALID_MODEL_ANIM)
	{
		g_pModelLT->SetCurAnim(m_hObject, MAIN_TRACKER, hAni,false);
		if (hAni != INVALID_MODEL_ANIM)
		{
			g_pModelLT->SetCurAnimTime(m_hObject, MAIN_TRACKER, nTime);
		}

	}


	// Turn off node tracking before we swap the model!
	// (Or we will crash).

	m_pAIBlackBoard->SetBBTargetTrackerFlags( kTrackerFlag_None );
	m_pNodeTrackerContext->UpdateNodeTrackers( this );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::SetBrain
//
//	PURPOSE:	Changes our current Brain
//
// ----------------------------------------------------------------------- //

bool CAI::SetBrain( char const* pszBrain )
{
	// Delete the old Brain

	if ( m_pBrain )
	{
		AI_FACTORY_DELETE(m_pBrain);
		m_pBrain = NULL;
	}

	m_pBrain = AI_FACTORY_NEW(CAIBrain);

	if ( !pszBrain || (-1 == g_pAIDB->GetAIBrainRecordID( pszBrain )) )
	{
		g_pLTServer->CPrint("AI ''%s'' does not have a valid brain specified! Using ''Default''.", GetName());
		m_pBrain->Init(this, "Default");
	}
	else
	{
		m_pBrain->Init( this, pszBrain );
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::SetDamageMask
//
//	PURPOSE:	Changes our current DamageMask
//
// ----------------------------------------------------------------------- //

void CAI::SetDamageMask( char const* pszMask )
{
	CDestructible* pDestructable = GetDestructible();
	if( !pDestructable )
	{
		AIASSERT( 0, m_hObject, "CAI::SetDamageMask: No destructable." );
		return;
	}

	// No mask.

	if( LTStrICmp( pszMask, "None" ) == 0 )
	{
		pDestructable->SetCantDamageFlags( 0 );
		return;
	}

	// Find mask in aibutes.txt

	int nDamageMaskID = g_pAIDB->GetAIDamageMaskRecordID( pszMask );
	if( nDamageMaskID != -1 )
	{
		pDestructable->SetCantDamageFlags( ~( g_pAIDB->GetAIDamageMaskRecord( nDamageMaskID )->dfDamageTypes ) );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::UpdateCharacterFXBodyState()
//
//	PURPOSE:	Update the characterFX BodyState.
//
// ----------------------------------------------------------------------- //

void CAI::UpdateCharacterFXBodyState()
{
	// Nothing to update.

	if( !m_pAIBlackBoard->GetBBUpdateClient() )
	{
		return;
	}

	BodyState eBodyState = eBodyStateNormal;
	EnumAnimDesc eCondition = kAD_None;

	// AI has a scripted body state

	if ( eBodyStateInvalid != m_pAIBlackBoard->GetBBScriptedBodyState() )
	{
		eBodyState = m_pAIBlackBoard->GetBBScriptedBodyState();
	}

	// AI is long recoiling.

	else if( m_pAnimationContext->GetCurrentProp( kAPG_Action ) == kAP_ACT_LongRecoil )
	{
		eBodyState = eBodyStateLongRecoiling;
		eCondition = m_pAnimationContext->GetDescriptor( kADG_Condition );
	}

	// AI can be kicked off

	else if ( m_pAnimationContext->GetCurrentProp( kAPG_Action ) == kAP_ACT_AttackBerserker )
	{
		CAIWMFact queryFact;
		queryFact.SetFactType( kFact_Knowledge );
		queryFact.SetKnowledgeType( kKnowledge_BerserkerAttachedPlayer );
		CAIWMFact* pFact = m_pAIWorkingMemory->FindWMFact( queryFact );
		AIASSERT( pFact, GetHOBJECT(), "CAI::UpdateCharacterFXBodyState: AI appears to be berserking without a record of a player it is attached to." );

		// The index we are comparing against needs to be kept in sync with the 
		// index specifying the BerserkRecoilDismount, so that the player and 
		// AI agree on when the next kick will be a separation

		if ( pFact && pFact->GetIndex() > 1 )
		{
			AITRACE( AIShowBerserker, ( m_hObject, "(AnimChange) Kick count: %d", pFact->GetIndex() ) );
			eBodyState = eBodyStateBerserked;
		}
		else
		{
			AITRACE( AIShowBerserker, ( m_hObject, "(AnimChange - next is dismount) Kick count: %d", pFact->GetIndex() ) );
			eBodyState = eBodyStateBerserkedOut;
		}
	}

	// AI just finished long recoiling or changed awareness.

	else if( m_pAIBlackBoard->GetBBAwareness() == kAware_Relaxed )
	{
		eBodyState = eBodyStateUnalert;
	}

	// Update the client with the AI's current BodyState.

	CAutoMessage cMsg;
	cMsg.Writeuint8(MID_SFX_MESSAGE);
	cMsg.Writeuint8(SFX_CHARACTER_ID);
	cMsg.WriteObject(m_hObject);
	cMsg.WriteBits(CFX_BODYSTATE_MSG, FNumBitsExclusive<CFX_COUNT>::k_nValue );
	cMsg.Writeuint32(eBodyState);
	cMsg.Writeuint32(eCondition);
	g_pLTServer->SendToClient(cMsg.Read(), NULL, 0);

	// Clear the update flag.

	m_pAIBlackBoard->SetBBUpdateClient( false );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::StartDeath()
//
//	PURPOSE:	Start dying - handle weapons dropped by unconscious AI
//
// ----------------------------------------------------------------------- //

void CAI::StartDeath()
{
	if (m_bStartedDeath) return;

	// AI is no longer alarmed... he is dead.  Remove the stimulus.

	if ( kStimID_Unset != m_eAlarmStimID )
	{
		g_pAIStimulusMgr->RemoveStimulus( m_eAlarmStimID );
		m_eAlarmStimID = kStimID_Unset;
	}

	// Other AI should avoid nodes where AI just died,
	// or where AI was heading when he died.

	HOBJECT hLockedNode = m_pAIBlackBoard->GetBBLockedNode();
	if( IsAINode( hLockedNode ) )
	{
		static AINODE_LIST ClusteredNodeList;
		ClusteredNodeList.resize( 0 );

		// Find all nodes in cluster where AI died.

		AINode* pNode = (AINode*)g_pLTServer->HandleToObject( hLockedNode );
		if( pNode->GetAINodeClusterID() != kNodeCluster_Invalid )
		{
			g_pAINodeMgr->FindNodesInCluster( pNode->GetAINodeClusterID(), pNode->GetType(), &ClusteredNodeList );
		}
		else {
			ClusteredNodeList.push_back( pNode );
		}

		// Ensure all AI avoid all nodes in cluster.

		AINODE_LIST::iterator itNode;
		for( itNode = ClusteredNodeList.begin(); itNode != ClusteredNodeList.end(); ++itNode )
		{
			pNode = *itNode;

			// An AI has been damaged at this node.

			CAIWMFact factQuery;
			factQuery.SetFactType(kFact_Knowledge);
			factQuery.SetKnowledgeType(kKnowledge_DamagedAtNode);
			factQuery.SetTargetObject(pNode->m_hObject);

			CAIWMFact* pFact = g_pAIWorkingMemoryCentral->FindWMFact( factQuery );
			float fCurrentDamage = 0.f;
			if (!pFact)
			{
				pFact = g_pAIWorkingMemoryCentral->CreateWMFact(kFact_Knowledge);
			}
			else
			{
				pFact->GetDamage( NULL, &fCurrentDamage, NULL );
			}

			if (pFact)
			{
				float fDelay = GetRandom( g_pAIDB->GetAIConstantsRecord()->fDamagedAtNodeAvoidanceTimeMin,
										g_pAIDB->GetAIConstantsRecord()->fDamagedAtNodeAvoidanceTimeMax );

				pFact->SetKnowledgeType( kKnowledge_DamagedAtNode, 1.f );
				pFact->SetTargetObject( pNode->m_hObject, 1.f );
				pFact->SetTime( g_pLTServer->GetTime() + fDelay, 1.f );
				pFact->SetDamage( m_damage.GetLastDamageType(), fCurrentDamage + m_damage.GetLastDamage(), m_damage.GetLastDamageDir(), 1.f );
			}
		}
	}


	// Fire weapon while dying according to probability set per weapon.
	// Only do this if the AI was firing at the time of death.
	// Only do this if the AI was standing while firing.

	EnumAnimProp eAction = m_pAnimationContext->GetCurrentProp( kAPG_Action );
	EnumAnimProp ePosture = m_pAnimationContext->GetCurrentProp( kAPG_Posture );
	if( ( ePosture == kAP_POS_Stand ) &&
		( ( eAction == kAP_ACT_Fire ) ||
		  ( eAction == kAP_ACT_Aim ) ) )
	{
		CWeapon* pWeapon = m_Arsenal.GetCurWeapon();
		if( pWeapon )
		{
			const AIDB_AIWeaponRecord* pAIWeaponRecord;
			pAIWeaponRecord = AIWeaponUtils::GetAIWeaponRecord( 
				pWeapon->GetWeaponRecord(), 
				m_pAIBlackBoard->GetBBAIWeaponOverrideSet() );

			if( pAIWeaponRecord &&
				( pAIWeaponRecord->fFireDuringDeathProbability > 0.f ) &&
				( pAIWeaponRecord->fFireDuringDeathProbability >= GetRandom( 0.f, 1.f ) ) )
			{
				FireWeaponDuringDeath( true );
			}
		}
	}

	super::StartDeath();

	// Stop updating the AI unless the AI is handling the death.  The AI may 
	// choose to handle death by 'cleaning up' the AIs state with this 
	// function.

	if ( false == m_pAIBlackBoard->GetBBAIHandlingDeath() )
	{
		SetUpdateAI( false );
	}

	//handle weapons dropped when AI was knocked out
	SetUnconscious(false);

	if (m_pAIWeaponMgr)
	{
		m_pAIWeaponMgr->SpawnHolsteredItems();
	}

	// Terminate sensors.

	m_pAISensorMgr->TermAISensorMgr();

	// Turn off node tracking (this shouldn't be turned back on, as the AI 
	// should no longer be receiving updates)

	m_pNodeTrackerContext->SetActiveTrackerGroups( kTrackerFlag_None );

	// Clear the AIs memory -- he is dead, and facts should be reused by 
	// living AIs.  We can't just mark them for deletion because our garbage 
	// collection won't activate as this happens at the start of a level..

	m_pAIWorkingMemory->ResetWorkingMemory();
}


//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAI::GetVerticalThreshold()
//              
//	PURPOSE:	Returns the distance up and down the AI checks for volumes
//              
//----------------------------------------------------------------------------
float CAI::GetVerticalThreshold() const
{
	return m_flVerticalThreshold;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::PreCreateSpecialFX
//
//	PURPOSE:	Last chance to change our characterfx struct
//
// ----------------------------------------------------------------------- //

void CAI::PreCreateSpecialFX( CHARCREATESTRUCT& cs )
{
	CCharacter::PreCreateSpecialFX( cs );

	cs.bIsCinematicAI	= m_bIsCinematicAI;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::IsScripted
//
//	PURPOSE:	Test to see if the AI is still being scripted or not...
//
// ----------------------------------------------------------------------- //

bool CAI::IsScripted()
{
	if( GetAIBlackBoard()->GetBBTaskStatus() == kTaskStatus_Set )
	{
		// Task is still running, so bail and remain blocked.
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::ScriptCleanup
//
//	PURPOSE:    Do some cleanup once an object as finished scripting...
//
// ----------------------------------------------------------------------- //

void CAI::ScriptCleanup()
{
	// jeffo 2/27/04
	// Removed contents of this function.
	// previously it was clearing all tasks, which 
	// interfered with cover behavior.
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::InterruptScript
//
//	PURPOSE:	The object was interrupted while scripting so make sure it can exit gracefully...
//
// ----------------------------------------------------------------------- //

void CAI::InterruptScript()
{
	// Just cleanup, no special handling for being interrupted...
	ScriptCleanup();
}

void CAI::HandleArsenalChange()
{
	if (m_pAIWeaponMgr)
	{
		m_pAIWeaponMgr->InitPrimaryWeapon();
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::SetCurrentWeapon
//
//	PURPOSE:	Handle setting the weapon to the weapon of the passed in 
//				type.
//
// ----------------------------------------------------------------------- //

void CAI::SetCurrentWeapon(ENUM_AIWeaponType eWeaponType)
{
	if (m_pAIWeaponMgr)
	{
		m_pAIWeaponMgr->SetCurrentWeapon(eWeaponType);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::HandleWeaponBroke
//
//	PURPOSE:	Handle when a weapon has sustain too much damage.
//
// ----------------------------------------------------------------------- //

void CAI::HandleWeaponBroke( HWEAPON hBrokenWeapon, HWEAPON hReplacementWeapon )
{
	if (m_pAIWeaponMgr)
	{
		m_pAIWeaponMgr->RemoveWeapon(hBrokenWeapon);
		m_pAIWeaponMgr->AddWeapon(hReplacementWeapon, "RIGHTHAND", !FAILURE_IS_ERROR);
	}

	// If this is the last broken weapon in a chain, give the AI a desire to 
	// react to the broken weapon.

	if ( NULL == hReplacementWeapon )
	{
		// Notify the AI that his weapon just broke.

		CAIWMFact queryFact;
		queryFact.SetFactType( kFact_Knowledge );
		queryFact.SetKnowledgeType( kKnowledge_WeaponBroke );
		CAIWMFact* pFact = m_pAIWorkingMemory->FindWMFact( queryFact );
		if ( NULL == pFact )
		{
			pFact = m_pAIWorkingMemory->CreateWMFact( kFact_Knowledge );
			if ( pFact )
			{
				pFact->SetKnowledgeType( kKnowledge_WeaponBroke );
			}
		}

		if ( pFact )
		{
			pFact->SetTime( g_pLTServer->GetTime() );
		}

		// Always reevaluate the behavior when the weapon breaks, as we never want 
		// to continue swinging with nothing in our hands.

		m_pAIBlackBoard->SetBBSelectAction( true );
		m_pAIBlackBoard->SetBBInvalidatePlan( true );
	}
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::SetMoveToFloor
//
//	PURPOSE:	Sets the move to floor flag and commands a move to floor.
//
// --------------------------------------------------------------------------- //

void CAI::SetMoveToFloor( bool bValue )
{
	if( bValue == m_bMoveToFloor )
		return;

	m_bMoveToFloor = bValue;

	// If we turned it off, we're done, otherwise we need to move the object to the floor.
	if( !m_bMoveToFloor)
		return;

	// Tell the ai to move to ground the nexxt time it gets a chance.
	m_bForceGround = true;
}

// --------------------------------------------------------------------------- //
//
//	ROUTINE:	CAI::DropWeapons
//
//	PURPOSE:	Handle dropping all weapons which may be used by a 
//			player.
//
// --------------------------------------------------------------------------- //

void CAI::DropWeapons()
{
	if ( m_pAIWeaponMgr )
	{
		m_pAIWeaponMgr->HandleDropWeapons();
	}
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAI::SpawnGearItemsOnDeath
//
//  PURPOSE:	Drops gear items when the character dies... 
//
// ----------------------------------------------------------------------- //

void CAI::SpawnGearItemsOnDeath( )
{
	LTVector vPos;
	LTRotation rRot;
	g_pLTServer->GetObjectPos(m_hObject,&vPos);
	g_pLTServer->GetObjectRotation(m_hObject,&rRot);

	HMODELSOCKET hSocket;

	if(g_pModelLT->GetSocket(m_hObject, "Back", hSocket) == LT_OK)
	{
		LTTransform transform;

		if( LT_OK == g_pModelLT->GetSocketTransform(m_hObject, hSocket, transform, true))
		{
			vPos = transform.m_vPos;
			rRot = transform.m_rRot;
		}
	}

	LTVector vSpawnPos = vPos;

	HRECORD hDroppedItems = g_pAIDB->GetDroppedItemRecord(m_pAIAttributes->strDroppedItems.c_str());
	if (!hDroppedItems) return;

	HATTRIBUTE hItems = g_pLTDatabase->GetAttribute(hDroppedItems,AIDB_DROPPEDITEMS_Items);
	if (!hItems) return;
	
	uint32 nItems = g_pLTDatabase->GetNumValues(hItems);
	if (!nItems) return;

	//map the global index to this particular list
	uint32 nIndex = (m_nDropIndex % nItems);

	//increment the global index 
	m_nDropIndex++;

	HATTRIBUTE hAtt = g_pAIDB->GetStructAttribute(hItems,nIndex,AIDB_DROPPEDITEMS_rGearItem);
	HGEAR hGear = g_pLTDatabase->GetRecordLink(hAtt,0,NULL);
	if (hGear)
	{
		char szSpawn[1024] = "";
		LTSNPrintF( szSpawn, ARRAY_LEN(szSpawn), "GearItem Gravity 0;MoveToFloor 1;GearType (%s);Placed 0", g_pWeaponDB->GetRecordName( hGear ));

		BaseClass* pObj = SpawnObject( szSpawn, vSpawnPos, rRot );
		if (!pObj)
		{
			LTASSERT_PARAM1(0, "CAI::SpawnGearItemsOnDeath : Failed to Spawn: %s", szSpawn);
			return;
		}

		GearItem* pGearItem = dynamic_cast< GearItem* >( pObj );
		if( pGearItem )
		{
			LTVector vImpulse = rRot.Forward() + rRot.Up();
			vImpulse *= GetConsoleFloat("DropImpulse",1000.0f);
			if( GetDestructible()->IsDead() && GetDestructible()->GetDeathType() == DT_EXPLODE)
			{
				vImpulse += GetDestructible()->GetDeathDir() * GetDestructible()->GetDeathImpulseForce();
			}

			LTVector vAng( GetRandom(-10.0f,10.0f),GetRandom(-10.0f,10.0f),GetRandom(-20.0f,20.0f));
			pGearItem->DropItem( vImpulse, LTVector::GetIdentity(), vAng, m_hObject );		
		}

		// Randomize the position for the next spawn...
		LTVector vDiff( GetRandom( -100.0f, 100.0f ), 0, GetRandom( -100.0f, 100.0f ));
		vSpawnPos = (vPos + vDiff);
	}
	else
	{
		HATTRIBUTE hAtt = g_pAIDB->GetStructAttribute(hItems,nIndex,AIDB_DROPPEDITEMS_rWeaponItem);
		HWEAPON hWeapon = g_pLTDatabase->GetRecordLink(hAtt,0,NULL);
		
		if (hWeapon)
		{
			char szSpawn[1024] = "";
			LTSNPrintF( szSpawn, ARRAY_LEN(szSpawn), "WeaponItem Gravity 0;MoveToFloor 1;WeaponType (%s);Placed 0", g_pWeaponDB->GetRecordName( hWeapon ));

			BaseClass* pObj = SpawnObject( szSpawn, vSpawnPos, rRot );
			if (!pObj)
			{
				LTASSERT_PARAM1(0, "CAI::SpawnGearItemsOnDeath : Failed to Spawn: %s", szSpawn);
				return;
			}

			WeaponItem* pWeaponItem = dynamic_cast< WeaponItem* >( pObj );
			if( pWeaponItem )
			{
				LTVector vImpulse = rRot.Forward() + rRot.Up();
				vImpulse *= GetConsoleFloat("DropImpulse",1000.0f);
				if( GetDestructible()->IsDead() && GetDestructible()->GetDeathType() == DT_EXPLODE)
				{
					vImpulse += GetDestructible()->GetDeathDir() * GetDestructible()->GetDeathImpulseForce();
				}

				LTVector vAng( GetRandom(-10.0f,10.0f),GetRandom(-10.0f,10.0f),GetRandom(-20.0f,20.0f));
				pWeaponItem->DropItem( vImpulse, LTVector::GetIdentity(), vAng, m_hObject );		
			}

			// Randomize the position for the next spawn...
			LTVector vDiff( GetRandom( -100.0f, 100.0f ), 0, GetRandom( -100.0f, 100.0f ));
			vSpawnPos = (vPos + vDiff);
		}

	}
}

bool CAI::IsCrouched()
{
	return ( GetAnimationContext()->IsPropSet(kAPG_Posture, kAP_POS_Crouch) );
}

bool CAI::IsKnockedDown()
{
	EnumAnimProp eAction = GetAnimationContext()->GetCurrentProp( kAPG_Action );
	if( ( eAction == kAP_ACT_LongRecoil ) 
		|| ( eAction == kAP_ACT_DefeatedRecoil ) 
		)
	{
		return true;
	}

	return ( GetAnimationContext()->IsPropSet(kAPG_Activity, kAP_ATVT_KnockDown) );
}



//----------------------------------------------------------------------------
//              
//	ROUTINE:	CAIPlugin::CAIPlugin, ~CAIPlugin, GetAttachmentsPlugin
//              
//	PURPOSE:	Now handles pointers instead to the CAttachmentsPlugin
//				instead of instances, as this lets us avoid coupling in the
//				header with attachments.h
//              
//----------------------------------------------------------------------------
CAIPlugin::CAIPlugin()
{
	m_pAttachmentsPlugin = debug_new( CAttachmentsPlugin );
}
CAIPlugin::~CAIPlugin()
{
	if ( m_pAttachmentsPlugin )
	{
		debug_delete( m_pAttachmentsPlugin );
		m_pAttachmentsPlugin = NULL;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CAIPlugin::PreHook_EditStringList
//
//	PURPOSE:	Requests a state change
//
// ----------------------------------------------------------------------- //

LTRESULT CAIPlugin::PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
	static bool bPluginInitted = false;

	if ( !bPluginInitted )
	{
		// Make sure the weaponmgr plugin is inited.
		g_pWeaponDBPlugin->PreHook_EditStringList(szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength);

		bPluginInitted = true;
	}


	// See if the attachments plugin will handle the property
	if ( LT_OK == m_pAttachmentsPlugin->PreHook_EditStringList(szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength) )
	{
		return LT_OK;
	}

	// Let the Arsenal plugin have a go at it...

	if( m_ArsenalPlugin.PreHook_EditStringList( szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength ) == LT_OK )
	{
		return LT_OK;
	}

	// Let the AIConfig plugin have a go at it...

	if( m_AIConfigPlugin.PreHook_EditStringList( szRezPath, szPropName, aszStrings, pcStrings, cMaxStrings, cMaxStringLength ) == LT_OK )
	{
		return LT_OK;
	}

	if ( !LTStrICmp("GoalSet", szPropName) )
	{
		// TODO: make sure we don't overflow cMaxStringLength or cMaxStrings
		uint32 cGoalSets = g_pAIDB->GetNumAIGoalSetRecords();
		AIDB_GoalSetRecord* pGoalSet;
		for ( uint32 iGoalSet = 0 ; iGoalSet < cGoalSets ; iGoalSet++ )
		{
			pGoalSet = g_pAIDB->GetAIGoalSetRecord(iGoalSet);
			if( pGoalSet && !( pGoalSet->dwGoalSetFlags & AIDB_GoalSetRecord::kGS_Hidden ) )
			{
				LTStrCpy(aszStrings[(*pcStrings)++], pGoalSet->strName.c_str(), cMaxStringLength);
			}
		}

		return LT_OK;
	}
	else if ( !LTStrICmp("ModelTemplate", szPropName) )
	{
		uint32 cModels = g_pModelsDB->GetNumModels();
		for ( uint32 iModel = 0 ; iModel < cModels ; iModel++ )
		{
			ModelsDB::HMODEL hModel = g_pModelsDB->GetModel( iModel );
			LTStrCpy(aszStrings[(*pcStrings)++], g_pModelsDB->GetRecordName(hModel), cMaxStringLength);
		}

		return LT_OK;
	}
	else if ( !LTStrICmp("AmmoLoad", szPropName))
	{
		uint32 nAmmoLoads = g_pAIDB->GetNumAIAmmoLoadRecords();
		for (uint32 iAmmoLoad = 0; iAmmoLoad < nAmmoLoads; ++iAmmoLoad)
		{
			AIDB_AIAmmoLoadRecord* pRecord = g_pAIDB->GetAIAmmoLoadRecord((ENUM_AIAmmoLoadRecordID)iAmmoLoad);
			if (pRecord && (*pcStrings+1) < cMaxStrings)
			{
				LTStrCpy(aszStrings[(*pcStrings)++], pRecord->strName.c_str(), cMaxStringLength);
			}
		}
	
		// Add an empty string if no ammo loads, so that the level error 
		// detector does not complain about an invalid string.

		if( nAmmoLoads == 0 )
		{
			LTStrCpy(aszStrings[(*pcStrings)++], "", cMaxStringLength);
		}

		return LT_OK;
	}
	else if ( !LTStrICmp("Brain", szPropName) )
	{
		// TODO: make sure we don't overflow cMaxStringLength or cMaxStrings
		uint32 cBrains = g_pAIDB->GetNumAIBrainRecords();
		for ( uint32 iBrain = 0 ; iBrain < cBrains ; iBrain++ )
		{
			LTStrCpy(aszStrings[(*pcStrings)++], g_pAIDB->GetAIBrainRecord(iBrain)->strName.c_str(), cMaxStringLength);
		}

		return LT_OK;
	}

	// No one wants it

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CAIPlugin::PreHook_PropChanged
//
//  PURPOSE:	Check our command strings
//
// ----------------------------------------------------------------------- //

LTRESULT CAIPlugin::PreHook_PropChanged( const char *szObjName,
											 const char *szPropName, 
											 const int  nPropType, 
											 const GenericProp &gpPropValue,
											 ILTPreInterface *pInterface,
											 const char *szModifiers )
{
	if( m_AIConfigPlugin.PreHook_PropChanged(szObjName,
											 szPropName, 
											 nPropType, 
											 gpPropValue,
											 pInterface,
											 szModifiers ) == LT_OK )
	{
		return LT_OK;
	}

	if( ( !LTStrICmp( szPropName, "Initial" ) 
		|| !LTStrICmp( szPropName, "ActivateOn" ) 
		|| !LTStrICmp( szPropName, "ActivateOff" ) ) 
		&& ( gpPropValue.GetCommand()[0] ) )
	{
		ConParse cpCmd;
		cpCmd.Init( gpPropValue.GetCommand() );

		while( LT_OK == pInterface->Parse( &cpCmd ))
		{
			if( cpCmd.m_nArgs < 1 )
				continue;

			if( m_CommandMgrPlugin.CommandExists( cpCmd.m_Args[0] ))
			{
				if( LT_OK != m_CommandMgrPlugin.PreHook_PropChanged( szObjName,
																	 szPropName,
																	 nPropType, 
																	 gpPropValue,
																	 pInterface,
																	 szModifiers ))
				{
					return LT_UNSUPPORTED;
				}
			}
			else
			{
				// Since we can send commands to AIs without using the command syntax, 
				// build the command like we were using propper syntax and and try to validate it...

				std::string sCmd = "";
				for( int i = 0; i < cpCmd.m_nArgs; ++i )
				{
					sCmd += cpCmd.m_Args[i];
					sCmd += " ";
				}

				std::string sFinalCommand = "msg <CAI> (";
				sFinalCommand += sCmd;
				sFinalCommand += ')';

				GenericProp gp(sFinalCommand.c_str(), LT_PT_COMMAND);

				if( LT_OK != m_CommandMgrPlugin.PreHook_PropChanged( szObjName,
																	 szPropName,
																	 nPropType, 
																	 gp,
																	 pInterface,
																	 szModifiers ))
				{
					return LT_UNSUPPORTED;
				}
			}
		}

		return LT_OK;
	}
	else if( m_CommandMgrPlugin.PreHook_PropChanged( szObjName, 
													 szPropName, 
													 nPropType, 
													 gpPropValue,
													 pInterface,
													 szModifiers ) == LT_OK )
	{
		return LT_OK;
	}

	return LT_UNSUPPORTED;
}

//-----------------------------------------------------------------------------
// Prefetching
//-----------------------------------------------------------------------------

void CAI::GetPrefetchResourceList(const char* pszObjectName, IObjectResourceGatherer* pInterface, ResourceList& Resources )
{
	char pszModelTemplate[256] = {'\0'};

	// Get AI's type
	pInterface->GetPropString(pszObjectName, "ModelTemplate", pszModelTemplate, LTARRAYSIZE(pszModelTemplate), "");
	if (LTStrEmpty(pszModelTemplate))
		return;

	// main model
	ModelsDB::HMODEL hModel = g_pModelsDB->GetModelByRecordName( pszModelTemplate );
	GetRecordResources(Resources, hModel, true);

	// child models
	for (uint32 nChildModel = 0 ; nChildModel < MAX_DEDIT_CHILD_MODELS ; ++nChildModel )
	{
		char szPropName[MAX_PATH];
		LTSNPrintF(szPropName, LTARRAYSIZE(szPropName), "ChildModel_%d", nChildModel + 1);

		char szChildModel[MAX_PATH];
		pInterface->GetPropString(pszObjectName, szPropName, szChildModel, LTARRAYSIZE(szChildModel), "");

		if (!LTStrEmpty(szChildModel))
		{
			// add it to the list of resources
			Resources.push_back(szChildModel);
		}
	}

	// arsenal
	for (uint32 nArsenalWeapon = 0; nArsenalWeapon < DEFAULT_ACTIVE_WEAPONS; ++nArsenalWeapon)
	{
		char szPropName[MAX_PATH];
		LTSNPrintF(szPropName, LTARRAYSIZE(szPropName), "Weapon%d", nArsenalWeapon);

		char szWeapon[MAX_PATH];
		pInterface->GetPropString(pszObjectName, szPropName, szWeapon, LTARRAYSIZE(szWeapon), "");

		if (!LTStrEmpty(szWeapon) && !LTStrIEquals(szWeapon, "<none>"))
		{
			HWEAPON hWeapon = g_pWeaponDB->GetWeaponRecord(szWeapon);	
			GetRecordResources(Resources, hWeapon, true);
		}
	}

	// sound template
	HRECORD hSoundTemplate = g_pAISoundDB->GetRecordLink(hModel,"SoundTemplate");
	if (hSoundTemplate)
	{
		// loop through all the AI sounds types in the sound template
		for (uint32 nSoundType = kAIS_InvalidType + 1; nSoundType < kAIS_Count; ++nSoundType)
		{
			// for each sound type, there is a list of sound lists
			HATTRIBUTE hSoundTypeAttribute = g_pAISoundDB->GetAttribute(hSoundTemplate, s_aszAISoundTypes[nSoundType]);
			if (hSoundTypeAttribute)
			{
				// loop through each sound list
				uint32 nSoundSetCount = g_pAISoundDB->GetNumValues(hSoundTypeAttribute);
				for(uint32 nSoundSet = 0; nSoundSet < nSoundSetCount; ++nSoundSet)
				{
					// prefetch a single sound list
					GetLocalizedSoundListResources(Resources, g_pAISoundDB->GetRecordLink(hSoundTypeAttribute, nSoundSet));
				}
			}
		}
	}
}

