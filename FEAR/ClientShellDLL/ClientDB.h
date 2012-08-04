
// ----------------------------------------------------------------------- //
//
// MODULE  : ClientDB.h
//
// PURPOSE : 
//
// CREATED : 3/10/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __CLIENTDB_H__
#define __CLIENTDB_H__

const char* const CDB_DebugKeyCat								= "Client/DebugKey";
	const char* const CDB_DebugKeyId							= "Key";
	const char* const CDB_DebugModifierId						= "Modifier";
	const char* const CDB_DebugKeyString						= "String";
	const char* const CDB_DebugKeyTitle							= "Title";

const char* const CDB_ClientSharedCat							= "Client/Shared";
	const char* const CDB_ClientSharedRecord					= "Shared";
		const char* const CDB_ClientShared_DeathDelay			= "DeathDelay";
		const char* const CDB_ClientShared_MPDeathDelay			= "MultiplayerDeathDelay";
		const char* const CDB_ClientShared_DeathMixer			= "DeathMixer";
		const char* const CDB_ClientShared_DeathSound			= "DeathSound";
		const char* const CDB_ClientShared_DeathClientFX		= "DeathClientFX";
		const char* const CDB_ClientShared_MPDeathClientFX		= "MultiplayerDeathClientFX";
		const char* const CDB_ClientShared_ForensicEnterClientFX= "ForensicEnterClientFX";
		const char* const CDB_ClientShared_ForensicExitClientFX = "ForensicExitClientFX";
		const char* const CDB_ClientShared_Reverb				= "Reverb";
		const char* const CDB_SearchBeam						= "SearchBeam";
		const char* const CDB_SearchMaterial0					= "SearchMaterial0";
		const char* const CDB_SearchMaterial1					= "SearchMaterial1";
		const char* const CDB_Letterbox							= "LetterBoxPercent";
		const char* const CDB_Cheats							= "Cheats";
		const char* const CDB_ChatFX							= "ChatFX";
		const char* const CDB_CameraCollisionModel				= "CameraCollisionModel";
		const char* const CDB_SplashScreenImage					= "SplashScreenImage";
		const char* const CDB_SplashScreenMPFreeOverlay			= "SplashScreenMPFreeOverlay";
		const char* const CDB_SplashScreenSound					= "SplashScreenSound";
		const char* const CDB_DemoEndScreenImage				= "DemoEndScreenImage";
		const char* const CDB_PauseScreenImage					= "PauseScreenImage";
		const char* const CDB_SplashMovies						= "SplashMovies";
		const char* const CDB_DebugLineSystemMaterial			= "DebugLineSystemMaterial";
		const char* const CDB_CameraHeightInterpSpeedUp			= "CameraHeightInterpSpeedUp";
		const char* const CDB_CameraHeightInterpSpeedDown		= "CameraHeightInterpSpeedDown";
		const char* const CDB_CameraHeightVelocityDampUp		= "CameraHeightVelocityDampUp";
		const char* const CDB_CameraHeightVelocityDampDown		= "CameraHeightVelocityDampDown";
		const char* const CDB_fFallDamageMinHeight				= "FallDamageMinHeight";
		const char* const CDB_fFallDamageMaxHeight				= "FallDamageMaxHeight";
		const char* const CDB_fFallDamageMin					= "FallDamageMin";
		const char* const CDB_fFallDamageMax					= "FallDamageMax";
		const char* const CDB_sClientShared_DefaultPatch		= "DefaultPlayerPatch";
		const char* const CDB_sClientShared_UnderwaterSound		= "UnderwaterSound";
		const char*	const CDB_bCameraSmoothingEnabled			= "CameraSmoothingEnabled";
		const char* const CDB_dwCameraSmoothingInterpolateTime	= "CameraSmoothingInterpolateTime";
		const char* const CDB_fCameraSmoothingAllowedDistance	= "CameraSmoothingAllowedDistance";
		const char* const CDB_fCameraSmoothingLeashLength		= "CameraSmoothingLeashLength";
		const char* const CDB_fMissionFailureDelay				= "MissionFailureDelay";
		const char* const CDB_sMissionFailureTips				= "MissionFailureTips";
		const char* const CDB_ClientShared_HeadshotClientFX		= "HeadshotClientFX";


//------------------------------------------------------
// HUD flash record attributes
const char* const CDB_HUDFlashCat								= "Client/HUDFlash";
	const char* const CDB_cFlashColor =				"Color";
	const char* const CDB_fFlashDuration =			"Duration";
	const char* const CDB_fFlashRate =				"Rate";



const char* const CDB_HeadBobCat								= "Client/HeadBob";
const char* const CDB_FlashlightCat								= "Client/Flashlight";
const char* const CDB_MovementCat								= "Client/Movement";
const char* const CDB_OverlayCat								= "Client/Overlay";

const char* const CDB_UsePostureBlending						= "PostureBlending.0.UsePostureBlending";
const char* const CDB_PostureBlendingDuration					= "PostureBlending.0.BlendDuration";

const char* const CDB_PlayerMovementCat							= "Client/PlayerMovement";
const char* const CDB_PlayerMovementRec							= "PlayerMovement";

// ClientFXSequence category...
const char* const CDB_FXSEQ_Category							= "Client/ClientFXSequence";
const char* const CDB_FXSEQ_EnterFX								= "EnterFX";
const char* const CDB_FXSEQ_LoopFX								= "LoopFX";
const char* const CDB_FXSEQ_ExitFX								= "ExitFX";


// ----------------------------------------------------------------------- //
//
//	CLASS:		ClientDB
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class ClientDB : public CGameDatabaseMgr
{
	DECLARE_SINGLETON( ClientDB );
	
public:
	// Constructors
	
	bool		Init( const char *szDatabaseFile = DB_Default_File );
	void		Term();

	HCATEGORY	GetDebugKeyCategory() const { return m_hDebugKeyCat; }
	HRECORD		GetClientSharedRecord() const { return m_hClientSharedRecord; }
	HCATEGORY	GetHeadBobCategory() const { return m_hHeadBobCat; }
	HCATEGORY	GetFlashlightCategory() const { return m_hFlashlightCat; }
	HCATEGORY	GetMovementCategory() const { return m_hMovementCat; }
	HCATEGORY	GetOverlayCategory() const { return m_hOverlayCat; }
	HRECORD		GetPlayerMovementRecord() const { return m_hPlayerMovementRecord; }
	HCATEGORY	GetClientFXSequenceCategory( ) const { return m_hClientFXSequenceCat; }
 
	// ----------------------------------------------------------------------- //
	//	ROUTINE:	GetActivationDistance()
	//	PURPOSE:	Distance at which activation should be indicated
	// ----------------------------------------------------------------------- //
	float GetActivationDistance() { return GetFloat(m_hClientSharedRecord, "ActivationDistance"); }

	// ----------------------------------------------------------------------- //
	//	ROUTINE:	GetTargetDistance()
	//	PURPOSE:	Distance at which target should be indicated
	// ----------------------------------------------------------------------- //
	float GetTargetDistance() { return GetFloat(m_hClientSharedRecord, "TargetDistance"); }

	// ----------------------------------------------------------------------- //
	//	ROUTINE:	GetPickupDistance()
	//	PURPOSE:	Distance at which target should be indicated
	// ----------------------------------------------------------------------- //
	float GetPickupDistance() { return GetFloat(m_hClientSharedRecord, "PickupDistance"); }

	// ----------------------------------------------------------------------- //
	//	ROUTINE:	GetMPAutoTargetSpeed()
	//	PURPOSE:	Scalar to control speed autotarget moves to its target in multiplayer
	// ----------------------------------------------------------------------- //
	float GetMPAutoTargetSpeed() { return GetFloat(m_hClientSharedRecord, "MPAutoTargetSpeed"); }

	// ----------------------------------------------------------------------- //
	//	ROUTINE:	GetAutoTargetSpeed()
	//	PURPOSE:	Scalar to control speed autotarget moves to its target in singleplayer
	// ----------------------------------------------------------------------- //
	float GetAutoTargetSpeed() { return GetFloat(m_hClientSharedRecord, "AutoTargetSpeed"); }

	//------------------------------------------------------
	// Access to records from HUD Flash Category
	HRECORD		GetHUDFlashRecord(const char* pszRecordName);

private:

	HCATEGORY	m_hDebugKeyCat;
	HRECORD		m_hClientSharedRecord;
	HCATEGORY	m_hHeadBobCat;
	HCATEGORY	m_hFlashlightCat;
	HCATEGORY	m_hMovementCat;
	HCATEGORY	m_hOverlayCat;
	HCATEGORY	m_hHUDFlashCat;
	HRECORD		m_hPlayerMovementRecord;
	HCATEGORY	m_hClientFXSequenceCat;
};

#endif // __CLIENTDB_H__
