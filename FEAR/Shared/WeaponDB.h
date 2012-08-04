// ----------------------------------------------------------------------- //
//
// MODULE  : WeaponDB.h
//
// PURPOSE : Weapon database definition
//
// CREATED : 06/16/03
//
// (c) 2003-2005 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __WEAPON_DATABASE_H__
#define __WEAPON_DATABASE_H__

//
// Includes...
//

#include "GameDatabaseMgr.h"
#include "DamageTypes.h"
#include "CategoryDB.h"	


//
// Defines...
//
	
#define WEAPON_SOUND_RADIUS					2000.0f
#define WEAPON_MIN_IDLE_TIME				5.0f
#define WEAPON_MAX_IDLE_TIME				15.0f
#define WEAPON_KEY_FIRE						"FIRE_KEY"
#define WEAPON_KEY_FINISH					"FINISH"
#define WEAPON_KEY_FINISH_RAGDOLL			"FINISH_RAGDOLL"
#define WEAPON_KEY_SOUND					"SOUND_KEY"
#define WEAPON_KEY_BUTE_SOUND				"BUTE_SOUND_KEY"
#define WEAPON_KEY_LOOPSOUND				"LOOP_SOUND_KEY"
#define WEAPON_KEY_FX						"FX"
#define WEAPON_KEY_FIREFX					"FIREFX_KEY"
#define WEAPON_KEY_FLASHLIGHT				"FLASHLIGHT"
#define WEAPON_KEY_MATERIAL					"MATERIAL"
#define WEAPON_KEY_HIDE_MODEL_PIECE			"HIDE_PIECE_KEY"  // usage: HIDE_PIECE_KEY <piece name>
#define WEAPON_KEY_SHOW_MODEL_PIECE			"SHOW_PIECE_KEY"  // usage: SHOW_PIECE_KEY <piece name>	
#define WEAPON_KEY_SHELLCASING				"SHELL_CASING"
#define WEAPON_KEY_HIDE_PVATTACHFX			"HIDE_PVATTACHFX"	// usage: HIDE_PVFX <index of a PVAttachFXName#>
#define WEAPON_KEY_SHOW_PVATTACHFX			"SHOW_PVATTACHFX"	// usage: SHOW_PVFX <index of a PVAttachFXName#>
#define WEAPON_KEY_HIDE_PVATTACHMENT		"HIDE_PVATTACHMENT" // usage: HIDE_PVATTACHMENT <index of a PlayerViewAttachment>
#define WEAPON_KEY_SHOW_PVATTACHMENT		"SHOW_PVATTACHMENT" // usage: SHOW_PVATTACHMENT <index of a PlayerViewAttachment>

#define WEAPONDB_DEFAULT_FILE				"Database\\" "Weapons." RESEXT_GAMEDATABASE_PACKED
	
// The main system category were all arsenal related data goes...
#define WDB_ARSENAL_CATEGORY				"Arsenal/"

#define WDB_ALL_bCanServerRestrict			"CanServerRestrict"
#define WDB_ALL_nDescriptionId				"DescriptionId"

#define WDB_WEAPON_CATEGORY					WDB_ARSENAL_CATEGORY "Weapons"

#define WDB_WEAPON_nShortNameId				"ShortNameId"
#define WDB_WEAPON_nLongNameId				"LongNameId"
#define WDB_WEAPON_nAniType					"AniType"
#define WDB_WEAPON_vPos						"Pos"
#define WDB_WEAPON_sMuzzleSocket			"MuzzleSocket"
#define WDB_WEAPON_sBreachSocket			"BreachSocket"
#define WDB_WEAPON_fHHScale					"HHModelScale"
#define WDB_WEAPON_sPVModel					"PVModel"
#define WDB_WEAPON_sHHModel					"HHModel"
#define WDB_WEAPON_sPVMaterial				"PVMaterial"
#define WDB_WEAPON_sHHMaterial				"HHMaterial"
#define	WDB_WEAPON_sHiddenPieceName			"HiddenPiece"
#define WDB_WEAPON_sRespawnWaitMaterial		"RespawnWaitMaterial"
#define WDB_WEAPON_rFireSnd					"FireSound"
#define WDB_WEAPON_rFireLoopSnd				"FireSoundLoop"
#define WDB_WEAPON_rFireLoopEndSnd			"FireSoundLoopEnd"
#define WDB_WEAPON_rAltFireSnd				"AltFireSound"
#define WDB_WEAPON_rSilencedFireSnd			"SilencedFireSnd"
#define WDB_WEAPON_rDryFireSnd				"DryFireSound"
#define WDB_WEAPON_rReloadSnd				"ReloadSounds"
#define WDB_WEAPON_rSelectSnd				"SelectSound"
#define WDB_WEAPON_rDeselectSnd				"DeselectSound"
#define WDB_WEAPON_rMiscSnd					"MiscSounds"
#define WDB_WEAPON_fAIFireSndRadius			"AIFireSndRadius"
#define WDB_WEAPON_bInfiniteAmmo			"InfiniteAmmo"
#define WDB_WEAPON_bHideWhenEmpty			"HideWhenEmpty"
#define WDB_WEAPON_bIsAmmo					"IsAmmo"
#define WDB_WEAPON_bIsGrenade				"IsGrenade"
#define WDB_WEAPON_bDropGrenadeOnDeath		"DropGrenadeOnDeath"
#define WDB_WEAPON_bTakesInventorySlot		"TakesInventorySlot"
#define WDB_WEAPON_bCanLastWeapon			"CanLastWeapon"
#define WDB_WEAPON_bUseUWMuzzleFX			"UseUWMuzzleFX"
#define WDB_WEAPON_nShotsPerClip			"ShotsPerClip"
#define WDB_WEAPON_bInfiniteClip			"InfiniteClip"
#define WDB_WEAPON_rAmmoName				"AmmoName"
#define WDB_WEAPON_rDefaultMeleeAmmo		"DefaultMeleeAmmo"
#define WDB_WEAPON_rModName					"ModName"
#define WDB_WEAPON_v2Perturb				"Perturb"
#define WDB_WEAPON_nRange					"Range"
#define WDB_WEAPON_nEffectiveVectorRange	"EffectiveVectorRange"
#define WDB_WEAPON_nVectorHalfDamageDist	"VectorHalfDamageDist"
#define WDB_WEAPON_nVectorsPerRound			"VectorsPerRound"
#define WDB_WEAPON_bSemiAuto				"Semi-Auto"
#define WDB_WEAPON_bAutoSwitchEnabled		"AutoSwitchEnabled"
#define WDB_WEAPON_bSupportsFinishingMoves	"SupportsFinishingMoves"
#define WDB_WEAPON_sCriticalHitSocket		"CriticalHitSocket"
#define WDB_WEAPON_sCriticalHitImpactSocket	"CriticalHitImpactSocket"
#define WDB_WEAPON_fCriticalHitDistance		"CriticalHitDistance"
#define WDB_WEAPON_fCriticalHitAngle		"CriticalHitAngle"
#define WDB_WEAPON_fCriticalHitViewAngle	"CriticalHitViewAngle"
#define WDB_WEAPON_hAIWeaponName			"AIWeaponName"
#define WDB_WEAPON_fFireRecoilKick			"FireRecoilKick"
#define WDB_WEAPON_fFireRecoilYawRatio		"FireRecoilYawRatio"
#define WDB_WEAPON_fFireRecoilDecay			"FireRecoilDecay"
#define WDB_WEAPON_sPVMuzzleFX				"PVMuzzleFX"
#define WDB_WEAPON_sPVMuzzleFX2				"PVMuzzleFX2"
#define WDB_WEAPON_sHHMuzzleFX				"HHMuzzleFX"
#define WDB_WEAPON_bFXAtFlashSocket			"FXAtFlashSocket"
#define WDB_WEAPON_nFireDelay				"FireDelay"
#define WDB_WEAPON_sHolsterAttachment		"HolsterAttachment"
#define WDB_WEAPON_fFireAnimRateScale		"FireAnimRateScale"
#define WDB_WEAPON_fReloadAnimRateScale		"ReloadAnimRateScale"
#define WDB_WEAPON_sPowerupFX				"PowerupFX"
#define WDB_WEAPON_bRespawnControlledSP		"RespawnControlledSP"
#define WDB_WEAPON_bRespawnControlledMP		"RespawnControlledMP"
#define WDB_WEAPON_fRespawnWait				"RespawnWait"
#define WDB_WEAPON_sRespawnWaitFX			"RespawnWaitFX"
#define WDB_WEAPON_bRespawnWaitVisible		"RespawnWaitVisible"
#define WDB_WEAPON_bRespawnWaitTranslucent	"RespawnWaitTranslucent"
#define WDB_WEAPON_fAutoTargetAngle			"AutoTargetAngle"
#define WDB_WEAPON_fAutoTargetRange			"AutoTargetRange"
#define WDB_WEAPON_nUseAutoTarget			"UseAutoTarget"
#define WDB_WEAPON_sIcon					"Icon"
#define WDB_WEAPON_sSilhouetteIcon			"SilhouetteIcon"
#define WDB_WEAPON_sAnimationProperty		"AnimationProperty"
#define WDB_WEAPON_sCountdownFX				"CountdownFX"
#define WDB_WEAPON_sEndFX					"EndFX"
#define WDB_WEAPON_bUsePreFire				"UsePreFire"
#define WDB_WEAPON_bUsePostFire				"UsePostFire"
#define WDB_WEAPON_bLoopPrePostFire			"LoopPrePostFire"
#define WDB_WEAPON_sMuzzleSmokeFX			"MuzzleSmokeFX"
#define WDB_WEAPON_fPerturbIncreaseRate		"Accuracy.0.PerturbIncreaseRate"
#define WDB_WEAPON_fPerturbDecreaseRate		"Accuracy.0.PerturbDecreaseRate"
#define WDB_WEAPON_fStand					"Accuracy.0.Stand"
#define WDB_WEAPON_fWalk					"Accuracy.0.Walk"
#define WDB_WEAPON_fRun						"Accuracy.0.Run"
#define WDB_WEAPON_fJump					"Accuracy.0.Jump"
#define WDB_WEAPON_fCrouch					"Accuracy.0.Crouch"
#define WDB_WEAPON_fCrouch_Walk				"Accuracy.0.Crouch_Walk"
#define WDB_WEAPON_fAim						"Accuracy.0.Aim"
#define WDB_WEAPON_fAim_Crouch				"Accuracy.0.Aim_Crouch"
#define WDB_WEAPON_fAim_Walk				"Accuracy.0.Aim_Walk"
#define WDB_WEAPON_fAim_Crouch_Walk			"Accuracy.0.Aim_Crouch_Walk"
#define WDB_WEAPON_fSwim					"Accuracy.0.Swim"
#define WDB_WEAPON_fTurn					"Accuracy.0.Turn"
#define WDB_WEAPON_fRecoil					"Accuracy.0.Recoil"
#define WDB_WEAPON_fMovementMultiplier		"MovementMultiplier"
#define WDB_WEAPON_rComplimentaryWeapon		"ComplimentaryWeapon"
#define WDB_WEAPON_bNotForPlayer			"NotForPlayer"
#define WDB_WEAPON_bWhenDroppedGiveAmmoInClip "WhenDroppedGiveAmmoInClip"
#define WDB_WEAPON_bDroppedMoveToFloor		"DroppedMoveToFloor"
#define WDB_WEAPON_nActivationType			"ActivationType"
#define WDB_WEAPON_rDualWeapon				"DualWeapon"
#define WDB_WEAPON_rSingleWeapon			"SingleWeapon"
#define WDB_WEAPON_RightHandWeapon			"RightHandWeapon"
#define WDB_WEAPON_LeftHandWeapon			"LeftHandWeapon"
#define WDB_WEAPON_nMaxFireFrequency		"MaxFireFrequency"
#define WDB_WEAPON_rLinkedWeapon			"LinkedWeapon"
#define WDB_WEAPON_bSelectLinked			"SelectLinked"
#define WDB_WEAPON_rCustomDisplay			"CustomDisplay"
#define WDB_WEAPON_LocalSoundInfo			"LocalFireSound"
#define WDB_WEAPON_NonLocalSoundInfo		"NonLocalFireSound"
#define WDB_WEAPON_rCollisionProperty		"CollisionProperty"
#define WDB_WEAPON_rFlashlights				"Flashlights"
#define WDB_WEAPON_sHUDTexture				"HUDTexture"
#define WDB_WEAPON_rActionIcon				"ActionIcon"
#define WDB_WEAPON_rAnimControllers			"AnimControllers"
#define WDB_WEAPON_v2MinProximity			"MinProximity"
#define WDB_WEAPON_CustomSelectStimulus		"CustomSelectStimulus"
#define WDB_WEAPON_aPersistentClientFX		"PersistentClientFX"
#define WDB_WEAPON_sClientFX				"ClientFX"
#define WDB_WEAPON_bVisible					"Visible"
#define WDB_WEAPON_bSmoothShutdown			"SmoothShutdown"
#define WDB_WEAPON_nMaxHealth				"BreakInfo.0.MaxHealth"
#define WDB_WEAPON_nWarnHealth				"BreakInfo.0.WarnHealth"
#define WDB_WEAPON_sShowPieces				"BreakInfo.0.ShowPieces"
#define WDB_WEAPON_sHidePieces				"BreakInfo.0.HidePieces"
#define WDB_WEAPON_sDebrisFX				"BreakInfo.0.DebrisFX"
#define WDB_WEAPON_sDebrisSocket			"BreakInfo.0.DebrisSocket"
#define WDB_WEAPON_rBrokenWeapon			"BreakInfo.0.BrokenWeapon"
#define WDB_WEAPON_bCanBePlaced				"CanBePlaced"

#define WDB_AMMO_CATEGORY					WDB_ARSENAL_CATEGORY "Ammo"

#define WDB_AMMO_nShortNameId				"ShortNameId"
#define WDB_AMMO_nLongNameId				"LongNameId"
#define WDB_AMMO_nPlacedAmount				"PlacedAmount"
#define WDB_AMMO_nPickupInitialAmount		"PickupInitialAmount"
#define WDB_AMMO_nPickupSupplementalAmount	"PickupSupplementalAmount"
#define WDB_AMMO_nSelectionAmount			"SelectionAmount"
#define WDB_AMMO_nMaxAmount					"MaxAmount"
#define WDB_AMMO_fInstDamage				"InstDamage"
#define WDB_AMMO_fAreaDamage				"AreaDamage"
#define WDB_AMMO_fInstDamageImpulseForce	"InstDamageImpulseForce"
#define WDB_AMMO_fHitNodeImpulseForceScale	"HitNodeImpulseForceScale"
#define WDB_AMMO_fAreaDamageImpulseForce	"AreaDamageImpulseForce"
#define WDB_AMMO_fInstPenetration			"InstPenetration"
#define WDB_AMMO_fAreaPenetration			"AreaPenetration"
#define WDB_AMMO_fAreaDamageRadius			"AreaDamageRadius"
#define WDB_AMMO_fAreaDamageRadiusMin		"AreaDamageRadiusMin"
#define WDB_AMMO_bCanAdjustInstDamage		"CanAdjustInstDamage"
#define WDB_AMMO_nPriority					"Priority"
#define WDB_AMMO_fFireRecoilMult			"FireRecoilMult"
#define WDB_AMMO_fProgDamage				"ProgDamage"
#define WDB_AMMO_fProgDamageDuration		"ProgDamageDuration"
#define WDB_AMMO_fProgDamageRadius			"ProgDamageRadius"
#define WDB_AMMO_fProgDamageLifetime		"ProgDamageLifetime"
#define WDB_AMMO_sSurfaceFXType				"SurfaceFXType"
#define WDB_AMMO_nType						"Type"
#define WDB_AMMO_sIcon						"Icon"
#define WDB_AMMO_sProjectileFX				"ProjectileFX"
#define WDB_AMMO_sImpactFX					"ImpactFX"
#define WDB_AMMO_sUWImpactFX				"UWImpactFX"
#define WDB_AMMO_sMovableImpactOverrideFX	"MoveableImpactOverrideFX"
#define WDB_AMMO_sFireFX					"FireFX"
#define WDB_AMMO_sTracerFX					"TracerFX"
#define WDB_AMMO_bHeatSeeking				"HeatSeeking"
#define WDB_AMMO_fHeatSeekingRange			"HeatSeekingRange"
#define WDB_AMMO_fHeatSeekingAngle			"HeatSeekingAngle"
#define WDB_AMMO_fHeatSeekingRateOfTurn		"HeatSeekingRateOfTurn"
#define WDB_AMMO_rInstDamageType			"InstDamage_Type"
#define WDB_AMMO_rAreaDamageType			"AreaDamage_Type"
#define WDB_AMMO_rProgDamageType			"ProgDamage_Type"
#define WDB_AMMO_nRange						"Range"
#define WDB_AMMO_sSourceImpactSocket		"SourceImpactSocket"
#define WDB_AMMO_sTriggerMessage			"TriggerMessage"
#define WDB_AMMO_bCanWallStick				"CanWallStick"
#define WDB_AMMO_rWallStickProp				"WallStickProp"
#define WDB_AMMO_sHUDTexture				"HUDTexture"
#define WDB_AMMO_sFlyBySound				"FlyBySound"

#define WDB_MELEECOLLIDER_CATEGORY			WDB_ARSENAL_CATEGORY "MeleeCollider"
#define WDB_MELEEDAMAGE_CATEGORY			WDB_ARSENAL_CATEGORY "MeleeDamage"

#define WDB_GEAR_CATEGORY					WDB_ARSENAL_CATEGORY "Gear"

#define WDB_GEAR_nNameId					"NameId"
#define WDB_GEAR_bRespawnWaitVisible		"RespawnWaitVisible"
#define WDB_GEAR_bRespawnWaitTranslucent	"RespawnWaitTranslucent"
#define WDB_GEAR_fProtection				"Protection"
#define WDB_GEAR_fArmor						"Armor"
#define WDB_GEAR_fHealth					"Health"
#define WDB_GEAR_fArmorMax					"ArmorMax"
#define WDB_GEAR_fHealthMax					"HealthMax"
#define WDB_GEAR_fStealth					"Stealth"
#define WDB_GEAR_fSlowMoTime				"SlowMoTime"
#define WDB_GEAR_fSlowMoMaxBonus			"SlowMoMaxBonus"
#define WDB_GEAR_sIcon						"Icon"
#define WDB_GEAR_sModel						"Model"
#define WDB_GEAR_rPickupSnd					"PickUp_Sound"
#define WDB_GEAR_rRespawnSnd				"Respawn_Sound"
#define WDB_GEAR_sActivateFX				"ActivateFX"
#define WDB_GEAR_sPowerupFX					"PowerupFX"
#define WDB_GEAR_sRespawnWaitFX				"RespawnWaitFX"
#define WDB_GEAR_sMaterial					"Material"
#define WDB_GEAR_sRespawnWaitMaterial		"RespawnWaitMaterial"
#define WDB_GEAR_sProtectionType			"ProtectionType"
#define WDB_GEAR_nMaxAmount					"MaxAmount"
#define WDB_GEAR_nThreshold					"Threshold"
#define WDB_GEAR_rNavMarker					"NavMarker"

#define WDB_MOD_CATEGORY					WDB_ARSENAL_CATEGORY "Mods"

#define WDB_MOD_sSocket						"Socket"
#define WDB_MOD_nNameId						"NameId"
#define WDB_MOD_nType						"Type"
#define WDB_MOD_sAttachModel				"AttachModel"
#define WDB_MOD_sAttachMaterial				"AttachMaterial"
#define WDB_MOD_fAttachScale				"AttachScale"
#define WDB_MOD_bIntegrated					"Integrated"
#define WDB_MOD_nPriority					"Priority"
#define WDB_MOD_fMagPower					"Scope.0.MagPower"
#define WDB_MOD_fZoomTimeIn					"Scope.0.ZoomTimeIn"
#define WDB_MOD_fZoomTimeOut				"Scope.0.ZoomTimeOut"
#define WDB_MOD_rScopeFXSequence			"Scope.0.ScopeFXSequence"
#define WDB_MOD_bIronSight					"Scope.0.IronSight"
#define WDB_MOD_bShowRangeFinder			"Scope.0.ShowRangeFinder"
#define WDB_MOD_v2RangeFinderPosition		"Scope.0.RangeFinderPosition"
#define WDB_MOD_nRangeFinderColor			"Scope.0.RangeFinderColor"
#define WDB_MOD_bShowCrosshair				"Scope.0.ShowCrosshair"
#define WDB_MOD_fHeadBobModifier			"Scope.0.HeadBobModifier"

#define WDB_MOD_nAISilencedFireSndRadius	"Silencer.0.AISilencedFireSndRadius"


#define WDB_GLOBAL_CATEGORY					WDB_ARSENAL_CATEGORY "Global"
#define	WDB_GLOBAL_RECORD					"Global"

#define WDB_GLOBAL_rPlayerWeapons			"PlayerWeapons"
#define WDB_GLOBAL_rPlayerGrenades			"PlayerGrenades"
#define WDB_GLOBAL_rWeaponPriority			"WeaponPriority"
#define WDB_GLOBAL_rConditionalPriority		"ConditionalPriority"
#define WDB_GLOBAL_rUnarmed					"Unarmed"
#define WDB_GLOBAL_rDefaultWeapons			"AdditionalDefaultWeapons"
#define WDB_GLOBAL_rMPDefaultWeapons		"MultiplayerDefaultWeapons"
#define WDB_GLOBAL_rGearOrder				"GearOrder"
#define WDB_GLOBAL_rLoadoutOrder			"LoadoutOrder"
#define WDB_GLOBAL_rFallbackLoadout			"FallbackLoadout"
#define WDB_GLOBAL_tDroppedWeaponLifeTime	"DroppedWeaponLifeTime"
#define WDB_GLOBAL_rDroppedGearItems		"DroppedGearItems"
#define WDB_GLOBAL_tDroppedGearLifeTime		"DroppedGearLifeTime"
#define WDB_GLOBAL_nMaxWeaponCapacity		"MaxWeaponCapacity"
#define WDB_GLOBAL_rDetonator				"Detonator"
#define WDB_GLOBAL_tSemiAutoBufferTime		"SemiAutoBufferTime"
#define WDB_GLOBAL_tProjectileExplosionDelay	"ProjectileExplosionDelay"
#define WDB_GLOBAL_sDefaultAnimationName	"DefaultAnimationName"
#define WDB_GLOBAL_sSlowMoRechargerFXName		"SlowMoRechargerFXName"

#define WDB_LOADOUT_CATEGORY					WDB_ARSENAL_CATEGORY "Loadout"
#define WDB_LOADOUT_nNameId						"NameId"
#define WDB_LOADOUT_nDescId						"DescId"
#define WDB_LOADOUT_rWeapons					"Weapons"
#define WDB_LOADOUT_rGear						"Gear"


#define WDB_TURRET_CATEGORY						WDB_ARSENAL_CATEGORY "Turrets"

#define WDB_TURRET_rWeapon						"Weapon"
#define WDB_TURRET_sBaseModel					"BaseModel"
#define WDB_TURRET_sBaseMaterial				"BaseMaterial"
#define WDB_TURRET_bHideBase					"HideBase"
#define WDB_TURRET_bHideHUD					"HideHUD"
#define WDB_TURRET_fActivateDistance			"ActivateDistance"
#define WDB_TURRET_fActivateApproachAngle		"ActivateApproachAngle"
#define WDB_TURRET_vPlayerOffset				"PlayerOffset"
#define WDB_TURRET_v2PitchRange					"PitchRange"
#define WDB_TURRET_fYawLimit					"YawLimit"
#define	WDB_TURRET_sLoopFX						"LoopFX"
#define WDB_TURRET_sDamageFX					"DamageFX"
#define WDB_TURRET_DamageState					"DamageState"
#define WDB_TURRET_fHealthPercent				"HealthPercent"
#define WDB_TURRET_fxClientFX					"ClientFX"
#define WDB_TURRET_bSmoothShutdown				"SmoothShutdown"
#define WDB_TURRET_bPersistAcrossDamageStates	"PersistAcrossDamageStates"
#define WDB_TURRET_tmDestroyedDeactivationDelay	"DestroyedDeactivationDelay"


BEGIN_DATABASE_CATEGORY( RefillStations, WDB_ARSENAL_CATEGORY "RefillStations" )
	DEFINE_GETRECORDATTRIB( Model, HRECORD );
	DEFINE_GETRECORDATTRIB( Radius, uint32 );
	DEFINE_GETRECORDATTRIB( UpdateRate, uint32 );
	DEFINE_GETRECORDATTRIB( RefillSound, HRECORD );
	DEFINE_GETRECORDATTRIB( DepletedSound, HRECORD );
	DEFINE_GETSTRUCTATTRIB( Health, AmountPerUpdate, uint32 );
	DEFINE_GETSTRUCTATTRIB( Health, TotalAmount, int32 );
	DEFINE_GETSTRUCTATTRIB( Armor, AmountPerUpdate, uint32 );
	DEFINE_GETSTRUCTATTRIB( Armor, TotalAmount, int32 );
	DEFINE_GETSTRUCTATTRIB( Ammo, Ammo, HRECORD );
	DEFINE_GETSTRUCTATTRIB( Ammo, AmountPerUpdate, uint32 );
	DEFINE_GETSTRUCTATTRIB( Ammo, TotalAmount, int32 );
	DEFINE_GETSTRUCTATTRIB( SlowMo, AmountPerUpdate, uint32 );
	DEFINE_GETSTRUCTATTRIB( SlowMo, TotalAmount, int32 );
END_DATABASE_CATEGORY( );

#define WDB_REFILL_CATEGORY						WDB_ARSENAL_CATEGORY "RefillStations"

#define WDB_REFILL_rModel						"Model"
#define WDB_REFILL_nRadius						"Radius"
#define WDB_REFILL_tUpdateRate					"UpdateRate"
#define WDB_REFILL_rRefillSound					"RefillSound"
#define WDB_REFILL_rDepletedSound				"DepletedSound"



#define WDB_INVALID_WEAPON_INDEX				(0xFF)

#define WDB_LOWEST_CONDITIONAL_PRIORITY			(-1)

#define USE_AI_DATA true

class CWeaponDB;
extern CWeaponDB* g_pWeaponDB;

enum AutoTargetType
{
	AT_NEVER = 0,
	AT_EASY,
	AT_ALWAYS
};

enum WeaponState
{
	W_INACTIVE,
	W_IDLE,
	W_FIRING,
	W_FIRED,
	W_ALT_FIRING,
	W_GREN_THROWING,
	W_RELOADING,
	W_FIRING_NOAMMO,
	W_SELECT,
	W_DESELECT,
	W_AUTO_SWITCH,
	W_BLOCKING,
	W_CHECKING_AMMO,
};

enum AmmoType
{
	// NOTE:  These values are important.  The Weapons database
	// refers to these numbers, DO NOT change them!
	VECTOR=0,
	PROJECTILE=1,
	TRIGGER=2,
	UNKNOWN_AMMO_TYPE=3
};

enum ModType
{
	SILENCER=0,
	SCOPE,
	UNKNOWN_MOD_TYPE
};


typedef HRECORD	HWEAPON;
typedef HRECORD	HWEAPONDATA;
typedef HRECORD	HAMMO;
typedef HRECORD	HAMMODATA;
typedef HRECORD	HGEAR;
typedef HRECORD	HMOD;
typedef HRECORD	HWEAPONANI;
typedef HRECORD HTURRET;

class CWeaponDB : public CGameDatabaseMgr
{
	DECLARE_SINGLETON( CWeaponDB );

	public :	// Methods...

		bool	Init( const char *szDatabaseFile = DB_Default_File );

		void	Term() {};

		void	ReadWeapon( const char* szWeaponString, HWEAPON &hWeapon, HAMMO &hAmmo, bool bUseAIData );

		HWEAPON	GetWeaponRecord( const char *pszWeapon ) const;
		HWEAPON	GetWeaponRecord( uint8 nIndex ) const;
		HWEAPONDATA GetWeaponData( HWEAPON hWeapon, bool bUseAIStats);

		HAMMO	GetAmmoRecord( const char *pszAmmo ) const;
		HAMMO	GetAmmoRecord( uint8 nIndex ) const;
		HAMMODATA GetAmmoData( HAMMO hAmmo, bool bUseAIStats = false);

		HGEAR	GetGearRecord( const char *pszGear ) const;
		HGEAR	GetGearRecord( uint8 nIndex ) const;

		// Preprocessed damagetype protection for gear.
		struct GearDamageTypeProtection
		{
			GearDamageTypeProtection( )
			{
				m_hGear = NULL;
				m_eDamageType = DT_INVALID;
			}
			HRECORD m_hGear;
			DamageType m_eDamageType;
		};
		typedef std::vector<GearDamageTypeProtection> GearDamageTypeProtectionList;
		GearDamageTypeProtectionList const& GetGearDamageTypeProtectionList( ) const { return m_vecGearDamageTypeProtection; }

		HMOD	GetModRecord( const char *pszMod ) const;
		HMOD	GetModRecord( uint8 nIndex ) const;

		HTURRET GetTurretRecord( const char *pszTurret ) const;
		HTURRET GetTurretRecord( uint8 nIndex ) const;

		HRECORD	GetMeleeColliderRecord( const char *pszMeleeCollider ) const;
		HRECORD	GetMeleeDamageRecord( const char *pszMeleeDamage ) const;

		const char* GetActionIcon( HRECORD hWeapon, int nIndex=0 );

		uint8		GetNumLoadouts( )	const;
		HRECORD		GetLoadout( uint8 nIndex ) const;
		HRECORD		GetFallbackLoadout(  ) const;

		uint8		GetNumWeapons( )	const	{ return m_nNumWeapons; }
		uint8		GetNumAmmo( )		const	{ return m_nNumAmmo; }
		uint8		GetNumGear( )		const	{ return m_nNumGear; }
		uint8		GetNumMods( )		const	{ return m_nNumMods; }
		uint8		GetNumTurrets( )	const	{ return m_nNumTurrets; }
		
		uint8		GetNumPlayerWeapons() const;
		bool		IsPlayerWeapon( HWEAPON hWeapon );
		uint8		GetPlayerWeaponIndex( HWEAPON hWeapon );
		HWEAPON		GetPlayerWeapon( uint8 nIndex ) const;

		uint8		GetNumPlayerGrenades() const;
		bool		IsPlayerGrenade( HWEAPON hWeapon );
		uint8		GetPlayerGrenadeIndex( HWEAPON hWeapon );
		HWEAPON		GetPlayerGrenade( uint8 nIndex ) const;

		uint8		GetNumDefaultWeaponPriorities() const;
		uint8		GetDefaultWeaponPriority( HWEAPON hWeapon );
		HWEAPON		GetWeaponFromDefaultPriority(uint8 nIndex) const;
		bool		IsBetterWeapon( HWEAPON hWeaponA, HWEAPON hWeaponB );

		uint32		GetConditionalPriority( HRECORD hGroup );
		bool		IsBetterConditionalGroup( HRECORD hGroupA, HRECORD hGroupB );

		uint8		GetNumDisplayGear() const;
		HGEAR		GetDisplayGear( uint8 nIndex ) const;

		uint8		GetWeaponCapacity() const;
		float		GetSemiAutoBufferTime() const;
		
		HCATEGORY	GetWeaponsCategory( )		const	{ return m_hCatWeapons; }
		HCATEGORY	GetAmmoCategory( )			const	{ return m_hCatAmmo; }
		HCATEGORY	GetGearCategory( )			const	{ return m_hCatGear; }
		HCATEGORY	GetModsCategory( )			const	{ return m_hCatMods; }
		HCATEGORY	GetTurretsCategory( )		const	{ return m_hCatTurrets; }
		HRECORD		GetGlobalRecord( )			const	{ return m_hRecGlobal; }

		
		//Find the first weapon that uses this type of ammo...
		HWEAPON		GetWeaponFromAmmo( HAMMO hAmmo, bool bUseAIData );
		//Can the specified weapon use the specified ammo
		bool		CanWeaponUseAmmo( HWEAPON hWeapon, HAMMO hAmmo, bool bUseAIData);

		HWEAPON		GetWeaponFromMod( HMOD hMod, bool bUseAIData );

		
		void		Restrict( HRECORD hRecord );
		bool		IsRestricted( HRECORD hRecord );

		uint8		GetAmmoSurfaceFXTypes( const char **pszSurfaceFXTypesArray, uint8 nArrayLen ) const;

		HWEAPON		GetUnarmedRecord( ) const { return m_hUnarmedRecord; }
		HWEAPON		GetDetonatorRecord( ) const { return m_hDetonatorRecord; }

		inline HRECORD	GetAmmoInstDamageTypeRecord(HAMMO hAmmo, bool bUseAIStats = false) const;
		inline HRECORD	GetAmmoAreaDamageTypeRecord(HAMMO hAmmo, bool bUseAIStats = false) const;
		inline HRECORD	GetAmmoProgDamageTypeRecord(HAMMO hAmmo, bool bUseAIStats = false) const;

		inline DamageType	GetAmmoInstDamageType(HAMMO hAmmo, bool bUseAIStats = false) const;
		inline DamageType	GetAmmoAreaDamageType(HAMMO hAmmo, bool bUseAIStats = false) const;
		inline DamageType	GetAmmoProgDamageType(HAMMO hAmmo, bool bUseAIStats = false) const;

		uint32 GetModelFilesCRC( );

		// Calculate the damage factor for the specified weapon based on vector distance
		float GetEffectiveVectorRangeDamageFactor(HWEAPON hWeapon, float fDist, bool bUseAIData);

		// Gets the type of mod.
		ModType		GetModType( HMOD hMod ) const;

	private	:	// Members...

		HCATEGORY	m_hCatWeapons;
		HCATEGORY	m_hCatAmmo;
		HCATEGORY	m_hCatMeleeCollider;
		HCATEGORY	m_hCatMeleeDamage;
		HCATEGORY	m_hCatGear;
		HCATEGORY	m_hCatMods;
		HCATEGORY	m_hCatLoadouts;
		HCATEGORY	m_hCatGlobal;
		HCATEGORY	m_hCatTurrets;
		HRECORD		m_hRecGlobal;
		
		uint8		m_nNumWeapons;
		uint8		m_nNumAmmo;
		uint8		m_nNumGear;
		uint8		m_nNumMods;
		uint8		m_nNumTurrets;
		
		HWEAPON		m_hUnarmedRecord;
		HWEAPON		m_hDetonatorRecord;

		typedef std::vector<HRECORD, LTAllocator<HRECORD, LT_MEM_TYPE_GAMECODE> > HRecordArray;
		HRecordArray	m_vecRestricted;

		GearDamageTypeProtectionList m_vecGearDamageTypeProtection;
};

inline bool FiredWeapon(WeaponState eState)
{
	return (eState == W_FIRED);
}

inline HRECORD CWeaponDB::GetAmmoInstDamageTypeRecord(HAMMO hAmmo, bool bUseAIStats) const
{
	return GetRecordLink(g_pWeaponDB->GetAmmoData(hAmmo,bUseAIStats),WDB_AMMO_rInstDamageType);
}
inline HRECORD CWeaponDB::GetAmmoAreaDamageTypeRecord(HAMMO hAmmo, bool bUseAIStats) const
{
	return GetRecordLink(g_pWeaponDB->GetAmmoData(hAmmo,bUseAIStats),WDB_AMMO_rAreaDamageType);
}
inline HRECORD CWeaponDB::GetAmmoProgDamageTypeRecord(HAMMO hAmmo, bool bUseAIStats) const
{
	return GetRecordLink(g_pWeaponDB->GetAmmoData(hAmmo,bUseAIStats),WDB_AMMO_rProgDamageType);
}

inline DamageType CWeaponDB::GetAmmoInstDamageType(HAMMO hAmmo, bool bUseAIStats) const
{
	return g_pDTDB->GetDamageType(GetAmmoInstDamageTypeRecord(hAmmo,bUseAIStats));
}
inline DamageType CWeaponDB::GetAmmoAreaDamageType(HAMMO hAmmo, bool bUseAIStats) const
{
	return g_pDTDB->GetDamageType(GetAmmoAreaDamageTypeRecord(hAmmo,bUseAIStats));
}
inline DamageType CWeaponDB::GetAmmoProgDamageType(HAMMO hAmmo, bool bUseAIStats) const
{
	return g_pDTDB->GetDamageType(GetAmmoProgDamageTypeRecord(hAmmo,bUseAIStats));
}



////////////////////////////////////////////////////////////////////////////
//
// CWeaponDBPlugin is used to help facilitate populating the WorldEdit object
// properties that use WeaponDB
//
////////////////////////////////////////////////////////////////////////////
#ifdef _SERVERBUILD

#include "iobjectplugin.h"

class CWeaponDBPlugin;
extern CWeaponDBPlugin *g_pWeaponDBPlugin;

class CWeaponDBPlugin : public IObjectPlugin
{
	private:

		CWeaponDBPlugin();
		CWeaponDBPlugin( const CWeaponDBPlugin &other );
		CWeaponDBPlugin& operator=( const CWeaponDBPlugin &other );
		~CWeaponDBPlugin();


	public:

		static CWeaponDBPlugin& Instance() { static CWeaponDBPlugin sPlugin; return sPlugin; }

		bool Init( );

		// This will be removed once the FXMgr and it's plugin are converted
		// to a database.
		LTRESULT PreHook_EditStringList(
			const char* szRezPath,
			const char* szPropName,
			char** aszStrings,
			uint32* pcStrings,
			const uint32 cMaxStrings,
			const uint32 cMaxStringLength);

		static void PopulateStringList(char** aszStrings, uint32* pcStrings,
			const uint32 cMaxStrings, const uint32 cMaxStringLength, bool bPickupList);

	protected :


};

#endif // _SERVERBUILD


#endif // __WEAPON_DATABASE_H__
