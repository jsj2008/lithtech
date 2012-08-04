// ----------------------------------------------------------------------- //
//
// MODULE  : PlayerBodyMgr.h
//
// PURPOSE : PlayerBodyMgr implementation - Manages the full-body player view
//
// CREATED : 10/06/03
//
// (c) 2003-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

//
// Includes...
//

#include "stdafx.h"
#include "PlayerBodyMgr.h"
#include "CMoveMgr.h"
#include "CharacterFX.h"
#include "ltbasedefs.h"
#include "ClientWeaponMgr.h"
#include "PlayerCamera.h"
#include "LTEulerAngles.h"
#include "objectdetector.h"
#include "AimMgr.h"
#include "LadderMgr.h"
#include "PhysicsUtilities.h"
#include "bindmgr.h"
#include "commandids.h"
#include "LadderFX.h"
#include "SpecialMoveFX.h"
#include "ProjectileFX.h"
#include "LeanMgr.h"
#include "TargetMgr.h"
#include "ClientMeleeCollisionController.h"
#include "ForensicObjectFX.h"
#include "EntryToolLockFX.h"
#include "ClientDB.h"


extern StopWatchTimer TestFireTimer;
extern double fLastTestFireTime;
extern uint32 nTestGrenadesThrown;
extern bool g_bInfiniteAmmo;

//
// Defines...
//
	
	#define	PLAYER_BODY_WEAPON			"Unarmed"

	#define	PLAYER_BODY_GRENADE			"Frag Grenade"

	#define	PLAYER_BODY_KEY_MELEE		"FIRE"
	#define	PLAYER_BODY_KEY_GRENADE		"GRENADE"
	#define PLAYER_BODY_KEY_FIREGRENADE	"FIREGRENADE"
	#define PLAYER_BODY_KEY_KILLPROJECTILES	"KILLPROJECTILES"
	#define	PLAYER_BODY_KEY_THROW		"THROW"
	#define PLAYER_BODY_KEY_SOUND_BUTE	"BUTE_SOUND_KEY"
	#define PLAYER_BODY_KEY_SONIC		"SONIC"
	#define PLAYER_BODY_KEY_ACTIVATE	"ACTIVATE"
	#define PLAYER_BODY_KEY_BLOCKWINDOW "BLOCKWINDOW"
	#define PLAYER_BODY_KEY_DODGEWINDOW "DODGEWINDOW"
	#define PLAYER_BODY_KEY_MELEECONTROL	"MELEE"
	#define PLAYER_BODY_KEY_BERSERKERKICK	"BERSERKERKICK"
	#define PLAYER_BODY_KEY_FINISHINGMOVE	"FINISHINGMOVE"
	#define PLAYER_BODY_KEY_CUSTOM_WEAPON_SELECT	"CUSTOMWEAPONSELECT"
	#define PLAYER_BODY_KEY_CLOSE_ENCOUNTER	"CLOSE_ENCOUNTER"
	#define PLAYER_BODY_KEY_FINISH			"FINISH"

	#define PLAYER_SHADOW_LOD			eEngineLOD_Medium

#ifdef PROJECT_DARK
	#define PLAYER_DEFAULT_STAND_DIMS	LTVector( 40.0f, 95.0f, 40.0f )
	#define PLAYER_DEFAULT_CROUCH_DIMS	LTVector( 40.0f, 61.0f, 40.0f )
#else
	#define PLAYER_DEFAULT_STAND_DIMS	LTVector( 40.0f, 90.0f, 40.0f )
	#define PLAYER_DEFAULT_CROUCH_DIMS	LTVector( 40.0f, 61.0f, 40.0f )
#endif


VarTrack g_vtPlayerBodyFireMode;
VarTrack g_vtPlayerBodyShowInfo;
VarTrack g_vtPlayerBodyShowDimsInfo;
VarTrack g_vtPlayerBodyLookLow;
VarTrack g_vtPlayerBodyLookHigh;
VarTrack g_vtPlayerBodySendAnimTimeRate;
VarTrack g_vtPlayerBodyShuffle;
VarTrack g_vtPlayerBodyWeapons;

VarTrack g_vtPlayerBodyAdjustAnimRate;

extern ObjectDetector g_iFocusObjectDetector;

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::CPlayerBodyMgr
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CPlayerBodyMgr::CPlayerBodyMgr( )
:	
	m_pMainAnimationContext		( NULL ),
	m_pLowerAnimationContext	( NULL ),
	m_pUpperAnimationContext	( NULL ),
	m_pCustomAnimationContext	( NULL ),
	m_hPlayerBody				( NULL ),
	m_pMeleeWeapon				( NULL ),
	m_pGrenadeWeapon			( NULL ),
	m_bMainClearCachedAni		( false ),
	m_bLowerClearCachedAni		( false ),
	m_bUpperClearCachedAni		( false ),
	m_bCustomClearCachedAni		( false ),
	m_bPlayingFullBody			( true ),
	m_bPlayingCustom			( false ),
	m_hModel					( NULL ),
	m_dwLastDimsAnim			( INVALID_MODEL_ANIM ),
	m_dwLastWeaponContextAnim	( INVALID_MODEL_ANIM ),
	m_vWantedDims				( 1.0f, 1.0f, 1.0f ),
	m_NodeTrackerContext		( ),
	m_AnimProps					( ),
	m_UpperAnimProps			( ),
	m_LowerAnimProps			( ),
	m_CustomAnimProps			( ),
	m_LastAnimProps				( ),
	m_MainLockedAnimProps		( ),
	m_UpperLockedAnimProps		( ),
	m_LowerLockedAnimProps		( ),
	m_CustomLockedAnimProps		( ),
	m_fSendAnimTime				( 0.0f ),
	m_bForceSendAnimTime		( false ),
	m_eMovementDescriptor		( kAD_None ),
	m_eCameraDescriptor			( kAD_None ),
	m_eInputDescriptor			( kAD_None ),
	m_bEnabled					( false ),
	m_bStartPlayerBody			( false ),
	m_bPlayingSpecial			( false ),
	m_bLingerSpecial			( false ),
	m_bInCloseEncounter			( false )
{
	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::~CPlayerBodyMgr
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CPlayerBodyMgr::~CPlayerBodyMgr( )
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::Term
//
//	PURPOSE:	Handle releasing the player body object and animation contexts...
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::Term()
{
	// Reset conditional animation controllers...
	CClientWeapon* pWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
	if (pWeapon)
	{
		pWeapon->ResetAnimControllers();
	}

	if( m_pMainAnimationContext )
	{
		debug_delete( m_pMainAnimationContext );
		m_pMainAnimationContext = NULL;
	}

	if( m_pLowerAnimationContext )
	{
		debug_delete( m_pLowerAnimationContext );
		m_pLowerAnimationContext = NULL;
	}

	if( m_pUpperAnimationContext )
	{
		debug_delete( m_pUpperAnimationContext );
		m_pUpperAnimationContext = NULL;
	}

	if( m_pCustomAnimationContext )
	{
		debug_delete( m_pCustomAnimationContext );
		m_pCustomAnimationContext = NULL;
	}

	if( m_pMeleeWeapon )
	{
		debug_delete( m_pMeleeWeapon );
		m_pMeleeWeapon = NULL;
	}
	if( m_pGrenadeWeapon )
	{
		debug_delete( m_pGrenadeWeapon );
		m_pGrenadeWeapon = NULL;
	}

	if( m_hPlayerBody )
	{
		g_pLTClient->RemoveObject( m_hPlayerBody );
		m_hPlayerBody = NULL;
	}

	m_hModel = NULL;

	m_bStartPlayerBody	= false;
	m_bEnabled			= false;
	m_bPlayingSpecial	= false;

	m_dwLastDimsAnim	= INVALID_MODEL_ANIM;
	m_vWantedDims		= LTVector::GetIdentity( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::Init
//
//	PURPOSE:	Initialize the player body
//
// ----------------------------------------------------------------------- //

bool CPlayerBodyMgr::Init()
{
	if( !g_vtPlayerBodyFireMode.IsInitted() )
	{
		g_vtPlayerBodyFireMode.Init( g_pLTClient, "PlayerBodyFireMode", NULL, 0.0f );
	}

	if( !g_vtPlayerBodyShowInfo.IsInitted() )
	{
		g_vtPlayerBodyShowInfo.Init( g_pLTClient, "PlayerBodyShowInfo", NULL, 0.0f );
	}

	if( !g_vtPlayerBodyShowDimsInfo.IsInitted( ))
	{
		g_vtPlayerBodyShowDimsInfo.Init( g_pLTClient, "PlayerBodyShowDimsInfo", NULL, 1.0f );
	}

	if( !g_vtPlayerBodyLookLow.IsInitted() )
	{
		g_vtPlayerBodyLookLow.Init( g_pLTClient, "PlayerBodyLookLow", NULL, 0.8f );
	}

	if( !g_vtPlayerBodyLookHigh.IsInitted() )
	{
		g_vtPlayerBodyLookHigh.Init( g_pLTClient, "PlayerBodyLookHigh", NULL, 0.05f );
	}

	if( !g_vtPlayerBodySendAnimTimeRate.IsInitted() )
	{
		g_vtPlayerBodySendAnimTimeRate.Init( g_pLTClient, "PlayerBodySendAnimTimeRate", NULL, 0.5f );
	}

	if( !g_vtPlayerBodyShuffle.IsInitted() )
	{
		g_vtPlayerBodyShuffle.Init( g_pLTClient, "PlayerBodyShuffle", NULL, 0.0f );
	}

	if( !g_vtPlayerBodyWeapons.IsInitted() )
	{
		g_vtPlayerBodyWeapons.Init( g_pLTClient, "PlayerBodyWeapons", NULL, 1.0f );
	}
	
	if( !g_vtPlayerBodyAdjustAnimRate.IsInitted() )
	{
		g_vtPlayerBodyAdjustAnimRate.Init( g_pLTClient, "PlayerBodyAdjustAnimRate", NULL, 1.0f );
	}

	if( !InitPlayerBody() )
		return false;

	if( g_vtPlayerBodyWeapons.GetFloat() >= 1.0f )
		Enable();

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::InitPlayerBody
//
//	PURPOSE:	Create the client player body model...
//
// ----------------------------------------------------------------------- //

bool CPlayerBodyMgr::InitPlayerBody()
{
	if( m_hPlayerBody )
	{
		LTERROR( "CPlayerBodyMgr::InitPlayerBody - Player Body model already created!" );
		return true;
	}
	
	ObjectCreateStruct ocs;
	
	ocs.m_ObjectType = OT_MODEL;
	m_hPlayerBody = g_pLTClient->CreateObject( &ocs );
	if( !m_hPlayerBody )
		return false;

	g_pMoveMgr->SetObject( m_hPlayerBody );

	if( m_pMeleeWeapon )
	{
		LTERROR( "CPlayerBodyMgr::InitPlayerBody - Melee weapon already created!" );
		return true;
	}

	// Create the melee weapon...
	m_pMeleeWeapon	= debug_new( CClientWeapon );
	if( !m_pMeleeWeapon )
	{
		LTERROR( "CPlayerBodyMgr::InitPlayerBody - Failed to create melee weapon!" );
		return false;
	}

	// Initialize the melee weapon...
	HWEAPON hWeapon = g_pWeaponDB->GetWeaponRecord( PLAYER_BODY_WEAPON );
	m_pMeleeWeapon->Init( hWeapon );

	if( m_pGrenadeWeapon )
	{
		LTERROR( "CPlayerBodyMgr::InitPlayerBody - Grenade weapon already created!" );
		return true;
	}

	// Create the Grenade weapon...
	m_pGrenadeWeapon	= debug_new( CClientWeapon );
	if( !m_pGrenadeWeapon )
	{
		LTERROR( "CPlayerBodyMgr::InitPlayerBody - Failed to create Grenade weapon!" );
		return false;
	}

	// Initialize the Grenade weapon...
	hWeapon = g_pWeaponDB->GetWeaponRecord( PLAYER_BODY_GRENADE );
	m_pGrenadeWeapon->Init( hWeapon );
	
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::ResetModel
//
//	PURPOSE:	Reset the player body model with the specified model template...
//
// ----------------------------------------------------------------------- //

bool CPlayerBodyMgr::ResetModel( ModelsDB::HMODEL hModel )
{
	LTASSERT(hModel != NULL, "NULL player body hModel encountered.");

	// If the player body model was not created do so now...
	if( !m_hPlayerBody )
		InitPlayerBody();

	// Don't try and reset to the same model template...
	if( m_hModel == hModel )
	{
		// Make sure to at least send up the tracker adds.
		SendTrackerAdds();
		return true;
	}

	m_hModel = hModel;

	// Hook up the engine timer on the playerbody.
	ObjectContextTimer( g_pMoveMgr->GetServerObject( )).ApplyTimerToObject( m_hPlayerBody );

	ResetModelFilenames( );

	// Initialize the torso tracking context...
	ModelsDB::HSKELETON hSkeleton = g_pModelsDB->GetModelSkeleton( m_hModel );
	if( hSkeleton )
	{
		m_NodeTrackerContext.Init( m_hPlayerBody, hSkeleton );
	}

	// Initialize the head bob node control
	g_pPlayerMgr->TermHeadBobNodeControl();
	g_pPlayerMgr->InitHeadBobNodeControl();

	g_pPlayerMgr->GetLeanMgr()->Init( m_hPlayerBody, hModel );


	// Initialize the animations to the new model file...	
	if( !InitAnimations() )
		return false;

	// Set up all physics objects...
	if( !InitPhysics( ))
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::ResetModelFilenames
//
//	PURPOSE:	Update the PlayerBody filenames... 
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::ResetModelFilenames( )
{
	if( !m_hModel || !m_hPlayerBody )
		return;

	const char *pszFilename = g_pModelsDB->GetModelFilename( m_hModel );
	if( !pszFilename || !pszFilename[0] )
		return;

	// Set the model filename and materials...
	ObjectCreateStruct ocs;
	ocs.SetFileName( pszFilename );
	g_pModelsDB->CopyMaterialFilenames( m_hModel, ocs.m_Materials[0], LTARRAYSIZE( ocs.m_Materials ), LTARRAYSIZE( ocs.m_Materials[0] ));

	g_pCommonLT->SetObjectFilenames( m_hPlayerBody, &ocs );

	// Make sure the grenade is using the correct filenames...
	if( m_pGrenadeWeapon )
	{
		m_pGrenadeWeapon->ResetWeaponFilenames( );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::SetAnimProp
//
//	PURPOSE:	Set the animation properties for the contexts of the player body..
//				When specifying to play animations with the property as locked, cache the locked
//				property. Returns true when the animation was set, false otherwise... 
//
// ----------------------------------------------------------------------- //

bool CPlayerBodyMgr::SetAnimProp( EnumAnimPropGroup eAnimPropGroup, EnumAnimProp eAnimProp, AnimPlay ePlay /*= kUnlocked*/ )
{
	if( !m_bEnabled )
		return false;

	bool bSet = true;

	// Dont override locked properties...
	if( ePlay == kLocked )
	{
		bSet = false;

		if( m_MainLockedAnimProps.IsSet( eAnimPropGroup, kAP_None ))
		{
			m_MainLockedAnimProps.Set( eAnimPropGroup, eAnimProp );
			bSet = true;
		}

		if( m_UpperLockedAnimProps.IsSet( eAnimPropGroup, kAP_None ))
			m_UpperLockedAnimProps.Set( eAnimPropGroup, eAnimProp );

		if( m_LowerLockedAnimProps.IsSet( eAnimPropGroup, kAP_None ))
			m_LowerLockedAnimProps.Set( eAnimPropGroup, eAnimProp );

		if( m_CustomLockedAnimProps.IsSet( eAnimPropGroup, kAP_None ))
			m_CustomLockedAnimProps.Set( eAnimPropGroup, eAnimProp );
	}

	if( bSet )
	{
		// Set the animation property in the main set of properties...
		m_AnimProps.Set( eAnimPropGroup, eAnimProp );
	}

	return bSet;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::SetAnimProp
//
//	PURPOSE:	Set the animation properties for a specific context of the player body..
//
// ----------------------------------------------------------------------- //

bool CPlayerBodyMgr::SetAnimProp( PlayerBodyContext kContext, EnumAnimPropGroup eAnimPropGroup, EnumAnimProp eAnimProp )
{
	if( !m_bEnabled )
		return false;

	switch (kContext)
	{
	case kUpperContext:
		{
			m_UpperAnimProps.Set( eAnimPropGroup, eAnimProp );
		}
		return true;

	case kLowerContext:
		{
			m_LowerAnimProps.Set( eAnimPropGroup, eAnimProp );
		}
		return true;

	case kCustomContext:
		{
			if( m_pCustomAnimationContext )
				m_CustomAnimProps.Set( eAnimPropGroup, eAnimProp );
		}
		return true;

	case kMainContext:
		{
			m_AnimProps.Set( eAnimPropGroup, eAnimProp );
		}
		return true;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::InitAnimations
//
//	PURPOSE:	Initialize the player body animation data...
//
// ----------------------------------------------------------------------- //

bool CPlayerBodyMgr::InitAnimations()
{
	if( !m_hPlayerBody )
		return false;

	// Get the number of anim trees.  If there are 4, then the custom animation context is
	// valid, otherwise, leave it uninitialized.
	uint32 cAnimTrees = g_pModelsDB->GetNumModelAnimationTrees( m_hModel );
	if( cAnimTrees < 3 )
	{
		LTERROR( "Invalid number of animation trees listed for player!" );
		return false;
	}

	// Send a message to the players server object to remove these trackers...
	CAutoMessage cRemoveTrkMsg;
	cRemoveTrkMsg.Writeuint8( MID_PLAYER_ANIMTRACKERS );
	cRemoveTrkMsg.Writeuint8( MID_ANIMTRACKERS_REMOVE );
	cRemoveTrkMsg.Writeuint8( cAnimTrees );

	// Destroy any previous contexts...

	if( m_pMainAnimationContext )
	{
		cRemoveTrkMsg.Writeuint8( m_pMainAnimationContext->GetTrackerID() );
		debug_delete( m_pMainAnimationContext );
		m_pMainAnimationContext = NULL;
	}
	else
		cRemoveTrkMsg.Writeuint8( MAIN_TRACKER );
	
    
	if( m_pLowerAnimationContext )
	{
		cRemoveTrkMsg.Writeuint8( m_pLowerAnimationContext->GetTrackerID() );
		debug_delete( m_pLowerAnimationContext );
		m_pLowerAnimationContext = NULL;
	}
	else
		cRemoveTrkMsg.Writeuint8( MAIN_TRACKER );

	if( m_pUpperAnimationContext )
	{
		cRemoveTrkMsg.Writeuint8( m_pUpperAnimationContext->GetTrackerID() );
		debug_delete( m_pUpperAnimationContext );
		m_pUpperAnimationContext = NULL;
	}
	else
		cRemoveTrkMsg.Writeuint8( MAIN_TRACKER );

	if( m_pCustomAnimationContext )
	{
		cRemoveTrkMsg.Writeuint8( m_pCustomAnimationContext->GetTrackerID() );
		debug_delete( m_pCustomAnimationContext );
		m_pCustomAnimationContext = NULL;
	}
	else
		cRemoveTrkMsg.Writeuint8( MAIN_TRACKER );

	g_pLTClient->SendToServer( cRemoveTrkMsg.Read(), MESSAGE_GUARANTEED );

	// Create the animation context...
	
	m_pMainAnimationContext = debug_new( CAnimationContext );
	if( !m_pMainAnimationContext )
	{
		LTERROR( "Failed to create main animation context for player!" );
		return false;
	}

	m_pLowerAnimationContext = debug_new( CAnimationContext );
	if( !m_pLowerAnimationContext )
	{
		LTERROR( "Failed to create lower animation context for player!" );
		return false;
	}

	m_pUpperAnimationContext = debug_new( CAnimationContext );
	if( !m_pUpperAnimationContext )
	{
		LTERROR( "Failed to create upper animation context for player!" );
		return false;
	}

	if( cAnimTrees > 3 )
	{
		m_pCustomAnimationContext = debug_new( CAnimationContext );
		if( !m_pCustomAnimationContext )
		{
			LTERROR( "Failed to create Custom animation context for player!" );
			return false;
		}
	}

	m_pMainAnimationContext->Init( m_hPlayerBody, m_hModel );
	m_pLowerAnimationContext->Init( m_hPlayerBody, m_hModel, kAD_TRK_Lower );
	m_pUpperAnimationContext->Init( m_hPlayerBody, m_hModel, kAD_TRK_Upper );
	if( m_pCustomAnimationContext )
		m_pCustomAnimationContext->Init( m_hPlayerBody, m_hModel, kAD_TRK_Custom );

	const char* const pszNullWeightSetName = g_pModelsDB->GetNullWeightSetName();
	HMODELWEIGHTSET hWS = INVALID_MODEL_WEIGHTSET;
	if ( LT_OK != g_pModelLT->FindWeightSet(m_hPlayerBody, pszNullWeightSetName, hWS) )
	{
#ifndef _FINAL
		g_pLTClient->CPrint("Critical error, no Null weightset on Character!  Missing: %s", pszNullWeightSetName);
#endif
		LTERROR( "Critical error, no 'Null' weightset on Character!" );
		return false;
	}

	m_pMainAnimationContext->SetNullWeightset( hWS );
	m_pLowerAnimationContext->SetNullWeightset( hWS );
	m_pUpperAnimationContext->SetNullWeightset( hWS );
	if( m_pCustomAnimationContext )
		m_pCustomAnimationContext->SetNullWeightset( hWS );
	
	ClientDB &ClientDatabase = ClientDB::Instance( );
	HRECORD hClientShared = ClientDatabase.GetClientSharedRecord( );

	// Add the blend trackers only if we need to...
	// NOTE: The blend trackers need to be added after the Animation contexts have set their null weight set..
	if( ClientDatabase.GetBool( hClientShared, CDB_UsePostureBlending ))
	{
		if( LT_OK != g_pModelLT->AddTracker( m_hPlayerBody, kAD_TRK_UpperBlend, true ))
		{
			LTERROR( "Failed to add animation blend tracker for the player!" );
		}
		else
		{
			m_pUpperAnimationContext->SetBlendTracker( kAD_TRK_UpperBlend );
		}
		
		if( LT_OK != g_pModelLT->AddTracker( m_hPlayerBody, kAD_TRK_LowerBlend, true ))
		{
			LTERROR( "Failed to add animation blend tracker for the player!" );
		}
		else
		{
			m_pLowerAnimationContext->SetBlendTracker( kAD_TRK_LowerBlend );
		}
	}

	const char* pszAnimTree;
	// The first tree is the main tree which should be added to all animation contexts...
	pszAnimTree = g_pModelsDB->GetModelAnimationTree( m_hModel, 0 );
	if( pszAnimTree[0] )
	{
		m_pMainAnimationContext->AddAnimTreePacked( pszAnimTree );
		m_pLowerAnimationContext->AddAnimTreePacked( pszAnimTree );
		m_pUpperAnimationContext->AddAnimTreePacked( pszAnimTree );
		if( m_pCustomAnimationContext )
			m_pCustomAnimationContext->AddAnimTreePacked( pszAnimTree );
	}

	// The second tree is the lower tree...
	pszAnimTree = g_pModelsDB->GetModelAnimationTree( m_hModel, 1 );
	if( pszAnimTree[0] )
	{
		m_pLowerAnimationContext->AddAnimTreePacked( pszAnimTree );
	}

	// The third tree is for the upper context...
	pszAnimTree = g_pModelsDB->GetModelAnimationTree( m_hModel, 2 );
	if( pszAnimTree[0] )
	{
		m_pUpperAnimationContext->AddAnimTreePacked( pszAnimTree );
	}

	if( m_pCustomAnimationContext )
	{
		// The last tree is for the Custom context...
		pszAnimTree = g_pModelsDB->GetModelAnimationTree( m_hModel, 3 );
		if( pszAnimTree[0] )
		{
			m_pCustomAnimationContext->AddAnimTreePacked( pszAnimTree );
		}
	}


	// Start all the trackers disabled...

	m_pMainAnimationContext->Disable();
	m_pLowerAnimationContext->Disable();
	m_pUpperAnimationContext->Disable();
	if( m_pCustomAnimationContext )
		m_pCustomAnimationContext->Disable();

	SendTrackerAdds();

	// Clear all of the animation properties.  This insures we don't maintain 
	// state between loads.

	m_LastAnimProps.Clear();
	m_AnimProps.Clear();
	m_UpperAnimProps.Clear();
	m_LowerAnimProps.Clear();
	m_CustomAnimProps.Clear();

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::SendTrackerAdds
//
//	PURPOSE:	Sends the trackers that have been added to the playerbody model.
//
// ----------------------------------------------------------------------- //

bool CPlayerBodyMgr::SendTrackerAdds()
{
	if( !m_hPlayerBody )
		return false;

	const CAnimationContext *apContexts[kNumContexts] = { m_pMainAnimationContext,
		m_pUpperAnimationContext,
		m_pLowerAnimationContext,
		m_pCustomAnimationContext };

	// Send a message to the players server object to add these trackers...
	CAutoMessage cAddTrkMsg;
	cAddTrkMsg.Writeuint8( MID_PLAYER_ANIMTRACKERS );
	cAddTrkMsg.Writeuint8( MID_ANIMTRACKERS_ADD );
	uint32 cAnimTrees = g_pModelsDB->GetNumModelAnimationTrees( m_hModel );
	cAddTrkMsg.Writeuint8( cAnimTrees );

	// Write the info for each context...
	for( uint8 nContext = 0; nContext < cAnimTrees; ++nContext )
	{
		if( !apContexts[nContext] )
			continue;
		ANIMTRACKERID trkID = apContexts[nContext]->GetTrackerID();
		cAddTrkMsg.Writeuint8( trkID );

		if( trkID != MAIN_TRACKER )
		{
			HMODELWEIGHTSET hWeightSet = INVALID_MODEL_WEIGHTSET;
			g_pModelLT->GetWeightSet( m_hPlayerBody, trkID, hWeightSet );
			cAddTrkMsg.Writeuint32( hWeightSet );
		}
	}

	// Write the tracker that is responsible for footsteps...
	cAddTrkMsg.Writeuint8( m_pLowerAnimationContext->GetTrackerID() );

	g_pLTClient->SendToServer( cAddTrkMsg.Read(), MESSAGE_GUARANTEED );

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::UpdateNotPlaying
//
//	PURPOSE:	Specifically update the player body when not in the playing state...
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::UpdateNotPlaying( )
{
	// Set properties to force the player in a talking idle animation...
	SetAnimProp( kAPG_Movement, kAP_MOV_Idle );
	SetAnimProp( kAPG_MovementDir, kAP_None );
	if( IsMultiplayerGameClient( ) && !LadderMgr::Instance( ).IsClimbing( ) )
	{
		SetAnimProp( kAPG_Action, kAP_ACT_Talk );
	}
	

	// Be sure we are not playing any looping sounds.
	// Normally the loop sound is handled via a locked animation, but
	// since we just over-rode the locked animations above, we need to
	// do what-ever a locked animation may have done.
	CClientWeapon* const pWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon( );
	if( pWeapon && pWeapon->HasLoopSound() )
	{
		pWeapon->StopLoopSound();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::Update
//
//	PURPOSE:	Update the player body
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::Update()
{
	// Make sure we actually have a body object and are enabled...
	if( /*!m_bEnabled ||*/ !m_hPlayerBody )
		return;

	// Pause the player body if the game is paused...
	bool bPaused = g_pGameClientShell->IsServerPaused( );
	if( bPaused )
		return;

	if( g_vtPlayerBodyWeapons.GetFloat() >= 1.0f )
		Enable();

// PLAYER_BODY - Remove this block once the player body is always enabled...	
	if( !m_bEnabled )
	{
		m_AnimProps.Clear( );
		m_UpperAnimProps.Clear( );
		m_LowerAnimProps.Clear( );
		m_CustomAnimProps.Clear( );
		m_LowerLockedAnimProps.Clear( );
		m_MainLockedAnimProps.Clear( );
		m_UpperLockedAnimProps.Clear( );
		m_CustomLockedAnimProps.Clear( );
        
		UpdateAimTracking();
		return;
	}

	if( !m_pMainAnimationContext || !m_pLowerAnimationContext || !m_pUpperAnimationContext )
		return;

	if( !g_pPlayerMgr->IsPlayerAlive( ))
	{
		ClearLockedProperties( );

		m_pUpperAnimationContext->ClearLock( );
		m_pLowerAnimationContext->ClearLock( );
		if( m_pCustomAnimationContext )
			m_pCustomAnimationContext->ClearLock( );

		m_pMainAnimationContext->Disable( );
		m_pUpperAnimationContext->Disable( );
		m_pLowerAnimationContext->Disable( );
		if( m_pCustomAnimationContext )
			m_pCustomAnimationContext->Disable( );

		m_eMovementDescriptor = kAD_None;
		m_eCameraDescriptor = kAD_None;
		m_eInputDescriptor = kAD_None;
		return;
	}

	if( g_pInterfaceMgr->GetGameState( ) != GS_PLAYING || g_pInterfaceMgr->IsChatting( ))
	{
		// Set animation props for when the player is not actually in the playing state...
		UpdateNotPlaying( );
	}

	if( m_bStartPlayerBody )
	{
		m_bStartPlayerBody = false;

		// Show the player body model and shadow...
		g_pCommonLT->SetObjectFlags( m_hPlayerBody, OFT_Client, CF_NOTIFYMODELKEYS, CF_NOTIFYMODELKEYS );
		g_pCommonLT->SetObjectFlags( m_hPlayerBody, OFT_Flags, FLAG_VISIBLE, FLAG_VISIBLE );
		g_pLTClient->SetObjectShadowLOD( m_hPlayerBody, PLAYER_SHADOW_LOD );

		g_pPlayerMgr->GetPlayerCamera()->AttachToTarget( true );
		g_pPlayerMgr->GetClientWeaponMgr()->HideWeapons();

		g_pPlayerMgr->GetClientWeaponMgr()->InitCustomWeapons();

		if( g_vtPlayerBodyWeapons.GetFloat() >= 1.0f )
		{
			m_pGrenadeWeapon->Activate();
			m_pGrenadeWeapon->SetDisable(false);
			m_pGrenadeWeapon->SetVisible(false);
		}
	}

	// Hide the client object model and shadow.  This is the object given to us by the server.
	g_pCommonLT->SetObjectFlags( g_pLTClient->GetClientObject(), OFT_Flags, 0, FLAG_VISIBLE );
	g_pPlayerMgr->HideShowAttachments( g_pLTClient->GetClientObject() );
	g_pLTClient->SetObjectShadowLOD( g_pLTClient->GetClientObject(), eEngineLOD_Never );

	UpdatePlayingSpecialAnimation( );

	if( !m_bPlayingSpecial )
	{

		// Determine the generic look context by checking values against the cameras pitch...
		UpdateLookContextProperties( );
		
		// Determine the appropriate action property...
		UpdateActionProperties( );

		// The PlayerBody is always going to use animations for the local client..
		m_AnimProps.Set( kAPG_Client, kAP_CLIENT_Local );

		// Clear all properties if the player has no weapon...
		if( m_AnimProps.Get( kAPG_Weapon ) == kAP_None )
		{
			m_AnimProps.Clear( );
			m_UpperAnimProps.Clear( );
			m_LowerAnimProps.Clear( );
			m_CustomAnimProps.Clear( );
		}

		// Set the props on each context...
		// Also fold in the overrides.  Lower overrides take precedence on the
		// main context since that contains the movement nodes.
		if( m_pCustomAnimationContext )
			m_pCustomAnimationContext->SetProps( m_AnimProps << m_CustomAnimProps );
		m_pLowerAnimationContext->SetProps( m_AnimProps << m_LowerAnimProps );
		m_AnimProps = (m_AnimProps << m_UpperAnimProps);
		m_pUpperAnimationContext->SetProps( m_AnimProps );
		m_AnimProps = (m_AnimProps << m_LowerAnimProps);
		m_pMainAnimationContext->SetProps( m_AnimProps );
		if( m_pCustomAnimationContext )
			m_AnimProps = (m_AnimProps << m_CustomAnimProps);

		// Try playing a full body animation first...
		if( !UpdateFullBodyAnimation( ))
		{
			// Update Upper and Lower trackers if there are no full body anis...
			UpdateUpperBodyAnimation( );
			UpdateLowerBodyAnimation( );
			UpdateCustomBodyAnimation( );
		}

		//initialize the posture here for cases where the animations are updated before CMoveMgr has had an update
		// to set the props
		if (m_AnimProps.Get( kAPG_Posture ) == kAP_None)
		{
			m_AnimProps.Set( kAPG_Posture, kAP_POS_Stand );
		}

	}

	// Blend data must be specified before updating the contexts...
	UpdateAnimationBlending( );

	// Update the context...
	UpdateAnimationContexts( );

	// Set movement based on current animations...
	UpdateMovementType( );

	// Set camera type based on current animations...
	UpdateCameraType( );

	// Set input based on current animations...
	UpdateInputType( );

	// Make sure the weapon model is playing the right animation...
	UpdateWeaponModel( );

	// Clear any properties that are no longer locked...
	UpdateLockedAnimationProperties( );

	// Update the model dimensions...
	UpdateDims( );

	// Update the torso tracking...
	UpdateAimTracking( );

	// Check if it's time to stop our close encounter...
	UpdateCloseEncounter();

	// Cache and clear the properties from this update..
	m_LastAnimProps = m_AnimProps;
	m_AnimProps.Clear();
	m_UpperAnimProps.Clear();
	m_LowerAnimProps.Clear();
	m_CustomAnimProps.Clear();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::UpdateFullBodyAnimation
//
//	PURPOSE:	Determines if a full body animation should play and handles playing it if it should...
//				Returns true if full body animation is playing, false otherwise...
//
// ----------------------------------------------------------------------- //

bool CPlayerBodyMgr::UpdateFullBodyAnimation( )
{
	// See if the animation properties have changed from the previous update...
	bool bPropsChanged = !(m_AnimProps == m_LastAnimProps);

	// Need to check if the main context should update...
	if( bPropsChanged || m_pMainAnimationContext->IsLocked() || m_pMainAnimationContext->WasLockCleared() )
	{
		// Check if any full-body animations want to play...
		uint32 nNumMainAnis = CountAnimations( kMainContext );
		m_bPlayingFullBody = (nNumMainAnis > 0);

		// Lock the main context if its going to play an animation that has a property set in our locked properties...
		if( !m_MainLockedAnimProps.IsClear() && m_AnimProps.Contains( m_MainLockedAnimProps ))
		{
			if( nNumMainAnis > 0 )
				m_pMainAnimationContext->Lock( );
		}

		// Continue to update the full body if it's locked...
		m_bPlayingFullBody |= m_pMainAnimationContext->IsLocked( ); 

		// The cached ani is cleard when we have multiple animations for a single set of properties...
		m_bMainClearCachedAni = (nNumMainAnis > 1);
	}

	if( m_bPlayingFullBody )
	{
		// Make sure the upper and lower contexts are enabled...
		m_pLowerAnimationContext->Enable( );
		m_pUpperAnimationContext->Enable( );

		if( m_pMainAnimationContext->IsLocked( ))
		{
			// Lock the contexts if they are going to play an animation that has a property set in our locked properties...
			if( !m_UpperLockedAnimProps.IsClear() && m_AnimProps.Contains( m_UpperLockedAnimProps ))
			{
				m_pUpperAnimationContext->Lock();
			}

			// Lock the contexts if they are going to play an animation that has a property set in our locked properties...
			if( !m_LowerLockedAnimProps.IsClear( ) && m_AnimProps.Contains( m_LowerLockedAnimProps ))
			{
				m_pLowerAnimationContext->Lock( );
			}
		}

		// Clear the cached anis so different anis can play after the current one finishes...
		if( m_bMainClearCachedAni )
			m_pMainAnimationContext->ClearCachedAni( );

		// The fullbody context just tells the upper and lower contexts to play the same animations...
		ANIM_QUERY_RESULTS AnimResults;
		if( m_pMainAnimationContext->FindAnimation( AnimResults ))
		{
			ANIM_TREE_CACHED CachedAnimation;
			CachedAnimation.iAnimation = AnimResults.Index.iAnimation;
			CachedAnimation.iAnimTree = AnimResults.Index.iAnimTree;
			CachedAnimation.Props = AnimResults.Props;

			m_pLowerAnimationContext->SetCachedAnimation( CachedAnimation );
			m_pUpperAnimationContext->SetCachedAnimation( CachedAnimation );
		}
	}

	return m_bPlayingFullBody;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::UpdateUpperBodyAnimation
//
//	PURPOSE:	Handles playing an animation on the upper body...
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::UpdateUpperBodyAnimation( )
{
	// See if the animation properties have changed from the previous update...
	bool bPropsChanged = !(m_AnimProps == m_LastAnimProps);

	if( bPropsChanged || m_pUpperAnimationContext->IsLocked( ) || m_pUpperAnimationContext->WasLockCleared( ))
	{
		// The cached ani is cleared when we have multiple animations for a single set of properties...
		uint32 nNumUpperAnis = CountAnimations( kUpperContext );
		m_bUpperClearCachedAni = (nNumUpperAnis > 1);
		
		// Clear any properties that are not supposed to lock the lower body...
		UpdateUpperLockedAnimProps( );

		// Lock the contexts if they are going to play an animation that has a property set in our locked properties...
		if( !m_UpperLockedAnimProps.IsClear() && m_AnimProps.Contains( m_UpperLockedAnimProps ))
		{
			if( nNumUpperAnis > 0 )
				m_pUpperAnimationContext->Lock();
		}
	}

	// Make sure the upper body context is enabled...
	m_pUpperAnimationContext->Enable();

	// Clear the cached anis so different anis can play after the current one finishes...
	if( m_bUpperClearCachedAni )
		m_pUpperAnimationContext->ClearCachedAni();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::UpdateUpperLockedAnimProps
//
//	PURPOSE:	Update the animation properties that lock the upper body...
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::UpdateUpperLockedAnimProps( )
{
	// Upper tracker should ignore certain properties that apply mainly to locking lower body animations...
	EnumAnimProp eMoveProp = m_UpperLockedAnimProps.Get( kAPG_Movement );
	if( (eMoveProp == kAP_MOV_Jump) ||
		(eMoveProp == kAP_MOV_Fall) ||
		(eMoveProp == kAP_MOV_Land) )
	{
		m_UpperLockedAnimProps.Clear( kAPG_Movement );
	}

	if( m_pUpperAnimationContext->IsLocked( ) )
	{
		// Look at the posture of the lower body and make sure the upper body posture matches...
		// NOTE: This could be tricky since the animations will be playing at different times.

		EnumAnimProp eLowerPos = m_pLowerAnimationContext->GetProp( kAPG_Posture );
		EnumAnimProp eUpperPos = m_pUpperAnimationContext->GetCurrentProp( kAPG_Posture );
		if( (eUpperPos != eLowerPos) && (eUpperPos != kAP_None) )
		{
			CAnimationProps CurProps;
			m_pUpperAnimationContext->GetCurrentProps( &CurProps );

			m_pUpperAnimationContext->ClearLock( );
			m_pUpperAnimationContext->SetProps( CurProps );
			m_pUpperAnimationContext->SetProp( kAPG_Posture, eLowerPos );
			m_pUpperAnimationContext->Lock( );

			uint32 dwCurAnimTime = 0;
			g_pModelLT->GetCurAnimTime( m_hPlayerBody, m_pUpperAnimationContext->GetTrackerID( ), dwCurAnimTime );
			m_pUpperAnimationContext->SetAnimStartingTime( dwCurAnimTime );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::UpdateLowerBodyAnimation
//
//	PURPOSE:	Handles playing an animation on the lower body...
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::UpdateLowerBodyAnimation( )
{
	// See if the animation properties have changed from the previous update...
	bool bPropsChanged = !(m_AnimProps == m_LastAnimProps);

    if( bPropsChanged || m_pLowerAnimationContext->IsLocked( ) || m_pLowerAnimationContext->WasLockCleared( ))
	{
		// The cached ani is cleard when we have multiple animations for a single set of properties...
		uint32 nNumLowerAnis = CountAnimations( kLowerContext );
		m_bLowerClearCachedAni = (nNumLowerAnis > 1);

		// Clear any properties that are not supposed to lock the lower body...
		UpdateLowerLockedAnimProps( );

		// Lock the contexts if they are going to play an animation that has a property set in our locked properties...
		if( !m_LowerLockedAnimProps.IsClear( ) && m_AnimProps.Contains( m_LowerLockedAnimProps ))
		{
			if( nNumLowerAnis > 0 )
				m_pLowerAnimationContext->Lock( );
		}
	}

	// Make sure the upper and lower contexts are enabled...
	m_pLowerAnimationContext->Enable();

	// Clear the cached anis so different anis can play after the current one finishes...
	if( m_bLowerClearCachedAni )
		m_pLowerAnimationContext->ClearCachedAni();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::UpdateLockedAnimationProperties
//
//	PURPOSE:	Update the animation properties that lock the lower body...
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::UpdateLowerLockedAnimProps( )
{
	EnumAnimProp eMovement = m_AnimProps.Get( kAPG_Movement );
	EnumAnimProp eAction = m_AnimProps.Get( kAPG_Action );
	EnumAnimProp ePosture = m_AnimProps.Get( kAPG_Posture );

	// Lower tracker should never be locked while running or walking...
	if( (eMovement == kAP_MOV_Run) ||
		(eMovement == kAP_MOV_Walk) )
	{
		m_LowerLockedAnimProps.Clear( );

		if( m_pLowerAnimationContext->IsLocked( ) && !IsMovementEncoded( ))
			m_pLowerAnimationContext->ClearLock( );
	}

	// We don't ever ever ever ever ever ever ever want the special fire to be able to lock the lower animations.
	m_LowerLockedAnimProps.Clear( kAPG_SpecialFire );
	m_LowerLockedAnimProps.Clear( kAPG_ConditionalAction );
	m_LowerLockedAnimProps.Clear( kAPG_ConditionalSubAction );

	// Weapon animations should not lock the lower tracker...
	if( (eAction == kAP_ACT_Fire) ||
		(eAction == kAP_ACT_PreFire) ||
		(eAction == kAP_ACT_PostFire) ||
		(eAction == kAP_ACT_FireSecondary) ||
		(eAction == kAP_ACT_Block) ||
		(eAction == kAP_ACT_CheckAmmo) ||
		(eAction == kAP_ACT_ThrowStart	) ||
		(eAction == kAP_ACT_ThrowHold	) ||
		(eAction == kAP_ACT_Throw) ||
		(eAction == kAP_ACT_Reload) ||
		(eAction == kAP_ACT_Select) ||
		(eAction == kAP_ACT_Deselect) )
	{
		CClientWeapon *pWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
		if( !pWeapon )
			return;

// PLAYER_BODY don't clear the lower when armed??
		if( pWeapon->GetWeaponRecord( ) != g_pWeaponDB->GetUnarmedRecord( ) || 
			(eAction == kAP_ACT_ThrowStart ) ||
			(eAction == kAP_ACT_ThrowHold ) ||
			(eAction == kAP_ACT_Throw) ||
			(eAction == kAP_ACT_Select) ||
			(eAction == kAP_ACT_Deselect) ||
			(ePosture == kAP_POS_Crouch) )
		{
			m_LowerLockedAnimProps.Clear( kAPG_Action );
		}
	}

}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::UpdateCustomBodyAnimation
//
//	PURPOSE:	Handles playing an animation on the Custom body...
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::UpdateCustomBodyAnimation( )
{
	if( !m_pCustomAnimationContext )
		return;

	// See if the animation properties have changed from the previous update...
	bool bPropsChanged = !(m_AnimProps == m_LastAnimProps);

	if( bPropsChanged || m_pCustomAnimationContext->IsLocked( ) || m_pCustomAnimationContext->WasLockCleared( ))
	{
		// The cached ani is cleared when we have multiple animations for a single set of properties...
		uint32 nNumCustomAnis = CountAnimations( kCustomContext );
		m_bPlayingCustom = (nNumCustomAnis > 0);
		m_bCustomClearCachedAni = (nNumCustomAnis > 1);

		// Clear any properties that are not supposed to lock the lower body...
		UpdateCustomLockedAnimProps( );

		// Lock the contexts if they are going to play an animation that has a property set in our locked properties...
		if( !m_CustomLockedAnimProps.IsClear() && m_AnimProps.Contains( m_CustomLockedAnimProps ))
		{
			if( nNumCustomAnis > 0 )
				m_pCustomAnimationContext->Lock();
		}

		// Continue to update the custom body if it's locked...
		m_bPlayingCustom |= m_pCustomAnimationContext->IsLocked( ); 
	}

	// Make sure the Custom body context is enabled...
	if (m_bPlayingCustom)
		m_pCustomAnimationContext->Enable();
	else
		m_pCustomAnimationContext->Reset();

	// Clear the cached anis so different anis can play after the current one finishes...
	if( m_bCustomClearCachedAni )
		m_pCustomAnimationContext->ClearCachedAni();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::UpdateCustomLockedAnimProps
//
//	PURPOSE:	Update the animation properties that lock the Custom body...
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::UpdateCustomLockedAnimProps( )
{
	if( !m_pCustomAnimationContext )
		return;

	// Custom tracker should ignore certain properties that apply mainly to locking lower body animations...
	EnumAnimProp eMoveProp = m_CustomLockedAnimProps.Get( kAPG_Movement );
	if( (eMoveProp == kAP_MOV_Jump) ||
		(eMoveProp == kAP_MOV_Fall) ||
		(eMoveProp == kAP_MOV_Land) )
	{
		m_CustomLockedAnimProps.Clear( kAPG_Movement );
	}

	// Weapon animations should not lock the custom tracker...
	EnumAnimProp eAction = m_AnimProps.Get( kAPG_Action );
	if( (eAction == kAP_ACT_Fire) ||
		(eAction == kAP_ACT_PreFire) ||
		(eAction == kAP_ACT_PostFire) ||
		(eAction == kAP_ACT_FireSecondary) ||
		(eAction == kAP_ACT_Block) ||
		(eAction == kAP_ACT_CheckAmmo) ||
		(eAction == kAP_ACT_ThrowStart	) ||
		(eAction == kAP_ACT_ThrowHold	) ||
		(eAction == kAP_ACT_Throw) ||
		(eAction == kAP_ACT_Reload) ||
		(eAction == kAP_ACT_Select) ||
		(eAction == kAP_ACT_Deselect) )
	{
		m_CustomLockedAnimProps.Clear( kAPG_Action );
	}

	if( m_pCustomAnimationContext->IsLocked( ) )
	{
		// Look at the posture of the lower body and make sure the Custom body posture matches...
		// NOTE: This could be tricky since the animations will be playing at different times.

		EnumAnimProp eLowerPos = m_pLowerAnimationContext->GetProp( kAPG_Posture );
		EnumAnimProp eCustomPos = m_pCustomAnimationContext->GetCurrentProp( kAPG_Posture );
		if( (eCustomPos != eLowerPos) && (eCustomPos != kAP_None) )
		{
			CAnimationProps CurProps;
			m_pCustomAnimationContext->GetCurrentProps( &CurProps );

			m_pCustomAnimationContext->ClearLock( );
			m_pCustomAnimationContext->SetProps( CurProps );
			m_pCustomAnimationContext->SetProp( kAPG_Posture, eLowerPos );
			m_pCustomAnimationContext->Lock( );

			uint32 dwCurAnimTime = 0;
			g_pModelLT->GetCurAnimTime( m_hPlayerBody, m_pCustomAnimationContext->GetTrackerID( ), dwCurAnimTime );
			m_pCustomAnimationContext->SetAnimStartingTime( dwCurAnimTime );
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::UpdateDims
//
//	PURPOSE:	Update the dimensions of the player body...
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::UpdateDims()
{
	if( !m_hPlayerBody )
		return;

	ANIMTRACKERID DimsTrackerID = MAIN_TRACKER;

	// Get the animation playing on the lower tracker, or main if lower is disabled...
	if( m_pLowerAnimationContext->IsEnabled() )
	{
		DimsTrackerID = m_pLowerAnimationContext->GetTrackerID();
	}
	
	HMODELANIM hDimsAnim = INVALID_MODEL_ANIM;
	g_pModelLT->GetCurAnim( m_hPlayerBody, DimsTrackerID, hDimsAnim );

	// Set the dims only if the animation has changed...
	if( m_dwLastDimsAnim != hDimsAnim )
	{
		m_dwLastDimsAnim = hDimsAnim;

		// Get our wanted dims.
		LTVector vOldDims = m_vWantedDims;
		m_vWantedDims.Init(1.0f, 1.0f, 1.0f);

		g_pModelLT->GetModelAnimUserDims( m_hPlayerBody, hDimsAnim, &m_vWantedDims );

		if( g_vtPlayerBodyShowDimsInfo.GetFloat( ) > 0.0f )
		{
			EnumAnimProp ePosture = m_AnimProps.Get( kAPG_Posture );
			if( ePosture == kAP_POS_Stand && m_vWantedDims != PLAYER_DEFAULT_STAND_DIMS ||
				ePosture == kAP_POS_Crouch && m_vWantedDims != PLAYER_DEFAULT_CROUCH_DIMS )
			{
				char szAnimName[256] = {0};
				g_pModelLT->GetAnimName( m_hPlayerBody, hDimsAnim, szAnimName, LTARRAYSIZE(szAnimName));
				g_pLTClient->CPrint( "Non Standard Dims: %f %f %f - Anim: %s", VEC_EXPAND(m_vWantedDims), szAnimName );
			}
		}
		
		// Figure out a position offset.
		LTVector vOffset( 0.0f, 0.0f, 0.0f);
		if( m_vWantedDims.y < vOldDims.y )
		{
			vOffset.y = -(vOldDims.y - m_vWantedDims.y);
			vOffset.y -= .01f; // Fudge factor
		}

		if( !AreDimsCorrect() )
		{
			ResetDims( vOffset );

			// HACK: drp080706 - if failed, restore m_vWantedDims as well (see comment at end of Load())
			if (!AreDimsCorrect())
			{
				m_vWantedDims = vOldDims;
			}
		}
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::AreDimsCorrect
//
//	PURPOSE:	Test if the current dims are the dims that are actually wanted...
//
// ----------------------------------------------------------------------- //

bool CPlayerBodyMgr::AreDimsCorrect()
{
	if( !m_hPlayerBody )
		return true;

	LTVector vCurDims;

	g_pPhysicsLT->GetObjectDims( m_hPlayerBody, &vCurDims );
	return vCurDims.NearlyEquals( m_vWantedDims, 0.1f );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::ResetDims
//
//	PURPOSE:	Set the wanted dims and factor in any offset...
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::ResetDims( LTVector &rvOffset )
{
	if( !m_hPlayerBody )
		return;

	// Remember our old position and size, just in case
	LTVector vOldDims;
	g_pPhysicsLT->GetObjectDims( m_hPlayerBody, &vOldDims );
	LTVector vOldPos;
	g_pLTClient->GetObjectPos( m_hPlayerBody, &vOldPos );

	// Save off our wanted dims they dont change;
	LTVector vNewDims = m_vWantedDims;

	LTVector vCurVel;
	g_pPhysicsLT->GetVelocity( m_hPlayerBody, &vCurVel );

	// Try to set our wanted dims... 
	if( g_pPhysicsLT->SetObjectDims( m_hPlayerBody, &vNewDims, SETDIMS_PUSHOBJECTS ) != LT_OK )
	{
		// Go back to where we were..
		g_pPhysicsLT->SetObjectDims( m_hPlayerBody, &vOldDims, 0 );
		g_pLTClient->SetObjectPos( m_hPlayerBody, vOldPos );
	}
	// Move them if they want
	else if( rvOffset != LTVector(0.0f, 0.0f, 0.0f) )
	{
		vOldPos += rvOffset;
		g_pPhysicsLT->MoveObject( m_hPlayerBody, vOldPos, 0 );
		g_pLTClient->SetObjectPos( m_hPlayerBody, vOldPos );
	}

	// Make sure our velocity doesn't change due to just changing dims.
	LTVector vNewVel;
	g_pPhysicsLT->GetVelocity( m_hPlayerBody, &vNewVel );
	if( vNewVel != vCurVel )
	{
		g_pPhysicsLT->SetVelocity( m_hPlayerBody, vCurVel );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::OnModelKey
//
//	PURPOSE:	Handle model keys coming from the player body...
//
// ----------------------------------------------------------------------- //

bool CPlayerBodyMgr::OnModelKey( HLOCALOBJ hObj, ANIMTRACKERID hTrackerID, ArgList* pArgList )
{
	static CParsedMsg::CToken s_cTok_PlayerBodyKeyMelee( PLAYER_BODY_KEY_MELEE );
	static CParsedMsg::CToken s_cTok_PlayerBodyKeySoundBute( PLAYER_BODY_KEY_SOUND_BUTE );
	static CParsedMsg::CToken s_cTok_PlayerBodyKeyThrow( PLAYER_BODY_KEY_THROW );
	static CParsedMsg::CToken s_cTok_PlayerBodyKeyGrenade( PLAYER_BODY_KEY_GRENADE );
	static CParsedMsg::CToken s_cTok_PlayerBodyKeyFireGrenade( PLAYER_BODY_KEY_FIREGRENADE );
	static CParsedMsg::CToken s_cTok_PlayerBodyKeyKillProjectiles( PLAYER_BODY_KEY_KILLPROJECTILES );
	static CParsedMsg::CToken s_cTok_PlayerBodyKeySonic( PLAYER_BODY_KEY_SONIC );
	static CParsedMsg::CToken s_cTok_PlayerBodyKeyActivate( PLAYER_BODY_KEY_ACTIVATE );
	static CParsedMsg::CToken s_cTok_PlayerBodyFXKeyBlockWindow( PLAYER_BODY_KEY_BLOCKWINDOW );
	static CParsedMsg::CToken s_cTok_PlayerBodyFXKeyDodgeWindow( PLAYER_BODY_KEY_DODGEWINDOW );
	static CParsedMsg::CToken s_cTok_PlayerBodyFXKeyMeleeControl( PLAYER_BODY_KEY_MELEECONTROL );
	static CParsedMsg::CToken s_cTok_PlayerBodyFXKeyBerserkerKick( PLAYER_BODY_KEY_BERSERKERKICK );
	static CParsedMsg::CToken s_cTok_PlayerBodyFXKeyFinishingMove( PLAYER_BODY_KEY_FINISHINGMOVE );
	static CParsedMsg::CToken s_cTok_PlayerBodyKeyCustomWeaponSelect( PLAYER_BODY_KEY_CUSTOM_WEAPON_SELECT );
	static CParsedMsg::CToken s_cTok_PlayerBodyKeyCloseEncounter( PLAYER_BODY_KEY_CLOSE_ENCOUNTER );
	static CParsedMsg::CToken s_cTok_PlayerBodyKeyFinish( PLAYER_BODY_KEY_FINISH );

	if( !hObj || !m_hPlayerBody || (hObj != m_hPlayerBody) || !pArgList || !pArgList->argv || (pArgList->argc == 0) )
		return false;

	// Get the key...
	const char *pszKey = pArgList->argv[0];
	if( !pszKey || !pszKey[0] )
		return false;

	// Make a token to compare against...
	CParsedMsg::CToken tok( pszKey );

	if( tok == s_cTok_PlayerBodyKeyMelee )
	{
// TEMP

		// Only care about frame strings from the upper tracker when playing a fullbody animation..
		if( (m_pUpperAnimationContext->GetDescriptor( kADG_Tracker ) == kAD_None) && 
			(hTrackerID == m_pLowerAnimationContext->GetTrackerID( )) )
			return true;

#if defined(PROJECT_DARK)
		CClientWeapon* pMeleeWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
#elif defined(PROJECT_FEAR)
		CClientWeapon* pMeleeWeapon = m_pMeleeWeapon;
#endif

		// Use our melee weapon to fire...
		// Need to design a better system after prototype (possible using physics objects?)

		if( g_vtPlayerBodyFireMode.GetFloat() >= 1 )
		{
			if( pArgList->argc != 2 )
			return false;

			LTVector vFirePos;
			LTVector vDir;

			// The fire pos will be the node specified in the model key...

			const char *pszNode = pArgList->argv[1];
			if( !pszNode || !pszNode[0] )
				return false;

			HMODELNODE hNode;
			if( (g_pModelLT->GetNode( m_hPlayerBody, pszNode, hNode ) != LT_OK) || (hNode == INVALID_MODEL_NODE) )
				return false;

			LTTransform trans;
			if( g_pModelLT->GetNodeTransform( m_hPlayerBody, hNode, trans, true ) != LT_OK )
				return false;

			vFirePos = trans.m_vPos;

			LTRotation rot;
			g_pLTClient->GetObjectRotation( m_hPlayerBody, &rot );

			pMeleeWeapon->SendFireMessage( 0.0f, vFirePos, rot.Forward() );
		}
		else
		{
// PLAYER_BODY - Temp: Kill the current weapon loop sound... the weapons alt-fire state should handle this..
			CClientWeapon *pWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
			if( !pWeapon )
				return false;

			pWeapon->KillLoopSound( );

			HAMMO hCurAmmo = pMeleeWeapon->GetAmmoRecord( );

			// FIRE <ammo> <node>

			HAMMO hNewAmmo = NULL;
			if( pArgList->argc > 1 )
			{
				const char *pszAmmo = pArgList->argv[1];
				hNewAmmo = g_pWeaponDB->GetAmmoRecord( pszAmmo );

			}
			else
			{
				// Use the default melee ammo from the current weapon, if one is specified...
				
				HWEAPONDATA hCurWeaponData = g_pWeaponDB->GetWeaponData( pWeapon->GetWeaponRecord( ), !USE_AI_DATA );
				if( hCurWeaponData )
				{
					hNewAmmo = g_pWeaponDB->GetRecordLink( hCurWeaponData, WDB_WEAPON_rDefaultMeleeAmmo );
				}
			}

			if( hNewAmmo )
			{
				g_pPlayerStats->UpdateAmmo( pMeleeWeapon->GetWeaponRecord( ), hNewAmmo, 1000, false, false, WDB_INVALID_WEAPON_INDEX );
				pMeleeWeapon->ChangeAmmoImmediate( hNewAmmo, 1000 );
			}

			if( pArgList->argc > 2 )
			{
				const char *pszNode = pArgList->argv[2];
				if( pszNode && pszNode[0] )
				{
					pMeleeWeapon->SetFireNode( pszNode );
				}
			}

			pMeleeWeapon->Fire( );

			// Reset values that could have changed from the fire string...
			pMeleeWeapon->ChangeAmmoImmediate( hCurAmmo, 1000 );
			pMeleeWeapon->SetFireNode( "" );

			// Switch back to the current weapon.
			pWeapon->ChangeAmmoImmediate( pWeapon->GetAmmoRecord( ));
		}
// !TEMP
	}
	else if( tok == s_cTok_PlayerBodyKeyThrow )
	{
// TEMP
		// Only care about frame strings from the upper tracker when playing a fullbody animation..
		if( (m_pUpperAnimationContext->GetDescriptor( kADG_Tracker ) == kAD_None) && 
			(hTrackerID == m_pLowerAnimationContext->GetTrackerID( )) )
			return true;

		// Use our grenade weapon to fire...
		// Need to design a better system after prototype...
		// PLAYER_BODY - Temp: Kill the current weapon loop sound... the weapons alt-fire state should handle this..
		CClientWeapon *pWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
		if( !pWeapon )
			return false;

		pWeapon->KillLoopSound( );

		// THROW
		m_pGrenadeWeapon->Fire( );

		if (GetConsoleBool("TestGrenadeThrow",false))
		{
			double fElapse = TestFireTimer.GetElapseTime() - fLastTestFireTime;
			fLastTestFireTime = TestFireTimer.GetElapseTime();
			nTestGrenadesThrown++;
			DebugCPrint(0,"Grenade thrown: %0.3f (%0.3f)",fLastTestFireTime,fElapse);
		}

		m_pGrenadeWeapon->SetVisible(false );

		int32 nAmmo = g_pPlayerStats->GetAmmoCount( m_pGrenadeWeapon->GetAmmoRecord() );
		if (!g_bInfiniteAmmo)
		{
			nAmmo--;
		}

		g_pPlayerStats->UpdateAmmo( m_pGrenadeWeapon->GetWeaponRecord( ), m_pGrenadeWeapon->GetAmmoRecord(), 
			nAmmo, false, true, WDB_INVALID_WEAPON_INDEX );

		g_pHUDMgr->QueueUpdate(kHUDGrenade);
// !TEMP
	}
	else if( tok == s_cTok_PlayerBodyKeyGrenade )
	{
		// TEMP
		//make sure we have the right grenade
		if (m_pGrenadeWeapon->GetWeaponRecord() != g_pPlayerStats->GetCurrentGrenadeRecord())
		{
			m_pGrenadeWeapon->Init(g_pPlayerStats->GetCurrentGrenadeRecord());
			m_pGrenadeWeapon->ResetWeaponFilenames( );
		}

//		m_pGrenadeWeapon->SetVisible(true);
	}
	else if( tok == s_cTok_PlayerBodyKeyFireGrenade )
	{
		m_pGrenadeWeapon->OffhandFire();
	}
	else if( tok == s_cTok_PlayerBodyKeyKillProjectiles )
	{
		//!!ARL: Should really send a single msg and iterate on the server,
		// but there's currently no good way to iterate projectiles on the server.

		CSpecialFXList* pList = g_pGameClientShell->GetSFXMgr()->GetFXList(SFX_PROJECTILE_ID);	
		if (pList->GetNumItems() > 0)
		{
			int nNumSFX  = pList->GetSize();
			for (int nProj=0; nProj < nNumSFX; nProj++)
			{
				CProjectileFX* pProj = (CProjectileFX*)(*pList)[nProj];
				if (pProj && pProj->GetShooter() == g_pLTClient->GetClientObject())
				{
					CAutoMessage cMsg;
					cMsg.Writeuint8( MID_OBJECT_MESSAGE );
					cMsg.WriteObject( g_pMoveMgr->GetServerObject() );
					cMsg.Writeuint32( MID_SFX_MESSAGE );
					cMsg.Writeuint8( SFX_CHARACTER_ID );
					cMsg.Writeuint8( CFX_KILLPROJECTILE_MSG );
					cMsg.WriteObject( pProj->GetServerObj() );
					g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
				}
			}
		}
	}
	else if( tok == s_cTok_PlayerBodyKeySoundBute )
	{
		// Only care about frame strings from the upper tracker when playing a fullbody animation..
		if( (m_pUpperAnimationContext->GetDescriptor( kADG_Tracker ) == kAD_None) && 
			(hTrackerID == m_pLowerAnimationContext->GetTrackerID( )) )
			return true;

		if( pArgList->argc > 1 && pArgList->argv[1] )
		{
			g_pClientSoundMgr->PlayDBSoundLocal( g_pSoundDB->GetSoundDBRecord(pArgList->argv[1]),
				SOUNDPRIORITY_INVALID, 0,SMGR_INVALID_VOLUME, 1.0f, 
				DEFAULT_SOUND_CLASS, PLAYSOUND_MIX_WEAPON_IMPACTS );
		}
	}
	else if( tok == s_cTok_PlayerBodyKeySonic )
	{
		if( ( pArgList->argc > 1 ) && pArgList->argv[ 1 ] )
		{
			g_pPlayerMgr->IntoneSonic( pArgList->argv[1] );
		}
	}
	else if( tok == s_cTok_PlayerBodyKeyActivate )
	{
		g_pPlayerMgr->DoActivate();
	}
	else if( tok == s_cTok_PlayerBodyFXKeyBlockWindow )
	{
		if( pArgList->argc > 1 )
		{
			int nDurationMS = atoi( pArgList->argv[ 1 ] );
			float flDurationS = nDurationMS / 1000.f;

			// Notify the server, so the AI can make decisions.

			CAutoMessage cMsg;
			cMsg.Writeuint8( MID_OBJECT_MESSAGE );
			cMsg.WriteObject( g_pMoveMgr->GetServerObject() );
			cMsg.Writeuint32( MID_SFX_MESSAGE );
			cMsg.Writeuint8( SFX_CHARACTER_ID );
			cMsg.WriteBits(CFX_BLOCKWINDOW_MSG, FNumBitsExclusive<CFX_COUNT>::k_nValue );
			cMsg.Writefloat( flDurationS );
			g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );

			return false;
		}
	}
	else if( tok == s_cTok_PlayerBodyFXKeyDodgeWindow )
	{
		if( pArgList->argc > 1 )
		{
			int nDurationMS = atoi( pArgList->argv[ 1 ] );
			float flDurationS = nDurationMS / 1000.f;

			// Notify the server, so the AI can make decisions.

			CAutoMessage cMsg;
			cMsg.Writeuint8( MID_OBJECT_MESSAGE );
			cMsg.WriteObject( g_pMoveMgr->GetServerObject() );
			cMsg.Writeuint32( MID_SFX_MESSAGE );
			cMsg.Writeuint8( SFX_CHARACTER_ID );
			cMsg.WriteBits(CFX_DODGEWINDOW_MSG, FNumBitsExclusive<CFX_COUNT>::k_nValue );
			cMsg.Writefloat( flDurationS );
			g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );

			return false;
		}
	}
	else if ( tok == s_cTok_PlayerBodyFXKeyMeleeControl )
	{
		// Find our CharacterFX
		HOBJECT hServerObject = g_pLTClient->GetClientObject();
		CCharacterFX* pFX = g_pGameClientShell->GetSFXMgr()->GetCharacterFX(hServerObject);
		if( pFX )
		{
			pFX->HandleMeleeModelKey(hObj, pArgList, hTrackerID);
		}
	}
	else if ( tok == s_cTok_PlayerBodyFXKeyBerserkerKick )
	{
		// Since this is usually played on the main context (which really means
		// it gets played on both upper and lower contexts) only use the model
		// key from the lower context to avoid sending duplicate messages.
		if (hTrackerID == m_pLowerAnimationContext->GetTrackerID())
		{
			CAutoMessage cMsg;
			cMsg.Writeuint8( MID_OBJECT_MESSAGE );
			cMsg.WriteObject( g_pMoveMgr->GetServerObject() );
			cMsg.Writeuint32( MID_SFX_MESSAGE );
			cMsg.Writeuint8( SFX_CHARACTER_ID );
			cMsg.Writeuint8( CFX_BERSERKERKICK_MSG );
			cMsg.WriteObject( g_pPlayerMgr->GetSyncObject() );
			g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
		}
	}
	else if ( tok == s_cTok_PlayerBodyFXKeyFinishingMove )
	{
		// Since this is usually played on the main context (which really means
		// it gets played on both upper and lower contexts) only use the model
		// key from the lower context to avoid sending duplicate messages.
		if (hTrackerID == m_pLowerAnimationContext->GetTrackerID())
		{
			HOBJECT	hTarget = g_pPlayerMgr->GetTargetMgr()->GetEnemyTarget();
			if (hTarget)
			{
				if( pArgList->argc > 1 )
				{
					HRECORD hSyncAction = g_pModelsDB->GetSyncActionRecord(pArgList->argv[1]);
					if (hSyncAction)
					{
						CAutoMessage cMsg;
						cMsg.Writeuint8( MID_OBJECT_MESSAGE );
						cMsg.WriteObject( g_pMoveMgr->GetServerObject() );
						cMsg.Writeuint32( MID_SFX_MESSAGE );
						cMsg.Writeuint8( SFX_CHARACTER_ID );
						cMsg.Writeuint8( CFX_FINISHINGMOVE_MSG );
						cMsg.WriteObject( hTarget );
						cMsg.WriteDatabaseRecord( g_pLTDatabase, hSyncAction );
						g_pLTClient->SendToServer( cMsg.Read(), MESSAGE_GUARANTEED );
					}
					else
					{
						char szError[256];
						LTSNPrintF( szError, LTARRAYSIZE( szError ), "FINISHINGMOVE: SyncAction record not found! ('%s')", pArgList->argv[1] );
						LTERROR( szError );
					}
				}
				else
				{
					LTERROR("FINISHINGMOVE: Not enough parameters!");
				}
			}
			else
			{
				LTERROR("FINISHINGMOVE: Player has no target!");
			}
		}
	}
	else if ( tok == s_cTok_PlayerBodyKeyCustomWeaponSelect )
	{
		// bring out the custom weapon

		// Figure out the correct tool.
		CForensicObjectFX* pFX = g_pPlayerMgr->GetForensicObject();
		if (pFX)
		{
			HWEAPON hTool = pFX->m_cs.m_rDetectionTool;
			if(hTool)
			{
				g_pClientWeaponMgr->SelectCustomWeapon(hTool);
			}
		}
	}
	else if ( tok == s_cTok_PlayerBodyKeyCloseEncounter )
	{
		m_fCloseEncounterEndTime = 0.0f;

		// optional end time
		if( pArgList->argc > 1 )
		{
			int nDurationMS = atoi( pArgList->argv[ 1 ] );
			m_fCloseEncounterEndTime = g_pLTClient->GetTime() + (nDurationMS / 1000.f);
		}

		float fNearZ = 1.0f;

		// optional nearz override

		if( pArgList->argc > 2 )
		{
			fNearZ = (float)atof( pArgList->argv[ 2 ] );
		}

		m_bInCloseEncounter = true;
		m_nCloseEncounterTrackerID = hTrackerID;
		g_pModelLT->GetCurAnim(m_hPlayerBody, hTrackerID, m_hCloseEncounterAnim);
		m_fCloseEncounterOldNearZ = g_pLTClient->GetConsoleVariableFloat(g_pLTClient->GetConsoleVariable("NearZ"));
		g_pLTClient->SetConsoleVariableFloat("NearZ", fNearZ);
	}
	else if ( tok == s_cTok_PlayerBodyKeyFinish )
	{
		if( hTrackerID == m_pLowerAnimationContext->GetTrackerID() )
		{
			CClientWeapon *pWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
			if( !pWeapon )
				return false;

			HOBJECT	hTarget = g_pPlayerMgr->GetTargetMgr()->GetEnemyTarget();
			if ( hTarget )
			{
				LTVector vTargetPos;
				g_pLTClient->GetObjectPos( hTarget, &vTargetPos );
				pWeapon->SendFinishMessage( hTarget, 0.0f, LTVector( 0, -1, 0 ), vTargetPos );
			}
		}		
	}
	else
	{
		if( hTrackerID == m_pUpperAnimationContext->GetTrackerID() )
		{
			CClientWeapon *pWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
			if( !pWeapon )
				return false;

			pWeapon->HandleModelKeyFromPlayerBody( m_hPlayerBody, hTrackerID, pArgList );
		}

		if( m_pCustomAnimationContext && hTrackerID == m_pCustomAnimationContext->GetTrackerID() )
		{
			CClientWeapon *pWeapon = g_pClientWeaponMgr->GetVisibleCustomWeapon();
			if( !pWeapon )
				return false;

			pWeapon->HandleModelKeyFromPlayerBody( m_hPlayerBody, hTrackerID, pArgList );
		}
	}

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::WriteAnimationInfo
//
//	PURPOSE:	Write the animation info to be sent to the player on the server...
//
// ----------------------------------------------------------------------- //

bool CPlayerBodyMgr::WriteAnimationInfo( ILTMessage_Write *pMsg )
{
	if( !pMsg || !m_hPlayerBody )
		return false;

	if( !m_pMainAnimationContext || !m_pLowerAnimationContext || !m_pUpperAnimationContext )
		return false;

	CAnimationContext *apContexts[kNumContexts] = { m_pMainAnimationContext,
													m_pUpperAnimationContext,
													m_pLowerAnimationContext,
													m_pCustomAnimationContext };

	uint32 cAnimTrees = g_pModelsDB->GetNumModelAnimationTrees( m_hModel );

	if( m_bEnabled )
	{
		bool bSendAnimTime = m_bForceSendAnimTime;
		m_bForceSendAnimTime = false;
		m_fSendAnimTime += RealTimeTimer::Instance().GetTimerElapsedS();
		
		// No need to constantly send the anim time...
		if( !m_pMainAnimationContext->IsEnabled() && (m_fSendAnimTime > g_vtPlayerBodySendAnimTimeRate.GetFloat( )) )
		{
			m_fSendAnimTime = 0.0f;
			
			// If nothing relies on the animation time... don't send it
			if( (m_pUpperAnimationContext->GetDescriptor( kADG_Synchronize ) != kAD_None) ||
				(m_pLowerAnimationContext->GetDescriptor( kADG_Synchronize ) != kAD_None) ||
				(m_pCustomAnimationContext && m_pCustomAnimationContext->GetDescriptor( kADG_Synchronize ) != kAD_None ))
			{
				bSendAnimTime = true;
			}
		}

// PLAYER_BODY
		pMsg->Writebool( true );
// !PLAYER_BODY

		// Write the total number of contexts...
		pMsg->Writeuint8( cAnimTrees );

		// Write the info for each context...
		for( uint8 nContext = 0; nContext < cAnimTrees; ++nContext )
		{
			if( !apContexts[nContext] )
				continue;

			ANIMTRACKERID trkID = MAIN_TRACKER;
			HMODELANIM hAnim = INVALID_MODEL_ANIM;
			HMODELWEIGHTSET hWeightSet = INVALID_MODEL_WEIGHTSET;

			// Write out the tracker...
			trkID = apContexts[nContext]->GetTrackerID();
			pMsg->Writeuint8( trkID );
		
			// Don't bother writing out the weightset for the main tracker...
			if( trkID != MAIN_TRACKER )
			{
				// Write out the weight set...
				g_pModelLT->GetWeightSet( m_hPlayerBody, trkID, hWeightSet );
				pMsg->Writeuint32( hWeightSet );
			}

			// Write the tracker info if it's enabled...
			bool bEnabled = apContexts[nContext]->IsEnabled();
			pMsg->Writebool( bEnabled );

			if( bEnabled )
			{
				// Get the non-local versions of the animations if they exist...
				if( IsMultiplayerGameClient( ) && apContexts[nContext]->PropsChanged( ))
				{
					CAnimationProps apNonLocal;
					apContexts[nContext]->GetCurrentProps( &apNonLocal );
					apNonLocal.Set( kAPG_Client, kAP_CLIENT_NonLocal );

					hAnim = apContexts[nContext]->GetAni( apNonLocal, false );
				}

				if( hAnim == INVALID_MODEL_ANIM )
				{
					// Write out the animation that is currently playing on the tracker...
					g_pModelLT->GetCurAnim( m_hPlayerBody, trkID, hAnim );
				}

				pMsg->Writeuint32( hAnim );

				// Write the animation time, for synchronizing...
				if( bSendAnimTime )
				{
					pMsg->Writebool( true );

					uint32 nAnimTime = 0;
					g_pModelLT->GetCurAnimTime( m_hPlayerBody, trkID, nAnimTime );
					pMsg->Writeuint32( nAnimTime );
				}
				else
				{
					pMsg->Writebool( false );
				}

				if( IsMultiplayerGameClient( ) && (trkID == kAD_TRK_Lower) )
				{
					// In multiplayer games Lowerbody should always loop to avoid "dead" animations...
					pMsg->Writebool( true );
				}
				else
				{
					bool bLooping = (g_pModelLT->GetLooping( m_hPlayerBody, trkID ) == LT_YES);
					pMsg->Writebool( bLooping );
				}

				float fRate = apContexts[nContext]->GetAnimRate();
				if (LTNearlyEquals(fRate,1.0f,MATH_EPSILON))
  				{
  					pMsg->Writebool( false );
 				}
				else
  				{
  					pMsg->Writebool( true );
  					pMsg->Writefloat(fRate);
  				}

			}
		}

		// Write the dims tracker...
		if( m_pLowerAnimationContext->IsEnabled() )
		{
			pMsg->Writeuint8( m_pLowerAnimationContext->GetTrackerID() );
		}
		else
		{
			pMsg->Writeuint8( MAIN_TRACKER );
		}
	}
	else
	{
// PLAYER_BODY - This entire block will go away once the CPlayerAnimatior bites the dust!
//				 For now we need to send the information so the two systems still work properly...
		pMsg->Writebool( false );

		// Write the total number of contexts...
		pMsg->Writeuint8( cAnimTrees );

		// Write the info for each context...
		for( uint8 nContext = 0; nContext < cAnimTrees; ++nContext )
		{
			ANIMTRACKERID trkID = MAIN_TRACKER;
			HMODELWEIGHTSET hWeightSet = INVALID_MODEL_WEIGHTSET;

			// Write out the tracker...
			trkID = apContexts[nContext]->GetTrackerID();
			pMsg->Writeuint8( trkID );

			// Don't bother writing out the weightset for the main tracker...
			if( trkID != MAIN_TRACKER )
			{
				// Write out the weight set...
				g_pModelLT->GetWeightSet( m_hPlayerBody, trkID, hWeightSet );
				pMsg->Writeuint32( hWeightSet );
			}

			// Write the tracker info if it's enabled...
			bool bEnabled = apContexts[nContext]->IsEnabled();
			pMsg->Writebool( bEnabled );
		}
	}
	
	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::Enable
//
//	PURPOSE:	...
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::Enable()
{
	if( m_bEnabled )
		return;

	m_bStartPlayerBody = m_bEnabled = true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::Disable
//
//	PURPOSE:	...
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::Disable()
{
	if( !m_bEnabled )
		return;

	m_bStartPlayerBody = m_bEnabled = false;

	if( m_hPlayerBody )
	{
		// Hide the move mgrs model and shadow...
		g_pCommonLT->SetObjectFlags( m_hPlayerBody, OFT_Client, 0, CF_NOTIFYMODELKEYS );
		g_pCommonLT->SetObjectFlags( m_hPlayerBody, OFT_Flags, 0, FLAG_VISIBLE);
		g_pLTClient->SetObjectShadowLOD( m_hPlayerBody, eEngineLOD_Never );

		// Show the client object model and shadow...
		g_pPlayerMgr->HideShowAttachments( g_pLTClient->GetClientObject() );
		g_pLTClient->SetObjectShadowLOD( g_pLTClient->GetClientObject(), PLAYER_SHADOW_LOD );
		g_pPlayerMgr->GetPlayerCamera()->AttachToTarget(false);

		if( m_pMainAnimationContext && m_pLowerAnimationContext && m_pUpperAnimationContext )
		{
			// Disable all of the trackers...
			m_pMainAnimationContext->Disable();
			m_pLowerAnimationContext->Disable();
			m_pUpperAnimationContext->Disable();
		}

		if( m_pCustomAnimationContext )
			m_pCustomAnimationContext->Disable();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::Disable
//
//	PURPOSE:	Handle leaving a world...
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::OnExitWorld()
{
	Term();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::UpdateAimTracking
//
//	PURPOSE:	Update the aim tracking...
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::UpdateAimTracking()
{
	// Get pos and rot for the camera to build the position to look at...
	LTRotation const& rCamRot = g_pPlayerMgr->GetPlayerCamera()->GetCameraRotation( );
	LTVector const&	vCamPos = g_pPlayerMgr->GetPlayerCamera()->GetCameraPos( );

	// Toso tracking uses the aiming group...

	if( GetCameraDescriptor( ) == kAD_CAM_Rotation ||
		LadderMgr::Instance().IsClimbing()  || 
		g_pPlayerMgr->IsOperatingTurret( ))
	{
		m_NodeTrackerContext.DisableTrackerGroup( kTrackerGroup_AimAt, true );
	}
	else
	{
		m_NodeTrackerContext.EnableTrackerGroup( kTrackerGroup_AimAt );
	}

	bool bEntryToolTracking = false;
	CSpecialMoveFX* pSpecialMove = NULL;

	CClientWeapon *pClientWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
	if( pClientWeapon )
	{
		CActivationData data = g_pPlayerMgr->GetTargetMgr()->GetActivationData();
		if (data.m_nType == MID_ACTIVATE_SPECIALMOVE)
		{
			pSpecialMove = g_pGameClientShell->GetSFXMgr()->GetSpecialMoveFX(data.m_hTarget);
			bEntryToolTracking = (pSpecialMove && (pSpecialMove->GetSFXID() == SFX_ENTRYTOOLLOCK_ID)
				&& (pClientWeapon->GetWeaponRecord() == ((CEntryToolLockFX*)pSpecialMove)->m_cs.m_rEntryTool));
		}
	}

	if (bEntryToolTracking)
	{
		m_NodeTrackerContext.EnableTrackerGroup( kTrackerGroup_Arm );

		HMODELNODE hNode;
		g_pLTClient->GetModelLT()->GetNode(m_hPlayerBody, "Aimer", hNode);

		LTTransform tNode;
		g_pLTClient->GetModelLT()->GetNodeTransform(m_hPlayerBody, hNode, tNode, true);

		LTVector vPos = pSpecialMove->m_vPos;
		vPos.y = tNode.m_vPos.y;

		m_NodeTrackerContext.SetTrackedTarget( kTrackerGroup_Arm, vPos );
	}
	else
	{
		m_NodeTrackerContext.DisableTrackerGroup( kTrackerGroup_Arm, true );
	}

	// See if we have an object we want to aim at...
	if( CBindMgr::GetSingleton().IsCommandOn( COMMAND_ID_FOCUS ) && g_iFocusObjectDetector.GetObject() )
	{
		LTVector vFocusObjectPos = g_iFocusObjectDetector.GetObjectSpatialPosition();
		m_NodeTrackerContext.SetTrackedTarget( kTrackerGroup_AimAt, vFocusObjectPos );
	}
	else
	{
		//if we're getting on a ladder, the aim tracker should be disabled
		//  but we need to set the tracked target based on the ladder rotation so that the 
		//  NodeTracker will handle its shutdown properly
		if (LadderMgr::Instance().IsClimbing())
		{
			CLadderFX *pLadder = LadderMgr::Instance().GetLadder();
			LTRotation rRot = pLadder->GetRotation();
			//rotate halfway around
			LTVector vUp = rRot.Up();
			rRot.Rotate(vUp,MATH_PI);
			m_NodeTrackerContext.SetTrackedTarget( kTrackerGroup_AimAt, vCamPos + ( rRot.Forward() * 10000.0f ) );
		}
		else
		{
			m_NodeTrackerContext.SetTrackedTarget( kTrackerGroup_AimAt, vCamPos + ( rCamRot.Forward() * 10000.0f ) );
		}
	}

	g_pPlayerMgr->GetLeanMgr( )->Update( );
	m_NodeTrackerContext.UpdateNodeTrackers();

	if( m_NodeTrackerContext.DidAimAtTarget( kTrackerGroup_AimAt ))
	{
		SetAimTrackingDefaultMaxSpeed( );
	}	
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::GetAimTrackingLimits
//
//	PURPOSE:	Retrieve the limits of aim tracking...
//
// ----------------------------------------------------------------------- //

bool CPlayerBodyMgr::GetAimTrackingLimits( LTRect2f &rLimits )
{
	return m_NodeTrackerContext.GetLimits( kTrackerGroup_AimAt, rLimits );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::SetAimTrackingLimits
//
//	PURPOSE:	Specify the limits of aim tracking...
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::SetAimTrackingLimits( const LTRect2f &rLimits )
{
	m_NodeTrackerContext.SetLimits( kTrackerGroup_AimAt, rLimits );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::SetAimTrackingDefaultLimits
//
//	PURPOSE:	Set the limits for the aim tracking to the default values...
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::SetAimTrackingDefaultLimits( )
{
	m_NodeTrackerContext.SetDefaultLimits( kTrackerGroup_AimAt );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::SetAimTrackingMaxSpeed
//
//	PURPOSE:	Specify the maximum speed used to reach the limits of the aim tracking...
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::SetAimTrackingMaxSpeed( float fMaxSpeed )
{
	m_NodeTrackerContext.SetMaxSpeed( kTrackerGroup_AimAt, MATH_DEGREES_TO_RADIANS(fMaxSpeed) );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::SetAimTrackingDefaultMaxSpeed
//
//	PURPOSE:	Set the maximum speed for the aim tracking to the default value...
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::SetAimTrackingDefaultMaxSpeed( )
{
	m_NodeTrackerContext.SetDefaultMaxSpeed( kTrackerGroup_AimAt );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::HidePlayerBody
//
//	PURPOSE:	Hide or show the player body model...
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::HidePlayerBody( bool bHide, bool bControlShadow /*=true*/ )
{
	g_pCommonLT->SetObjectFlags( m_hPlayerBody, OFT_Flags, (bHide ? 0 : FLAG_VISIBLE), FLAG_VISIBLE );
	
	if( bControlShadow )
		g_pLTClient->SetObjectShadowLOD( m_hPlayerBody, (bHide ? eEngineLOD_Never : PLAYER_SHADOW_LOD) );

	if (bHide && m_pGrenadeWeapon)
	{
		m_pGrenadeWeapon->SetVisible(false);
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::IsPlayerBodyHidden
//
//	PURPOSE:	Check if playerbody is hidden.
//
// ----------------------------------------------------------------------- //

bool CPlayerBodyMgr::IsPlayerBodyHidden( ) const
{
	uint32 nFlags = 0;
	g_pCommonLT->GetObjectFlags( m_hPlayerBody, OFT_Flags, nFlags );
	return !( nFlags & FLAG_VISIBLE );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::IsLocked
//
//	PURPOSE:	Test to see if any of the contexts are locked and have the specified property set...
//
// ----------------------------------------------------------------------- //

bool CPlayerBodyMgr::IsLocked( EnumAnimPropGroup eAnimPropGroup, EnumAnimProp eAnimProp )
{
	// Make sure the property is in the list of locked properties...
	if( m_MainLockedAnimProps.IsSet( eAnimPropGroup, eAnimProp ))
		return true;

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::UpdateLockedAnimationProperties
//
//	PURPOSE:	Update the locked animation properties (ie, clear them if no longer locked)...
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::UpdateLockedAnimationProperties()
{
	// Don't update if no locked properties are currently set...
	if( m_MainLockedAnimProps.IsClear() )
		return;

	for( uint32 nGroup = 0; nGroup < kAPG_Count; ++nGroup )
	{
		EnumAnimPropGroup	eGroup = (EnumAnimPropGroup)nGroup;
		EnumAnimProp		eProp = m_MainLockedAnimProps.Get( eGroup );

		// If none of the animation contexts have the property locked, remove it from the list...
		if( IsLocked( eGroup, eProp ))
		{
			// Check each context to see if it is enabled, locked and has the property set...
			
			bool bLocked = false;

			if( m_pMainAnimationContext->IsEnabled() && m_pMainAnimationContext->IsLocked() )
			{
				if( m_pMainAnimationContext->GetCurrentProp( eGroup ) == eProp )
					bLocked = true;
			}
			else if( m_pMainAnimationContext->WasLocked() )
			{
				m_pMainAnimationContext->ClearLock();
				m_MainLockedAnimProps.Clear( );
			}

			if( m_pUpperAnimationContext->IsEnabled() && m_pUpperAnimationContext->IsLocked() )
			{
				if( m_pUpperAnimationContext->GetCurrentProp( eGroup ) == eProp )
					bLocked = true;
			}
			else if( m_pUpperAnimationContext->WasLocked())
			{
				m_pUpperAnimationContext->ClearLock();
				m_UpperLockedAnimProps.Clear( );
			}

			if( m_pLowerAnimationContext->IsEnabled() && m_pLowerAnimationContext->IsLocked() )
			{
				if( m_pLowerAnimationContext->GetCurrentProp( eGroup ) == eProp )
					bLocked = true;
			}
			else if( m_pLowerAnimationContext->WasLocked() )
			{
				m_pLowerAnimationContext->ClearLock();
				m_LowerLockedAnimProps.Clear( );
			}

			if( m_pCustomAnimationContext )
			{
				if( m_pCustomAnimationContext->IsEnabled() && m_pCustomAnimationContext->IsLocked() )
				{
					if( m_pCustomAnimationContext->GetCurrentProp( eGroup ) == eProp )
						bLocked = true;
				}
				else if( m_pCustomAnimationContext->WasLocked())
				{
					m_pCustomAnimationContext->ClearLock();
					m_CustomLockedAnimProps.Clear( );
				}
			}

			// The property is no longer locked so clear it out from the locked properties...
			if( !bLocked )
			{
				m_MainLockedAnimProps.Clear( eGroup );
				m_UpperLockedAnimProps.Clear( eGroup );
				m_LowerLockedAnimProps.Clear( eGroup );
				m_CustomLockedAnimProps.Clear( eGroup );
			}
		}
	}

	if( m_MainLockedAnimProps.IsClear( ))
	{
		// Start the contexts off fresh after a locked anim is finished...
		m_pMainAnimationContext->ClearProps( );
		m_pUpperAnimationContext->ClearProps( );
		m_pLowerAnimationContext->ClearProps( );
		if( m_pCustomAnimationContext )
			m_pCustomAnimationContext->ClearProps( );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::GetCurrentAnimProp
//
//	PURPOSE:	Retrieve the current animatio property for the specified group of the specified tracker...
//
// ----------------------------------------------------------------------- //

EnumAnimProp CPlayerBodyMgr::GetCurrentAnimProp( PlayerBodyContext eContext, EnumAnimPropGroup eAnimPropGroup )
{
	switch( eContext )
	{
		case kMainContext:
		{
			return m_pMainAnimationContext->GetCurrentProp( eAnimPropGroup );
		}
		break;

		case kUpperContext:
		{
			return m_pUpperAnimationContext->GetCurrentProp( eAnimPropGroup );
		}
		break;

		case kLowerContext:
		{
			return m_pLowerAnimationContext->GetCurrentProp( eAnimPropGroup );
		}
		break;

		case kCustomContext:
		{
			if( !m_pCustomAnimationContext )
				return kAP_Invalid; 
			return m_pCustomAnimationContext->GetCurrentProp( eAnimPropGroup );
		}
		break;

		default:
		break;
	};

	return kAP_Invalid;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::GetWeaponContext
//
//	PURPOSE:	Get the current context used for weapon animations...
//
// ----------------------------------------------------------------------- //

CPlayerBodyMgr::PlayerBodyContext CPlayerBodyMgr::GetWeaponContext( )
{
	// Weapon context is always upper...
	return kUpperContext;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::CountAnimations
//
//	PURPOSE:	Count how many animations the context has with the animation properties set...
//
// ----------------------------------------------------------------------- //

uint32 CPlayerBodyMgr::CountAnimations( PlayerBodyContext eContext )
{
	CAnimationContext *pContext = NULL;
	switch( eContext )
	{
		case kMainContext:	pContext = m_pMainAnimationContext;		break;
		case kLowerContext:	pContext = m_pLowerAnimationContext;	break;
		case kUpperContext:	pContext = m_pUpperAnimationContext;	break;
		case kCustomContext:	pContext = m_pCustomAnimationContext;	break;

		default:
		break;
	};

	LTASSERT( pContext, "ERROR - CPlayerBodyMgr::CountAnimations: Invalid context!" );
	if( !pContext )
		return 0;

	// Check anis using the currently set prpoerties...
	uint32 nNumAnis = pContext->CountAnimations( pContext->GetProps() );
	return nNumAnis;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::UpdateLookContextProperties
//
//	PURPOSE:	Update the property for look context sensitivity...
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::UpdateLookContextProperties( )
{
	// Determine the generic look context by checking values against the cameras pitch...
	EulerAngles EA = Eul_FromQuat( g_pPlayerMgr->GetPlayerCamera( )->GetLocalCameraRotation( ), EulOrdYXZr );
	float fPitch = EA.y;

	if( fPitch > g_vtPlayerBodyLookLow.GetFloat() )
	{
		// Looking below mid section...
		SetAnimProp( kAPG_LookContext, kAP_LOOK_Low );
	}
	else if( fPitch < g_vtPlayerBodyLookHigh.GetFloat() )
	{
		// Looking above mid section...
		SetAnimProp( kAPG_LookContext, kAP_LOOK_High );
	}
	else
	{
		// Looking at mid section...
		SetAnimProp( kAPG_LookContext, kAP_LOOK_Mid );
	}

	if( g_vtPlayerBodyShowInfo.GetFloat() > 0.0f )
	{
		g_pLTClient->CPrint( "Pitch: %f", fPitch );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::UpdateActionProperties
//
//	PURPOSE:	Update the property for action...
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::UpdateActionProperties( )
{
	if( g_pPlayerMgr->IsCarryingObject( ))
	{
		ClearLockedProperties( kAPG_Action );
		SetAnimProp( kAPG_Action, kAP_ACT_Carrying );
		return;
	}

	bool bJump		= (m_AnimProps.IsSet( kAPG_Movement, kAP_MOV_Jump ) || IsLocked( kAPG_Movement, kAP_MOV_Jump ));
	bool bFall		= (m_AnimProps.IsSet( kAPG_Movement, kAP_MOV_Fall ) || IsLocked( kAPG_Movement, kAP_MOV_Fall ));
	bool bAltFire	= (m_AnimProps.IsSet( kAPG_Action, kAP_ACT_FireSecondary ) || IsLocked( kAPG_Action, kAP_ACT_FireSecondary ));
	bool bDir		= !m_AnimProps.IsSet( kAPG_MovementDir, kAP_None );
	bool bCrouch	= m_AnimProps.IsSet( kAPG_Posture, kAP_POS_Crouch );

	// Determine what action to perform based on the current context...

	if( bAltFire )
	{
		if( bCrouch )
		{
			// [RP] 12/07/04 Crouch idle kick has been removed since the animation wasn't able to look correct.
			//		Leaving this here, just commented out, in case a different animation can be created for it.
/*
			if( !bDir && !IsLocked( kAPG_Action, kAP_ACT_CrouchIdleKick ))
			{
				ClearLockedProperties( kAPG_Action );
				ClearLockedProperties( kAPG_Movement );

				m_pUpperAnimationContext->ClearLock( );
				m_pLowerAnimationContext->ClearLock( );
				if( m_pCustomAnimationContext )
					m_pCustomAnimationContext->ClearLock( );

				SetAnimProp( kAPG_Movement, kAP_None, kUnlocked );
				SetAnimProp( kAPG_Action, kAP_ACT_CrouchIdleKick, kLocked );
			}
			else
*/			
			if( m_AnimProps.IsSet( kAPG_MovementDir, kAP_MDIR_Forward ) &&
				g_pMoveMgr->InPostureDownWindow( ) && !g_pMoveMgr->InPostureUpWindow( ) &&
				!IsLocked( kAPG_Action, kAP_ACT_SlideKick ))
			{
				ClearLockedProperties( kAPG_Posture );
				ClearLockedProperties( kAPG_Action );
				ClearLockedProperties( kAPG_Movement );

				m_pUpperAnimationContext->ClearLock( );
				m_pLowerAnimationContext->ClearLock( );
				if( m_pCustomAnimationContext )
					m_pCustomAnimationContext->ClearLock( );

				SetAnimProp( kAPG_Posture, kAP_POS_Crouch, kUnlocked );
				SetAnimProp( kAPG_Movement, kAP_None, kUnlocked );
				SetAnimProp( kAPG_Action, kAP_ACT_SlideKick, kLocked );

				// Clear the ducklock if one was set.
				g_pMoveMgr->SetDuckLock( false );
			}
		}
		else if( (bJump || bFall) && !g_pMoveMgr->SwimmingJumped( ))
		{
			if( !bDir )
			{
				ClearLockedProperties( kAPG_Action );
				ClearLockedProperties( kAPG_Movement );

				m_pUpperAnimationContext->ClearLock( );
				m_pLowerAnimationContext->ClearLock( );
				if( m_pCustomAnimationContext )
					m_pCustomAnimationContext->ClearLock( );

				SetAnimProp( kAPG_Movement, kAP_None, kUnlocked );
				SetAnimProp( kAPG_Action, kAP_ACT_JumpIdleKick, kLocked );
			}
			else if( m_AnimProps.IsSet( kAPG_MovementDir, kAP_MDIR_Forward ))
			{
				ClearLockedProperties( kAPG_Action );
				ClearLockedProperties( kAPG_Movement );

				m_pUpperAnimationContext->ClearLock( );
				m_pLowerAnimationContext->ClearLock( );
				if( m_pCustomAnimationContext )
					m_pCustomAnimationContext->ClearLock( );

				SetAnimProp( kAPG_Movement, kAP_None, kUnlocked );
				SetAnimProp( kAPG_Action, kAP_ACT_JumpRunKick, kLocked );
			}
		}
	}

	// Update conditional animation controllers...
	CClientWeapon* pWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
	if (pWeapon)
	{
		pWeapon->UpdateAnimControllers();
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::UpdateAnimationContexts
//
//	PURPOSE:	Actually update each animation context...
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::UpdateAnimationContexts( )
{
	if( g_vtPlayerBodyShowInfo.GetFloat() > 0.0f )
	{
		char szProps[256] = {'\0'};

		if( m_pMainAnimationContext->IsEnabled() )
		{
			m_pMainAnimationContext->GetPropString( szProps, LTARRAYSIZE(szProps) );
			g_pLTClient->CPrint( "PlayerBody Main props: %s", szProps );
		}
		else
		{
			g_pLTClient->CPrint( "PlayerBody Main - Disabled" );
		}

		if( m_pLowerAnimationContext->IsEnabled() )
		{
			m_pLowerAnimationContext->GetPropString( szProps, LTARRAYSIZE(szProps) );
			g_pLTClient->CPrint( "PlayerBody Lower props: %s", szProps );
		}
		else
		{
			g_pLTClient->CPrint( "PlayerBody Lower - Disabled" );
		}

		if( m_pUpperAnimationContext->IsEnabled() )
		{
			m_pUpperAnimationContext->GetPropString( szProps, LTARRAYSIZE(szProps) );
			g_pLTClient->CPrint( "PlayerBody Upper props: %s", szProps );
		}
		else
		{
			g_pLTClient->CPrint( "PlayerBody Upper - Disabled" );
		}

		if( m_pCustomAnimationContext )
		{
			if( m_pCustomAnimationContext->IsEnabled() )
			{
				m_pCustomAnimationContext->GetPropString( szProps, LTARRAYSIZE(szProps) );
				g_pLTClient->CPrint( "PlayerBody Custom props: %s", szProps );
			}
			else
			{
				g_pLTClient->CPrint( "PlayerBody Custom - Disabled" );
			}
		}
	}

	// Update all of our context...
	m_pMainAnimationContext->Update();
	m_pLowerAnimationContext->Update();
	m_pUpperAnimationContext->Update();
	if( m_pCustomAnimationContext )
		m_pCustomAnimationContext->Update();

	SetupMovementScaling();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::GetContextTrackerID
//
//	PURPOSE:	Retrieve the tracker ID for the given context...
//
// ----------------------------------------------------------------------- //

ANIMTRACKERID CPlayerBodyMgr::GetContextTrackerID( CPlayerBodyMgr::PlayerBodyContext eContext )
{
	CAnimationContext *pContext = NULL;
	switch( eContext )
	{
	case kMainContext:	pContext = m_pMainAnimationContext;		break;
	case kLowerContext:	pContext = m_pLowerAnimationContext;	break;
	case kUpperContext:	pContext = m_pUpperAnimationContext;	break;
	case kCustomContext:	pContext = m_pCustomAnimationContext;	break;

	default:
		break;
	};

	LTASSERT( pContext, "ERROR - CPlayerBodyMgr::GetContextTrackerID: Invalid context!" );
	if( !pContext )
		return MAIN_TRACKER;

	return pContext->GetTrackerID( );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::GetAnimationContext
//
//	PURPOSE:	Retrieve a specific context interface
//
// ----------------------------------------------------------------------- //

CAnimationContext* CPlayerBodyMgr::GetAnimationContext( PlayerBodyContext eContext )
{
	switch( eContext )
	{
		case kMainContext:		return m_pMainAnimationContext;
		case kLowerContext:		return m_pLowerAnimationContext;
		case kUpperContext:		return m_pUpperAnimationContext;
		case kCustomContext:		return m_pCustomAnimationContext;

		default:				LTERROR( "CPlayerBodyMgr::GetContext() -- Invalid context index!" ); return NULL;
	};
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::GetAnimationProps
//
//	PURPOSE:	Get the current animation props
//
// ----------------------------------------------------------------------- //

const CAnimationProps& CPlayerBodyMgr::GetAnimationProps()
{
	return m_AnimProps;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::UpdateWeaponModel
//
//	PURPOSE:	The weapon model should be plyaing the same animation as the PlayerBody if it has it...
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::UpdateWeaponModel( )
{
	CAnimationContext *pContext = NULL;
	switch( GetWeaponContext( ))
	{
	case kMainContext:	pContext = m_pMainAnimationContext;		break;
	case kLowerContext:	pContext = m_pLowerAnimationContext;	break;
	case kUpperContext:	pContext = m_pUpperAnimationContext;	break;
	case kCustomContext:	pContext = m_pCustomAnimationContext;	break;

	default:
		break;
	};

	LTASSERT( pContext, "ERROR - CPlayerBodyMgr::UpdateWeaponModel: Invalid weapon context!" );
	if( !pContext )
		return;

	// Make sure we have a valid current weapon...
	CClientWeapon *pWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon( );
	if( !pWeapon )
		return;
    
	uint32 dwAnimIndex = INVALID_MODEL_ANIM;
	g_pModelLT->GetCurAnim( m_hPlayerBody, pContext->GetTrackerID( ), dwAnimIndex );

	// Try and play the weapon animation if the PlayerBody animation changed...
	if( m_dwLastWeaponContextAnim != dwAnimIndex )
	{
		char szAnimName[64] = {0};
		g_pModelLT->GetAnimName( m_hPlayerBody, dwAnimIndex, szAnimName, LTARRAYSIZE(szAnimName) );

		pWeapon->SetWeaponModelAnimation( szAnimName );

		m_dwLastWeaponContextAnim = dwAnimIndex;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::UpdateGrenadePosition
//
//	PURPOSE:	Update the grenade attachment so that is in the player's hand
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::UpdateGrenadePosition( )
{
	if( g_vtPlayerBodyWeapons.GetFloat() >= 1.0f )
			m_pGrenadeWeapon->UpdateWeaponPosition( LTVector( 0.0f, 0.0f, 0.0f ));
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::UpdateMovementType
//
//	PURPOSE:	Update the movement type based on which contexts are enabled
//				and which animations are playing...
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::UpdateMovementType( )
{
	// The ordering of this array is important, priority goes to main, then lower...  
	const CAnimationContext *apContexts[kNumContexts] = { m_pMainAnimationContext,
														  m_pLowerAnimationContext,
														  m_pUpperAnimationContext,
														  m_pCustomAnimationContext };


	uint32 cAnimTrees = g_pModelsDB->GetNumModelAnimationTrees( m_hModel );

	// Go through the contexts in order and check if a movement encoded animation is playing...
	for( uint8 nContext = 0; nContext < cAnimTrees; ++nContext )
	{
		if( !apContexts[nContext] )
			continue;

		if( apContexts[nContext]->IsEnabled( ))
		{
			EnumAnimDesc eMovementType = apContexts[nContext]->GetAnimMovementType( );

			switch( eMovementType )
			{
				case kAD_None: 
				case kAD_MOV_Set:
				case kAD_MOV_Walk:
				case kAD_MOV_Run:
				case kAD_MOV_JumpUp:
				case kAD_MOV_JumpOver:
				case kAD_MOV_Fall:
				case kAD_MOV_Climb:
				case kAD_MOV_Swim:
				case kAD_MOV_Encode_GB:
				case kAD_MOV_Encode_GL:
				case kAD_MOV_Encode_GR:
				case kAD_MOV_Encode_V:
				{
					// PlayerBody is not currently taking advantage of the above movement types...
				}
				break;

				case kAD_MOV_Encode_NG:
				case kAD_MOV_Encode_G:
				case kAD_MOV_Encode_NP:
				case kAD_MOV_Encode_Velocity:
				{
					// The tracker is playing a movment encoded animation, this takes priority...
					g_pModelLT->SetMovementEncodingTracker( m_hPlayerBody, apContexts[nContext]->GetTrackerID( ));
					m_eMovementDescriptor = eMovementType;
					return;
				}
				break;

				default:
					LTERROR( "ERROR - CPlayerBodyMgr::UpdateMovementEncodingTracker: Unknown Movement type!" );
				break;
			}
		}
	}

	m_eMovementDescriptor = kAD_None;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::UpdateCameraType
//
//	PURPOSE:	Update the camera type based on which contexts are enabled and which animations are playing...
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::UpdateCameraType( )
{
	// The ordering of this array is important, priority goes to main, then lower...  
	const CAnimationContext *apContexts[kNumContexts] = { m_pMainAnimationContext,
														  m_pLowerAnimationContext,
														  m_pUpperAnimationContext,
														  m_pCustomAnimationContext };

	uint32 cAnimTrees = g_pModelsDB->GetNumModelAnimationTrees( m_hModel );


	// Go through the contexts in order and check if a movement encoded animation is playing...
	for( uint8 nContext = 0; nContext < cAnimTrees; ++nContext )
	{
		if( !apContexts[nContext] )
			continue;

		if( apContexts[nContext]->IsEnabled( ))
		{
			EnumAnimDesc eCameraType = apContexts[nContext]->GetDescriptor( kADG_Camera );

			switch( eCameraType )
			{
				case kAD_None: 
				{ 

				}
				break;

				case kAD_CAM_Rotation:
				case kAD_CAM_RotationAim:
				{
					m_eCameraDescriptor = eCameraType;
					return;
				}
				break;

				default:
					LTERROR( "ERROR - CPlayerBodyMgr::UpdateCameraType: Unknown Camera type!" );
				break;
			}
		}
	}

	m_eCameraDescriptor = kAD_None;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::UpdateInputType
//
//	PURPOSE:	Update the input based on which contexts are enabled and which animations are playing...
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::UpdateInputType( )
{
	// The ordering of this array is important, priority goes to main, then lower...  
	const CAnimationContext *apContexts[kNumContexts] = { m_pMainAnimationContext,
		m_pLowerAnimationContext,
		m_pUpperAnimationContext,
		m_pCustomAnimationContext };


	uint32 cAnimTrees = g_pModelsDB->GetNumModelAnimationTrees( m_hModel );

	// Go through the contexts in order and check for special input settings...
	for( uint8 nContext = 0; nContext < cAnimTrees; ++nContext )
	{
		if( !apContexts[nContext] )
			continue;

		if( apContexts[nContext]->IsEnabled( ))
		{
			EnumAnimDesc eInputType = apContexts[nContext]->GetDescriptor( kADG_Input );

			switch( eInputType )
			{
			case kAD_None: 
				{ 

				}
				break;

			case kAD_IN_Locked:
				{
					m_eInputDescriptor = eInputType;
					return;
				}
				break;

			default:
				LTERROR( "ERROR - CPlayerBodyMgr::UpdateInputType: Unknown Input type!" );
				break;
			}
		}
	}

	m_eInputDescriptor = kAD_None;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::IsMovementEncoded
//
//	PURPOSE:	Test the movement descriptor for movement encoding...
//
// ----------------------------------------------------------------------- //

bool CPlayerBodyMgr::IsMovementEncoded( )
{
	switch( m_eMovementDescriptor )
	{
		case kAD_None: 
		case kAD_MOV_Set:
		case kAD_MOV_Walk:
		case kAD_MOV_Run:
		case kAD_MOV_JumpUp:
		case kAD_MOV_JumpOver:
		case kAD_MOV_Fall:
		case kAD_MOV_Climb:
		case kAD_MOV_Swim:
		case kAD_MOV_Encode_GB:
		case kAD_MOV_Encode_GL:
		case kAD_MOV_Encode_GR:
		case kAD_MOV_Encode_V:
		{
			// PlayerBody is not currently taking advantage of the above movement types...
		}
		break;

		case kAD_MOV_Encode_NG:
		case kAD_MOV_Encode_G:
		case kAD_MOV_Encode_NP:
		case kAD_MOV_Encode_Velocity:
		{
			return true;
		}
		break;

		default:
			LTERROR( "ERROR - CPlayerBodyMgr::IsMovementEncoded: Unknown Movement type!" );
		break;
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::ClearLockedProperties
//
//	PURPOSE:	Clears the group from each locked properties...
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::ClearLockedProperties( EnumAnimPropGroup eGroup /* = kAPG_Invalid  */)
{
	if( eGroup == kAPG_Invalid )
	{
		m_MainLockedAnimProps.Clear( );
		m_UpperLockedAnimProps.Clear( );
		m_LowerLockedAnimProps.Clear( );
		m_CustomLockedAnimProps.Clear( );
	}
	else
	{
		m_MainLockedAnimProps.Clear( eGroup );
		m_UpperLockedAnimProps.Clear( eGroup );
		m_LowerLockedAnimProps.Clear( eGroup );
		m_CustomLockedAnimProps.Clear( eGroup );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::OnMessage
//
//	PURPOSE:	Recieve a message...
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::OnMessage(  ILTMessage_Read *pMsg )
{
	if( !pMsg )
		return;

	uint8 nMessageID = pMsg->Readuint8( );

	switch( nMessageID )
	{
		case kPlayerBodyAnimate:
		{
			// Get the animation index...
			HMODELANIM hAnim = pMsg->Readuint32();
			if( hAnim != INVALID_MODEL_ANIM )
			{
				// Kill weapon firing...
				CClientWeapon *pWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon( );
				if( pWeapon )
				{
					pWeapon->KillLoopSound( );
					pWeapon->ClearFiring( );
				}

				// To get the animation to play once and then let the PlayerBody update as usual, the
				// special animation needs to be played as a linger but don't set the internal linger flag...
				
				m_pUpperAnimationContext->SetSpecial( hAnim, kAD_MOV_Encode_NP );
				m_pUpperAnimationContext->LingerSpecial( );

				m_pLowerAnimationContext->SetSpecial( hAnim, kAD_MOV_Encode_NP );
				m_pLowerAnimationContext->LingerSpecial( );

				if( m_pCustomAnimationContext )
				{
					m_pCustomAnimationContext->SetSpecial( hAnim, kAD_MOV_Encode_NP );
					m_pCustomAnimationContext->LingerSpecial( );
				}

				m_bPlayingSpecial = true;
				m_bLingerSpecial = false;
			}
			
		}
		break;

		case kPlayerBodyAnimateLoop:
		{
			// Kill weapon firing...
			CClientWeapon *pWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon( );
			if( pWeapon )
			{
				pWeapon->KillLoopSound( );
				pWeapon->ClearFiring( );
			}

			// Get the animation index...
			HMODELANIM hAnim = pMsg->Readuint32();
			if( hAnim != INVALID_MODEL_ANIM )
			{
				m_pUpperAnimationContext->SetSpecial( hAnim, kAD_MOV_Encode_NP );
				m_pUpperAnimationContext->LoopSpecial( );

				m_pLowerAnimationContext->SetSpecial( hAnim, kAD_MOV_Encode_NP );
				m_pLowerAnimationContext->LoopSpecial( );

				if( m_pCustomAnimationContext )
				{
					m_pCustomAnimationContext->SetSpecial( hAnim, kAD_MOV_Encode_NP );
					m_pCustomAnimationContext->LoopSpecial( );
				}

				m_bPlayingSpecial = true;
			}
		}
		break;

		case kPlayerBodyAnimateLinger:
		{
			// Kill weapon firing...
			CClientWeapon *pWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon( );
			if( pWeapon )
			{
				pWeapon->KillLoopSound( );
				pWeapon->ClearFiring( );
			}

			// Get the animation index...
			HMODELANIM hAnim = pMsg->Readuint32();
			if( hAnim != INVALID_MODEL_ANIM )
			{
				m_pUpperAnimationContext->SetSpecial( hAnim, kAD_MOV_Encode_NP );
				m_pUpperAnimationContext->LingerSpecial( );

				m_pLowerAnimationContext->SetSpecial( hAnim, kAD_MOV_Encode_NP );
				m_pLowerAnimationContext->LingerSpecial( );

				if( m_pCustomAnimationContext )
				{
					m_pCustomAnimationContext->SetSpecial( hAnim, kAD_MOV_Encode_NP );
					m_pCustomAnimationContext->LingerSpecial( );
				}

				m_bPlayingSpecial = true;
				m_bLingerSpecial = true;
			}

		}
		break;

		case kPlayerBodyAnimateStop:
		{
			bool bWasPlayingSpecial = IsPlayingSpecial();
			ClearPlayingSpecial( );

			if( g_pPlayerMgr->IsSpectating( ) && bWasPlayingSpecial )
				Update( );
		}
		break;

		case kPlayerBodyAnimateCamRot:
		{
			bool bAnimateCamRot = pMsg->Readbool();
			EnumAnimDesc eCamera = (bAnimateCamRot ? kAD_CAM_Rotation : kAD_None);

			m_pUpperAnimationContext->SetSpecialCameraType( eCamera );
			m_pLowerAnimationContext->SetSpecialCameraType( eCamera );
			if( m_pCustomAnimationContext )
				m_pCustomAnimationContext->SetSpecialCameraType( eCamera );
		}
		break;

		case kPlayerBodyWeightSet:
		{
			char szWeightSet[32] = {0};
			pMsg->ReadString( szWeightSet, LTARRAYSIZE(szWeightSet) );
			
			if( szWeightSet[0] )
				PhysicsUtilities::SetPhysicsWeightSet( m_hPlayerBody, szWeightSet, true );
		}
		break;

		default:
		break;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::UpdatePlayingSpecialAnimation
//
//	PURPOSE:	Update the state of the special animation, if one's playing...
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::UpdatePlayingSpecialAnimation( )
{
	if( !m_bPlayingSpecial )
		return;

	// If playing a non-looping, non-linger animation clear the special once it finishes...
	if( !m_bLingerSpecial &&
		m_pLowerAnimationContext->IsSpecialDone() &&
		m_pUpperAnimationContext->IsSpecialDone() &&
		( !m_pCustomAnimationContext || m_pCustomAnimationContext->IsSpecialDone() ))
	{
		ClearPlayingSpecial( );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::ClearPlayingSpecial
//
//	PURPOSE:	Clears the special animation information.
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::ClearPlayingSpecial( )
{
	if( !m_bPlayingSpecial )
		return;

	m_pUpperAnimationContext->ClearSpecial( );
	m_pLowerAnimationContext->ClearSpecial( );
	if( m_pCustomAnimationContext )
		m_pCustomAnimationContext->ClearSpecial( );

	m_bPlayingSpecial = false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::InitPhysics
//
//	PURPOSE:	Initialize the physics data for the player body model...
//
// ----------------------------------------------------------------------- //

bool CPlayerBodyMgr::InitPhysics( )
{
	if( !m_hPlayerBody )
	{
		LTERROR( "Trying to initialize physics without a PlayerBody object" );
		return false;
	}
    
	if( g_pLTClient->PhysicsSim( )->SetObjectPhysicsGroup( m_hPlayerBody, PhysicsUtilities::ePhysicsGroup_UserPlayer ) != LT_OK )
	{
		LTERROR( "Failed to set the physics group for the PlayerBody." );
		return false;
	}

	// Success... 
	return true;
}



// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::SetupMovementScaling
//
//	PURPOSE:	Calculate the amount to scale each axis.
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::SetupMovementScaling()
{
	//only do the scaling in MP
	if (!IsMultiplayerGameClient())
	{
		return;
	}

	if( m_pLowerAnimationContext->IsTransitioning() || IsMovementEncoded() )
	{
		m_pLowerAnimationContext->ClearOverrideAnimRate();
		return;
	}

	// Calculate  scaling.
	m_fLowerAnimRate = g_pMoveMgr->GetMoveMultiplier();
	
	if (g_vtPlayerBodyAdjustAnimRate.GetFloat() > 0.0f) 
	{
		m_pLowerAnimationContext->SetOverrideAnimRate(m_fLowerAnimRate);
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::UpdateCloseEncounter
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::UpdateCloseEncounter()
{
	if (m_bInCloseEncounter)
	{
		if (m_fCloseEncounterEndTime > 0.0f)
		{
			if (g_pLTClient->GetTime() >= m_fCloseEncounterEndTime)
			{
				m_bInCloseEncounter = false;
			}
		}
		else
		{
			HMODELANIM hAnim;
			g_pModelLT->GetCurAnim(m_hPlayerBody, m_nCloseEncounterTrackerID, hAnim);

			if (hAnim != m_hCloseEncounterAnim)
			{
				m_bInCloseEncounter = false;
			}
		}

		if (!m_bInCloseEncounter)
		{
			g_pLTClient->SetConsoleVariableFloat("NearZ", m_fCloseEncounterOldNearZ);
		}
	}
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::HandleAnimationStimulus
//
//	PURPOSE:	Dispatch animation stimulus to conditional animation controllers.
//
// ----------------------------------------------------------------------- //

bool CPlayerBodyMgr::HandleAnimationStimulus( const char* pszStimulus )
{
	CClientWeapon* pWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
	if (pWeapon)
	{
		return pWeapon->HandleAnimationStimulus(pszStimulus);
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::HandlingAnimationStimulus
//
//	PURPOSE:	See if any of our anim controllers are currently handling the specified stimulus.
//
// ----------------------------------------------------------------------- //

bool CPlayerBodyMgr::HandlingAnimationStimulus( const char* pszStimulus ) const
{
	CClientWeapon* pWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
	if (pWeapon)
	{
		return pWeapon->HandlingAnimationStimulus(pszStimulus);
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::HandlingAnimationStimulusGroup
//
//	PURPOSE:	See if any of our anim controllers are currently handling the specified stimulus (prefix).
//
// ----------------------------------------------------------------------- //

bool CPlayerBodyMgr::HandlingAnimationStimulusGroup( const char* pszStimulus ) const
{
	CClientWeapon* pWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
	if (pWeapon)
	{
		return pWeapon->HandlingAnimationStimulusGroup(pszStimulus);
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::ActiveAnimationStimulus
//
//	PURPOSE:	Check to see if any action is currently playing on our
//				conditional animation controllers.
//
// ----------------------------------------------------------------------- //

bool CPlayerBodyMgr::ActiveAnimationStimulus() const
{
	CClientWeapon* pWeapon = g_pClientWeaponMgr->GetCurrentClientWeapon();
	if (pWeapon)
	{
		return pWeapon->ActiveAnimationStimulus();
	}

	return false;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::UpdateAnimationBlending
//
//	PURPOSE:	Determine if animation blending is needed and set the proper blend data...
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::UpdateAnimationBlending( )
{
	ClientDB &ClientDatabase = ClientDB::Instance( );
	HRECORD hClientShared = ClientDatabase.GetClientSharedRecord( );

	if( !ClientDatabase.GetBool( hClientShared, CDB_UsePostureBlending ))
		return;

	// Blend the animations if the posture has changed...
	EnumAnimProp eCurrentPosture = m_AnimProps.Get( kAPG_Posture );
	EnumAnimProp ePreviousPosture = m_LastAnimProps.Get( kAPG_Posture );
	if( eCurrentPosture != ePreviousPosture && !IsLocked( kAPG_Action, kAP_ACT_SlideKick ))
	{
		BLENDDATA BlendData;
		BlendData.fBlendDuration = ClientDatabase.GetInt32( hClientShared, CDB_PostureBlendingDuration ) * 0.001f;
		BlendData.szBlendWeightSet = m_pUpperAnimationContext->GetWeightSetName( );

		m_pUpperAnimationContext->SetBlendData( BlendData );

		BlendData.szBlendWeightSet = m_pLowerAnimationContext->GetWeightSetName( );
		m_pLowerAnimationContext->SetBlendData( BlendData );

		m_pLowerAnimationContext->DoInterpolation( false );
		m_pUpperAnimationContext->DoInterpolation( false );

		// Clear any locked jump animations from the lower body so crouching is allowed...
		EnumAnimProp eMoveProp = m_LowerLockedAnimProps.Get( kAPG_Movement );
		if( (eMoveProp == kAP_MOV_Jump) ||
			(eMoveProp == kAP_MOV_Fall) ||
			(eMoveProp == kAP_MOV_Land) )
		{
			m_LowerLockedAnimProps.Clear( kAPG_Movement );
			m_pLowerAnimationContext->ClearLock( );
		}
	}
	else
	{
		m_pLowerAnimationContext->ClearBlendData( );
		m_pUpperAnimationContext->ClearBlendData( );
		
		m_pLowerAnimationContext->DoInterpolation( true );
		m_pUpperAnimationContext->DoInterpolation( true );
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::CanUseHeadBob
//
//	PURPOSE:	Certain animations may not want to use head bob, so disable it for those...
//
// ----------------------------------------------------------------------- //

bool CPlayerBodyMgr::CanUseHeadBob( )
{
	// Scripted and specific melee animations should not use headbob...
	if( IsPlayingSpecial( ) ||
		IsLocked( kAPG_Action, kAP_ACT_SlideKick ) ||
		IsLocked( kAPG_Action, kAP_ACT_JumpIdleKick ) ||
		IsLocked( kAPG_Action, kAP_ACT_JumpRunKick ) ||
		IsLocked( kAPG_Action, kAP_ACT_LowRunKick ) )
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::CanCrouch
//
//	PURPOSE:	Certain animations may not allow crouching...
//
// ----------------------------------------------------------------------- //

bool CPlayerBodyMgr::CanCrouch( )
{
	// Scripted and specific melee animations should not allow crouch...
	if( IsPlayingSpecial( ) ||
		IsLocked( kAPG_Action, kAP_ACT_SlideKick ) ||
		IsLocked( kAPG_Action, kAP_ACT_JumpIdleKick ) ||
		IsLocked( kAPG_Action, kAP_ACT_JumpRunKick ) ||
		IsLocked( kAPG_Action, kAP_ACT_LowRunKick ) )
		return false;

	return true;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::CanSave
//
//	PURPOSE:	Test if the PlayerBody will allow saves...
//
// ----------------------------------------------------------------------- //

bool CPlayerBodyMgr::CanSave( )
{
	// Don't allow saves when playing special animations...
	// [RP] This is temporary to disallow saving for the E3 demo.
	return !m_bPlayingSpecial;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::Save
//
//	PURPOSE:	Save the PlayerBody...
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::Save( ILTMessage_Write *pMsg )
{
	if( !pMsg )
		return;

	pMsg->WriteDatabaseRecord( g_pLTDatabase, m_hModel );

	m_pMainAnimationContext->Save( pMsg, true );
	m_pUpperAnimationContext->Save( pMsg, true );
	m_pLowerAnimationContext->Save( pMsg, true );
	if( m_pCustomAnimationContext )
		m_pCustomAnimationContext->Save( pMsg, true );

	m_AnimProps.Save( pMsg );
	m_UpperAnimProps.Save( pMsg );
	m_LowerAnimProps.Save( pMsg );
	m_CustomAnimProps.Save( pMsg );

    m_LastAnimProps.Save( pMsg );

	m_MainLockedAnimProps.Save( pMsg );
	m_UpperLockedAnimProps.Save( pMsg );
	m_LowerLockedAnimProps.Save( pMsg );
	m_CustomLockedAnimProps.Save( pMsg );

	SAVE_FLOAT( m_fSendAnimTime );

	SAVE_INT( m_eMovementDescriptor );
	SAVE_INT( m_eCameraDescriptor );
	SAVE_INT( m_eInputDescriptor );

	SAVE_bool( m_bPlayingSpecial );
	SAVE_bool( m_bLingerSpecial );

	SAVE_VECTOR( m_vWantedDims );

	SAVE_bool( m_bMainClearCachedAni );
	SAVE_bool( m_bLowerClearCachedAni );
	SAVE_bool( m_bUpperClearCachedAni );
	SAVE_bool( m_bCustomClearCachedAni );

	SAVE_bool( m_bPlayingFullBody );
	SAVE_bool( m_bPlayingCustom );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CPlayerBodyMgr::Load
//
//	PURPOSE:	Load the PlayerBody...
//
// ----------------------------------------------------------------------- //

void CPlayerBodyMgr::Load( ILTMessage_Read *pMsg )
{
	if( !pMsg )
		return;

	ModelsDB::HMODEL hModel = pMsg->ReadDatabaseRecord( g_pLTDatabase, g_pModelsDB->GetModelsCategory( ));

	//HACK!!
	hModel = g_pModelsDB->GetModelByRecordName( DEFAULT_PLAYERNAME );

	ResetModel( hModel );
	
	m_pMainAnimationContext->Load( pMsg );
	m_pUpperAnimationContext->Load( pMsg );
	m_pLowerAnimationContext->Load( pMsg );
	if( m_pCustomAnimationContext )
		m_pCustomAnimationContext->Load( pMsg );

	m_AnimProps.Load( pMsg );
	m_UpperAnimProps.Load( pMsg );
	m_LowerAnimProps.Load( pMsg );
	m_CustomAnimProps.Load( pMsg );

	m_LastAnimProps.Load( pMsg );

	m_MainLockedAnimProps.Load( pMsg );
	m_UpperLockedAnimProps.Load( pMsg );
	m_LowerLockedAnimProps.Load( pMsg );
	m_CustomLockedAnimProps.Load( pMsg );

	LOAD_FLOAT( m_fSendAnimTime );

	LOAD_INT_CAST( m_eMovementDescriptor, EnumAnimDesc );
	LOAD_INT_CAST( m_eCameraDescriptor, EnumAnimDesc );
	LOAD_INT_CAST( m_eInputDescriptor, EnumAnimDesc );

	LOAD_bool( m_bPlayingSpecial );
	LOAD_bool( m_bLingerSpecial );

	LOAD_VECTOR( m_vWantedDims );

	LOAD_bool( m_bMainClearCachedAni );
	LOAD_bool( m_bLowerClearCachedAni );
	LOAD_bool( m_bUpperClearCachedAni );
	LOAD_bool( m_bCustomClearCachedAni );

	LOAD_bool( m_bPlayingFullBody );
	LOAD_bool( m_bPlayingCustom );

	m_bForceSendAnimTime = true;

	// Make sure our dims are now correct.
	if (!AreDimsCorrect())
	{
		LTVector vOffset( 0.0f, 0.0f, 0.0f);
		ResetDims(vOffset);
	}


	m_AnimProps.Set( kAPG_Posture, m_LastAnimProps.Get( kAPG_Posture ) );
	m_AnimProps.Set( kAPG_Movement, m_LastAnimProps.Get( kAPG_Movement ) );
	m_AnimProps.Set( kAPG_MovementDir, m_LastAnimProps.Get( kAPG_MovementDir ) );
}

// EOF
