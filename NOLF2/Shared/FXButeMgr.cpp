// ----------------------------------------------------------------------- //
//
// MODULE  : FXButeMgr.cpp
//
// PURPOSE : FXButeMgr implementation - Controls attributes of special fx
//
// CREATED : 12/08/98
//
// (c) 1999-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#include "stdafx.h"
#include "FXButeMgr.h"
#include "WeaponFXTypes.h"
#include "CommonUtilities.h"
#include "SurfaceFunctions.h"
#include "WeaponMgr.h"
#include "FXFlags.h"

#ifdef _CLIENTBUILD
#include "ParticleShowerFX.h"
#include "PolyDebrisFX.h"
#include "ParticleExplosionFX.h"
#include "GameClientShell.h"
#include "InterfaceMgr.h"
#include "BeamFX.h"
#include "CMoveMgr.h"

extern CGameClientShell* g_pGameClientShell;

#else

#include "AIStimulusMgr.h"

#endif


#define FXBMGR_IMPACTFX_TAG				"ImpactFX"
#define FXBMGR_IMPACTFX_NAME			"Name"
#define FXBMGR_IMPACTFX_SOUND			"Sound"
#define FXBMGR_IMPACTFX_SOUNDRADIUS		"SoundRadius"
#define FXBMGR_IMPACTFX_AISOUNDRADIUS	"AISoundRadius"
#define FXBMGR_IMPACTFX_AIIGNORESURFACE	"AIIgnoreSurface"
#define FXBMGR_IMPACTFX_AIALARMLEVEL    "AIAlarmLevel"
#define FXBMGR_IMPACTFX_AISTIMULUSTYPE  "AIStimulusType"
#define FXBMGR_IMPACTFX_CREATEMARK		"CreateMark"
#define FXBMGR_IMPACTFX_CREATESMOKE		"CreateSmoke"
#define FXBMGR_IMPACTFX_IMPACTONSKY		"ImpactOnSky"
#define FXBMGR_IMPACTFX_IGNOREFLESH		"IgnoreFlesh"
#define FXBMGR_IMPACTFX_IGNORELIQUID	"IgnoreLiquid"
#define FXBMGR_IMPACTFX_DOSURFACEFX		"DoSurfaceFX"
#define FXBMGR_IMPACTFX_SCREENTINT		"ScreenTint"
#define FXBMGR_IMPACTFX_BLASTMARK		"BlastMark"
#define FXBMGR_IMPACTFX_PUSHERNAME		"PusherName"
#define FXBMGR_IMPACTFX_MARK			"Mark"
#define FXBMGR_IMPACTFX_MARKSCALE		"MarkScale"
#define FXBMGR_IMPACTFX_TINTCOLOR		"TintColor"
#define FXBMGR_IMPACTFX_TINTRAMPUP		"TintRampUp"
#define FXBMGR_IMPACTFX_TINTRAMPDOWN	"TintRampDown"
#define FXBMGR_IMPACTFX_TINTMAXTIME		"TintMaxTime"
#define FXBMGR_IMPACTFX_BLASTCOLOR		"BlastColor"
#define FXBMGR_IMPACTFX_BLASTTIMEMIN	"BlastTimeMin"
#define FXBMGR_IMPACTFX_BLASTTIMEMAX	"BlastTimeMax"
#define FXBMGR_IMPACTFX_BLASTFADEMIN	"BlastFadeMin"
#define FXBMGR_IMPACTFX_BLASTFADEMAX	"BlastFadeMax"
#define	FXBMGR_IMPACTFX_FXNAME			"FXName"


#define FXBMGR_FIREFX_TAG				"FireFX"
#define FXBMGR_FIREFX_NAME				"Name"
#define	FXBMGR_FIREFX_FXNAME			"FXName"
#define FXBMGR_FIREFX_EJECTSHELLS		"EjectShells"
#define FXBMGR_FIREFX_FIRESOUND			"FireSound"
#define FXBMGR_FIREFX_EXITMARK			"ExitMark"
#define FXBMGR_FIREFX_EXITDEBRIS		"ExitDebris"
#define FXBMGR_FIREFX_SHELLMODEL		"ShellModel"
#define FXBMGR_FIREFX_SHELLSKIN			"ShellSkin"
#define FXBMGR_FIREFX_SHELLSCALE		"ShellScale"
#define FXBMGR_FIREFX_BEAMFXNAME		"BeamFXName"

#define FXBMGR_PROJECTILEFX_TAG                   "ProjectileFX"
#define FXBMGR_PROJECTILEFX_NAME                  "Name"
#define FXBMGR_PROJECTILEFX_FXNAME                "FXName"
#define FXBMGR_PROJECTILEFX_FXLOOP                "FXLoop"
#define FXBMGR_PROJECTILEFX_FXSMOOTHSHUTDOWN      "FXSmoothShutdown"
#define FXBMGR_PROJECTILEFX_SMOKETRAIL            "SmokeTrail"
#define FXBMGR_PROJECTILEFX_FLARE                 "Flare"
#define FXBMGR_PROJECTILEFX_LIGHT                 "Light"
#define FXBMGR_PROJECTILEFX_FLYSOUND              "FlySound"
#define FXBMGR_PROJECTILEFX_CLASS                 "Class"
#define FXBMGR_PROJECTILEFX_CLASSDATA             "ClassData"
#define FXBMGR_PROJECTILEFX_MODEL                 "Model"
#define FXBMGR_PROJECTILEFX_MODELSCALE            "ModelScale"
#define FXBMGR_PROJECTILEFX_SKIN                  "Skin"
#define FXBMGR_PROJECTILEFX_SOUND                 "Sound"
#define FXBMGR_PROJECTILEFX_SOUNDRADIUS           "SoundRadius"
#define FXBMGR_PROJECTILEFX_FLARESPRITE           "FlareSprite"
#define FXBMGR_PROJECTILEFX_FLARESCALE            "FlareScale"
#define FXBMGR_PROJECTILEFX_LIGHTCOLOR            "LightColor"
#define FXBMGR_PROJECTILEFX_LIGHTRADIUS           "LightRadius"
#define FXBMGR_PROJECTILEFX_GRAVITY               "Gravity"
#define FXBMGR_PROJECTILEFX_GRAVITYOVERRIDE       "GravityOverride"
#define FXBMGR_PROJECTILEFX_LIFETIME              "Lifetime"
#define FXBMGR_PROJECTILEFX_VELOCITY              "Velocity"
#define FXBMGR_PROJECTILEFX_ALTVELOCITY           "AltVelocity"
#define FXBMGR_PROJECTILEFX_FIRE_OFFSET           "FireOffset"
#define FXBMGR_PROJECTILEFX_SMOKETRAILTYPE        "SmokeTrailType"
#define FXBMGR_PROJECTILEFX_CANHITSAMEKIND        "CanHitSameKind"
#define FXBMGR_PROJECTILEFX_RICOCHETFXNAME        "RicochetFXName"
#define FXBMGR_PROJECTILEFX_MAXRICOCHETANGLE      "MaxRicochetAngle"
#define FXBMGR_PROJECTILEFX_MAXRICOCHETS          "MaxRicochets"

#define FXBMGR_CLASSDATA_NAME			"Name"

#define FXBMGR_PROXCLASS_TAG			"ProxClassData"
#define FXBMGR_PROXCLASS_ACTRADIUS		"ActivateRadius"
#define FXBMGR_PROXCLASS_ARMDELAY		"ArmDelay"
#define FXBMGR_PROXCLASS_ARMSND			"ArmSound"
#define FXBMGR_PROXCLASS_ARMSNDRADIUS	"ArmSndRadius"
#define FXBMGR_PROXCLASS_ACTDELAY		"ActivateDelay"
#define FXBMGR_PROXCLASS_ACTSND			"ActivateSound"
#define FXBMGR_PROXCLASS_ACTSNDRADIUS	"ActivateSndRadius"

#define FXBMGR_KITTYCLASS_TAG			"KittyClassData"
#define	FXBMGR_KITTYCLASS_ARMSND		"ArmSound"
#define	FXBMGR_KITTYCLASS_ACTSND		"ActivateSound"
#define FXBMGR_KITTYCLASS_SNDRADIUS		"SoundRadius"
#define	FXBMGR_KITTYCLASS_ACTRADIUS		"ActivateRadius"
#define FXBMGR_KITTYCLASS_DETRADIUS		"DetonateRadius"
#define FXBMGR_KITTYCLASS_ARMDELAY		"ArmDelay"
#define FXBMGR_KITTYCLASS_DETTIME		"DetonateTime"
#define FXBMGR_KITTYCLASS_CHASEVEL		"ChaseVelocity"
#define FXBMGR_KITTYCLASS_LOOPARMSND	"LoopArmSound"
#define FXBMGR_KITTYCLASS_ARMEDFX		"ArmedFX"
#define FXBMGR_KITTYCLASS_ARMEDFX_RED	"ArmedFXRedTeam"
#define FXBMGR_KITTYCLASS_ARMEDFX_BLUE	"ArmedFXBlueTeam"
#define FXBMGR_KITTYCLASS_LOOPARMEDFX	"LoopArmedFX"	

#define FXBMGR_BEARTRAPCLASS_TAG		"BearTrapClassData"
#define	FXBMGR_BEARTRAPCLASS_ARMDELAY	"ArmDelay"
#define FXBMGR_BEARTRAPCLASS_DETRADIUS	"DetonateRadius"

#define FXBMGR_BANANACLASS_TAG			"BananaClassData"
#define FXBMGR_BANANACLASS_ARMDELAY		"ArmDelay"
#define	FXBMGR_BANANACLASS_DETRADIUS	"DetonateRadius"

#define FXBMGR_SPEARCLASS_TAG			"SpearClassData"
#define FXBMGR_SPEARCLASS_STICKPERCENT	"StickPercent"
#define FXBMGR_SPEARCLASS_CANWALLSTICK	"CanWallStick"
#define FXBMGR_SPEARCLASS_DIMSSCALE		"DimsScale"

#define FXBMGR_SPAWNCLASS_TAG			"SpawnClassData"
#define	FXBMGR_SPAWNCLASS_SPAWNOBJECT	"SpawnObject"
#define	FXBMGR_SPAWNCLASS_OBJECTPROP	"ObjectProp"
#define FXBMGR_SPAWNCLASS_STICKTOWORLD	"StickToWorld"

#define FXBMGR_DISCCLASS_TAG									"DiscClassData"
#define FXBMGR_DISCCLASS_INITIALANGLEMIN						"InitialAngleMin"
#define FXBMGR_DISCCLASS_INITIALANGLEMAX						"InitialAngleMax"
#define FXBMGR_DISCCLASS_INITIALANGLESURGE						"InitialAngleSurge"
#define FXBMGR_DISCCLASS_RETURNVELOCITY							"ReturnVelocity"
#define FXBMGR_DISCCLASS_RETURNHEIGHTOFFSET						"ReturnHeightOffset"
#define FXBMGR_DISCCLASS_RETURNFXNAME							"ReturnFXName"
#define FXBMGR_DISCCLASS_TURNRATEMIN							"TurnRateMin"
#define FXBMGR_DISCCLASS_TURNRATEMAX							"TurnRateMax"
#define FXBMGR_DISCCLASS_TURNRATESURGE							"TurnRateSurge"
#define FXBMGR_DISCCLASS_INCIDENTANGLETOCONTROLLINEMIN			"IncidentAngleToControlLineMin"
#define FXBMGR_DISCCLASS_INCIDENTANGLETOCONTROLLINEMAX			"IncidentAngleToControlLineMax"
#define FXBMGR_DISCCLASS_INCIDENTANGLETOCONTROLLINESURGE		"IncidentAngleToControlLineSurge"
#define FXBMGR_DISCCLASS_INCIDENTANGLETOCONTROLLINEDECAYMIN		"IncidentAngleToControlLineDecayMin"
#define FXBMGR_DISCCLASS_INCIDENTANGLETOCONTROLLINEDECAYMAX		"IncidentAngleToControlLineDecayMax"
#define FXBMGR_DISCCLASS_INCIDENTANGLETOCONTROLLINEDECAYSURGE	"IncidentAngleToControlLineDecaySurge"
#define FXBMGR_DISCCLASS_SWATDEFENDPVFXNAME						"SwatDefendPVFXName"
#define FXBMGR_DISCCLASS_SWATCRITICALDEFENDPVFXNAME				"SwatCriticalDefendPVFXName"
#define FXBMGR_DISCCLASS_SWATCRITICALDEFENDTHRESHOLD			"SwatCriticalDefendThreshold"
#define FXBMGR_DISCCLASS_HOLDDEFENDPVFXNAME						"HoldDefendPVFXName"
#define FXBMGR_DISCCLASS_ARMDEFENDPVFXNAME						"ArmDefendPVFXName"
#define FXBMGR_DISCCLASS_SWATDEFENDSTARTDEFENDPERCENTAGE		"SwatDefendStartDefendPercentage"
#define FXBMGR_DISCCLASS_SWATDEFENDENDDEFENDPERCENTAGE			"SwatDefendEndDefendPercentage"
#define FXBMGR_DISCCLASS_SWATDEFENDMIDPOINT						"SwatDefendMidpoint"
#define FXBMGR_DISCCLASS_SWATDEFENDMAXDEFENDPERCENTAGE			"SwatDefendMaxDefendPercentage"
#define FXBMGR_DISCCLASS_SWATDEFENDSTARTMAXDEFENDPERCENTAGE		"SwatDefendStartMaxDefendPercentage"
#define FXBMGR_DISCCLASS_SWATDEFENDENDMAXDEFENDPERCENTAGE		"SwatDefendEndMaxDefendPercentage"
#define FXBMGR_DISCCLASS_HOLDDEFENDSTARTDEFENDPERCENTAGE		"HoldDefendStartDefendPercentage"
#define FXBMGR_DISCCLASS_HOLDDEFENDENDDEFENDPERCENTAGE			"HoldDefendEndDefendPercentage"
#define FXBMGR_DISCCLASS_HOLDDEFENDMAXDEFENDPERCENTAGE			"HoldDefendMaxDefendPercentage"
#define FXBMGR_DISCCLASS_HOLDDEFENDSTARTMAXDEFENDPERCENTAGE		"HoldDefendStartMaxDefendPercentage"
#define FXBMGR_DISCCLASS_HOLDDEFENDENDMAXDEFENDPERCENTAGE		"HoldDefendEndMaxDefendPercentage"
#define FXBMGR_DISCCLASS_SWATDEFENDORIENTATIONMINDEFENDPERCENTAGE		"SwatDefendOrientationMinDefendPercentage"
#define FXBMGR_DISCCLASS_SWATDEFENDORIENTATIONMAXDEFENDPERCENTAGE		"SwatDefendOrientationMaxDefendPercentage"
#define FXBMGR_DISCCLASS_SWATDEFENDORIENTATIONDEADZONE					"SwatDefendOrientationDeadZone"
#define FXBMGR_DISCCLASS_SWATDEFENDORIENTATIONMAXZONE					"SwatDefendOrientationMaxZone"
#define FXBMGR_DISCCLASS_HOLDDEFENDORIENTATIONMINDEFENDPERCENTAGE		"HoldDefendOrientationMinDefendPercentage"
#define FXBMGR_DISCCLASS_HOLDDEFENDORIENTATIONMAXDEFENDPERCENTAGE		"HoldDefendOrientationMaxDefendPercentage"
#define FXBMGR_DISCCLASS_HOLDDEFENDORIENTATIONDEADZONE					"HoldDefendOrientationDeadZone"
#define FXBMGR_DISCCLASS_HOLDDEFENDORIENTATIONMAXZONE					"HoldDefendOrientationMaxZone"

#define FXBMGR_CLUSTERDISCCLASS_TAG								"ClusterDiscClassData"
#define FXBMGR_CLUSTERDISCCLASS_SHARDHORIZONTALSPREADMIN		"ShardHorizontalSpreadMin"
#define FXBMGR_CLUSTERDISCCLASS_SHARDHORIZONTALSPREADMAX		"ShardHorizontalSpreadMax"
#define FXBMGR_CLUSTERDISCCLASS_SHARDHORIZONTALSPREADSURGE		"ShardHorizontalSpreadSurge"
#define FXBMGR_CLUSTERDISCCLASS_SHARDHORIZONTALPERTURBMIN		"ShardHorizontalPerturbMin"
#define FXBMGR_CLUSTERDISCCLASS_SHARDHORIZONTALPERTURBMAX		"ShardHorizontalPerturbMax"
#define FXBMGR_CLUSTERDISCCLASS_SHARDHORIZONTALPERTURBSURGE		"ShardHorizontalPerturbSurge"
#define FXBMGR_CLUSTERDISCCLASS_SHARDVERTICALSPREADMIN			"ShardVerticalSpreadMin"
#define FXBMGR_CLUSTERDISCCLASS_SHARDVERTICALSPREADMAX			"ShardVerticalSpreadMax"
#define FXBMGR_CLUSTERDISCCLASS_SHARDVERTICALSPREADSURGE		"ShardVerticalSpreadSurge"
#define FXBMGR_CLISTERDISCCLASS_SHARDWEAPONNAME					"ShardProjectileName"
#define FXBMGR_CLUSTERDISCCLASS_SHARDSTOTALMIN					"ShardsTotalMin"
#define FXBMGR_CLUSTERDISCCLASS_SHARDSTOTALMAX					"ShardsTotalMax"
#define FXBMGR_CLUSTERDISCCLASS_SHARDSTOTALSURGE				"ShardsTotalSurge"
#define FXBMGR_CLUSTERDISCCLASS_SHARDSTOTALPERTURBMIN			"ShardsTotalPerturbMin"
#define FXBMGR_CLUSTERDISCCLASS_SHARDSTOTALPERTURBMAX			"ShardsTotalPerturbMax"
#define FXBMGR_CLUSTERDISCCLASS_SHARDSTOTALPERTURBSURGE			"ShardsTotalPerturbSurge"

const char FXBMGR_CLUSTERAUTOBURSTDISCCLASS_TAG[] = 			"ClusterAutoBurstDiscClassData";
const char FXBMGR_CLUSTERAUTOBURSTCLASS_MINBURSTDIST[] =		"AutoBurstMinDist";
const char FXBMGR_CLUSTERAUTOBURSTCLASS_MAXBURSTDIST[] =		"AutoBurstMaxDist";

#define FXBMGR_SCALEFX_TAG				"ScaleFX"
#define FXBMGR_PSHOWERFX_TAG			"PShowerFX"
#define FXBMGR_POLYDEBRISFX_TAG			"PolyDebrisFX"

#define FXBMGR_PEXPLFX_TAG				"PExplFX"
#define	FXBMGR_PEXPLFX_NAME				"Name"
#define	FXBMGR_PEXPLFX_FILE				"File"
#define	FXBMGR_PEXPLFX_POSOFFSET		"PosOffset"
#define	FXBMGR_PEXPLFX_NUMPERPUFF		"NumPerPuff"
#define	FXBMGR_PEXPLFX_NUMEMITTERS		"NumEmitters"
#define	FXBMGR_PEXPLFX_NUMSTEPS			"NumSteps"
#define	FXBMGR_PEXPLFX_CREATEDEBRIS		"CreateDebris"
#define	FXBMGR_PEXPLFX_ROTATEDEBRIS		"RotateDebris"
#define	FXBMGR_PEXPLFX_IGNOREWIND		"IgnoreWind"
#define	FXBMGR_PEXPLFX_DOBUBBLES		"DoBubbles"
#define	FXBMGR_PEXPLFX_COLOR1			"Color1"
#define	FXBMGR_PEXPLFX_COLOR2			"Color2"
#define	FXBMGR_PEXPLFX_MINVEL			"MinVel"
#define	FXBMGR_PEXPLFX_MAXVEL			"MaxVel"
#define	FXBMGR_PEXPLFX_MINDRIFTVEL		"MinDriftVel"
#define	FXBMGR_PEXPLFX_MAXDRIFTVEL		"MaxDriftVel"
#define	FXBMGR_PEXPLFX_LIFETIME			"LifeTime"
#define	FXBMGR_PEXPLFX_FADETIME			"FadeTime"
#define	FXBMGR_PEXPLFX_OFFSETTIME		"OffsetTime"
#define	FXBMGR_PEXPLFX_RADIUS			"Radius"
#define	FXBMGR_PEXPLFX_GRAVITY			"Gravity"
#define	FXBMGR_PEXPLFX_ADDITIVE			"Additive"
#define	FXBMGR_PEXPLFX_MULTIPLY			"Multiply"

#define FXBMGR_DLIGHTFX_TAG				"DLightFX"
#define	FXBMGR_DLIGHTFX_NAME			"Name"
#define	FXBMGR_DLIGHTFX_COLOR			"Color"
#define	FXBMGR_DLIGHTFX_MINRADIUS		"MinRadius"
#define	FXBMGR_DLIGHTFX_MAXRADIUS		"MaxRadius"
#define	FXBMGR_DLIGHTFX_MINTIME			"MinTime"
#define	FXBMGR_DLIGHTFX_MAXTIME			"MaxTime"
#define	FXBMGR_DLIGHTFX_RAMPUPTIME		"RampUpTime"
#define	FXBMGR_DLIGHTFX_RAMPDOWNTIME	"RampDownTime"

#define FXBMGR_SOUNDFX_TAG				"SoundFX"
#define	FXBMGR_SOUNDFX_NAME				"Name"
#define	FXBMGR_SOUNDFX_FILE				"File"
#define	FXBMGR_SOUNDFX_LOOP				"Loop"
#define	FXBMGR_SOUNDFX_RADIUS			"Radius"
#define	FXBMGR_SOUNDFX_PITCHSHIFT		"PitchShift"

#define FXBMGR_PUSHERFX_TAG				"PusherFX"
#define	FXBMGR_PUSHERFX_NAME			"Name"
#define	FXBMGR_PUSHERFX_RADIUS			"Radius"
#define	FXBMGR_PUSHERFX_STARTDELAY		"StartDelay"
#define	FXBMGR_PUSHERFX_DURATION		"Duration"
#define	FXBMGR_PUSHERFX_STRENGTH		"Strength"

#define FXBMGR_PVFX_TAG					"PVFX"
#define FXBMGR_PVFX_NAME				"Name"
#define FXBMGR_PVFX_SOCKET				"Socket"
#define FXBMGR_PVFX_SCALENAME			"ScaleName"
#define FXBMGR_PVFX_DLIGHTNAME			"DLightName"
#define FXBMGR_PVFX_SOUNDNAME			"SoundName"

#define FXBMGR_PARTMUZZLEFX_TAG			"ParticleMuzzleFX"
#define FXBMGR_PARTMUZZLEFX_NAME		"Name"
#define FXBMGR_PARTMUZZLEFX_FILE		"File"
#define FXBMGR_PARTMUZZLEFX_LENGTH		"Length"
#define FXBMGR_PARTMUZZLEFX_DURATION	"Duration"
#define FXBMGR_PARTMUZZLEFX_RADIUS		"Radius"
#define FXBMGR_PARTMUZZLEFX_MAXSCALE	"MaxScale"
#define FXBMGR_PARTMUZZLEFX_NUMBER		"NumParticles"
#define FXBMGR_PARTMUZZLEFX_COLOR1		"Color1"
#define FXBMGR_PARTMUZZLEFX_COLOR2		"Color2"
#define	FXBMGR_PARTMUZZLEFX_ADDITIVE	"Additive"
#define	FXBMGR_PARTMUZZLEFX_MULTIPLY	"Multiply"

#define FXBMGR_MUZZLEFX_TAG				"MuzzleFX"
#define FXBMGR_MUZZLEFX_DURATION		"Duration"
#define FXBMGR_MUZZLEFX_NAME			"Name"
#define FXBMGR_MUZZLEFX_PMUZZLEFXNAME	"PMuzzleFXName"
#define FXBMGR_MUZZLEFX_SCALEFXNAME		"ScaleFXName"
#define FXBMGR_MUZZLEFX_DLIGHTFXNAME	"DLightFXName"

#define FXBMGR_TRACERFX_TAG				"TracerFX"
#define FXBMGR_TRACERFX_NAME			"Name"
#define FXBMGR_TRACERFX_TEXTURE			"Texture"
#define FXBMGR_TRACERFX_FREQUENCY		"Frequency"
#define FXBMGR_TRACERFX_VELOCITY		"Velocity"
#define FXBMGR_TRACERFX_WIDTH			"Width"
#define FXBMGR_TRACERFX_INITIALALPHA	"InitialAlpha"
#define FXBMGR_TRACERFX_FINALALPHA		"FinalAlpha"
#define FXBMGR_TRACERFX_COLOR			"Color"

#define FXBMGR_BEAMFX_TAG				"BeamFX"
#define FXBMGR_BEAMFX_NAME				"Name"
#define FXBMGR_BEAMFX_TEXTURE			"Texture"
#define FXBMGR_BEAMFX_DURATION			"Duration"
#define FXBMGR_BEAMFX_WIDTH				"Width"
#define FXBMGR_BEAMFX_INITIALALPHA		"InitialAlpha"
#define FXBMGR_BEAMFX_FINALALPHA		"FinalAlpha"
#define FXBMGR_BEAMFX_COLOR				"Color"
#define FXBMGR_BEAMFX_ALIGNUP			"AlignUp"
#define FXBMGR_BEAMFX_ALIGNFLAT			"AlignFlat"

#define	FXBMGR_SPRINKLEFX_TAG			"SprinkleFX"
#define FXBMGR_SPRINKLEFX_NAME			"Name"
#define FXBMGR_SPRINKLEFX_FILENAME		"FileName"
#define	FXBMGR_SPRINKLEFX_SKINNAME		"SkinName"
#define FXBMGR_SPRINKLEFX_COUNT			"Count"
#define FXBMGR_SPRINKLEFX_SPEED			"Speed"
#define FXBMGR_SPRINKLEFX_SIZE			"Size"
#define FXBMGR_SPRINKLEFX_SPAWNRADIUS	"SpawnRadius"
#define	FXBMGR_SPRINKLEFX_ANGLESVEL		"AnglesVel"
#define FXBMGR_SPRINKLEFX_COLORMIN		"ColorMin"
#define FXBMGR_SPRINKLEFX_COLORMAX		"ColorMax"


#define FXBMGR_MIN_BLASTMARK_RADIUS		100.0f

static char s_aTagName[30];
static char s_aAttName[100];
static char s_FileBuffer[MAX_CS_FILENAME_LEN];

CFXButeMgr* g_pFXButeMgr = LTNULL;


#ifndef _CLIENTBUILD
// Plugin statics

CFXButeMgr CFXButeMgrPlugin::sm_FXButeMgr;

#endif // _CLIENTBUILD

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::CFXButeMgr()
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CFXButeMgr::CFXButeMgr()
{
    m_ProjectileFXList.Init(LTTRUE);
    m_ProjClassDataList.Init(LTTRUE);
    m_ImpactFXList.Init(LTTRUE);
    m_FireFXList.Init(LTTRUE);
    m_ScaleFXList.Init(LTTRUE);
    m_PShowerFXList.Init(LTTRUE);
    m_PolyDebrisFXList.Init(LTTRUE);
    m_PExplFXList.Init(LTTRUE);
    m_DLightFXList.Init(LTTRUE);
    m_SoundFXList.Init(LTTRUE);
    m_PusherFXList.Init(LTTRUE);
    m_PVFXList.Init(LTTRUE);
    m_PartMuzzleFXList.Init(LTTRUE);
    m_MuzzleFXList.Init(LTTRUE);
    m_TracerFXList.Init(LTTRUE);
    m_BeamFXList.Init(LTTRUE);
    m_SprinkleFXList.Init( LTTRUE );
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::~CFXButeMgr()
//
//	PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CFXButeMgr::~CFXButeMgr()
{
	Term();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::Init()
//
//	PURPOSE:	Init mgr
//
// ----------------------------------------------------------------------- //

LTBOOL CFXButeMgr::Init(const char* szAttributeFile)
{
    if (g_pFXButeMgr || !szAttributeFile) return LTFALSE;
    if (!Parse(szAttributeFile)) return LTFALSE;


	// Set up global pointer...

	g_pFXButeMgr = this;


	// Build our lists...

	BuildScaleFXList(m_ScaleFXList, m_buteMgr, FXBMGR_SCALEFX_TAG);
	BuildPShowerFXList(m_PShowerFXList, m_buteMgr, FXBMGR_PSHOWERFX_TAG);
	BuildPolyDebrisFXList(m_PolyDebrisFXList, m_buteMgr, FXBMGR_POLYDEBRISFX_TAG);


	// Read in the properties for each projectile class data record...
	// NOTE: This must be done before the ProjectileFX records are
	// read in...

	// Read in the properties for the ProxClassData records...

	int nNum = 0;
	sprintf( s_aTagName, "%s%d", FXBMGR_PROXCLASS_TAG, nNum );

	while( m_buteMgr.Exist( s_aTagName ))
	{
		PROJECTILECLASSDATA* pPCD = debug_new(PROXCLASSDATA);

		if (pPCD && pPCD->Init(m_buteMgr, s_aTagName))
		{
			m_ProjClassDataList.AddTail(pPCD);
		}
		else
		{
			debug_delete( pPCD );
			return LTFALSE;
		}

		++nNum;
		sprintf( s_aTagName, "%s%d", FXBMGR_PROXCLASS_TAG, nNum );
	}

	// Read in the properties for the KittyClassData records...

	nNum = 0;
	sprintf( s_aTagName, "%s%d", FXBMGR_KITTYCLASS_TAG, nNum );

	while( m_buteMgr.Exist( s_aTagName ))
	{	
		PROJECTILECLASSDATA *pPCD = debug_new( KITTYCLASSDATA );

		if( pPCD && pPCD->Init( m_buteMgr, s_aTagName ))
		{
			m_ProjClassDataList.AddTail( pPCD );
		}
		else
		{
			debug_delete( pPCD );
			return LTFALSE;
		}

		++nNum;
		sprintf( s_aTagName, "%s%d", FXBMGR_KITTYCLASS_TAG, nNum );
	}

	// Read in the properties for the BearTrapClassData records...

	nNum = 0;
	sprintf( s_aTagName, "%s%d", FXBMGR_BEARTRAPCLASS_TAG, nNum );

	while( m_buteMgr.Exist( s_aTagName ))
	{
		PROJECTILECLASSDATA *pPCD = debug_new( BEARTRAPCLASSDATA );

		if( pPCD && pPCD->Init( m_buteMgr, s_aTagName ))
		{
			m_ProjClassDataList.AddTail( pPCD );
		}
		else
		{
			debug_delete( pPCD );
			return LTFALSE;
		}

		++nNum;
		sprintf( s_aTagName, "%s%d", FXBMGR_BEARTRAPCLASS_TAG, nNum );
	}

	// Read in the properties for the BananaClassData records...

	nNum = 0;
	sprintf( s_aTagName, "%s%d", FXBMGR_BANANACLASS_TAG, nNum );

	while( m_buteMgr.Exist( s_aTagName ))
	{
		PROJECTILECLASSDATA *pPCD = debug_new( BANANACLASSDATA );

		if( pPCD && pPCD->Init( m_buteMgr, s_aTagName ))
		{
			m_ProjClassDataList.AddTail( pPCD );
		}
		else
		{
			debug_delete( pPCD );
			return LTFALSE;
		}

		++nNum;
		sprintf( s_aTagName, "%s%d", FXBMGR_BANANACLASS_TAG, nNum );
	}

	// Read in the properties for the SpearClassData records...

	nNum = 0;
	sprintf( s_aTagName, "%s%d", FXBMGR_SPEARCLASS_TAG, nNum );

	while( m_buteMgr.Exist( s_aTagName ))
	{
		PROJECTILECLASSDATA *pPCD = debug_new( SPEARCLASSDATA );

		if( pPCD && pPCD->Init( m_buteMgr, s_aTagName ))
		{
			m_ProjClassDataList.AddTail( pPCD );
		}
		else
		{
			debug_delete( pPCD );
			return LTFALSE;
		}

		++nNum;
		sprintf( s_aTagName, "%s%d", FXBMGR_SPEARCLASS_TAG, nNum );
	}

	// Read in the properties for the SpawnClassData records...

	nNum = 0;
	sprintf( s_aTagName, "%s%d", FXBMGR_SPAWNCLASS_TAG, nNum );

	while( m_buteMgr.Exist( s_aTagName ))
	{
		PROJECTILECLASSDATA *pPCD = debug_new( SPAWNCLASSDATA );

		if( pPCD && pPCD->Init( m_buteMgr, s_aTagName ))
		{
			m_ProjClassDataList.AddTail( pPCD );
		}
		else
		{
			debug_delete( pPCD );
			return LTFALSE;
		}

		++nNum;
		sprintf( s_aTagName, "%s%d", FXBMGR_SPAWNCLASS_TAG, nNum );
	}	

	// Read in the properties for the Disc records...

	nNum = 0;
	sprintf( s_aTagName, "%s%d", FXBMGR_DISCCLASS_TAG, nNum );

	while( m_buteMgr.Exist( s_aTagName ))
	{
		PROJECTILECLASSDATA *pPCD = debug_new( DISCCLASSDATA );

		if( pPCD && pPCD->Init( m_buteMgr, s_aTagName ))
		{
			m_ProjClassDataList.AddTail( pPCD );
		}
		else
		{
			debug_delete( pPCD );
			return LTFALSE;
		}

		++nNum;
		sprintf( s_aTagName, "%s%d", FXBMGR_DISCCLASS_TAG, nNum );
	}	

	// Read in the properties for the Cluster Disc records...

	nNum = 0;
	sprintf( s_aTagName, "%s%d", FXBMGR_CLUSTERDISCCLASS_TAG, nNum );

	while( m_buteMgr.Exist( s_aTagName ))
	{
		PROJECTILECLASSDATA *pPCD = debug_new( CLUSTERDISCCLASSDATA );

		if( pPCD && pPCD->Init( m_buteMgr, s_aTagName ))
		{
			m_ProjClassDataList.AddTail( pPCD );
		}
		else
		{
			debug_delete( pPCD );
			return LTFALSE;
		}

		++nNum;
		sprintf( s_aTagName, "%s%d", FXBMGR_CLUSTERDISCCLASS_TAG, nNum );
	}	

	// Read in the properties for the Auto Burst Disc

	nNum = 0;
	sprintf( s_aTagName, "%s%d", FXBMGR_CLUSTERAUTOBURSTDISCCLASS_TAG, nNum );

	while( m_buteMgr.Exist( s_aTagName ))
	{
		PROJECTILECLASSDATA *pPCD = debug_new( CLUSTERAUTOBURSTDISCCLASSDATA );

		if( pPCD && pPCD->Init( m_buteMgr, s_aTagName ))
		{
			m_ProjClassDataList.AddTail( pPCD );
		}
		else
		{
			debug_delete( pPCD );
			return LTFALSE;
		}

		++nNum;
		sprintf( s_aTagName, "%s%d", FXBMGR_CLUSTERAUTOBURSTDISCCLASS_TAG, nNum );
	}	

	// Read in the properties for each projectile fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_PROJECTILEFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		PROJECTILEFX* pPFX = debug_new(PROJECTILEFX);

		if (pPFX && pPFX->Init(m_buteMgr, s_aTagName))
		{
			pPFX->nId = nNum;
			m_ProjectileFXList.AddTail(pPFX);
		}
		else
		{
			debug_delete(pPFX);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_PROJECTILEFX_TAG, nNum);
	}


	// Read in the properties for each beam fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_BEAMFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		BEAMFX* pFX = debug_new(BEAMFX);

		if (pFX && pFX->Init(m_buteMgr, s_aTagName))
		{
			pFX->nId = nNum;
			m_BeamFXList.AddTail(pFX);
		}
		else
		{
			debug_delete(pFX);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_BEAMFX_TAG, nNum);
	}


	// Read in the properties for each fire fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_FIREFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		FIREFX* pFFX = debug_new(FIREFX);

		if (pFFX && pFFX->Init(m_buteMgr, s_aTagName))
		{
			pFFX->nId = nNum;
			m_FireFXList.AddTail(pFFX);
		}
		else
		{
			debug_delete(pFFX);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_FIREFX_TAG, nNum);
	}


	// Read in the properties for each particle explosion fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_PEXPLFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		PEXPLFX* pPEFX = debug_new(PEXPLFX);

		if (pPEFX && pPEFX->Init(m_buteMgr, s_aTagName))
		{
			pPEFX->nId = nNum;
			m_PExplFXList.AddTail(pPEFX);
		}
		else
		{
			debug_delete(pPEFX);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_PEXPLFX_TAG, nNum);
	}


	// Read in the properties for each dynamic light fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_DLIGHTFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		DLIGHTFX* pDLightFX = debug_new(DLIGHTFX);

		if (pDLightFX && pDLightFX->Init(m_buteMgr, s_aTagName))
		{
			pDLightFX->nId = nNum;
			m_DLightFXList.AddTail(pDLightFX);
		}
		else
		{
			debug_delete(pDLightFX);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_DLIGHTFX_TAG, nNum);
	}


	// Read in the properties for each sound light fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_SOUNDFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		SOUNDFX* pFX = debug_new(SOUNDFX);

		if (pFX && pFX->Init(m_buteMgr, s_aTagName))
		{
			pFX->nId = nNum;
			m_SoundFXList.AddTail(pFX);
		}
		else
		{
			debug_delete(pFX);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_SOUNDFX_TAG, nNum);
	}


	// Read in the properties for each pusher fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_PUSHERFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		PUSHERFX* pFX = debug_new(PUSHERFX);

		if (pFX && pFX->Init(m_buteMgr, s_aTagName))
		{
			pFX->nId = nNum;
			m_PusherFXList.AddTail(pFX);
		}
		else
		{
			debug_delete(pFX);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_PUSHERFX_TAG, nNum);
	}


	// Read in the properties for each impact fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_IMPACTFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		IMPACTFX* pIFX = debug_new(IMPACTFX);

		if (pIFX && pIFX->Init(m_buteMgr, s_aTagName))
		{
			pIFX->nId = nNum;
			m_ImpactFXList.AddTail(pIFX);
		}
		else
		{
			debug_delete(pIFX);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_IMPACTFX_TAG, nNum);
	}


	// Read in the properties for each pv fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_PVFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		PVFX* pFX = debug_new(PVFX);

		if (pFX && pFX->Init(m_buteMgr, s_aTagName))
		{
			pFX->nId = nNum;
			m_PVFXList.AddTail(pFX);
		}
		else
		{
			debug_delete(pFX);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_PVFX_TAG, nNum);
	}


	// Read in the properties for each particle muzzle fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_PARTMUZZLEFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		CParticleMuzzleFX* pFX = debug_new(CParticleMuzzleFX);

		if (pFX && pFX->Init(m_buteMgr, s_aTagName))
		{
			pFX->nId = nNum;
			m_PartMuzzleFXList.AddTail(pFX);
		}
		else
		{
			debug_delete(pFX);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_PARTMUZZLEFX_TAG, nNum);
	}


	// Read in the properties for each muzzle fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_MUZZLEFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		CMuzzleFX* pFX = debug_new(CMuzzleFX);

		if (pFX && pFX->Init(m_buteMgr, s_aTagName))
		{
			pFX->nId = nNum;
			m_MuzzleFXList.AddTail(pFX);
		}
		else
		{
			debug_delete(pFX);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_MUZZLEFX_TAG, nNum);
	}


	// Read in the properties for each tracer fx type...

	nNum = 0;
	sprintf(s_aTagName, "%s%d", FXBMGR_TRACERFX_TAG, nNum);

	while (m_buteMgr.Exist(s_aTagName))
	{
		TRACERFX* pFX = debug_new(TRACERFX);

		if (pFX && pFX->Init(m_buteMgr, s_aTagName))
		{
			pFX->nId = nNum;
			m_TracerFXList.AddTail(pFX);
		}
		else
		{
			debug_delete(pFX);
            return LTFALSE;
		}

		nNum++;
		sprintf(s_aTagName, "%s%d", FXBMGR_TRACERFX_TAG, nNum);
	}


	// Read in the properties for each sprinkle fx type...

	nNum = 0;
	sprintf( s_aTagName, "%s%d", FXBMGR_SPRINKLEFX_TAG, nNum );

	while( m_buteMgr.Exist( s_aTagName ))
	{
		SPRINKLEFX *pFX = debug_new( SPRINKLEFX );

		if( pFX && pFX->Init( m_buteMgr, s_aTagName ))
		{
			pFX->nId = nNum;
			m_SprinkleFXList.AddTail( pFX );
		}
		else
		{
			debug_delete( pFX );
			return LTFALSE;
		}

		++nNum;
		sprintf( s_aTagName, "%s%d", FXBMGR_SPRINKLEFX_TAG, nNum );
	}


	// Free up the bute mgr's memory...

	m_buteMgr.Term();


    return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::Term()
//
//	PURPOSE:	Clean up.
//
// ----------------------------------------------------------------------- //

void CFXButeMgr::Term()
{
	g_pFXButeMgr = LTNULL;

	m_ProjectileFXList.Clear();
	m_ProjClassDataList.Clear();
	m_ImpactFXList.Clear();
	m_FireFXList.Clear();
	m_ScaleFXList.Clear();
	m_PShowerFXList.Clear();
	m_PolyDebrisFXList.Clear();
	m_PExplFXList.Clear();
	m_DLightFXList.Clear();
	m_PVFXList.Clear();
	m_SprinkleFXList.Clear();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::Reload()
//
//	PURPOSE:	Reload data from the bute file
//
// ----------------------------------------------------------------------- //

void CFXButeMgr::Reload()
{
	Term();
	Init();
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetScaleFX
//
//	PURPOSE:	Get the specified scale fx struct
//
// ----------------------------------------------------------------------- //

CScaleFX* CFXButeMgr::GetScaleFX(int nScaleFXId)
{
	CScaleFX** pCur  = LTNULL;

	pCur = m_ScaleFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nScaleFXId)
		{
			return *pCur;
		}

		pCur = m_ScaleFXList.GetItem(TLIT_NEXT);
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetScaleFX
//
//	PURPOSE:	Get the specified scale fx struct
//
// ----------------------------------------------------------------------- //

CScaleFX* CFXButeMgr::GetScaleFX(char* pName)
{
	if (!pName) return LTNULL;

	CScaleFX** pCur  = LTNULL;

	pCur = m_ScaleFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_ScaleFXList.GetItem(TLIT_NEXT);
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetPShowerFX
//
//	PURPOSE:	Get the specified PShower fx struct
//
// ----------------------------------------------------------------------- //

CPShowerFX* CFXButeMgr::GetPShowerFX(int nPShowerFXId)
{
	if (nPShowerFXId < 0 || nPShowerFXId > m_PShowerFXList.GetLength()) return LTNULL;

	CPShowerFX** pCur = m_PShowerFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nPShowerFXId)
		{
			return *pCur;
		}

		pCur = m_PShowerFXList.GetItem(TLIT_NEXT);
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetPShowerFX
//
//	PURPOSE:	Get the specified PShower fx struct
//
// ----------------------------------------------------------------------- //

CPShowerFX* CFXButeMgr::GetPShowerFX(char* pName)
{
	if (!pName) return LTNULL;

	CPShowerFX** pCur = m_PShowerFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_PShowerFXList.GetItem(TLIT_NEXT);
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetPolyDebrisFX
//
//	PURPOSE:	Get the specified PolyDebris fx struct
//
// ----------------------------------------------------------------------- //

CPolyDebrisFX* CFXButeMgr::GetPolyDebrisFX(int nPolyDebrisFXId)
{
	if (nPolyDebrisFXId < 0 || nPolyDebrisFXId > m_PolyDebrisFXList.GetLength()) return LTNULL;

	CPolyDebrisFX** pCur = m_PolyDebrisFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nPolyDebrisFXId)
		{
			return *pCur;
		}

		pCur = m_PolyDebrisFXList.GetItem(TLIT_NEXT);
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetPolyDebrisFX
//
//	PURPOSE:	Get the specified PolyDebris fx struct
//
// ----------------------------------------------------------------------- //

CPolyDebrisFX* CFXButeMgr::GetPolyDebrisFX(char* pName)
{
	if (!pName) return LTNULL;

	CPolyDebrisFX** pCur = m_PolyDebrisFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_PolyDebrisFXList.GetItem(TLIT_NEXT);
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetProjectileFX
//
//	PURPOSE:	Get the specified projectile fx struct
//
// ----------------------------------------------------------------------- //

PROJECTILEFX* CFXButeMgr::GetProjectileFX(int nProjectileFXId)
{
	PROJECTILEFX** pCur  = LTNULL;

	pCur = m_ProjectileFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nProjectileFXId)
		{
			return *pCur;
		}

		pCur = m_ProjectileFXList.GetItem(TLIT_NEXT);
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetProjectileFX
//
//	PURPOSE:	Get the specified projectile fx struct
//
// ----------------------------------------------------------------------- //

PROJECTILEFX* CFXButeMgr::GetProjectileFX(char* pName)
{
	if (!pName) return LTNULL;

	PROJECTILEFX** pCur  = LTNULL;

	pCur = m_ProjectileFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_ProjectileFXList.GetItem(TLIT_NEXT);
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetProjectileClassData
//
//	PURPOSE:	Get the specified projectile class data struct
//
// ----------------------------------------------------------------------- //

PROJECTILECLASSDATA* CFXButeMgr::GetProjectileClassData(char* pName)
{
	if (!pName) return LTNULL;

	PROJECTILECLASSDATA** pCur  = LTNULL;

	pCur = m_ProjClassDataList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_ProjClassDataList.GetItem(TLIT_NEXT);
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetImpactFX
//
//	PURPOSE:	Get the specified impact fx struct
//
// ----------------------------------------------------------------------- //

IMPACTFX* CFXButeMgr::GetImpactFX(int nImpactFXId)
{
	IMPACTFX** pCur  = LTNULL;

	pCur = m_ImpactFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nImpactFXId)
		{
			return *pCur;
		}

		pCur = m_ImpactFXList.GetItem(TLIT_NEXT);
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetImpactFX
//
//	PURPOSE:	Get the specified impact fx struct
//
// ----------------------------------------------------------------------- //

IMPACTFX* CFXButeMgr::GetImpactFX(char* pName)
{
	if (!pName) return LTNULL;

	IMPACTFX** pCur  = LTNULL;

	pCur = m_ImpactFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_ImpactFXList.GetItem(TLIT_NEXT);
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetFireFX
//
//	PURPOSE:	Get the specified fire fx struct
//
// ----------------------------------------------------------------------- //

FIREFX* CFXButeMgr::GetFireFX(int nFireFXId)
{
	FIREFX** pCur  = LTNULL;

	pCur = m_FireFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nFireFXId)
		{
			return *pCur;
		}

		pCur = m_FireFXList.GetItem(TLIT_NEXT);
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetFireFX
//
//	PURPOSE:	Get the specified fire fx struct
//
// ----------------------------------------------------------------------- //

FIREFX* CFXButeMgr::GetFireFX(char* pName)
{
	if (!pName) return LTNULL;

	FIREFX** pCur  = LTNULL;

	pCur = m_FireFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_FireFXList.GetItem(TLIT_NEXT);
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetPExplFX
//
//	PURPOSE:	Get the specified PExpl fx struct
//
// ----------------------------------------------------------------------- //

PEXPLFX* CFXButeMgr::GetPExplFX(int nPExpFXId)
{
	PEXPLFX** pCur  = LTNULL;

	pCur = m_PExplFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nPExpFXId)
		{
			return *pCur;
		}

		pCur = m_PExplFXList.GetItem(TLIT_NEXT);
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetPExplFX
//
//	PURPOSE:	Get the specified PExpl fx struct
//
// ----------------------------------------------------------------------- //

PEXPLFX* CFXButeMgr::GetPExplFX(char* pName)
{
	if (!pName) return LTNULL;

	PEXPLFX** pCur  = LTNULL;

	pCur = m_PExplFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_PExplFXList.GetItem(TLIT_NEXT);
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetDLightFX
//
//	PURPOSE:	Get the specified DLIGHT fx struct
//
// ----------------------------------------------------------------------- //

DLIGHTFX* CFXButeMgr::GetDLightFX(int nDLightFXId)
{
	DLIGHTFX** pCur  = LTNULL;

	pCur = m_DLightFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nDLightFXId)
		{
			return *pCur;
		}

		pCur = m_DLightFXList.GetItem(TLIT_NEXT);
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetDLightFX
//
//	PURPOSE:	Get the specified DLIGHT fx struct
//
// ----------------------------------------------------------------------- //

DLIGHTFX* CFXButeMgr::GetDLightFX(char* pName)
{
	if (!pName) return LTNULL;

	DLIGHTFX** pCur  = LTNULL;

	pCur = m_DLightFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_DLightFXList.GetItem(TLIT_NEXT);
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetSoundFX
//
//	PURPOSE:	Get the specified SOUNDFX struct
//
// ----------------------------------------------------------------------- //

SOUNDFX* CFXButeMgr::GetSoundFX(int nSoundFXId)
{
	SOUNDFX** pCur  = LTNULL;

	pCur = m_SoundFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nSoundFXId)
		{
			return *pCur;
		}

		pCur = m_SoundFXList.GetItem(TLIT_NEXT);
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetSoundFX
//
//	PURPOSE:	Get the specified SOUNDFX struct
//
// ----------------------------------------------------------------------- //

SOUNDFX* CFXButeMgr::GetSoundFX(char* pName)
{
	if (!pName) return LTNULL;

	SOUNDFX** pCur  = LTNULL;

	pCur = m_SoundFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_SoundFXList.GetItem(TLIT_NEXT);
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetPusherFX
//
//	PURPOSE:	Get the specified PUSHERFX struct
//
// ----------------------------------------------------------------------- //

PUSHERFX* CFXButeMgr::GetPusherFX(int nSoundFXId)
{
	PUSHERFX** pCur  = LTNULL;

	pCur = m_PusherFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nSoundFXId)
		{
			return *pCur;
		}

		pCur = m_PusherFXList.GetItem(TLIT_NEXT);
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetPusherFX
//
//	PURPOSE:	Get the specified PUSHERFX struct
//
// ----------------------------------------------------------------------- //

PUSHERFX* CFXButeMgr::GetPusherFX(char* pName)
{
	if (!pName) return LTNULL;

	PUSHERFX** pCur  = LTNULL;

	pCur = m_PusherFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_PusherFXList.GetItem(TLIT_NEXT);
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetPVFX
//
//	PURPOSE:	Get the specified pv fx struct
//
// ----------------------------------------------------------------------- //

PVFX* CFXButeMgr::GetPVFX(int nPVFXId)
{
	PVFX** pCur = LTNULL;

	pCur = m_PVFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nPVFXId)
		{
			return *pCur;
		}

		pCur = m_PVFXList.GetItem(TLIT_NEXT);
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetPVFX
//
//	PURPOSE:	Get the specified pv fx struct
//
// ----------------------------------------------------------------------- //

PVFX* CFXButeMgr::GetPVFX(char* pName)
{
	if (!pName) return LTNULL;

	PVFX** pCur  = LTNULL;

	pCur = m_PVFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_PVFXList.GetItem(TLIT_NEXT);
	}

	return LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetParticleMuzzleFX
//
//	PURPOSE:	Get the specified particle muzzle fx struct
//
// ----------------------------------------------------------------------- //

CParticleMuzzleFX* CFXButeMgr::GetParticleMuzzleFX(int nPMFXId)
{
	CParticleMuzzleFX** pCur = LTNULL;

	pCur = m_PartMuzzleFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nPMFXId)
		{
			return *pCur;
		}

		pCur = m_PartMuzzleFXList.GetItem(TLIT_NEXT);
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetParticleMuzzleFX
//
//	PURPOSE:	Get the specified particle muzzle fx struct
//
// ----------------------------------------------------------------------- //

CParticleMuzzleFX* CFXButeMgr::GetParticleMuzzleFX(char* pName)
{
	if (!pName) return LTNULL;

	CParticleMuzzleFX** pCur  = LTNULL;

	pCur = m_PartMuzzleFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_PartMuzzleFXList.GetItem(TLIT_NEXT);
	}

	return LTNULL;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetMuzzleFX
//
//	PURPOSE:	Get the specified muzzle fx struct
//
// ----------------------------------------------------------------------- //

CMuzzleFX* CFXButeMgr::GetMuzzleFX(int nMuzzleFXId)
{
	CMuzzleFX** pCur = LTNULL;

	pCur = m_MuzzleFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nMuzzleFXId)
		{
			return *pCur;
		}

		pCur = m_MuzzleFXList.GetItem(TLIT_NEXT);
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetMuzzleFX
//
//	PURPOSE:	Get the specified muzzle fx struct
//
// ----------------------------------------------------------------------- //

CMuzzleFX* CFXButeMgr::GetMuzzleFX(char* pName)
{
	if (!pName) return LTNULL;

	CMuzzleFX** pCur  = LTNULL;

	pCur = m_MuzzleFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_MuzzleFXList.GetItem(TLIT_NEXT);
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetTracerFX
//
//	PURPOSE:	Get the specified tracer fx struct
//
// ----------------------------------------------------------------------- //

TRACERFX* CFXButeMgr::GetTracerFX(int nTracerFXId)
{
	TRACERFX** pCur = LTNULL;

	pCur = m_TracerFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nTracerFXId)
		{
			return *pCur;
		}

		pCur = m_TracerFXList.GetItem(TLIT_NEXT);
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetTracerFX
//
//	PURPOSE:	Get the specified tracer fx struct
//
// ----------------------------------------------------------------------- //

TRACERFX* CFXButeMgr::GetTracerFX(char* pName)
{
	if (!pName) return LTNULL;

	TRACERFX** pCur  = LTNULL;

	pCur = m_TracerFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_TracerFXList.GetItem(TLIT_NEXT);
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetBeamFX
//
//	PURPOSE:	Get the specified beam fx struct
//
// ----------------------------------------------------------------------- //

BEAMFX* CFXButeMgr::GetBeamFX(int nBeamFXId)
{
	BEAMFX** pCur = LTNULL;

	pCur = m_BeamFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->nId == nBeamFXId)
		{
			return *pCur;
		}

		pCur = m_BeamFXList.GetItem(TLIT_NEXT);
	}

	return LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::GetBeamFX
//
//	PURPOSE:	Get the specified beam fx struct
//
// ----------------------------------------------------------------------- //

BEAMFX* CFXButeMgr::GetBeamFX(char* pName)
{
	if (!pName) return LTNULL;

	BEAMFX** pCur  = LTNULL;

	pCur = m_BeamFXList.GetItem(TLIT_FIRST);

	while (pCur)
	{
		if (*pCur && (*pCur)->szName[0] && (_stricmp((*pCur)->szName, pName) == 0))
		{
			return *pCur;
		}

		pCur = m_BeamFXList.GetItem(TLIT_NEXT);
	}

	return LTNULL;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CFXButeMgr::GetSprinkleFX
//
//  PURPOSE:	Get the specified SprinkleFX struct
//
// ----------------------------------------------------------------------- //

SPRINKLEFX*	CFXButeMgr::GetSprinkleFX( int nSprinkleFXId )
{
	SPRINKLEFX	**pCur = LTNULL;

	pCur = m_SprinkleFXList.GetItem( TLIT_FIRST );

	while( pCur )
	{
		if( *pCur && (*pCur)->nId == nSprinkleFXId )
		{
			return *pCur;
		}

		pCur = m_SprinkleFXList.GetItem( TLIT_NEXT );
	}

	return LTNULL;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CFXButeMgr::GetSprinkleFX
//
//  PURPOSE:	Get the specified SprinkleFX struct
//
// ----------------------------------------------------------------------- //

SPRINKLEFX* CFXButeMgr::GetSprinkleFX( char *pName )
{
	if( !pName ) return LTNULL;

	SPRINKLEFX	**pCur = LTNULL;

	pCur = m_SprinkleFXList.GetItem( TLIT_FIRST );

	while( pCur )
	{
		if( *pCur && (*pCur)->szName[0] && ( !_stricmp( (*pCur)->szName, pName ) ))
		{
			return *pCur;
		}

		pCur = m_SprinkleFXList.GetItem( TLIT_NEXT );
	}

	return LTNULL;
}



/////////////////////////////////////////////////////////////////////////////
//
//	P E X P L  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PEXPLFX::PEXPLFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

PEXPLFX::PEXPLFX()
{
	nId				= FXBMGR_INVALID_ID;

	szName[0]		= '\0';
	szFile[0]		= '\0';

	nNumPerPuff		= 0;
	nNumEmitters	= 0;
	nNumSteps		= 0;
	bCreateDebris   = LTFALSE;
	bRotateDebris   = LTFALSE;
	bIgnoreWind     = LTFALSE;
	bDoBubbles      = LTFALSE;
	bAdditive       = LTFALSE;
	bMultiply       = LTFALSE;
	fLifeTime		= 0.0f;
	fFadeTime		= 0.0f;
	fOffsetTime		= 0.0f;
	fRadius			= 0.0f;
	fGravity		= 0.0f;

	vPosOffset.Init();
	vColor1.Init();
	vColor2.Init();
	vMinVel.Init();
	vMaxVel.Init();
	vMinDriftVel.Init();
	vMaxDriftVel.Init();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PEXPLFX::Init
//
//	PURPOSE:	Build the particle explosion struct
//
// ----------------------------------------------------------------------- //

LTBOOL PEXPLFX::Init(CButeMgr & buteMgr, char* aTagName)
{
	if (!aTagName) return LTFALSE;

	nNumPerPuff		= buteMgr.GetInt(aTagName, FXBMGR_PEXPLFX_NUMPERPUFF);
	nNumEmitters	= buteMgr.GetInt(aTagName, FXBMGR_PEXPLFX_NUMEMITTERS);
	nNumSteps		= buteMgr.GetInt(aTagName, FXBMGR_PEXPLFX_NUMSTEPS);

    bCreateDebris   = (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_PEXPLFX_CREATEDEBRIS);
    bRotateDebris   = (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_PEXPLFX_ROTATEDEBRIS);
    bIgnoreWind     = (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_PEXPLFX_IGNOREWIND);
    bDoBubbles      = (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_PEXPLFX_DOBUBBLES);
    bAdditive       = (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_PEXPLFX_ADDITIVE);
    bMultiply       = (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_PEXPLFX_MULTIPLY);

    fLifeTime       = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PEXPLFX_LIFETIME);
    fFadeTime       = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PEXPLFX_FADETIME);
    fOffsetTime     = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PEXPLFX_OFFSETTIME);
    fRadius         = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PEXPLFX_RADIUS);
    fGravity        = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PEXPLFX_GRAVITY);

	vPosOffset		= buteMgr.GetVector(aTagName, FXBMGR_PEXPLFX_POSOFFSET);
	vColor1			= buteMgr.GetVector(aTagName, FXBMGR_PEXPLFX_COLOR1);
	vColor2			= buteMgr.GetVector(aTagName, FXBMGR_PEXPLFX_COLOR2);
	vMinVel			= buteMgr.GetVector(aTagName, FXBMGR_PEXPLFX_MINVEL);
	vMaxVel			= buteMgr.GetVector(aTagName, FXBMGR_PEXPLFX_MAXVEL);
	vMinDriftVel	= buteMgr.GetVector(aTagName, FXBMGR_PEXPLFX_MINDRIFTVEL);
	vMaxDriftVel	= buteMgr.GetVector(aTagName, FXBMGR_PEXPLFX_MAXDRIFTVEL);

	buteMgr.GetString(aTagName, FXBMGR_PEXPLFX_FILE, szFile, ARRAY_LEN(szFile));
	buteMgr.GetString(aTagName, FXBMGR_PEXPLFX_NAME, szName, ARRAY_LEN(szName));

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PEXPLFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the particle
//				explosion struct
//
// ----------------------------------------------------------------------- //

void PEXPLFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD
	if (!pFXButeMgr) return;

	if (szFile[0])
	{
		if (strstr(szFile, ".spr"))
		{
            g_pLTServer->CacheFile(FT_SPRITE, szFile);
		}
		else
		{
            g_pLTServer->CacheFile(FT_TEXTURE, szFile);
		}
	}
#endif
}


/////////////////////////////////////////////////////////////////////////////
//
//	D L I G H T  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DLIGHTFX::DLIGHTFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

DLIGHTFX::DLIGHTFX()
{
	nId		= FXBMGR_INVALID_ID;

	szName[0]		= '\0';

	vColor.Init();

	fMinRadius		= 0.0f;
	fMaxRadius		= 0.0f;
	fMinTime		= 0.0f;
	fMaxTime		= 0.0f;
	fRampUpTime		= 0.0f;
	fRampDownTime	= 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DLIGHTFX::Init
//
//	PURPOSE:	Build the dynamic light struct
//
// ----------------------------------------------------------------------- //

LTBOOL DLIGHTFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

    fMinRadius      = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_DLIGHTFX_MINRADIUS);
    fMaxRadius      = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_DLIGHTFX_MAXRADIUS);
    fMinTime        = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_DLIGHTFX_MINTIME);
    fMaxTime        = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_DLIGHTFX_MAXTIME);
    fRampUpTime     = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_DLIGHTFX_RAMPUPTIME);
    fRampDownTime   = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_DLIGHTFX_RAMPDOWNTIME);

	vColor			= buteMgr.GetVector(aTagName, FXBMGR_DLIGHTFX_COLOR);
	vColor /= 255.0f;

	buteMgr.GetString(aTagName, FXBMGR_DLIGHTFX_NAME, szName, ARRAY_LEN(szName));

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	DLIGHTFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the dynamic
//				light struct
//
// ----------------------------------------------------------------------- //

void DLIGHTFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD
#endif
}



/////////////////////////////////////////////////////////////////////////////
//
//	I M P A C T  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IMPACTFX::IMPACTFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

IMPACTFX::IMPACTFX()
{
	nId		= FXBMGR_INVALID_ID;

	szName[0]	= '\0';
	szSound[0]	= '\0';
	szMark[0]	= '\0';

	nSoundRadius	= 0;
	nAISoundRadius	= 0;
	bAIIgnoreSurface = LTFALSE;
	nAIStimulusType = 0;
	nAIAlarmLevel	= 0;
	nFlags			= 0;
	fMarkScale		= 0.0f;
	fTintRampUp		= 0.0f;
	fTintRampDown	= 0.0f;
	fTintMaxTime	= 0.0f;
	fBlastTimeMin	= 0.0f;
	fBlastTimeMax	= 0.0f;
	fBlastFadeMin	= 0.0f;
	fBlastFadeMax	= 0.0f;
    bDoSurfaceFX    = LTFALSE;
	bIgnoreFlesh	= LTFALSE;
	bIgnoreLiquid	= LTFALSE;

	vTintColor.Init();
	vBlastColor.Init();

	pPusherFX		= LTNULL;

	szFXName[0] = '\0';
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IMPACTFX::Init
//
//	PURPOSE:	Build the impact fx struct
//
// ----------------------------------------------------------------------- //

LTBOOL IMPACTFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	buteMgr.GetString(aTagName, FXBMGR_IMPACTFX_SOUND, szSound, ARRAY_LEN(szSound));

	buteMgr.GetString(aTagName, FXBMGR_IMPACTFX_MARK, szMark, ARRAY_LEN(szMark));

	buteMgr.GetString(aTagName, FXBMGR_IMPACTFX_NAME, szName, ARRAY_LEN(szName));

	char szStr[128] = "";
	buteMgr.GetString(aTagName, FXBMGR_IMPACTFX_PUSHERNAME, szStr, ARRAY_LEN(szStr));
	if (strlen(szStr))
	{
		pPusherFX = g_pFXButeMgr->GetPusherFX(szStr);
	}

	nSoundRadius	= buteMgr.GetInt(aTagName, FXBMGR_IMPACTFX_SOUNDRADIUS);
	nAISoundRadius	= buteMgr.GetInt(aTagName, FXBMGR_IMPACTFX_AISOUNDRADIUS);
	bAIIgnoreSurface= buteMgr.GetInt(aTagName, FXBMGR_IMPACTFX_AIIGNORESURFACE);
	nAIAlarmLevel	= buteMgr.GetInt(aTagName, FXBMGR_IMPACTFX_AIALARMLEVEL);

#ifndef _CLIENTBUILD

	szStr[0] = '\0';
	buteMgr.GetString(aTagName, FXBMGR_IMPACTFX_AISTIMULUSTYPE, szStr, ARRAY_LEN(szStr));
	if (strlen(szStr))
	{
		nAIStimulusType = CAIStimulusMgr::StimulusFromString( szStr );
	}

#endif

    fMarkScale      = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_IMPACTFX_MARKSCALE);
    fTintRampUp     = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_IMPACTFX_TINTRAMPUP);
    fTintRampDown   = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_IMPACTFX_TINTRAMPDOWN);
    fTintMaxTime    = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_IMPACTFX_TINTMAXTIME);
    fBlastTimeMin   = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_IMPACTFX_BLASTTIMEMIN);
    fBlastTimeMax   = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_IMPACTFX_BLASTTIMEMAX);
    fBlastFadeMin   = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_IMPACTFX_BLASTFADEMIN);
    fBlastFadeMax   = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_IMPACTFX_BLASTFADEMAX);

	vTintColor		= buteMgr.GetVector(aTagName, FXBMGR_IMPACTFX_TINTCOLOR);
	vTintColor /= 255.0f;

	vBlastColor		= buteMgr.GetVector(aTagName, FXBMGR_IMPACTFX_BLASTCOLOR);
	vBlastColor /= 255.0f;

    bDoSurfaceFX    = (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_IMPACTFX_DOSURFACEFX);
    bIgnoreFlesh    = (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_IMPACTFX_IGNOREFLESH);
    bIgnoreLiquid   = (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_IMPACTFX_IGNORELIQUID);


	// Read the name of the FxED created FX...

	buteMgr.GetString( aTagName, FXBMGR_IMPACTFX_FXNAME, szFXName, ARRAY_LEN(szFXName) );


	nFlags = 0;

	if (buteMgr.GetInt(aTagName, FXBMGR_IMPACTFX_CREATEMARK))
	{
		nFlags |= WFX_MARK;
	}

	if (buteMgr.GetInt(aTagName, FXBMGR_IMPACTFX_SCREENTINT))
	{
		nFlags |= WFX_TINTSCREEN;
	}

	if (buteMgr.GetInt(aTagName, FXBMGR_IMPACTFX_BLASTMARK))
	{
		nFlags |= WFX_BLASTMARK;
	}

	if (buteMgr.GetInt(aTagName, FXBMGR_IMPACTFX_IMPACTONSKY))
	{
		nFlags |= WFX_IMPACTONSKY;
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	IMPACTFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the impact
//				fx struct
//
// ----------------------------------------------------------------------- //

void IMPACTFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD

	if (pPusherFX)
	{
		pPusherFX->Cache(pFXButeMgr);
	}

	if (szSound[0] && strstr(szSound, ".wav"))
	{
        g_pLTServer->CacheFile(FT_SOUND, szSound);
	}

	if (szMark[0] && strstr(szMark, ".spr"))
	{
        g_pLTServer->CacheFile(FT_SPRITE, szMark);
	}

   	
#endif
}




/////////////////////////////////////////////////////////////////////////////
//
//	F I R E  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FIREFX::FIREFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

FIREFX::FIREFX()
:	nId				( FXBMGR_INVALID_ID ),
	szName			( LTNULL ),
	szShellModel	( LTNULL ),
	szShellSkin		( LTNULL ),
	szFXName		( LTNULL ),
	szBeamFXName	( LTNULL ),
	nFlags			( 0 ),
	vShellScale		( 1, 1, 1 )
{

}

FIREFX::~FIREFX()
{
	debug_deletea( szName );
	debug_deletea( szShellModel );
	debug_deletea( szShellSkin );
	debug_deletea( szFXName );
	debug_deletea( szBeamFXName );
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FIREFX::Init
//
//	PURPOSE:	Build the fire fx struct
//
// ----------------------------------------------------------------------- //

LTBOOL FIREFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	szName			= GetString( buteMgr, aTagName, FXBMGR_FIREFX_NAME, FXBMGR_MAX_NAME_LENGTH );
	szShellModel	= GetString( buteMgr, aTagName, FXBMGR_FIREFX_SHELLMODEL, FXBMGR_MAX_FILE_PATH );
	szShellSkin		= GetString( buteMgr, aTagName, FXBMGR_FIREFX_SHELLSKIN, FXBMGR_MAX_FILE_PATH );
	szFXName		= GetString( buteMgr, aTagName, FXBMGR_FIREFX_FXNAME, FXBMGR_MAX_NAME_LENGTH );
	szBeamFXName	= GetString( buteMgr, aTagName, FXBMGR_FIREFX_BEAMFXNAME, FXBMGR_MAX_NAME_LENGTH );

	vShellScale = buteMgr.GetVector(aTagName, FXBMGR_FIREFX_SHELLSCALE, CAVector( 1.0f, 1.0f, 1.0f ));

	nFlags = 0;

	if (buteMgr.GetInt(aTagName, FXBMGR_FIREFX_EJECTSHELLS))
	{
		nFlags |= WFX_SHELL;
	}

	if (buteMgr.GetInt(aTagName, FXBMGR_FIREFX_FIRESOUND))
	{
		nFlags |= WFX_FIRESOUND;
	}

	if (buteMgr.GetInt(aTagName, FXBMGR_FIREFX_EXITMARK))
	{
		nFlags |= WFX_EXITMARK;
	}

	if (buteMgr.GetInt(aTagName, FXBMGR_FIREFX_EXITDEBRIS))
	{
		nFlags |= WFX_EXITDEBRIS;
	}

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	FIREFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the fire
//				fx struct
//
// ----------------------------------------------------------------------- //

void FIREFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD

	if (szShellModel[0])
	{
        g_pLTServer->CacheFile(FT_MODEL, szShellModel);
	}

	if (szShellSkin[0])
	{
        g_pLTServer->CacheFile(FT_TEXTURE, szShellSkin);
	}

#endif
}





/////////////////////////////////////////////////////////////////////////////
//
//	P R O J E C T I L E  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PROJECTILEFX::PROJECTILEFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

PROJECTILEFX::PROJECTILEFX()
{
	nId                    = FXBMGR_INVALID_ID;

	szName[0]              = '\0';
	szFlareSprite[0]       = '\0';
	szSound[0]             = '\0';
	szClass[0]             = '\0';
	szModel[0]             = '\0';
	szSkin[0]              = '\0';

	// ClientFX effects
	szFXName[ 0 ] = '\0';
	dwFXFlags = 0;

	fMaxRicochetAngle = 0.0f;
	nMaxRicochets = 0;
	szRicochetFXName[ 0 ] = '\0';

	nVelocity              = 0;
	nAltVelocity           = 0;
	fFireOffset            = 0.0f;
	fLifeTime              = 0.0f;
	fGravityOverride       = 0.0f;
	nFlags                 = 0;
	vLightColor.Init();
	nLightRadius           = 0;
	nSoundRadius           = 0;
	fFlareScale            = 0.0f;
	dwObjectFlags          = 0;
	vModelScale.Init();
	nSmokeTrailType        = 0;
	nCanImpactSameKind     = 0;

	pClassData             = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PROJECTILEFX::Init
//
//	PURPOSE:	Build the projectile fx struct
//
// ----------------------------------------------------------------------- //

LTBOOL PROJECTILEFX::Init(CButeMgr & buteMgr, char* aTagName)
{
	if (!aTagName) return LTFALSE;

	buteMgr.GetString(aTagName, FXBMGR_PROJECTILEFX_FLARESPRITE, szFlareSprite, ARRAY_LEN(szFlareSprite));

	buteMgr.GetString(aTagName, FXBMGR_PROJECTILEFX_SOUND, szSound, ARRAY_LEN(szSound));

	buteMgr.GetString(aTagName, FXBMGR_PROJECTILEFX_CLASS, szClass, ARRAY_LEN(szClass));

	char szStr[128] = "";
	buteMgr.GetString(aTagName, FXBMGR_PROJECTILEFX_CLASSDATA, szStr, ARRAY_LEN(szStr));
	if (strlen(szStr))
	{
		pClassData = g_pFXButeMgr->GetProjectileClassData(szStr);
	}

	buteMgr.GetString(aTagName, FXBMGR_PROJECTILEFX_MODEL, szModel, ARRAY_LEN(szModel));

	buteMgr.GetString(aTagName, FXBMGR_PROJECTILEFX_SKIN, szSkin, ARRAY_LEN(szSkin));

	buteMgr.GetString(aTagName, FXBMGR_PROJECTILEFX_NAME, szName, ARRAY_LEN(szName));

	// get the ClientFX effect to play when the projectile is created
	buteMgr.GetString(aTagName, FXBMGR_PROJECTILEFX_FXNAME, szFXName, ARRAY_LEN(szFXName));

	int fxFlag = buteMgr.GetInt(aTagName, FXBMGR_PROJECTILEFX_FXLOOP, 0);
	if( fxFlag > 0 )
	{
		// determine if the ClientFX effect should loop
		dwFXFlags |= FXFLAG_LOOP;
	}

	fxFlag = buteMgr.GetInt(aTagName, FXBMGR_PROJECTILEFX_FXSMOOTHSHUTDOWN, 0);
	if( fxFlag == 0 )
	{
		// determine if the ClientFX effect should shutdown smoothly
		dwFXFlags |= FXFLAG_NOSMOOTHSHUTDOWN;
	}

	// get the ClientFX effect to play when the projectile is created
	buteMgr.GetString(aTagName, FXBMGR_PROJECTILEFX_RICOCHETFXNAME, "", szRicochetFXName, ARRAY_LEN(szRicochetFXName));
	fMaxRicochetAngle    = DegreesToRadians( static_cast< LTFLOAT >( buteMgr.GetDouble(aTagName, FXBMGR_PROJECTILEFX_MAXRICOCHETANGLE, 0.0 ) ) );
	nMaxRicochets        = buteMgr.GetInt(aTagName, FXBMGR_PROJECTILEFX_MAXRICOCHETS, 0 );

	nSmokeTrailType      = buteMgr.GetInt(aTagName, FXBMGR_PROJECTILEFX_SMOKETRAILTYPE);
	nVelocity            = buteMgr.GetInt(aTagName, FXBMGR_PROJECTILEFX_VELOCITY);
	nAltVelocity         = buteMgr.GetInt(aTagName, FXBMGR_PROJECTILEFX_ALTVELOCITY);
	fFireOffset          = static_cast< LTFLOAT >( buteMgr.GetDouble(aTagName, FXBMGR_PROJECTILEFX_FIRE_OFFSET) );
	nLightRadius         = buteMgr.GetInt(aTagName, FXBMGR_PROJECTILEFX_LIGHTRADIUS);
	nSoundRadius         = buteMgr.GetInt(aTagName, FXBMGR_PROJECTILEFX_SOUNDRADIUS);
	fLifeTime            = static_cast< LTFLOAT >( buteMgr.GetDouble(aTagName, FXBMGR_PROJECTILEFX_LIFETIME) );
	fGravityOverride     = static_cast< LTFLOAT >( buteMgr.GetDouble(aTagName, FXBMGR_PROJECTILEFX_GRAVITYOVERRIDE) );
	fFlareScale          = static_cast< LTFLOAT >( buteMgr.GetDouble(aTagName, FXBMGR_PROJECTILEFX_FLARESCALE) );

	vLightColor      = buteMgr.GetVector(aTagName, FXBMGR_PROJECTILEFX_LIGHTCOLOR);
	vLightColor     /= 255.0f;

	vModelScale      = buteMgr.GetVector(aTagName, FXBMGR_PROJECTILEFX_MODELSCALE);

	dwObjectFlags    = 0;

	if (buteMgr.GetInt(aTagName, FXBMGR_PROJECTILEFX_GRAVITY))
	{
		dwObjectFlags |= FLAG_GRAVITY;
	}

	nFlags = 0;

	if (buteMgr.GetInt(aTagName, FXBMGR_PROJECTILEFX_SMOKETRAIL))
	{
		nFlags |= PFX_SMOKETRAIL;
	}

	if (buteMgr.GetInt(aTagName, FXBMGR_PROJECTILEFX_FLARE))
	{
		nFlags |= PFX_FLARE;
	}

	if (buteMgr.GetInt(aTagName, FXBMGR_PROJECTILEFX_LIGHT))
	{
		nFlags |= PFX_LIGHT;
	}

	if (buteMgr.GetInt(aTagName, FXBMGR_PROJECTILEFX_FLYSOUND))
	{
		nFlags |= PFX_FLYSOUND;
	}


	switch (nSmokeTrailType)
	{
		case 2 :
			nSmokeTrailType = PT_SMOKE_LONG;
		break;

		case 3 :
			nSmokeTrailType = PT_SMOKE_BLACK;
		break;

		case 1 :
		default :
			nSmokeTrailType = PT_SMOKE;
		break;
	}

	// (boolean) true if the projectile
	nCanImpactSameKind   = !!buteMgr.GetInt( aTagName, FXBMGR_PROJECTILEFX_CANHITSAMEKIND );

	return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PROJECTILEFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the projectile
//				fx struct
//
// ----------------------------------------------------------------------- //

void PROJECTILEFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD

	if (szFlareSprite[0])
	{
		g_pLTServer->CacheFile(FT_SPRITE, szFlareSprite);
	}

	if (szSound[0])
	{
		g_pLTServer->CacheFile(FT_SOUND, szSound);
	}

	if (szModel[0])
	{
		g_pLTServer->CacheFile(FT_MODEL, szModel);
	}

	if (szSkin[0])
	{
		g_pLTServer->CacheFile(FT_TEXTURE, szSkin);
	}

	if (pClassData)
	{
		pClassData->Cache(g_pFXButeMgr);
	}
#endif
}




/////////////////////////////////////////////////////////////////////////////
//
//	P R O J E C T I L E  C L A S S  D A T A  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PROJECTILECLASSDATA::Init
//
//	PURPOSE:	Build the projectile class data struct
//
// ----------------------------------------------------------------------- //

LTBOOL PROJECTILECLASSDATA::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	buteMgr.GetString( aTagName, FXBMGR_CLASSDATA_NAME, szName, ARRAY_LEN( szName ));
	
    return LTTRUE;
}

/////////////////////////////////////////////////////////////////////////////
//
//	P R O X  C L A S S  D A T A  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PROXCLASSDATA::PROXCLASSDATA
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

PROXCLASSDATA::PROXCLASSDATA()
{
	nActivateRadius		= 0;
	nArmSndRadius		= 0;
	nActivateSndRadius	= 0;

	fArmDelay		= 0.0f;
	fActivateDelay	= 0.0f;

	szArmSound[0]		= '\0';
	szActivateSound[0]	= '\0';
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PROXCLASSDATA::Init
//
//	PURPOSE:	Build the prox class data class data struct
//
// ----------------------------------------------------------------------- //

LTBOOL PROXCLASSDATA::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;
    if (!PROJECTILECLASSDATA::Init(buteMgr, aTagName)) return LTFALSE;

	buteMgr.GetString(aTagName, FXBMGR_PROXCLASS_ARMSND, szArmSound, ARRAY_LEN(szArmSound));
	buteMgr.GetString(aTagName, FXBMGR_PROXCLASS_ACTSND, szActivateSound, ARRAY_LEN(szActivateSound));

	nActivateRadius		= buteMgr.GetInt(aTagName, FXBMGR_PROXCLASS_ACTRADIUS);
	nArmSndRadius		= buteMgr.GetInt(aTagName, FXBMGR_PROXCLASS_ARMSNDRADIUS);
	nActivateSndRadius	= buteMgr.GetInt(aTagName, FXBMGR_PROXCLASS_ACTSNDRADIUS);

    fArmDelay       = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PROXCLASS_ARMDELAY);
    fActivateDelay  = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PROXCLASS_ACTDELAY);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PROXCLASSDATA::Cache
//
//	PURPOSE:	Cache all the resources associated with the prox class
//				data struct
//
// ----------------------------------------------------------------------- //

void PROXCLASSDATA::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD

	PROJECTILECLASSDATA::Cache(pFXButeMgr);

	if (szArmSound[0])
	{
        g_pLTServer->CacheFile(FT_SOUND, szArmSound);
	}

	if (szActivateSound[0])
	{
        g_pLTServer->CacheFile(FT_SOUND, szActivateSound);
	}

#endif
}

/////////////////////////////////////////////////////////////////////////////
//
//	K I T T Y  C L A S S  D A T A  Related functions...
//
/////////////////////////////////////////////////////////////////////////////


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	KITTYCLASSDATA::KITTYCLASSDATA
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

KITTYCLASSDATA::KITTYCLASSDATA( )
:	PROJECTILECLASSDATA	( ),
	pArmSound			( LTNULL ),
	pActivateSound		( LTNULL ),
	pArmedFX			( LTNULL ),
	pArmedFXRed			( LTNULL ),
	pArmedFXBlue		( LTNULL ),
	bLoopArmSound		( false ),
	bLoopArmedFX		( false ),
	nActivateRadius		( 0 ),
	nDetonateRadius		( 0 ),
	nSoundRadius		( 0 ),
	fArmDelay			( 0.0f ),
	fDetonateTime		( 0.0f )
{

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	KITTYCLASSDATA::~KITTYCLASSDATA
//
//  PURPOSE:	NONE
//
// ----------------------------------------------------------------------- //

KITTYCLASSDATA::~KITTYCLASSDATA( )
{
	debug_deletea( pArmSound );
	debug_deletea( pActivateSound );
	debug_deletea( pArmedFX );
	debug_deletea( pArmedFXRed );
	debug_deletea( pArmedFXBlue );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	KITTYCLASSDATA::Init
//
//  PURPOSE:	Read the props for the Kitty class data struct...
//
// ----------------------------------------------------------------------- //

LTBOOL KITTYCLASSDATA::Init( CButeMgr &buteMgr, char *aTagName )
{
	if( !aTagName ) return LTFALSE;
	if( !PROJECTILECLASSDATA::Init( buteMgr, aTagName )) return LTFALSE;

	pArmSound		= GetString( buteMgr, aTagName, FXBMGR_KITTYCLASS_ARMSND, FXBMGR_MAX_FILE_PATH );
	pActivateSound	= GetString( buteMgr, aTagName, FXBMGR_KITTYCLASS_ACTSND, FXBMGR_MAX_FILE_PATH );
	pArmedFX		= GetString( buteMgr, aTagName, FXBMGR_KITTYCLASS_ARMEDFX, FXBMGR_MAX_NAME_LENGTH );
	pArmedFXRed		= GetString( buteMgr, aTagName, FXBMGR_KITTYCLASS_ARMEDFX_RED, FXBMGR_MAX_NAME_LENGTH );
	pArmedFXBlue	= GetString( buteMgr, aTagName, FXBMGR_KITTYCLASS_ARMEDFX_BLUE, FXBMGR_MAX_NAME_LENGTH );

	bLoopArmSound	= !!buteMgr.GetInt( aTagName, FXBMGR_KITTYCLASS_LOOPARMSND, 0 );
	bLoopArmedFX	= !!buteMgr.GetInt( aTagName, FXBMGR_KITTYCLASS_LOOPARMEDFX, 0 );

	nSoundRadius	= buteMgr.GetInt( aTagName, FXBMGR_KITTYCLASS_SNDRADIUS );
	nActivateRadius	= buteMgr.GetInt( aTagName, FXBMGR_KITTYCLASS_ACTRADIUS );
	nDetonateRadius	= buteMgr.GetInt( aTagName, FXBMGR_KITTYCLASS_DETRADIUS );

	fArmDelay		= (LTFLOAT)buteMgr.GetDouble( aTagName, FXBMGR_KITTYCLASS_ARMDELAY );
	fDetonateTime	= (LTFLOAT)buteMgr.GetDouble( aTagName, FXBMGR_KITTYCLASS_DETTIME );
	fChaseVelocity	= (LTFLOAT)buteMgr.GetDouble( aTagName, FXBMGR_KITTYCLASS_CHASEVEL );

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	KITTYCLASSDATA::Cache
//
//  PURPOSE:	Cache all our resources we might have...
//
// ----------------------------------------------------------------------- //

void KITTYCLASSDATA::Cache( CFXButeMgr *pFXButeMgr )
{
#ifndef _CLIENTBUILD

	PROJECTILECLASSDATA::Cache( pFXButeMgr );

	if( pArmSound[0] )
	{
		g_pLTServer->CacheFile( FT_SOUND, pArmSound );
	}

	if( pActivateSound[0] )
	{
        g_pLTServer->CacheFile( FT_SOUND, pActivateSound );
	}

#endif
}

/////////////////////////////////////////////////////////////////////////////
//
//	B E A R  T R A P  C L A S S  D A T A  Related functions...
//
/////////////////////////////////////////////////////////////////////////////


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	BEARTRAPCLASSDATA::BEARTRAPCLASSDATA
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

BEARTRAPCLASSDATA::BEARTRAPCLASSDATA( )
:	PROJECTILECLASSDATA	( ),
	nDetonateRadius		( 0 ),
	fArmDelay			( 0.0f )
{

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	BEARTRAPCLASSDATA::~BEARTRAPCLASSDATA
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

BEARTRAPCLASSDATA::~BEARTRAPCLASSDATA( )
{

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	BEARTRAPCLASSDATA::Init
//
//  PURPOSE:	Read the props for the Bear trap class data struct...
//
// ----------------------------------------------------------------------- //

LTBOOL BEARTRAPCLASSDATA::Init( CButeMgr &buteMgr, char *aTagName )
{
	if( !aTagName ) return LTFALSE;
	if( !PROJECTILECLASSDATA::Init( buteMgr, aTagName )) return LTFALSE;

	nDetonateRadius	= buteMgr.GetInt( aTagName, FXBMGR_BEARTRAPCLASS_DETRADIUS, 64 );
	fArmDelay		= (LTFLOAT)buteMgr.GetDouble( aTagName, FXBMGR_BEARTRAPCLASS_ARMDELAY, 1.0f );

	return LTTRUE;
}

/////////////////////////////////////////////////////////////////////////////
//
//	B A N A N A  C L A S S  D A T A  Related functions...
//
/////////////////////////////////////////////////////////////////////////////


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	BANANACLASSDATA::BANANACLASSDATA
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

BANANACLASSDATA::BANANACLASSDATA( )
:	PROJECTILECLASSDATA	( ),
	nDetonateRadius		( 0 ),
	fArmDelay			( 0.0f )
{

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	BANANACLASSDATA::~BANANACLASSDATA
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

BANANACLASSDATA::~BANANACLASSDATA( )
{

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	BANANACLASSDATA::Init
//
//  PURPOSE:	Read the props for the banana class data struct...
//
// ----------------------------------------------------------------------- //

LTBOOL BANANACLASSDATA::Init( CButeMgr &buteMgr, char *aTagName )
{
	if( !aTagName ) return LTFALSE;
	if( !PROJECTILECLASSDATA::Init( buteMgr, aTagName )) return LTFALSE;

	nDetonateRadius	= buteMgr.GetInt( aTagName, FXBMGR_BANANACLASS_DETRADIUS, 64 );
	fArmDelay		= (LTFLOAT)buteMgr.GetDouble( aTagName, FXBMGR_BANANACLASS_ARMDELAY, 1.0f );

	return LTTRUE;
}

/////////////////////////////////////////////////////////////////////////////
//
//	S P E A R  C L A S S  D A T A  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SPEARCLASSDATA::SPEARCLASSDATA
//
//  PURPOSE:	Constructor...
//
// ----------------------------------------------------------------------- //

SPEARCLASSDATA::SPEARCLASSDATA()
:	PROJECTILECLASSDATA	( ),
	fStickPercent		( 0.0f ),
	bCanWallStick		( false ),
	vDimsScale			( 1.0f, 1.0f, 1.0f )
{

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SPEARCLASSDATA::~SPEARCLASSDATA
//
//  PURPOSE:	Destructor...
//
// ----------------------------------------------------------------------- //

SPEARCLASSDATA::~SPEARCLASSDATA()
{

}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SPEARCLASSDATA::Init
//
//  PURPOSE:	Read the props for the spear class data struct...
//
// ----------------------------------------------------------------------- //

LTBOOL SPEARCLASSDATA::Init( CButeMgr &buteMgr, char *aTagName )
{
	if( !aTagName ) return LTFALSE;
	if( !PROJECTILECLASSDATA::Init( buteMgr, aTagName )) return LTFALSE;

	fStickPercent	= (float)buteMgr.GetDouble( aTagName, FXBMGR_SPEARCLASS_STICKPERCENT, 0.5f );
	bCanWallStick	= !!(buteMgr.GetInt( aTagName, FXBMGR_SPEARCLASS_CANWALLSTICK, 0 ));
	vDimsScale		= buteMgr.GetVector(aTagName, FXBMGR_SPEARCLASS_DIMSSCALE);
	
	return LTTRUE;
}

/////////////////////////////////////////////////////////////////////////////
//
//	S P A W N  C L A S S  D A T A  Related functions...
//
/////////////////////////////////////////////////////////////////////////////


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SPAWNCLASSDATA::SPAWNCLASSDATA
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SPAWNCLASSDATA::SPAWNCLASSDATA( )
:	PROJECTILECLASSDATA	( ),
	szSpawnObject		( LTNULL ),
	bStickToWorld		( LTFALSE )
{

}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SPAWNCLASSDATA::~SPAWNCLASSDATA
//
//  PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

SPAWNCLASSDATA::~SPAWNCLASSDATA()
{
	debug_deletea( szSpawnObject );
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SPAWNCLASSDATA::Init
//
//  PURPOSE:	Read the props for the SpawnerProjectile Class Data...
//
// ----------------------------------------------------------------------- //

LTBOOL SPAWNCLASSDATA::Init( CButeMgr &buteMgr, char *aTagName )
{
	if (!aTagName) return LTFALSE;
    if (!PROJECTILECLASSDATA::Init(buteMgr, aTagName)) return LTFALSE;

	szSpawnObject	= GetString( buteMgr, aTagName, FXBMGR_SPAWNCLASS_SPAWNOBJECT, FXBMGR_MAX_NAME_LENGTH );

	blrObjectProps.Read( &buteMgr, aTagName, FXBMGR_SPAWNCLASS_OBJECTPROP, FXBMGR_MAX_NAME_LENGTH + FXBMGR_MAX_FILE_PATH );

	bStickToWorld	= (LTBOOL)buteMgr.GetInt( aTagName, FXBMGR_SPAWNCLASS_STICKTOWORLD );

	return LTTRUE;
}


/////////////////////////////////////////////////////////////////////////////
//
//	D I S C  C L A S S  D A T A  Related functions...
//
/////////////////////////////////////////////////////////////////////////////


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DISCCLASSDATA::DISCCLASSDATA
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

DISCCLASSDATA::DISCCLASSDATA( )
:	PROJECTILECLASSDATA	( ),
	fInitialAngleMin( 0.0f ),
	fInitialAngleMax( 0.0f ),
	fInitialAngleSurge( 0.0f ),
	fReturnVelocity( 0.0f ),
	fReturnHeightOffset( 0.0f ),
	fTurnRateMin( 0.0f ),
	fTurnRateMax( 0.0f ),
	fTurnRateSurge( 0.0f ),
	fIncidentAngleToControlLineMin( 0.0f ),
	fIncidentAngleToControlLineMax( 0.0f ),
	fIncidentAngleToControlLineSurge( 0.0f ),
	fIncidentAngleToControlLineDecayMin( 0.0f ),
	fIncidentAngleToControlLineDecayMax( 0.0f ),
	fIncidentAngleToControlLineDecaySurge( 0.0f ),
	fSwatCriticalDefendThreshold( 0.0f ),
	fSwatDefendStartDefendPercentage( 0.0f ),
	fSwatDefendEndDefendPercentage( 0.0f ),
	fSwatDefendMidpoint( 0.0f ),
	fSwatDefendMaxDefendPercentage( 0.0f ),
	fSwatDefendStartMaxDefendPercentage( 0.0f ),
	fSwatDefendEndMaxDefendPercentage( 0.0f ),
	fHoldDefendStartDefendPercentage( 0.0f ),
	fHoldDefendEndDefendPercentage( 0.0f ),
	fHoldDefendMaxDefendPercentage( 0.0f ),
	fHoldDefendStartMaxDefendPercentage( 0.0f ),
	fHoldDefendEndMaxDefendPercentage( 0.0f ),
	fSwatDefendOrientationMinDefendPercentage( 0.0f ),
	fSwatDefendOrientationMaxDefendPercentage( 0.0f ),
	fSwatDefendOrientationDeadZone( 0.0f ),
	fSwatDefendOrientationMaxZone( 0.0f ),
	fHoldDefendOrientationMinDefendPercentage( 0.0f ),
	fHoldDefendOrientationMaxDefendPercentage( 0.0f ),
	fHoldDefendOrientationDeadZone( 0.0f ),
	fHoldDefendOrientationMaxZone( 0.0f ),
	bClusterDiscInfo( false )
{
	szReturnFXName[ 0 ] = '\0';
	szSwatDefendPVFXName[ 0 ] = '\0';
	szSwatCriticalDefendPVFXName[ 0 ] = '\0';
	szHoldDefendPVFXName[ 0 ] = '\0';
	szArmDefendPVFXName[ 0 ] = '\0';
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DISCCLASSDATA::~DISCCLASSDATA
//
//  PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

DISCCLASSDATA::~DISCCLASSDATA()
{
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	DISCCLASSDATA::Init
//
//  PURPOSE:	Read the props for the Disc Class Data...
//
// ----------------------------------------------------------------------- //

LTBOOL DISCCLASSDATA::Init( CButeMgr &buteMgr, char *aTagName )
{
	if (!aTagName) return LTFALSE;
	if (!PROJECTILECLASSDATA::Init(buteMgr, aTagName)) return LTFALSE;

	fInitialAngleMin                      = DegreesToRadians( static_cast< LTFLOAT >( buteMgr.GetDouble( aTagName, FXBMGR_DISCCLASS_INITIALANGLEMIN ) ) );
	fInitialAngleMax                      = DegreesToRadians( static_cast< LTFLOAT >( buteMgr.GetDouble( aTagName, FXBMGR_DISCCLASS_INITIALANGLEMAX ) ) );
	fInitialAngleSurge                    = DegreesToRadians( static_cast< LTFLOAT >( buteMgr.GetDouble( aTagName, FXBMGR_DISCCLASS_INITIALANGLESURGE ) ) );

	fReturnVelocity                       = static_cast< LTFLOAT >( buteMgr.GetDouble( aTagName, FXBMGR_DISCCLASS_RETURNVELOCITY ) );
	fReturnHeightOffset                   = static_cast< LTFLOAT >( buteMgr.GetDouble( aTagName, FXBMGR_DISCCLASS_RETURNHEIGHTOFFSET ) );
	buteMgr.GetString( aTagName, FXBMGR_DISCCLASS_RETURNFXNAME, szReturnFXName, ARRAY_LEN( szReturnFXName ) );

	// NOTE: the angles are specified in DEGREES/SEC, convert to RADIANS/SEC
	fTurnRateMin                          = DegreesToRadians( static_cast< LTFLOAT >( buteMgr.GetDouble( aTagName, FXBMGR_DISCCLASS_TURNRATEMIN ) ) );
	fTurnRateMax                          = DegreesToRadians( static_cast< LTFLOAT >( buteMgr.GetDouble( aTagName, FXBMGR_DISCCLASS_TURNRATEMAX ) ) );
	fTurnRateSurge                        = DegreesToRadians( static_cast< LTFLOAT >( buteMgr.GetDouble( aTagName, FXBMGR_DISCCLASS_TURNRATESURGE ) ) );

	// NOTE: the angles are specified in DEGREES, convert to RADIANS
	fIncidentAngleToControlLineMin        = DegreesToRadians( static_cast< LTFLOAT >( buteMgr.GetDouble( aTagName, FXBMGR_DISCCLASS_INCIDENTANGLETOCONTROLLINEMIN ) ) );
	fIncidentAngleToControlLineMax        = DegreesToRadians( static_cast< LTFLOAT >( buteMgr.GetDouble( aTagName, FXBMGR_DISCCLASS_INCIDENTANGLETOCONTROLLINEMAX ) ) );
	fIncidentAngleToControlLineSurge      = DegreesToRadians( static_cast< LTFLOAT >( buteMgr.GetDouble( aTagName, FXBMGR_DISCCLASS_INCIDENTANGLETOCONTROLLINESURGE ) ) );

	// NOTE: the angles are specified in DEGREES/SEC, convert to RADIANS/SEC
	fIncidentAngleToControlLineDecayMin   = DegreesToRadians( static_cast< LTFLOAT >( buteMgr.GetDouble( aTagName, FXBMGR_DISCCLASS_INCIDENTANGLETOCONTROLLINEDECAYMIN ) ) );
	fIncidentAngleToControlLineDecayMax   = DegreesToRadians( static_cast< LTFLOAT >( buteMgr.GetDouble( aTagName, FXBMGR_DISCCLASS_INCIDENTANGLETOCONTROLLINEDECAYMAX ) ) );
	fIncidentAngleToControlLineDecaySurge = DegreesToRadians( static_cast< LTFLOAT >( buteMgr.GetDouble( aTagName, FXBMGR_DISCCLASS_INCIDENTANGLETOCONTROLLINEDECAYSURGE ) ) );

	buteMgr.GetString( aTagName, FXBMGR_DISCCLASS_SWATDEFENDPVFXNAME, szSwatDefendPVFXName, ARRAY_LEN( szSwatDefendPVFXName ) );
	buteMgr.GetString( aTagName, FXBMGR_DISCCLASS_SWATCRITICALDEFENDPVFXNAME, szSwatCriticalDefendPVFXName, ARRAY_LEN( szSwatCriticalDefendPVFXName ) );
	fSwatCriticalDefendThreshold          = static_cast< LTFLOAT >( buteMgr.GetDouble( aTagName, FXBMGR_DISCCLASS_SWATCRITICALDEFENDTHRESHOLD ) );
	
	buteMgr.GetString( aTagName, FXBMGR_DISCCLASS_HOLDDEFENDPVFXNAME, szHoldDefendPVFXName, ARRAY_LEN( szHoldDefendPVFXName ) );
	
	buteMgr.GetString( aTagName, FXBMGR_DISCCLASS_ARMDEFENDPVFXNAME, szArmDefendPVFXName, ARRAY_LEN( szArmDefendPVFXName ) );
	
	//
	// Defense values
	//

	// swat timing values
	fSwatDefendStartDefendPercentage      = static_cast< float >( buteMgr.GetDouble( aTagName, FXBMGR_DISCCLASS_SWATDEFENDSTARTDEFENDPERCENTAGE ) );
	fSwatDefendEndDefendPercentage        = static_cast< float >( buteMgr.GetDouble( aTagName, FXBMGR_DISCCLASS_SWATDEFENDENDDEFENDPERCENTAGE ) );
	fSwatDefendMidpoint                   = static_cast< float >( buteMgr.GetDouble( aTagName, FXBMGR_DISCCLASS_SWATDEFENDMIDPOINT ) );
	fSwatDefendMaxDefendPercentage        = static_cast< float >( buteMgr.GetDouble( aTagName, FXBMGR_DISCCLASS_SWATDEFENDMAXDEFENDPERCENTAGE ) );
	fSwatDefendStartMaxDefendPercentage   = static_cast< float >( buteMgr.GetDouble( aTagName, FXBMGR_DISCCLASS_SWATDEFENDSTARTMAXDEFENDPERCENTAGE ) );
	fSwatDefendEndMaxDefendPercentage     = static_cast< float >( buteMgr.GetDouble( aTagName, FXBMGR_DISCCLASS_SWATDEFENDENDMAXDEFENDPERCENTAGE ) );

	// hold timing values
	fHoldDefendStartDefendPercentage      = static_cast< float >( buteMgr.GetDouble( aTagName, FXBMGR_DISCCLASS_HOLDDEFENDSTARTDEFENDPERCENTAGE ) );
	fHoldDefendEndDefendPercentage        = static_cast< float >( buteMgr.GetDouble( aTagName, FXBMGR_DISCCLASS_HOLDDEFENDENDDEFENDPERCENTAGE ) );
	fHoldDefendMaxDefendPercentage        = static_cast< float >( buteMgr.GetDouble( aTagName, FXBMGR_DISCCLASS_HOLDDEFENDMAXDEFENDPERCENTAGE ) );
	fHoldDefendStartMaxDefendPercentage   = static_cast< float >( buteMgr.GetDouble( aTagName, FXBMGR_DISCCLASS_HOLDDEFENDSTARTMAXDEFENDPERCENTAGE ) );
	fHoldDefendEndMaxDefendPercentage     = static_cast< float >( buteMgr.GetDouble( aTagName, FXBMGR_DISCCLASS_HOLDDEFENDENDMAXDEFENDPERCENTAGE ) );

	// swat orientation values
	fSwatDefendOrientationMinDefendPercentage  = static_cast< float >( buteMgr.GetDouble( aTagName, FXBMGR_DISCCLASS_SWATDEFENDORIENTATIONMINDEFENDPERCENTAGE ) );
	fSwatDefendOrientationMaxDefendPercentage  = static_cast< float >( buteMgr.GetDouble( aTagName, FXBMGR_DISCCLASS_SWATDEFENDORIENTATIONMAXDEFENDPERCENTAGE ) );
	fSwatDefendOrientationDeadZone             = DegreesToRadians( static_cast< float >( buteMgr.GetDouble( aTagName, FXBMGR_DISCCLASS_SWATDEFENDORIENTATIONDEADZONE ) ) ) / 2.0f;
	fSwatDefendOrientationMaxZone              = DegreesToRadians( static_cast< float >( buteMgr.GetDouble( aTagName, FXBMGR_DISCCLASS_SWATDEFENDORIENTATIONMAXZONE ) ) ) / 2.0f;

	// check for overlapping angles
	ASSERT( fSwatDefendOrientationMaxZone < ( MATH_HALFPI - fSwatDefendOrientationDeadZone ) );

	// hold orientation values
	fHoldDefendOrientationMinDefendPercentage  = static_cast< float >( buteMgr.GetDouble( aTagName, FXBMGR_DISCCLASS_HOLDDEFENDORIENTATIONMINDEFENDPERCENTAGE ) );
	fHoldDefendOrientationMaxDefendPercentage  = static_cast< float >( buteMgr.GetDouble( aTagName, FXBMGR_DISCCLASS_HOLDDEFENDORIENTATIONMAXDEFENDPERCENTAGE ) );
	fHoldDefendOrientationDeadZone             = DegreesToRadians( static_cast< float >( buteMgr.GetDouble( aTagName, FXBMGR_DISCCLASS_HOLDDEFENDORIENTATIONDEADZONE ) ) ) / 2.0f;
	fHoldDefendOrientationMaxZone              = DegreesToRadians( static_cast< float >( buteMgr.GetDouble( aTagName, FXBMGR_DISCCLASS_HOLDDEFENDORIENTATIONMAXZONE ) ) ) / 2.0f;

	// check for overlapping angles
	ASSERT( fHoldDefendOrientationMaxZone < ( MATH_HALFPI - fHoldDefendOrientationDeadZone ) );

	return LTTRUE;
}

/////////////////////////////////////////////////////////////////////////////
//
//	C L U S T E R  D I S C  C L A S S  D A T A  Related functions...
//
/////////////////////////////////////////////////////////////////////////////


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CLUSTERDISCCLASSDATA::CLUSTERDISCCLASSDATA
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CLUSTERDISCCLASSDATA::CLUSTERDISCCLASSDATA()
	: DISCCLASSDATA()
	, fShardHorizontalSpreadMin( 0.0f )
	, fShardHorizontalSpreadMax( 0.0f )
	, fShardHorizontalSpreadSurge( 0.0f )
	, fShardHorizontalPerturbMin( 0.0f )
	, fShardHorizontalPerturbMax( 0.0f )
	, fShardHorizontalPerturbSurge( 0.0f )
	, fShardVerticalSpreadMin( 0.0f )
	, fShardVerticalSpreadMax( 0.0f )
	, fShardVerticalSpreadSurge( 0.0f )
	, nShardsTotalMin( 0 )
	, nShardsTotalMax( 0 )
	, nShardsTotalSurge( 0 )
	, nShardsTotalPerturbMin( 0 )
	, nShardsTotalPerturbMax( 0 )
	, nShardsTotalPerturbSurge( 0 )
{
	szShardWeaponName[ 0 ] = '\0';

	// reinit this to show there is extra info
	bClusterDiscInfo = true;
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CLUSTERDISCCLASSDATA::~CLUSTERDISCCLASSDATA
//
//  PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

CLUSTERDISCCLASSDATA::~CLUSTERDISCCLASSDATA()
{
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	CLUSTERDISCCLASSDATA::Init
//
//  PURPOSE:	Read the props for the Cluster Disc Class Data...
//
// ----------------------------------------------------------------------- //

LTBOOL CLUSTERDISCCLASSDATA::Init( CButeMgr &buteMgr, char *aTagName )
{
	if (!aTagName) return LTFALSE;
	if (!DISCCLASSDATA::Init(buteMgr, aTagName)) return LTFALSE;

	// horizontal spread
	fShardHorizontalSpreadMin     = DegreesToRadians( static_cast< LTFLOAT >( buteMgr.GetDouble( aTagName, FXBMGR_CLUSTERDISCCLASS_SHARDHORIZONTALSPREADMIN ) ) );
	fShardHorizontalSpreadMax     = DegreesToRadians( static_cast< LTFLOAT >( buteMgr.GetDouble( aTagName, FXBMGR_CLUSTERDISCCLASS_SHARDHORIZONTALSPREADMAX ) ) );
	fShardHorizontalSpreadSurge   = DegreesToRadians( static_cast< LTFLOAT >( buteMgr.GetDouble( aTagName, FXBMGR_CLUSTERDISCCLASS_SHARDHORIZONTALSPREADSURGE ) ) );

	// randomization of the shard direction
	fShardHorizontalPerturbMin    = static_cast< LTFLOAT >( buteMgr.GetDouble( aTagName, FXBMGR_CLUSTERDISCCLASS_SHARDHORIZONTALPERTURBMIN ) );
	fShardHorizontalPerturbMax    = static_cast< LTFLOAT >( buteMgr.GetDouble( aTagName, FXBMGR_CLUSTERDISCCLASS_SHARDHORIZONTALPERTURBMAX ) );
	fShardHorizontalPerturbSurge  = static_cast< LTFLOAT >( buteMgr.GetDouble( aTagName, FXBMGR_CLUSTERDISCCLASS_SHARDHORIZONTALPERTURBSURGE ) );

	// vertical spread
	fShardVerticalSpreadMin       = DegreesToRadians( static_cast< LTFLOAT >( buteMgr.GetDouble( aTagName, FXBMGR_CLUSTERDISCCLASS_SHARDVERTICALSPREADMIN ) ) );
	fShardVerticalSpreadMax       = DegreesToRadians( static_cast< LTFLOAT >( buteMgr.GetDouble( aTagName, FXBMGR_CLUSTERDISCCLASS_SHARDVERTICALSPREADMAX ) ) );
	fShardVerticalSpreadSurge     = DegreesToRadians( static_cast< LTFLOAT >( buteMgr.GetDouble( aTagName, FXBMGR_CLUSTERDISCCLASS_SHARDVERTICALSPREADSURGE ) ) );

	// get the shard weapon's name
	buteMgr.GetString( aTagName, FXBMGR_CLISTERDISCCLASS_SHARDWEAPONNAME, szShardWeaponName, ARRAY_LEN( szShardWeaponName ) );

	// number of shards to emit
	nShardsTotalMin               = buteMgr.GetInt( aTagName, FXBMGR_CLUSTERDISCCLASS_SHARDSTOTALMIN );
	nShardsTotalMax               = buteMgr.GetInt( aTagName, FXBMGR_CLUSTERDISCCLASS_SHARDSTOTALMAX );
	nShardsTotalSurge             = buteMgr.GetInt( aTagName, FXBMGR_CLUSTERDISCCLASS_SHARDSTOTALSURGE );

	// number of shards perturb
	nShardsTotalPerturbMin        = buteMgr.GetInt( aTagName, FXBMGR_CLUSTERDISCCLASS_SHARDSTOTALPERTURBMIN );
	nShardsTotalPerturbMax        = buteMgr.GetInt( aTagName, FXBMGR_CLUSTERDISCCLASS_SHARDSTOTALPERTURBMAX );
	nShardsTotalPerturbSurge      = buteMgr.GetInt( aTagName, FXBMGR_CLUSTERDISCCLASS_SHARDSTOTALPERTURBSURGE );

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//	CLUSTER AUTO BURST  Related functions...
//
// ----------------------------------------------------------------------- //
CLUSTERAUTOBURSTDISCCLASSDATA::CLUSTERAUTOBURSTDISCCLASSDATA()
{
	// 
	fMinBurstDistance = 256.0f;
	fMaxBurstDistance = 512.0f;
}

/*virtual*/ CLUSTERAUTOBURSTDISCCLASSDATA::~CLUSTERAUTOBURSTDISCCLASSDATA()
{
	// Destruct
}

LTBOOL CLUSTERAUTOBURSTDISCCLASSDATA::Init(CButeMgr &buteMgr,char *aTagName)
{
	if ( !aTagName ) return LTFALSE;
	if ( !CLUSTERDISCCLASSDATA::Init( buteMgr, aTagName ) ) return LTFALSE;

	fMinBurstDistance = static_cast<LTFLOAT>(buteMgr.GetDouble( aTagName, FXBMGR_CLUSTERAUTOBURSTCLASS_MINBURSTDIST ));
	fMaxBurstDistance = static_cast<LTFLOAT>(buteMgr.GetDouble( aTagName, FXBMGR_CLUSTERAUTOBURSTCLASS_MAXBURSTDIST ));

	return LTTRUE;
}


/////////////////////////////////////////////////////////////////////////////
//
//	P V  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PVFX::PVFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

PVFX::PVFX()
{
	nId		= FXBMGR_INVALID_ID;

	szName[0]	= '\0';
	szSocket[0] = '\0';

	nNumScaleFXTypes = 0;
    int i;
    for (i=0; i < PV_MAX_SCALEFX_TYPES; i++)
	{
		aScaleFXTypes[i] = FXBMGR_INVALID_ID;
	}

	nNumDLightFX = 0;
	for (i=0; i < PV_MAX_DLIGHTFX_TYPES; i++)
	{
		aDLightFXTypes[i] = FXBMGR_INVALID_ID;
	}

	nNumSoundFX = 0;
	for (i=0; i < PV_MAX_DLIGHTFX_TYPES; i++)
	{
		aSoundFXTypes[i] = FXBMGR_INVALID_ID;
	}
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PVFX::Init
//
//	PURPOSE:	Build the pv fx struct
//
// ----------------------------------------------------------------------- //

LTBOOL PVFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	buteMgr.GetString(aTagName, FXBMGR_PVFX_NAME, szName, ARRAY_LEN(szName));
	buteMgr.GetString(aTagName, FXBMGR_PVFX_SOCKET, szSocket, ARRAY_LEN(szSocket));


	// Build our scale fx types id array...

	nNumScaleFXTypes = 0;

	while (nNumScaleFXTypes < PV_MAX_SCALEFX_TYPES)
	{
		char szStr[128];
		sprintf(s_aAttName, "%s%d", FXBMGR_PVFX_SCALENAME, nNumScaleFXTypes);
		buteMgr.GetString(aTagName, s_aAttName, "", szStr, ARRAY_LEN(szStr));
		if( !buteMgr.Success( ))
			break;

		if( szStr[0] )
		{
			CScaleFX* pScaleFX = g_pFXButeMgr->GetScaleFX(szStr);
			if (pScaleFX)
			{
				aScaleFXTypes[nNumScaleFXTypes] = pScaleFX->nId;
			}
		}

		nNumScaleFXTypes++;
	}


	// Build our dynamic light fx types id array...

	nNumDLightFX = 0;

	while ( nNumDLightFX < PV_MAX_DLIGHTFX_TYPES)
	{
		char szStr[128] = "";
		sprintf(s_aAttName, "%s%d", FXBMGR_PVFX_DLIGHTNAME, nNumDLightFX);
		buteMgr.GetString(aTagName, s_aAttName, "", szStr, ARRAY_LEN(szStr));
		if( !buteMgr.Success( ))
			break;

		if( szStr[0] )
		{
			DLIGHTFX* pDLightFX = g_pFXButeMgr->GetDLightFX(szStr);
			if (pDLightFX)
			{
				aDLightFXTypes[nNumDLightFX] = pDLightFX->nId;
			}
		}

		nNumDLightFX++;
	}

	// Build our sound fx types id array...

	nNumSoundFX = 0;

	while ( nNumSoundFX < PV_MAX_SOUNDFX_TYPES)
	{
		char szStr[128] = "";
		sprintf(s_aAttName, "%s%d", FXBMGR_PVFX_SOUNDNAME, nNumSoundFX);
		buteMgr.GetString(aTagName, s_aAttName, "", szStr, ARRAY_LEN(szStr));
		if( !buteMgr.Success( ))
			break;

		if( szStr[0] )
		{
			SOUNDFX* pFX = g_pFXButeMgr->GetSoundFX(szStr);
			if (pFX)
			{
				aSoundFXTypes[nNumSoundFX] = pFX->nId;
			}
		}

		nNumSoundFX++;
	}

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PVFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the pv
//				fx struct
//
// ----------------------------------------------------------------------- //

void PVFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD
	if (!pFXButeMgr) return;

    int i;
    for (i=0; i < nNumScaleFXTypes; i++)
	{
		CScaleFX* pScaleFX = pFXButeMgr->GetScaleFX(aScaleFXTypes[i]);
		if (pScaleFX)
		{
			pScaleFX->Cache();
		}
	}

	for (i=0; i < nNumDLightFX; i++)
	{
		DLIGHTFX* pDLightFX = pFXButeMgr->GetDLightFX(aDLightFXTypes[i]);
		if (pDLightFX)
		{
			pDLightFX->Cache(pFXButeMgr);
		}
	}

	for (i=0; i < nNumSoundFX; i++)
	{
		SOUNDFX* pFX = pFXButeMgr->GetSoundFX(aSoundFXTypes[i]);
		if (pFX)
		{
			pFX->Cache(pFXButeMgr);
		}
	}
#endif
}



/////////////////////////////////////////////////////////////////////////////
//
//	P A R T I C L E  M U Z Z L E  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleMuzzleFX::CParticleMuzzleFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CParticleMuzzleFX::CParticleMuzzleFX()
{
	nId				= FXBMGR_INVALID_ID;
	szName[0]		= '\0';
	szFile[0]		= '\0';
	fLength			= 0.0f;
	fDuration		= 0.0f;
	fRadius			= 0.0f;
	fMaxScale		= 0.0f;
	nNumParticles	= 0;

    bAdditive       = LTFALSE;
    bMultiply       = LTFALSE;

	vColor1.Init();
	vColor2.Init();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleMuzzleFX::Init
//
//	PURPOSE:	Build the particle muzzle fx struct
//
// ----------------------------------------------------------------------- //

LTBOOL CParticleMuzzleFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	buteMgr.GetString(aTagName, FXBMGR_PARTMUZZLEFX_NAME, szName, ARRAY_LEN(szName));

	buteMgr.GetString(aTagName, FXBMGR_PARTMUZZLEFX_FILE, szFile, ARRAY_LEN(szFile));

    fLength         = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PARTMUZZLEFX_LENGTH);
    fDuration       = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PARTMUZZLEFX_DURATION);
    fRadius         = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PARTMUZZLEFX_RADIUS);
    fMaxScale       = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PARTMUZZLEFX_MAXSCALE);
	nNumParticles	= buteMgr.GetInt(aTagName, FXBMGR_PARTMUZZLEFX_NUMBER);

    bAdditive       = (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_PARTMUZZLEFX_ADDITIVE);
    bMultiply       = (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_PARTMUZZLEFX_MULTIPLY);

	vColor1			= buteMgr.GetVector(aTagName, FXBMGR_PARTMUZZLEFX_COLOR1);
	vColor2			= buteMgr.GetVector(aTagName, FXBMGR_PARTMUZZLEFX_COLOR2);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CParticleMuzzleFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the particle
//				muzzle fx struct
//
// ----------------------------------------------------------------------- //

void CParticleMuzzleFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD
	if (!pFXButeMgr) return;

	if (szFile[0])
	{
		if (strstr(szFile, ".spr"))
		{
            g_pLTServer->CacheFile(FT_SPRITE, szFile);
		}
		else
		{
            g_pLTServer->CacheFile(FT_TEXTURE, szFile);
		}
	}

#endif
}




/////////////////////////////////////////////////////////////////////////////
//
//	M U Z Z L E  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMuzzleFX::CMuzzleFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

CMuzzleFX::CMuzzleFX()
{
	nId				= FXBMGR_INVALID_ID;
	szName[0]		= '\0';
	fDuration		= 0.0f;

    pPMuzzleFX      = LTNULL;
    pScaleFX        = LTNULL;
    pDLightFX       = LTNULL;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMuzzleFX::Init
//
//	PURPOSE:	Build the muzzle fx struct
//
// ----------------------------------------------------------------------- //

LTBOOL CMuzzleFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	buteMgr.GetString(aTagName, FXBMGR_MUZZLEFX_NAME, szName, ARRAY_LEN(szName));

	char szStr[128] = "";
	buteMgr.GetString(aTagName, FXBMGR_MUZZLEFX_PMUZZLEFXNAME, szStr, ARRAY_LEN(szStr));
	if (strlen(szStr))
	{
		pPMuzzleFX = g_pFXButeMgr->GetParticleMuzzleFX(szStr);
	}

	buteMgr.GetString(aTagName, FXBMGR_MUZZLEFX_SCALEFXNAME, szStr, ARRAY_LEN(szStr));
	if (strlen(szStr))
	{
		pScaleFX = g_pFXButeMgr->GetScaleFX(szStr);
	}

	buteMgr.GetString(aTagName, FXBMGR_MUZZLEFX_DLIGHTFXNAME, szStr, ARRAY_LEN(szStr));
	if (strlen(szStr))
	{
		pDLightFX = g_pFXButeMgr->GetDLightFX(szStr);
	}

    fDuration = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_MUZZLEFX_DURATION);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CMuzzleFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the muzzle fx struct
//
// ----------------------------------------------------------------------- //

void CMuzzleFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD
	if (!pFXButeMgr) return;

	if (pPMuzzleFX)
	{
		pPMuzzleFX->Cache(pFXButeMgr);
	}

	if (pScaleFX)
	{
		pScaleFX->Cache();
	}

	if (pDLightFX)
	{
		pDLightFX->Cache(pFXButeMgr);
	}
#endif
}


/////////////////////////////////////////////////////////////////////////////
//
//	T R A C E R  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TRACERFX::TRACERFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

TRACERFX::TRACERFX()
{
	nId				= FXBMGR_INVALID_ID;
	szName[0]		= '\0';
	szTexture[0]	= '\0';

	nFrequency		= 1;
	fVelocity		= 0.0f;
	fWidth			= 0.0f;
	fInitialAlpha	= 0.0f;
	fFinalAlpha		= 0.0f;
	vColor.Init();
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TRACERFX::Init
//
//	PURPOSE:	Build the tracer fx struct
//
// ----------------------------------------------------------------------- //

LTBOOL TRACERFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	buteMgr.GetString(aTagName, FXBMGR_TRACERFX_NAME, szName, ARRAY_LEN(szName));
	buteMgr.GetString(aTagName, FXBMGR_TRACERFX_TEXTURE, szTexture, ARRAY_LEN(szTexture));

	nFrequency		= buteMgr.GetInt(aTagName, FXBMGR_TRACERFX_FREQUENCY);
    fVelocity       = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_TRACERFX_VELOCITY);
    fWidth          = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_TRACERFX_WIDTH);
    fInitialAlpha   = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_TRACERFX_INITIALALPHA);
    fFinalAlpha     = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_TRACERFX_FINALALPHA);
	vColor			= buteMgr.GetVector(aTagName, FXBMGR_TRACERFX_COLOR);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	TRACERFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the tracer fx struct
//
// ----------------------------------------------------------------------- //

void TRACERFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD
	if (szTexture[0])
	{
		g_pLTServer->CacheFile(FT_TEXTURE, szTexture);
	}
#endif
}


/////////////////////////////////////////////////////////////////////////////
//
//  B E A M  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BEAMFX::BEAMFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

BEAMFX::BEAMFX()
{
	nId				= FXBMGR_INVALID_ID;
	szName[0]		= '\0';
	szTexture[0]	= '\0';

	fDuration		= 0.0f;
	fWidth			= 0.0f;
	fInitialAlpha	= 0.0f;
	fFinalAlpha		= 0.0f;
	vColor.Init();
	bAlignUp		= LTFALSE;
	bAlignFlat		= LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BEAMFX::Init
//
//	PURPOSE:	Build the beam fx struct
//
// ----------------------------------------------------------------------- //

LTBOOL BEAMFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	buteMgr.GetString(aTagName, FXBMGR_BEAMFX_NAME, szName, ARRAY_LEN(szName));
	buteMgr.GetString(aTagName, FXBMGR_BEAMFX_TEXTURE, szTexture, ARRAY_LEN(szTexture));

    fDuration       = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_BEAMFX_DURATION);
    fWidth          = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_BEAMFX_WIDTH);
    fInitialAlpha   = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_BEAMFX_INITIALALPHA);
    fFinalAlpha     = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_BEAMFX_FINALALPHA);
	vColor			= buteMgr.GetVector(aTagName, FXBMGR_BEAMFX_COLOR);
    bAlignUp        = (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_BEAMFX_ALIGNUP);
    bAlignFlat      = (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_BEAMFX_ALIGNFLAT);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	BEAMFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the beam fx struct
//
// ----------------------------------------------------------------------- //

void BEAMFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD
	if (szTexture[0])
	{
		g_pLTServer->CacheFile(FT_TEXTURE, szTexture);
	}
#endif
}


/////////////////////////////////////////////////////////////////////////////
//
//  S O U N D  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SOUNDFX::SOUNDFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SOUNDFX::SOUNDFX()
{
	nId				= FXBMGR_INVALID_ID;
	szName[0]		= '\0';
	szFile[0]		= '\0';

	fRadius			= 0.0f;
	fPitchShift		= 1.0f;
	bLoop			= LTFALSE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SOUNDFX::Init
//
//	PURPOSE:	Build the sound fx struct
//
// ----------------------------------------------------------------------- //

LTBOOL SOUNDFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	buteMgr.GetString(aTagName, FXBMGR_SOUNDFX_NAME, szName, ARRAY_LEN(szName));
	buteMgr.GetString(aTagName, FXBMGR_SOUNDFX_FILE, szFile, ARRAY_LEN(szFile));

    fRadius     = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_SOUNDFX_RADIUS);
    fPitchShift	= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_SOUNDFX_PITCHSHIFT);
    bLoop		= (LTBOOL) buteMgr.GetInt(aTagName, FXBMGR_SOUNDFX_LOOP);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	SOUNDFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the sound fx struct
//
// ----------------------------------------------------------------------- //

void SOUNDFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD

	if (szFile[0] && strstr(szFile, ".wav"))
	{
        g_pLTServer->CacheFile(FT_SOUND, szFile);
	}

#endif
}


/////////////////////////////////////////////////////////////////////////////
//
//  P U S H E R  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PUSHERFX::PUSHERFX
//
//	PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

PUSHERFX::PUSHERFX()
{
	nId				= FXBMGR_INVALID_ID;
	szName[0]		= '\0';

	fRadius			= 0.0f;
	fStartDelay		= 0.0f;
	fDuration		= 0.0f;
	fStrength		= 0.0f;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PUSHERFX::Init
//
//	PURPOSE:	Build the pusher fx struct
//
// ----------------------------------------------------------------------- //

LTBOOL PUSHERFX::Init(CButeMgr & buteMgr, char* aTagName)
{
    if (!aTagName) return LTFALSE;

	buteMgr.GetString(aTagName, FXBMGR_PUSHERFX_NAME, szName, ARRAY_LEN(szName));

    fRadius		= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PUSHERFX_RADIUS);
    fStartDelay = (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PUSHERFX_STARTDELAY);
    fDuration	= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PUSHERFX_DURATION);
    fStrength	= (LTFLOAT) buteMgr.GetDouble(aTagName, FXBMGR_PUSHERFX_STRENGTH);

    return LTTRUE;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	PUSHERFX::Cache
//
//	PURPOSE:	Cache all the resources associated with the pusher fx struct
//
// ----------------------------------------------------------------------- //

void PUSHERFX::Cache(CFXButeMgr* pFXButeMgr)
{
#ifndef _CLIENTBUILD
#endif
}



////////////////////////////////////////////////////////////////////////////
//
//  S P R I N K L E  F X  Related functions...
//
/////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SPRINKLEFX::SPRINKLEFX
//
//  PURPOSE:	Constructor
//
// ----------------------------------------------------------------------- //

SPRINKLEFX::SPRINKLEFX()
:	nId				( FXBMGR_INVALID_ID ),
	szName			( LTNULL ),
	szFileName		( LTNULL ),
	szSkinName		( LTNULL ),
	dwCount			( 0 ),
	fSpeed			( 0.0f ),
	fSize			( 0.0f ),
	fSpawnRadius	( 0.0f ),
	vAnglesVel		( 0.0f, 0.0f, 0.0f ),
	vColorMin		( 0.0f, 0.0f, 0.0f ),
	vColorMax		( 0.0f, 0.0f, 0.0f )
{

}
// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SPRINKLEFX::~SPRINKLEFX
//
//  PURPOSE:	Destructor
//
// ----------------------------------------------------------------------- //

SPRINKLEFX::~SPRINKLEFX()
{
	debug_deletea( szName );
	debug_deletea( szFileName );
	debug_deletea( szSkinName );
}

// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SPRINKLEFX::Init
//
//  PURPOSE:	Build the SprinkleFX struct
//
// ----------------------------------------------------------------------- //

LTBOOL SPRINKLEFX::Init( CButeMgr &ButeMgr, char *aTagName )
{
	if( !aTagName ) return LTFALSE;

	szName			= GetString( ButeMgr, aTagName, FXBMGR_SPRINKLEFX_NAME, FXBMGR_MAX_NAME_LENGTH );
	szFileName		= GetString( ButeMgr, aTagName, FXBMGR_SPRINKLEFX_FILENAME, FXBMGR_MAX_FILE_PATH );
	szSkinName		= GetString( ButeMgr, aTagName, FXBMGR_SPRINKLEFX_SKINNAME, FXBMGR_MAX_FILE_PATH );
	dwCount			= (uint32)ButeMgr.GetInt( aTagName, FXBMGR_SPRINKLEFX_COUNT );
	fSize			= (LTFLOAT)ButeMgr.GetDouble( aTagName, FXBMGR_SPRINKLEFX_SIZE );
	fSpeed			= (LTFLOAT)ButeMgr.GetDouble( aTagName, FXBMGR_SPRINKLEFX_SPEED );
	fSpawnRadius	= (LTFLOAT)ButeMgr.GetDouble( aTagName, FXBMGR_SPRINKLEFX_SPAWNRADIUS );
	vAnglesVel		= ButeMgr.GetVector( aTagName, FXBMGR_SPRINKLEFX_ANGLESVEL );
	vColorMin		= ButeMgr.GetVector( aTagName, FXBMGR_SPRINKLEFX_COLORMIN );
	vColorMax		= ButeMgr.GetVector( aTagName, FXBMGR_SPRINKLEFX_COLORMAX );

	return LTTRUE;
}


// ----------------------------------------------------------------------- //
//
//  ROUTINE:	SPRINKLEFX::Cache
//
//  PURPOSE:	Cache all the resources
//
// ----------------------------------------------------------------------- //

void SPRINKLEFX::Cache( CFXButeMgr *pFXButeMgr )
{
#ifndef _CLIENTBUILD

	if( szFileName[0] && strstr( szFileName, ".LTB" ))
	{
        g_pLTServer->CacheFile( FT_MODEL, szFileName );
	}
	else if( szFileName[0] && strstr( szFileName, ".DTX" ))
	{
		g_pLTServer->CacheFile( FT_TEXTURE, szFileName );
	}
	else if( szFileName[0] && strstr( szFileName, ".SPR" ))
	{
		g_pLTServer->CacheFile( FT_SPRITE, szFileName );
	}

	if( szSkinName[0] && strstr( szSkinName, ".DTX" ))
	{
		g_pLTServer->CacheFile( FT_TEXTURE, szSkinName );
	}

#endif
}








/////////////////////////////////////////////////////////////////////////////
//
//	C L I E N T - S I D E  U T I L I T Y  F U N C T I O N S
//
/////////////////////////////////////////////////////////////////////////////

#if defined(_CLIENTBUILD) || defined(__PSX2)

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::CreateScaleFX()
//
//	PURPOSE:	Create a scale fx
//
// ----------------------------------------------------------------------- //

CSpecialFX* CFXButeMgr::CreateScaleFX(CScaleFX* pScaleFX, const LTVector &vPos,
                                     const LTVector &vDir, const LTVector * pvSurfaceNormal,
                                     const LTRotation* prRot, CBaseScaleFX* pFX)
{
    if (!pScaleFX || !pScaleFX->szFile[0]) return LTNULL;

	// Create scale fx...

	BSCREATESTRUCT scale;
	scale.dwFlags = FLAG_VISIBLE;// | FLAG_NOLIGHT;

	if (!pScaleFX->bUseLight)
		scale.dwFlags |= FLAG_NOLIGHT;
	if (prRot)
	{
		scale.rRot = *prRot;
	}

	// Set up fx flags...

	if (pScaleFX->eType == SCALEFX_SPRITE)
	{
		if (pScaleFX->bAlignToSurface && pvSurfaceNormal)
		{
			scale.dwFlags |= FLAG_ROTATEABLESPRITE;
			scale.rRot = LTRotation(*pvSurfaceNormal, LTVector(0.0f, 1.0f, 0.0f));
		}
		else if (!pScaleFX->bReallyClose)
		{
			// We want FLAG_REALLYCLOSE to override the default
			// FLAG_SPRITEBIAS...

			scale.dwFlags |= FLAG_SPRITEBIAS;
		}

		if (pScaleFX->bNoZ)
		{
			scale.dwFlags |= FLAG_SPRITE_NOZ;
		}

		if (pScaleFX->bRotate)
		{
			scale.dwFlags |= FLAG_ROTATEABLESPRITE;
		}
	}

	LTVector vFinalPos = vPos;

	if (pScaleFX->bReallyClose)
	{
		scale.dwFlags |= FLAG_REALLYCLOSE;

		// Get the position into camera-relative space...

		HOBJECT hCam = LTNULL;
		if (g_pInterfaceMgr->UseInterfaceCamera())
			hCam = g_pInterfaceMgr->GetInterfaceCamera();
		else
			hCam = g_pPlayerMgr->GetCamera();

        LTVector vOffset;
		g_pLTClient->GetObjectPos(hCam, &vOffset);

		vFinalPos -= vOffset;
	}

	// Adjust the position based on the offsets...

	scale.vPos = vFinalPos + (vDir * pScaleFX->fDirOffset);

	if (pScaleFX->fDirROffset || pScaleFX->fDirUOffset)
	{
        LTRotation rTempRot(vDir, LTVector(0.0f, 1.0f, 0.0f));

		scale.vPos += (rTempRot.Right() * pScaleFX->fDirROffset);
		scale.vPos += (rTempRot.Up() * pScaleFX->fDirUOffset);
	}

	scale.pFilename			= pScaleFX->szFile;

	CButeListReader blrSkinReader;
	blrSkinReader.SetItem(0, pScaleFX->szSkin, MAX_CS_FILENAME_LEN+1);
	scale.pSkinReader = &blrSkinReader;

	scale.vVel				= pScaleFX->vVel;
	scale.vInitialScale		= pScaleFX->vInitialScale;
	scale.vFinalScale		= pScaleFX->vFinalScale;
	scale.vInitialColor		= pScaleFX->vInitialColor;
	scale.vFinalColor		= pScaleFX->vFinalColor;
	scale.bUseUserColors	= pScaleFX->bUseColors;
	scale.fLifeTime			= pScaleFX->fLifeTime;
	scale.fInitialAlpha		= pScaleFX->fInitialAlpha;
	scale.fFinalAlpha		= pScaleFX->fFinalAlpha;
	scale.bLoop				= pScaleFX->bLoop;
	scale.fDelayTime		= pScaleFX->fDelayTime;
	scale.bAdditive			= pScaleFX->bAdditive;
	scale.bMultiply			= pScaleFX->bMultiply;
	scale.nType				= (pScaleFX->eType == SCALEFX_MODEL) ? OT_MODEL : OT_SPRITE;
    scale.bUseUserColors    = LTTRUE;
	scale.bFaceCamera		= pScaleFX->bFaceCamera;
	scale.nRotationAxis		= pScaleFX->nRotationAxis;
	scale.bRotate			= pScaleFX->bRotate;
	scale.fMinRotateVel		= pScaleFX->fMinRotVel;
	scale.fMaxRotateVel		= pScaleFX->fMaxRotVel;
	scale.nMenuLayer		= pScaleFX->nMenuLayer;

	if (!pFX)
	{
		CSpecialFX* pNewFX = g_pGameClientShell->GetSFXMgr()->CreateSFX(SFX_SCALE_ID, &scale);
		if (pNewFX)
		{
			pNewFX->Update();
		}

		return pNewFX;
	}
	else
	{
		pFX->Init(&scale);
        if (!pFX->CreateObject(g_pLTClient))
			return LTNULL;
	}

	return pFX;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::CreatePShowerFX()
//
//	PURPOSE:	Create a particle shower fx
//
// ----------------------------------------------------------------------- //

CSpecialFX* CFXButeMgr::CreatePShowerFX(CPShowerFX* pPShowerFX, const LTVector &vPos,
                                        const LTVector &vDir, const LTVector &vSurfaceNormal)
{
    if (!pPShowerFX || !pPShowerFX->szTexture[0]) return LTNULL;

	// Create particle shower fx...

	PARTICLESHOWERCREATESTRUCT ps;

	ps.vPos				= vPos + (vDir * pPShowerFX->fDirOffset);
	ps.vDir				= (vSurfaceNormal * GetRandom(pPShowerFX->fMinVel, pPShowerFX->fMaxVel));
	ps.vColor1			= pPShowerFX->vColor1;
	ps.vColor2			= pPShowerFX->vColor2;
	ps.pTexture			= pPShowerFX->szTexture;
	ps.nParticles		= GetRandom(pPShowerFX->nMinParticles, pPShowerFX->nMaxParticles);
	ps.fDuration		= GetRandom(pPShowerFX->fMinDuration, pPShowerFX->fMaxDuration);
	ps.fEmissionRadius	= pPShowerFX->fEmissionRadius;
	ps.fRadius			= pPShowerFX->fRadius;
	ps.fGravity			= pPShowerFX->fGravity;
	ps.bAdditive		= pPShowerFX->bAdditive;
	ps.bMultiply		= pPShowerFX->bMultiply;

	return g_pGameClientShell->GetSFXMgr()->CreateSFX(SFX_PARTICLESHOWER_ID, &ps);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::CreatePolyDebrisFX()
//
//	PURPOSE:	Create a poly debris fx
//
// ----------------------------------------------------------------------- //

CSpecialFX* CFXButeMgr::CreatePolyDebrisFX(CPolyDebrisFX* pPolyDebrisFX, const LTVector &vPos,
                                          const LTVector &vDir, const LTVector &vSurfaceNormal)
{
    if (!pPolyDebrisFX) return LTNULL;

	POLYDEBRISCREATESTRUCT pdebris;

	pdebris.vNormal			= vSurfaceNormal;
	pdebris.vPos			= vPos;
	pdebris.vDir			= vDir;
	pdebris.PolyDebrisFX	= *pPolyDebrisFX;

	return g_pGameClientShell->GetSFXMgr()->CreateSFX(SFX_POLYDEBRIS_ID, &pdebris);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::CreateBeamFX()
//
//	PURPOSE:	Create a beam fx
//
// ----------------------------------------------------------------------- //

CSpecialFX* CFXButeMgr::CreateBeamFX(BEAMFX* pBeamFX, const LTVector &vStartPos,
                                     const LTVector &vEndPos)
{
    if (!pBeamFX) return LTNULL;

	BEAMCREATESTRUCT beam;

	beam.vStartPos		= vStartPos;
	beam.vEndPos		= vEndPos;
	beam.pBeamFX		= pBeamFX;

	return g_pGameClientShell->GetSFXMgr()->CreateSFX(SFX_BEAM_ID, &beam);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::CreateSoundFX()
//
//	PURPOSE:	Create a sound fx
//
// ----------------------------------------------------------------------- //

CSpecialFX* CFXButeMgr::CreateSoundFX(SOUNDFX* pSoundFX, const LTVector &vPos,
                                     CSoundFX* pFX)
{
    if (!pSoundFX) return LTNULL;

	SNDCREATESTRUCT snd;

    snd.bLocal      = vPos.NearlyEquals(LTVector(0,0,0), 0.0f) ? LTTRUE : LTFALSE;
	snd.bLoop		= pSoundFX->bLoop;
	snd.fPitchShift	= pSoundFX->fPitchShift;
	snd.fRadius		= pSoundFX->fRadius;
	snd.pSndName	= pSoundFX->szFile;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
    if (!psfxMgr) return LTNULL;

	if (pFX)
	{
		pFX->Init(&snd);
        pFX->CreateObject(g_pLTClient);

		return pFX;
	}

	return psfxMgr->CreateSFX(SFX_SOUND_ID, &snd);
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::CreatePusherFX()
//
//	PURPOSE:	Create a pusher fx
//
// ----------------------------------------------------------------------- //

void CFXButeMgr::CreatePusherFX(PUSHERFX* pPusherFX, const LTVector &vPos)
{
    if (!pPusherFX) return;

	g_pPlayerMgr->GetMoveMgr()->AddPusher(vPos, pPusherFX->fRadius,
		pPusherFX->fStartDelay, pPusherFX->fDuration, pPusherFX->fStrength);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::CreatePExplFX()
//
//	PURPOSE:	Create a paritlce explosion specific fx
//
// ----------------------------------------------------------------------- //

CSpecialFX* CFXButeMgr::CreatePExplFX(PEXPLFX* pPExplFX, const LTRotation &rSurfaceRot,
                                     SurfaceType eSurfaceType, const LTVector &vPos,
									 ContainerCode eCode)
{
    if (!pPExplFX) return LTNULL;

	// Create a particle explosion...

	PESCREATESTRUCT pe;
    pe.rSurfaceRot = rSurfaceRot;

	pe.nSurfaceType		= eSurfaceType;
	pe.vPos				= vPos + pPExplFX->vPosOffset;
	pe.bCreateDebris	= pPExplFX->bCreateDebris;
	pe.bRotateDebris	= pPExplFX->bRotateDebris;
	pe.bIgnoreWind		= pPExplFX->bIgnoreWind;
	pe.vColor1			= pPExplFX->vColor1;
	pe.vColor2			= pPExplFX->vColor2;
	pe.vMinVel			= pPExplFX->vMinVel;
	pe.vMaxVel			= pPExplFX->vMaxVel;
	pe.vMinDriftVel		= pPExplFX->vMinDriftVel;
	pe.vMaxDriftVel		= pPExplFX->vMaxDriftVel;
	pe.fLifeTime		= pPExplFX->fLifeTime;
	pe.fFadeTime		= pPExplFX->fFadeTime;
	pe.fOffsetTime		= pPExplFX->fOffsetTime;
	pe.fRadius			= pPExplFX->fRadius;
	pe.fGravity			= pPExplFX->fGravity;
	pe.nNumPerPuff		= pPExplFX->nNumPerPuff;
	pe.nNumEmitters		= pPExplFX->nNumEmitters;
	pe.nNumSteps		= pPExplFX->nNumSteps;
	pe.pFilename		= pPExplFX->szFile;
	pe.bAdditive		= pPExplFX->bAdditive;
	pe.bMultiply		= pPExplFX->bMultiply;

	if (IsLiquid(eCode) && pPExplFX->bDoBubbles)
	{
		GetLiquidColorRange(eCode, &pe.vColor1, &pe.vColor2);
		pe.pFilename = DEFAULT_BUBBLE_TEXTURE;
	}

	CSpecialFX* pFX = g_pGameClientShell->GetSFXMgr()->CreateSFX(SFX_PARTICLEEXPLOSION_ID, &pe);
	if (pFX) pFX->Update();

	return pFX;
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::CreateDLightFX()
//
//	PURPOSE:	Create a dynamic light specific fx
//
// ----------------------------------------------------------------------- //

CSpecialFX* CFXButeMgr::CreateDLightFX(DLIGHTFX* pDLightFX, const LTVector &vPos,
									   CDynamicLightFX* pFX)
{
    if (!pDLightFX) return LTNULL;

	DLCREATESTRUCT dl;
	dl.vPos			 = vPos;
	dl.vColor		 = pDLightFX->vColor;
	dl.fMinRadius    = pDLightFX->fMinRadius;
	dl.fMaxRadius	 = pDLightFX->fMaxRadius;
	dl.fRampUpTime	 = pDLightFX->fRampUpTime;
	dl.fMaxTime		 = pDLightFX->fMaxTime;
	dl.fMinTime		 = pDLightFX->fMinTime;
	dl.fRampDownTime = pDLightFX->fRampDownTime;
	dl.dwFlags		 = FLAG_VISIBLE | FLAG_DONTLIGHTBACKFACING;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
    if (!psfxMgr) return LTNULL;

	if (pFX)
	{
		pFX->Init(&dl);
        pFX->CreateObject(g_pLTClient);

		return pFX;
	}

	return psfxMgr->CreateSFX(SFX_DYNAMICLIGHT_ID, &dl);
}


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::CreateImpactFX()
//
//	PURPOSE:	Create the specified impact fx
//
// ----------------------------------------------------------------------- //

void CFXButeMgr::CreateImpactFX(IMPACTFX* pImpactFX, IFXCS & cs)
{
	// Sanity checks...

	if (!pImpactFX) return;

	CGameSettings* pSettings = g_pInterfaceMgr->GetSettings();
	if (!pSettings) return;

	CSFXMgr* psfxMgr = g_pGameClientShell->GetSFXMgr();
	if (!psfxMgr) return;

	int nDetailLevel = pSettings->SpecialFXSetting();

	// Add a pusher if necessary...

	CreatePusherFX(pImpactFX->pPusherFX, cs.vPos);

	// Play the impact sound if appropriate...

	if (cs.bPlaySound && pImpactFX->szSound[0])
	{
        LTFLOAT fSndRadius = (LTFLOAT) pImpactFX->nSoundRadius;

		g_pClientSoundMgr->PlaySoundFromPos(cs.vPos, pImpactFX->szSound, fSndRadius,
				SOUNDPRIORITY_MISC_MEDIUM);
	}


	// See if we should ignore certain surfaces...

	if (cs.eSurfType == ST_FLESH && pImpactFX->bIgnoreFlesh) return;
	if (cs.eSurfType == ST_LIQUID && pImpactFX->bIgnoreLiquid) return;

	// Create the FxED created FX for the impact FX....

	CLIENTFX_CREATESTRUCT	fxCS( pImpactFX->szFXName, 0, cs.vPos );
	fxCS.m_vTargetNorm = cs.vSurfNormal;
	g_pClientFXMgr->CreateClientFX(LTNULL, fxCS, LTTRUE );

	// Create blast mark if appropriate...

	if ((pImpactFX->nFlags & WFX_BLASTMARK) && ShowsMark(cs.eSurfType) &&
		(nDetailLevel == RS_HIGH) && cs.fBlastRadius >= FXBMGR_MIN_BLASTMARK_RADIUS)
	{
		// Create a dynamic light for the blast mark...

		DLCREATESTRUCT dl;

		dl.vPos			 = cs.vPos;
		dl.vColor		 = pImpactFX->vBlastColor;
		dl.fMinRadius    = cs.fBlastRadius / 2.0f;
		dl.fMaxRadius	 = cs.fBlastRadius / 4.0f;
		dl.fRampUpTime	 = 0.0f;
		dl.fMaxTime		 = GetRandom(pImpactFX->fBlastTimeMin, pImpactFX->fBlastTimeMax);
		dl.fMinTime		 = 0.0f;
		dl.fRampDownTime = GetRandom(pImpactFX->fBlastFadeMin, pImpactFX->fBlastFadeMax);
		dl.dwFlags		 = FLAG_VISIBLE | FLAG_ONLYLIGHTWORLD | FLAG_DONTLIGHTBACKFACING;

		psfxMgr->CreateSFX(SFX_DYNAMICLIGHT_ID, &dl);
	}

	// Tint screen if appropriate...

	if ((pImpactFX->nFlags & WFX_TINTSCREEN))
	{
        LTVector vTintColor  = pImpactFX->vTintColor;
        LTFLOAT fRampUp      = pImpactFX->fTintRampUp;
        LTFLOAT fRampDown    = pImpactFX->fTintRampDown;
        LTFLOAT fTintTime    = pImpactFX->fTintMaxTime;

		g_pGameClientShell->FlashScreen(vTintColor, cs.vPos, cs.fTintRange,
			fRampUp, fTintTime, fRampDown);

		// If close enough, shake the screen...

        HLOCALOBJ hPlayerObj = g_pLTClient->GetClientObject();
		if (hPlayerObj)
		{
            LTVector vPlayerPos, vDir;
			g_pLTClient->GetObjectPos(hPlayerObj, &vPlayerPos);

			vDir = vPlayerPos - cs.vPos;
            LTFLOAT fDist = vDir.Length();

            LTFLOAT fRadius = cs.fBlastRadius;

			if (fDist < fRadius * 2.0f)
			{
                LTFLOAT fVal = fDist < 1.0f ? 3.0f : fRadius / fDist;
				fVal = fVal > 3.0f ? 3.0f : fVal;

                LTVector vShake(fVal, fVal, fVal);
				g_pPlayerMgr->ShakeScreen(vShake);
			}
		}
	}
}
#endif

#ifndef _CLIENTBUILD

/////////////////////////////////////////////////////////////////////////////
//
//	S E R V E R - S I D E  U T I L I T Y  F U N C T I O N S
//
/////////////////////////////////////////////////////////////////////////////


// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgr::ReadImpactFXProp
//
//	PURPOSE:	Read in the impact fx properties
//
// ----------------------------------------------------------------------- //

LTBOOL CFXButeMgr::ReadImpactFXProp(char* pPropName, uint8 & nImpactFXId)
{
    if (!pPropName || !pPropName[0]) return LTFALSE;

	GenericProp genProp;

    if (g_pLTServer->GetPropGeneric(pPropName, &genProp) == LT_OK)
	{
		// Get the impact fx

		IMPACTFX* pImpactFX = GetImpactFX(genProp.m_String);
		if (pImpactFX)
		{
			nImpactFXId = pImpactFX->nId;
		}

        return LTTRUE;
	}

    return LTFALSE;
}

////////////////////////////////////////////////////////////////////////////
//
// CFXButeMgrPlugin is used to help facilitate populating the DEdit object
// properties that use FXButeMgr
//
////////////////////////////////////////////////////////////////////////////

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgrPlugin::PreHook_EditStringList
//
//	PURPOSE:	Fill the string list
//
// ----------------------------------------------------------------------- //
#ifndef __PSX2

LTRESULT CFXButeMgrPlugin::PreHook_EditStringList(
	const char* szRezPath,
	const char* szPropName,
	char** aszStrings,
    uint32* pcStrings,
    const uint32 cMaxStrings,
    const uint32 cMaxStringLength)
{
	if (!g_pFXButeMgr)
	{
		// Make sure debris mgr is inited...

		m_DebrisMgrPlugin.PreHook_EditStringList(szRezPath, szPropName,
			aszStrings,	pcStrings, cMaxStrings, cMaxStringLength);

		// This will set the g_pFXButeMgr...Since this could also be
		// set elsewhere, just check for the global bute mgr...

		char szFile[256];
		sprintf(szFile, "%s\\%s", szRezPath, FXBMGR_DEFAULT_FILE);
        sm_FXButeMgr.SetInRezFile(LTFALSE);
        sm_FXButeMgr.Init(szFile);
	}

	return LT_UNSUPPORTED;
}

// ----------------------------------------------------------------------- //
//
//	ROUTINE:	CFXButeMgrPlugin::PopulateStringList
//
//	PURPOSE:	Populate the list
//
// ----------------------------------------------------------------------- //

LTBOOL CFXButeMgrPlugin::PopulateStringList(char** aszStrings, uint32* pcStrings,
    const uint32 cMaxStrings, const uint32 cMaxStringLength)
{
    if (!aszStrings || !pcStrings) return LTFALSE;
	_ASSERT(aszStrings && pcStrings);

	// Add an entry for each impact fx

	int nNumImpactFX = g_pFXButeMgr->GetNumImpactFX();
	_ASSERT(nNumImpactFX > 0);

    IMPACTFX* pImpactFX = LTNULL;

	for (int i=0; i < nNumImpactFX; i++)
	{
		_ASSERT(cMaxStrings > (*pcStrings) + 1);

		pImpactFX = g_pFXButeMgr->GetImpactFX(i);
        uint32 dwImpactFXNameLen = strlen(pImpactFX->szName);

		if (pImpactFX && pImpactFX->szName[0] &&
			dwImpactFXNameLen < cMaxStringLength &&
			((*pcStrings) + 1) < cMaxStrings)
		{
			strcpy(aszStrings[(*pcStrings)++], pImpactFX->szName);
		}
	}

    return LTTRUE;
}

#endif // !__PSX2
#endif // _CLIENTBUILD
