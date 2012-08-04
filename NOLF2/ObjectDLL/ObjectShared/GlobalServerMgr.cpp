// ----------------------------------------------------------------------- //
//
// MODULE  : GlobalServerMgr.cpp
//
// PURPOSE : Implementations of server global definitions
//
// CREATED : 7/07/99
//
// (c) 1999-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "GlobalServerMgr.h"
#include "ServerSoundMgr.h"
#include "AIButeMgr.h"
#include "AIGoalButeMgr.h"
#include "AttachButeMgr.h"
#include "AnimationMgr.h"
#include "PropTypeMgr.h"
#include "IntelMgr.h"
#include "KeyMgr.h"
#include "SearchItemMgr.h"
#include "GadgetTargetMgr.h"
#include "CommandButeMgr.h"
#include "RelationMgr.h"
#include "InventoryButeMgr.h"
#include "TransitionMgr.h"
#include "RadarTypeMgr.h"
#include "ActivateTypeMgr.h"
#include "TriggerTypeMgr.h"

// These includes necessary to establish correct linkage.  They rely on the header
// using a LINKTO_MODULE macro, which places a global variable into the cpp file.  This
// variable then references a global variable defined in the cpp associated with the
// header.  The other cpp must have a LINKFROM_MODULE macro in it to define the referenced
// global variable.  This is needed because the BEGIN_CLASS/END_CLASS engine macros
// create global variables that rely on the constructors to get called.  If the global
// variable is inside a static lib and isn't referenced outside the module, the module's
// obj want be part of the link with the dll.  Each new engine based object that must
// use the LINKTO_MODULE/LINKFROM_MODULE pair and put the header in this file.
#include "AI.h"
#include "AIHuman.h"
#include "AINode.h"
#include "AINodeGuard.h"
#include "AINodeSensing.h"
#include "AINodeDisturbance.h"
#include "AIRegion.h"
#include "AIVolume.h"
#include "ActiveWorldModel.h"
#include "Alarm.h"
#include "AmmoBox.h"
#include "Body.h"
#include "Bombable.h"
#include "Breakable.h"
#include "Camera.h"
#include "Character.h"
#include "CharacterHitBox.h"
#include "ClientLightFX.h"
#include "ClientSFX.h"
#include "CommandObject.h"
#include "Controller.h"
#include "DebugLineSystem.h"
#include "DecisionObject.h"
#include "Dialogue.h"
#include "DisplayMeter.h"
#include "DisplayTimer.h"
#include "DoomsdayDevice.h"
#include "DoomsdayPiece.h"
#include "Door.h"
#include "DoorKnob.h"
#include "DynamicOccluderVolume.h"
#include "EventCounter.h"
#include "ExitTrigger.h"
#include "Explosion.h"
#include "Fire.h"
#include "GadgetTarget.h"
#include "GameBase.h"
#include "GameStartPoint.h"
#include "GearItems.h"
#include "Group.h"
#include "HHWeaponModel.h"
#include "Intelligence.h"
#include "JumpVolume.h"
#include "Key.h"
#include "KeyFramer.h"
#include "KeyItem.h"
#include "KeyPad.h"
#include "LaserTrigger.h"
#include "LightGroup.h"
#include "Lightning.h"
#include "Lock.h"
#include "Mine.h"
#include "ModItem.h"
#include "NodeLine.h"
#include "NoPlayerTrigger.h"
#include "ObjectRemover.h"
#include "ObjectiveSprite.h"
#include "ParticleSystem.h"
#include "PickupItem.h"
#include "PlayerLure.h"
#include "PlayerObj.h"
#include "PlayerTrigger.h"
#include "PlayerVehicle.h"
#include "Point.h"
#include "PolyGrid.h"
#include "Projectile.h"
#include "ProjectileTypes.h"
#include "Prop.h"
#include "PropType.h"
#include "RadarObject.h"
#include "RandomSpawner.h"
#include "RotatingDoor.h"
#include "RotatingSwitch.h"
#include "RotatingWorldModel.h"
#include "ScaleSprite.h"
#include "Scanner.h"
#include "ScreenShake.h"
#include "SearchLight.h"
#include "SearchProp.h"
#include "SecurityCamera.h"
#include "ServerMark.h"
#include "ServerSoundFX.h"
#include "ServerSpecialFX.h"
#include "SlidingDoor.h"
#include "SlidingSwitch.h"
#include "SlidingWorldModel.h"
#include "SnowVolume.h"
#include "ScatterVolume.h"
#include "SoundButeFX.h"
#include "Spawner.h"
#include "Speaker.h"
#include "SpinningWorldModel.h"
#include "Sprinkles.h"
#include "StartupCommand.h"
#include "Steam.h"
#include "Switch.h"
#include "TeleportPoint.h"
#include "TextureFX.h"
#include "TransitionArea.h"
#include "Trigger.h"
#include "VolumeBrush.h"
#include "VolumeBrushTypes.h"
#include "VolumeEffect.h"
#include "WeaponItems.h"
#include "WorldModel.h"
#include "WorldModelDebris.h"
#include "WorldProperties.h"
#include "PowerArmor.h"

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
	m_pAIButeMgr = NULL;		
	m_pAIGoalButeMgr = NULL;	
	m_pAttachButeMgr = NULL;	
	m_pAnimationMgrList = NULL;
	m_pPropTypeMgr = NULL;		
	m_pIntelMgr = NULL;		
	m_pKeyMgr = NULL;			
	m_pSearchItemMgr = NULL;			
	m_pGadgetTargetMgr = NULL;
	m_pCommandButeMgr = NULL;	
	m_pServerButeMgr = NULL;
	m_pInventoryButeMgr = NULL;
	m_pTransitionMgr = NULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CGlobalServerMgr::Init()
//
//	PURPOSE:	Initialize
//
// ----------------------------------------------------------------------- //

LTBOOL CGlobalServerMgr::Init()
{
	if( !g_pGameServerShell )
		return LTFALSE;

	m_pServerSoundMgr = new CServerSoundMgr;
	m_pServerSoundMgr->Init();

    // Since Client & Server are the same on PS2, we only need to do this once (when called by the client)
    if (!CGlobalMgr::Init()) 
		return LTFALSE;

    m_pAIButeMgr = new CAIButeMgr;
	m_pAIButeMgr->Init();

    m_pAIGoalButeMgr = new CAIGoalButeMgr;
	m_pAIGoalButeMgr->Init();

    m_pAttachButeMgr = new CAttachButeMgr;
	m_pAttachButeMgr->Init();

    m_pServerButeMgr = new CServerButeMgr;
	m_pServerButeMgr->Init();

    m_pAnimationMgrList = new CAnimationMgrList;
	m_pAnimationMgrList->Init();

	m_pPropTypeMgr = new CPropTypeMgr;
	m_pPropTypeMgr->Init();

	m_pIntelMgr = new CIntelMgr;
	m_pIntelMgr->Init();

	m_pKeyMgr = new CKeyMgr;
	m_pKeyMgr->Init();
    	
	m_pSearchItemMgr = new CSearchItemMgr;
	m_pSearchItemMgr->Init();
    	

        CRelationMgr::GetGlobalRelationMgr()->Init();

	m_pGadgetTargetMgr = new CGadgetTargetMgr;
	m_pGadgetTargetMgr->Init();

	m_pCommandButeMgr = new CCommandButeMgr;
	m_pCommandButeMgr->Init();

	m_pInventoryButeMgr = new CInventoryButeMgr;
	m_pInventoryButeMgr->Init();

	m_pTransitionMgr = debug_new( CTransitionMgr );
	if( !m_pTransitionMgr )
		return LTFALSE;

	
	if( g_pGameServerShell->ShouldUseRadar() )
	{
		// Get the singleton instance of the radar type mgr and initialize it...
		if( !g_pRadarTypeMgr )
		{
			CRadarTypeMgr &RadarTypeMgr = CRadarTypeMgr::Instance();
			if( !RadarTypeMgr.Init() )
			{
				ShutdownWithError( "RadarTypeMgr", RTMGR_DEFAULT_FILE );
				return LTFALSE;
			}
		}
	}
	
	if( !g_pActivateTypeMgr )
	{
		CActivateTypeMgr &ActivateTypeMgr = CActivateTypeMgr::Instance();
		ActivateTypeMgr.Init();
	}
	
	if( !g_pTriggerTypeMgr )
	{
		CTriggerTypeMgr & TriggerTypeMgr = CTriggerTypeMgr::Instance();
		TriggerTypeMgr.Init();
	}

    return LTTRUE;
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

	if( m_pAIButeMgr )
	{
		delete m_pAIButeMgr;
		m_pAIButeMgr = NULL;
	}

	if( m_pAIGoalButeMgr )
	{
		delete m_pAIGoalButeMgr;
		m_pAIGoalButeMgr = NULL;
	}

	if( m_pAttachButeMgr )
	{
		delete m_pAttachButeMgr;
		m_pAttachButeMgr = NULL;
	}

	if( m_pServerButeMgr )
	{
		delete m_pServerButeMgr;
		m_pServerButeMgr = NULL;
	}

	if( m_pAnimationMgrList )
	{
		delete m_pAnimationMgrList;
		m_pAnimationMgrList = NULL;
	}

	if( m_pPropTypeMgr )
	{
		delete m_pPropTypeMgr;
		m_pPropTypeMgr = NULL;
	}

	if( m_pIntelMgr )
	{
		delete m_pIntelMgr;
		m_pIntelMgr = NULL;
	}

	if( m_pKeyMgr )
	{
		delete m_pKeyMgr;
		m_pKeyMgr = NULL;
	}

	if( m_pSearchItemMgr )
	{
		delete m_pSearchItemMgr;
		m_pSearchItemMgr = NULL;
	}

	if( m_pGadgetTargetMgr )
	{
		delete m_pGadgetTargetMgr;
		m_pGadgetTargetMgr = NULL;
	}

	CRelationMgr::GetGlobalRelationMgr()->Term();

	if( m_pCommandButeMgr )
	{
		delete m_pCommandButeMgr;
		m_pCommandButeMgr = NULL;
	}

	if( m_pInventoryButeMgr )
	{
		delete m_pInventoryButeMgr;
		m_pInventoryButeMgr = NULL;
	}

	if( m_pTransitionMgr )
	{
		debug_delete( m_pTransitionMgr );
		m_pTransitionMgr = NULL;
	}

	if( g_pRadarTypeMgr )
	{
		g_pRadarTypeMgr->Term();
	}

	if( g_pActivateTypeMgr )
	{
		g_pActivateTypeMgr->Term();
	}

	if( g_pTriggerTypeMgr )
	{
		g_pTriggerTypeMgr->Term();
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

void CGlobalServerMgr::ShutdownWithError(char* pMgrName, char* pButeFilePath)
{
	char errorBuf[256];
	sprintf(errorBuf, "ERROR in CGlobalServerMgr::Init()\n\nCouldn't initialize %s.  Make sure the %s file is valid!", pMgrName, pButeFilePath);
    g_pLTServer->CPrint(errorBuf);

	// TO DO:
	// Send a message to all clients to shut down (also send error string!!!)
	//
}
