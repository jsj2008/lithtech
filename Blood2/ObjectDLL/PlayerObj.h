
#ifndef __PLAYEROBJ_H__
#define __PLAYEROBJ_H__


#include <math.h>
#include "serverobj_de.h"
#include "engineobjects_de.h"
#include "BaseCharacter.h"
#include "SharedDefs.h"
#include "GameWeapons.h"
//#include "gib.h"
#include "AI_Shared.h"
#include "Music.h"
#include "SharedMovement.h"
#include "CVarTrack.h"
#include "SoundTypes.h"

enum ModelSize { MS_NORMAL=0, MS_SMALL, MS_LARGE, NUM_MODELSIZES };

// Player movement states, used to track animations playing.
#define PMOVE_NONE			0
#define PMOVE_IDLE			1
#define PMOVE_WALK			2
#define PMOVE_RUN			3
#define PMOVE_STRAFERIGHT	4
#define PMOVE_STRAFELEFT	5

#define PMOVE_JUMP			6
#define PMOVE_CROUCH		7
#define PMOVE_CRAWL			8
#define PMOVE_FIRE_STAND	9
#define PMOVE_FIRE_WALK		10
#define PMOVE_FIRE_RUN		11
#define PMOVE_FIRE_JUMP		12
#define PMOVE_FIRE_CROUCH	13
#define PMOVE_FIRE_CRAWL	14
#define PMOVE_HUMILIATION	15
#define PMOVE_DEAD			16


// Control flags
#define  CTRLFLAG_RUN			0x0001
#define  CTRLFLAG_JUMP			0x0002
#define  CTRLFLAG_CROUCH		0x0004
#define  CTRLFLAG_FORWARD		0x0008
#define  CTRLFLAG_BACKWARD		0x0010
#define  CTRLFLAG_LEFT			0x0020
#define  CTRLFLAG_RIGHT			0x0040
#define  CTRLFLAG_STRAFE		0x0080
#define  CTRLFLAG_STRAFERIGHT	0x0100	
#define  CTRLFLAG_STRAFELEFT	0x0200
#define  CTRLFLAG_FIRE			0x0400
#define  CTRLFLAG_ALTFIRE		0x0800
#define  CTRLFLAG_GRAB			0x1000
#define  CTRLFLAG_TAUNT			0x2000

#define PU_MAXSTACK				15
enum PowerupType
{
	PU_NONE = -1,
	PU_INVULNERABLE,
	PU_INVISIBLE,
	PU_TRIPLEDAMAGE,
	PU_INCORPOREAL,
	PU_SILENT,
	PU_MAXTYPES
};

class AI_Shared;

class CPlayerObj : public CBaseCharacter
{
	public:
		CPlayerObj();
		virtual		~CPlayerObj();

		void		SetClient(HCLIENT hClient) { m_hClient = hClient; m_InventoryMgr.SetClient(hClient); }
		HCLIENT		GetClient() const		{ return m_hClient; }
		DBYTE		GetCharacter() const	{ return m_nCharacter; }
		void		SetCharacter(DBYTE nCharacter);
		DBOOL		IsActivated() { return m_bActivated; }

		void		PostPropRead(ObjectCreateStruct *pStruct);
		DBOOL		InitialUpdate(int nData);
		DBOOL		Update(DVector *pMovement);
		void		ObjectTouch(HOBJECT hObj, DFLOAT fData);
		void		ObjectCrush(HOBJECT hObj, DFLOAT fData);
		void		OnMessage(DBYTE messageID, HMESSAGEREAD hMessage);
		void		OnCommandOn(DDWORD commandNum);
		void		OnCommandOff(DDWORD commandNum);
		void		OnStringKey(ArgList* pArgList);
		void		SendWonkyVisionMsg(DFLOAT fTime, DBOOL bNoMove);
		void		SendFadeMsg(DBYTE byFadeIn);
		void		SendVoiceMessage(TheVoice eVoiceType, D_WORD wIndex, DBYTE nAttackerCharacter, DBYTE nVictimCharacter);
		void		SendMessageToClient(DBYTE messageID);
		void		SendConsoleMessage(char *msg);
		HSTRING		GetPlayerName() { return m_hstrPlayerName; }
		void		GetPlayerRotation(DRotation *rRotation) { ROT_COPY(*rRotation, m_rRotation); }
		void		GetPlayerForward(DVector *vForward) { VEC_COPY(*vForward, m_vForward); }
		DFLOAT		GetEyeLevel() { return m_fEyeLevel; }
		int			GetFrags() { return m_Frags; }
		void		SetFrags(int nFrags);
		DBOOL		IsFemale() { return (m_nCharacter == CHARACTER_OPHELIA || m_nCharacter == CHARACTER_GABREILLA); }
		void		AddFrag(HSTRING hstrPlayerName, CPlayerObj* pVictim);
		int			GetMeleeKills() { return m_MeleeKills; }
		void		AddMeleeKill() { m_MeleeKills++; }
		void		RemoveFrag();
		void		SetZoomMode(DBOOL bZoom) { m_bZoomView = bZoom; m_nZoomType = 0; SendMessageToClient(SMSG_ZOOMVIEW); }
		void		SetZoomMode(DBOOL bZoom, DBYTE nType) { m_bZoomView = bZoom; m_nZoomType = nType; SendMessageToClient(SMSG_ZOOMVIEW); }
		DBOOL		GetZoomMode( ) { return m_bZoomView; }
		DBOOL		HasSoulStealingBinding() { return m_bBindingSoulStealing; }
		void		Respawn();
		void		GoToStartPoint();
		ContainerCode GetContainer() { return m_eContainerCode; }
		DDWORD		GetTeamID() { return(m_dwTeamID); }
		void		UpdateTeamID();

		DBOOL		IsPowerupActive(int nIndex)	{ return m_bPowerupActivated[nIndex]; }
		DBOOL		IsPlayerDamageOk(CPlayerObj* pPlayer2);
		DBOOL		IsPlayerDamageOk(HOBJECT hPlayer2);


		// Player reset functions
		void		ResetPlayerInventory();

		// ADDED BY ANDY 9-20-98
		void		Slow(DFLOAT time);

		// For adding more bombs to the remote list
		void		AddRemoteBomb(CProjectile* pBomb);
		void		RemoveRemoteBomb(CProjectile* pBomb);

		// Set the orb object
		void		SetOrbObject(HOBJECT hObj);

		// Added by Loki 10/06/1998
		void		Imprison(DBOOL bImprison) { m_bImprisoned = bImprison; }

		// Added by Loki 10/21/1998
		DBOOL		IsImprisoned() { return m_bImprisoned; }

		// Added by Goble 10/11/98
		DBOOL		PlayVoiceGroupEventOnClient(int iGroupEvent, DBOOL bSinglePlayerOnly = DFALSE);
		DBOOL		PlayVoiceUniqueEventOnClient(int iUniqueEvent);
		void		PlayWeaponVoiceSoundOnClient(int nWeaponType);

		void		ResetIdleTime() { m_fIdleTime = 0; }

		void		SetClientSaveData( HMESSAGEREAD hClientSaveData );

		HOBJECT		m_hCameraObj;

		DLink		m_CabalLink;
		DLink		m_MonsterLink;

		void		SetMoveVel(float vel)		{ChangeSpeedsVar(m_fMoveVel, vel);}
//		void		SetWalkVel(float vel)		{ChangeSpeedsVar(m_fWalkVel, vel);}
		void		SetJumpVel(float vel)		{ChangeSpeedsVar(m_fJumpVel, vel);}
//		void		SetSwimVel(float vel)		{ChangeSpeedsVar(m_fSwimVel, vel);}
		void		SetMoveMul(float vel)		{ChangeSpeedsVar(m_fMoveMultiplier, vel);}
		void		SetMoveAccelMul(float vel)	{ChangeSpeedsVar(m_fMoveAccelMultiplier, vel);}
		void		SetJumpVelMul(float vel)	{ChangeSpeedsVar(m_fJumpMultiplier, vel);}
		void		SetLadderVel(float vel)		{ChangeSpeedsVar(m_fLadderVel, vel);}

		void		HandleWeaponFireMessage(HMESSAGEREAD hRead);
		void		HandleWeaponStateMessage(HMESSAGEREAD hRead);
		void		HandleWeaponSoundMessage(HMESSAGEREAD hRead);
		void		DoWeaponChange(DBYTE nWeaponId);

		void		GrabFlag(HOBJECT hFlag);
		void		GiveFlag(HOBJECT hFlagStand);
		void		DropFlag(DBOOL bNotifyClient = DTRUE);

#ifdef _ADDON
		void		SoccerGoal( DBYTE nTeamID );
#endif //_ADDON

		DBOOL		IsOnGround( ) { return m_bOnGround; }
		DBOOL		IsInSlowDeath() { return(m_bInSlowDeath); }
		DBOOL		IsDead() { return(m_bDead); }

		void		DoDeath(DBOOL bHumiliation = DFALSE);
		void		TweakSaveValuesForCharacterChange(int nNewCharacter);

	protected :

		void ChangeSpeedsVar(float &var, float val)
		{
			if(var != val)
				m_PStateChangeFlags |= PSTATE_SPEEDS;
			var = val;
		}


		DDWORD		EngineMessageFn(DDWORD messageID, void *pData, DFLOAT lData);
		DDWORD		ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);
	
	private:

		void		ToggleSpectatorMode();
		void		UpdateControlFlags();
		DBOOL		HandleMovement();
		DBOOL		HandleFiring();
		void		UpdateAnim();
		void		ValidateAttributes();
		void		ProcessCheat(DBYTE nCheatCode, DBOOL bState);
		void		SetPowerupValue(DBYTE nType);
		void		ClearPowerupValues();
		void		AddPowerup(HOBJECT hSender, DBYTE nType, DFLOAT fValue = 0);
		void		CheckPowerups();
		void		ResetPowerups();
		void		CheckItemsAndSpells();
		DBOOL		ProcessTriggerMsg(HOBJECT hSender, HMESSAGEREAD hRead);
		void		GrabObject();
		DBOOL		SetAnimation(DDWORD dwAni, DBOOL bLooping);
		void		SetForceUpdateList(ForceUpdate* pFU);
		void		UpdateFragStatus();
		void		IdleTime();
		void		DoWalkAnim();
		void		DoRunAnim();
		void		DoJumpAnim();
		void		DoCrouchAnim();
		void		DoCrawlAnim();
		void		DoFireStandAnim();
		void		DoFireWalkAnim();
		void		DoFireRunAnim();
		void		DoFireJumpAnim();
		void		DoFireCrouchAnim();
		void		DoFireCrawlAnim();
		void		DoStrafeRightAnim();
		void		DoStrafeLeftAnim();
		void		DoHumiliationAnim();
		void		DoTaunt();
		DBOOL		CreateCorpse();
		DBOOL		CreateGibs(DVector vDir, int nNumGibs, int nType, DFLOAT fDamage);
		void		UpdateStats( );

		DBOOL		PlayPlayerSound(char* szSound, DFLOAT fRadius, DBYTE byVolume, DBOOL bStream, DBYTE nPriority = SOUNDPRIORITYMOD_HIGH );
		DBOOL		GetFootstepSound(char* szBuf);
		DBOOL		GetLandingSound(char* szBuf);
		void		PlayJumpSound();
		
		void		Save(HMESSAGEWRITE hWrite, DDWORD dwSaveFlags);
		void		Load(HMESSAGEREAD hWrite, DDWORD dwLoadFlags);

		void		UpdateAirLevel();
		void		HandleDeath();

		void		StartPowerupSound( PowerupType ePowerupType );
		void		EndPowerupSound( PowerupType ePowerupType );
		void		SetMultiplayerAmmo();
		void		UpdateClientPhysics();
		void		HandlePlayerPositionMessage(HMESSAGEREAD hRead);
		void		TeleportClientToServerPos();
		void SetOnGround(DBOOL bOn)		
		{
			m_bLastOnGround = m_bOnGround;
			m_bOnGround		= bOn;
		}
		HMESSAGEWRITE StartMessageToClient( DBYTE msgID );


	private:
		// Member Variables
		// Structure to hold various data items sent by the client
		DFLOAT		m_fPitch;
		DFLOAT		m_fYaw;
		DRotation	m_rRotation;	// Current view rotation
		DVector		m_vForward;		// Forward vector derived from rotation
		DVector		m_vGunMuzzlePos;
		DVector		m_vlGunMuzzlePos;
		HSTRING		m_hstrPlayerName;
		DBYTE		m_byFlags;
		DFLOAT		m_fMouseAxis0;
		DFLOAT		m_fMouseAxis1;

        AI_Shared   AIShared;       // Shared functions
	
		// animation stuff
		DBYTE		m_byMoveState;
		int			m_nInjuredLeg;
		int			m_nNodeHit;
		int			m_nCorpseType;
		int			m_nSideHit;
		DFLOAT		m_fAttackLoadTime;
		DFLOAT		m_fLoadTimeStart;

		DVector		m_vScale;			//scale
        
		DBOOL		m_bZoomView;
		DBYTE		m_nZoomType;
		DDWORD		m_CurGun;			// which gun are we currently using?
		int			m_CurGunSlot;
		DBYTE		m_CurItem;
		DBYTE		m_DropGunSlot;
		DBYTE		m_DropItemSlot;
		DBOOL		m_bFirstUpdate;
		DBOOL		m_bDemoPlayback;
		DDWORD		m_dwControlFlags;

		DBOOL		m_bFiringWeapon;	// Are we currently firing?
		DBOOL		m_bAnimating;		// Is the model playing an animation?

		DFLOAT		m_fEyeLevel;		// Player's viewpoint
		DBOOL		m_bLastCrouchCommand;
		DBOOL		m_bForcedCrouch;		// Forced to crouch by geometry
		DBOOL		m_bLastJumpCommand;
		DBOOL		m_bFalling;
		DBOOL		m_bServerFalling;
		DFLOAT		m_startFall;
		DFLOAT		m_startIdle;
		DFLOAT		m_fMinFallPos;
		DFLOAT		m_fMaxFallPos;

		HCONVAR		m_hPlrPosVar;

		DBOOL		m_bSlowMode;
		DFLOAT		m_fSlowTime;

		// Remote bombs list
		ObjectList	*m_pBombList;
		HOBJECT		m_hOrbObj;

		int			m_Frags;
		int			m_MeleeKills;
		DDWORD		m_nLastHitPoints;
		DDWORD		m_nLastDamage;
		DDWORD		m_nLastAmmo;
		DDWORD		m_nLastAltAmmo;
		DDWORD		m_nLastArmor;
		DDWORD		m_nLastFocus;

		DBYTE		m_nCharacter;	// Caleb, Ophelia, Ishmael, Gabriella
		DBYTE		m_nSkin;
		DBYTE		m_nAttribStrength;
		DBYTE		m_nAttribSpeed;
		DBYTE		m_nAttribResistance;
		DBYTE		m_nAttribMagic;
	
		DBYTE		m_nBindingStrength;
		DBYTE		m_nBindingSpeed;
		DBYTE		m_nBindingResistance;
		DBYTE		m_nBindingMagic;
		DBOOL		m_bBindingConstitution;
		DBOOL		m_bBindingBlending;
		DBOOL		m_bBindingSoulStealing;
		DBOOL		m_bBindingRegeneration;
		DFLOAT		m_fBindingRegenPeriod;
		DBOOL		m_bServerRegeneration;
		DFLOAT		m_fServerRegenPeriod;
		DBOOL		m_bBindingQuickness;
		DBOOL		m_bBindingIncreasedDamage;
		DBOOL		m_bBindingImpactResistance;

		DFLOAT		m_fBindingRegenTime;
		DFLOAT		m_fServerRegenTime;

		DBOOL		m_bSpectatorMode;
		HCLIENT		m_hClient;

        DFLOAT		m_fSmellTime;       // next smell time
        DFLOAT		m_fSmellDelay;      // Delay between smell drops
	
		DBOOL		m_bDead;
		DBOOL		m_bMovementBlocked;
		DFLOAT		m_fDeathTimer;
		DBOOL		m_bInSlowDeath;
		DFLOAT		m_fSlowDeathSafeTimer;
		DFLOAT		m_fSlowDeathStayTimer;

		// Added by Loki on 10/06/1998 - for Undead Gideon
		DBOOL		m_bImprisoned;
		DFLOAT		m_fImprisonStart;
		DFLOAT		m_fImprisonLength;

		DBOOL		m_bBurning;
		DFLOAT		m_fBurningTime;

		DBYTE		m_byAirborneCount;	// Count frames that I'm in the air

		// Powerup stuff...

		DBOOL		m_bPowerupActivated[PU_MAXTYPES];

		DFLOAT		m_fNighInvulnerableTime;
		DFLOAT		m_fInvisibleTime;
		DFLOAT		m_fTripleDamageTime;
		DFLOAT		m_fIncorporealTime;
		DFLOAT		m_fSilentTime;

		PowerupType m_ePowerupSoundStack[PU_MAXSTACK];
		DBYTE		m_nPowerupSoundStackPos;
		HSOUNDDE	m_hPowerupStartSound;
		HSOUNDDE	m_hPowerupEndSound;
		HSOUNDDE	m_hPowerupLoopSound;


		DBYTE		m_nStatsFlags;

		HSTRING		m_hstrTrigger;

		DVector		m_vLastPos;
		DVector		m_vLastVel;

		HOBJECT		m_hEnemyAttach;

		// Music
		CMusic		m_Music;

		DBOOL		m_bActivated;

		// Sound
		HSOUNDDE	m_hCurSound;
		HSOUNDDE	m_hWeapSound;

		// Multiplayer stuff
		HSTRING		m_hstrWhoKilledMeLast;
		HSTRING		m_hstrWhoIKilledLast;

		DFLOAT		m_fOldAirLevel;
		DFLOAT		m_fAirLevel;

		SurfaceType	m_eLastSurface;

		DBOOL		m_bSwimmingJump;
		DBOOL		m_bCanSwimJump;

		DBOOL		m_bNextFootstepLeft;
		DBOOL		m_bDisableFootsteps;

		// Idle stuff
		DFLOAT		m_fIdleTime;

		// Ping
		DFLOAT		m_fNextPingTime;

		// Save game stuff...
		HMESSAGEREAD m_hClientSaveData;

		HOBJECT		m_CurContainers[MAX_TRACKED_CONTAINERS];
		DDWORD		m_nCurContainers;
		DDWORD		m_PStateChangeFlags;	// Which things need to get sent to client.
		CVarTrack	m_LeashLenTrack;
		float		m_fLeashLen;
		DFLOAT		m_fBaseMoveAccel;			// The base (starting) move acceleration
		DFLOAT		m_fLadderVel;				// How fast we swim
//		DFLOAT		m_fSwimVel;					// How fast we swim
		DFLOAT		m_fMoveVel;					// How fast we run
//		DFLOAT		m_fWalkVel;					// How fast we walk
		DFLOAT		m_fJumpVel;					// How fast we jump
		DFLOAT		m_fDimsScale[NUM_MODELSIZES];	// Normal, small, large scale value

		DFLOAT		m_fMoveMultiplier;
		DFLOAT		m_fMoveAccelMultiplier;
		DFLOAT		m_fJumpMultiplier;
		DBYTE		m_byClientMoveCode;
		DBOOL		m_bOnGround;
		DBOOL		m_bLastOnGround;

		// Team stuff...
		DDWORD		m_dwTeamID;

		// Capture the flag stuff...
		HOBJECT		m_hFlagObject;
		HATTACHMENT	m_hFlagAttach;

		DBOOL		m_bDoorPush;
		DVector		m_vDoorPush;

		// To let the server know if the player is in input mode on the client
		DBOOL		m_bInputMode;

		DBOOL		m_bExternalCamera;

		// List to keep track of attached objects, like the seeing eye
		DList		m_AttachedObjectsList;

		// Time to wait until before teleporting server object to client object position
		float		m_fTeleportTime;
};


struct CharacterValues
{
	char*	szName;
	char*	szModelName;
	char*	szSkinName;
	DFLOAT	fHeightStanding;
	DFLOAT	fHeightCrouching;
	DFLOAT	fHeightDead;
	DFLOAT	fEyeLevelStanding;
	DFLOAT	fEyeLevelCrouching;
	DFLOAT	fEyeLevelDead;
};


// Provide a global so that the start point is known.
extern char msgbuf[255];
void BPrint(char *);

#endif  // __PLAYEROBJ_H__