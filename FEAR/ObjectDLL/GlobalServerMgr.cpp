// ----------------------------------------------------------------------- //
//
// MODULE  : GlobalServerMgr.cpp
//
// PURPOSE : Implementations of server global definitions
//
// CREATED : 7/07/99
//
// (c) 1999-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "Stdafx.h"
#include "GlobalServerMgr.h"
#include "ServerSoundMgr.h"
#include "AIDB.h"
#include "CharacterDB.h"
#include "AnimationContext.h"
#include "CommandDB.h"
#include "ServerDB.h"
#include "TransitionMgr.h"

// These includes necessary to establish correct linkage.  They rely on the header
// using a LINKTO_MODULE macro, which places a global variable into the cpp file.  This
// variable then references a global variable defined in the cpp associated with the
// header.  The other cpp must have a LINKFROM_MODULE macro in it to define the referenced
// global variable.  This is needed because the BEGIN_CLASS/END_CLASS engine macros
// create global variables that rely on the constructors to get called.  If the global
// variable is inside a static lib and isn't referenced outside the module, the module's
// obj want be part of the link with the dll.  Each new engine based object that must
// use the LINKTO_MODULE/LINKFROM_MODULE pair and put the header in this file.
#include "ActiveWorldModel.h"
#include "AI.h"
#include "AINavMesh.h"
#include "AINavMeshLink.h"
#include "AINavMeshLinkAbstract.h"
#include "AINavMeshLinkAbstractOneAnim.h"
#include "AINavMeshLinkClimb.h"
#include "AINavMeshLinkCrawl.h"
#include "AINavMeshLinkDiveThru.h"
#include "AINavMeshLinkDoor.h"
#include "AINavMeshLinkDuckUnder.h"
#include "AINavMeshLinkFlyThru.h"
#include "AINavMeshLinkJump.h"
#include "AINavMeshLinkJumpOver.h"
#include "AINavMeshLinkLoseTarget.h"
#include "AINavMeshLinkPlayer.h"
#include "AINavMeshLinkStairs.h"
#include "AINode.h"
#include "AINodeDarkChant.h"
#include "AINodeDarkWait.h"
#include "AINodeGuard.h"
#include "AINodeHide.h"
#include "AINodeSafety.h"
#include "AINodeSafetyFirePosition.h"
#include "AINodeStalk.h"
#include "AIRegion.h"
#include "AISoundDB.h"
#include "AISpawner.h"
#include "Alarm.h"
#include "AmmoBox.h"
#include "AutoAimMagnet.h"
#include "Camera.h"
#include "Character.h"
#include "CharacterHitBox.h"
#include "CommandObject.h"
#include "ConstraintBase.h"
#include "ConstraintHinge.h"
#include "ConstraintLimitedHinge.h"
#include "ConstraintPoint.h"
#include "ConstraintPrismatic.h"
#include "ConstraintRagdoll.h"
#include "ConstraintStiffSpring.h"
#include "ConstraintWheel.h"
#include "Controller.h"
#include "DebugLineSystem.h"
#include "DecisionObject.h"
#include "Dialogue.h"
#include "DisplayMeter.h"
#include "DisplayTimer.h"
#include "Door.h"
#include "DynamicSectorVolume.h"
#include "EventCounter.h"
#include "Explosion.h"
#include "GameBase.h"
#include "GameStartPoint.h"
#include "GearItems.h"
#include "Group.h"
#include "GunMount.h"
#include "HHWeaponModel.h"
#include "Flicker.h"
#include "ForensicObject.h"
#include "JumpVolume.h"
#include "Key.h"
#include "KeyFramer.h"
#include "Ladder.h"
#include "LightBase.h"
#include "LightCube.h"
#include "LightDirectional.h"
#include "LightPoint.h"
#include "LightPointFill.h"
#include "LightSpot.h"
#include "LookAtTrigger.h"
#include "NavMarker.h"
#include "NoPlayerTrigger.h"
#include "ObjectRemover.h"
#include "PhysicsCollisionSystem.h"
#include "PhysicsImpulseDirectional.h"
#include "PhysicsImpulseRadial.h"
#include "PickupItem.h"
#include "PlayerLure.h"
#include "PlayerLeash.h"
#include "PlayerNodeGoto.h"
#include "PlayerObj.h"
#include "PlayerTrigger.h"
#include "Point.h"
#include "PolyGrid.h"
#include "PositionalDamage.h"
#include "Projectile.h"
#include "ProjectileTypes.h"
#include "Prop.h"
#include "RefillStation.h"
#include "RemoteTurret.h"
#include "RenderTarget.h"
#include "RenderTargetGroup.h"
#include "RotatingDoor.h"
#include "RotatingSwitch.h"
#include "RotatingWorldModel.h"
#include "ScatterVolume.h"
#include "ScreenEffect.h"
#include "ServerNonPointSound.h"
#include "ServerSoundFX.h"
#include "ServerSoundZoneVolume.h"
#include "ServerSpecialFX.h"
#include "ScreenEffect.h"
#include "SlidingDoor.h"
#include "SlidingSwitch.h"
#include "SlidingWorldModel.h"
#include "SnowVolume.h"
#include "SoundButeFX.h"
#include "Spawner.h"
#include "Speaker.h"
#include "SpecialMove.h"
#include "SpinningWorldModel.h"
#include "StairVolume.h"
#include "StartupCommand.h"
#include "Switch.h"
#include "TeleportPoint.h"
#include "TransitionArea.h"
#include "Trigger.h"
#include "Turret.h"
#include "VolumeBrush.h"
#include "VolumeBrushTypes.h"
#include "VolumeEffect.h"
#include "WeaponItems.h"
#include "WorldModel.h"
#include "WorldProperties.h"

// Note : DEditHook must be referenced explicitly, since it doesn't have an associated header files
LINKTO_MODULE( DEditHook );

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGlobalServerMgr::CGlobalServerMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CGlobalServerMgr::CGlobalServerMgr()
{
	m_pServerSoundMgr = NULL;	
	m_pTransitionMgr = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGlobalServerMgr::Init()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

bool CGlobalServerMgr::Init()
{
	if( !g_pGameServerShell )
		return false;

	m_pServerSoundMgr = debug_new(CServerSoundMgr);
	m_pServerSoundMgr->Init();

	CErrorHandler cError;
    if (!CGlobalMgr::Init(cError)) 
		return false;

	if (!CServerDB::Instance().Init())
	{
		cError.ShutdownWithError( "ServerDB", DB_Default_File );
		return false;
	}

	if( !g_pAIDB )
	{
		CAIDB &AIDB = CAIDB::Instance();
		if( !AIDB.Init() )
		{
			cError.ShutdownWithError( "AIDB", DB_Default_File );
			return false;
		}
	}

	if (!CAISoundDB::Instance().Init(DB_Default_File))
	{
		cError.ShutdownWithError( "AISoundDB", DB_Default_File );
		return false;
	}

	if( !g_pCharacterDB )
	{
		CCharacterDB &CharacterDB = CCharacterDB::Instance();
		if( !CharacterDB.Init() )
		{
			cError.ShutdownWithError( "CharacterDB", DB_Default_File );
			return false;
		}
	}

	if (!CCommandDB::Instance().Init())
	{
		cError.ShutdownWithError( "CommandDB", DB_Default_File );
		return false;
	}


	m_pTransitionMgr = debug_new( CTransitionMgr );
	if( !m_pTransitionMgr )
		return false;

	return true;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGlobalServerMgr::Term()
//
//	PURPOSE:	Terminator
//
// ----------------------------------------------------------------------- //

void CGlobalServerMgr::Term()
{
	if( m_pServerSoundMgr )
	{
		delete m_pServerSoundMgr;
		m_pServerSoundMgr = NULL;
	}

	if( g_pAIDB )
	{
		g_pAIDB->Term();
	}

	if( g_pCharacterDB )
	{
		g_pCharacterDB->Term();
	}


	if( m_pTransitionMgr )
	{
		debug_delete( m_pTransitionMgr );
		m_pTransitionMgr = NULL;
	}

	CGlobalMgr::Term( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGlobalServerMgr::ShutdownWithError()
//
//	PURPOSE:	Shutdown all clients with an error
//
// ----------------------------------------------------------------------- //

void CGlobalServerMgr::CErrorHandler::ShutdownWithError(char* pMgrName, const char* pButeFilePath) const
{
	char errorBuf[256];
	LTSNPrintF( errorBuf, LTARRAYSIZE( errorBuf ), "ERROR in CGlobalServerMgr::Init()\n\nCouldn't initialize %s.  Make sure the %s file is valid!", pMgrName, pButeFilePath);
    g_pLTServer->CPrint(errorBuf);

	// TO DO:
	// Send a message to all clients to shut down (also send error string!!!)
	//
}
