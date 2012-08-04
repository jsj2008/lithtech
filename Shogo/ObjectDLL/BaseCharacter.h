// ----------------------------------------------------------------------- //
//
// MODULE  : BaseCharacter.h
//
// PURPOSE : Base class for player and AI
//
// CREATED : 10/6/97
//
// ----------------------------------------------------------------------- //

#ifndef __BASE_CHARACTER_H__
#define __BASE_CHARACTER_H__

#include "cpp_engineobjects_de.h"
#include "Weapons.h"
#include "Destructable.h"
#include "Activation.h"
#include "Inventory.h"
#include "RiotSoundTypes.h"
#include "CharacterAttributes.h"
#include "CharacterAlignment.h"
#include "ContainerCodes.h"
#include "Powerup.h"
#include "GibTypes.h"
#include "InventoryTypes.h"
#include "ModelFuncs.h"
#include "ModelNodes.h"
#include "SharedMovement.h"


// Control flags (movement, weapons)...

#define BC_CFLG_FORWARD			(1<<0)
#define BC_CFLG_REVERSE			(1<<1)
#define BC_CFLG_RIGHT			(1<<2)
#define BC_CFLG_LEFT			(1<<3)
#define BC_CFLG_JUMP			(1<<4)
#define BC_CFLG_DOUBLEJUMP		(1<<5)
#define BC_CFLG_DUCK			(1<<6)
#define BC_CFLG_STRAFE			(1<<7)
#define BC_CFLG_STRAFE_LEFT		(1<<8)
#define BC_CFLG_STRAFE_RIGHT	(1<<9)
#define BC_CFLG_RUN				(1<<10)
#define BC_CFLG_FIRING			(1<<11)	
#define BC_CFLG_MOVING			(1<<12)	
#define BC_CFLG_POSING			(1<<13)


#define INVALID_ANI				((HMODELANIM)-1)

#define	ANIM_TEARS							"TEARS"


// Other defines...

#define MAX_TIMED_POWERUPS				8

#define KEY_FOOTSTEP_SOUND		"FOOTSTEP_KEY"
#define KEY_SET_DIMS			"SETDIMS"
#define KEY_MOVE				"MOVE"
#define KEY_PLAYSOUND			"PLAYSOUND"
#define TRIGGER_PLAY_SOUND		"PLAYSOUND"

class  VolumeBrush;

enum CharacterDeath { CD_NORMAL=0, CD_GIB, CD_FREEZE, CD_VAPORIZE, CD_BURST };

enum CharacterSoundType { CST_NONE=0, CST_DAMAGE, CST_DEATH, CST_DIALOG, CST_EXCLAMATION };

class CBaseCharacter : public BaseClass
{
	public :

		CBaseCharacter();
		~CBaseCharacter();

		DBOOL			IsDead()			const { return m_damage.IsDead(); }
		ContainerCode	GetContainerCode()	const { return m_eContainerCode; }
		ModelSize		GetModelSize()		const { return m_eModelSize; }
		virtual DBOOL	IsMecha()				  { return m_bIsMecha; }
		Inventory*		GetInventory()			  { return &m_inventory; }

		DBOOL HasDamageUpgrade()					{ return m_inventory.HaveItem (IT_UPGRADE, IST_UPGRADE_DAMAGE); }
		DBOOL HasProtectionUpgrade()				{ return m_inventory.HaveItem (IT_UPGRADE, IST_UPGRADE_PROTECTION); }
		DBOOL HasRegenUpgrade()						{ return m_inventory.HaveItem (IT_UPGRADE, IST_UPGRADE_REGEN); }
		DBOOL HasHealthUpgrade()					{ return m_inventory.HaveItem (IT_UPGRADE, IST_UPGRADE_HEALTH); }
		DBOOL HasArmorUpgrade()						{ return m_inventory.HaveItem (IT_UPGRADE, IST_UPGRADE_ARMOR); }
		DBOOL HasTargetingUpgrade()					{ return m_inventory.HaveItem (IT_UPGRADE, IST_UPGRADE_TARGETING); }
		DBOOL HasDamageEnhancement()				{ return m_inventory.HaveItem (IT_ENHANCEMENT, IST_ENHANCEMENT_DAMAGE); }
		DBOOL HasMeleeDamageEnhancement()			{ return m_inventory.HaveItem (IT_ENHANCEMENT, IST_ENHANCEMENT_MELEEDAMAGE); }
		DBOOL HasProtectionEnhancement()			{ return m_inventory.HaveItem (IT_ENHANCEMENT, IST_ENHANCEMENT_PROTECTION); }
		DBOOL HasEnergyProtectionEnhancement()		{ return m_inventory.HaveItem (IT_ENHANCEMENT, IST_ENHANCEMENT_ENERGYPROTECTION); }
		DBOOL HasProjectileProtectionEnhancement()	{ return m_inventory.HaveItem (IT_ENHANCEMENT, IST_ENHANCEMENT_PROJECTILEPROTECTION); }
		DBOOL HasExplosiveProtectionEnhancement()	{ return m_inventory.HaveItem (IT_ENHANCEMENT, IST_ENHANCEMENT_EXPLOSIVEPROTECTION); }
		DBOOL HasRegenEnhancement()					{ return m_inventory.HaveItem (IT_ENHANCEMENT, IST_ENHANCEMENT_REGEN); }
		DBOOL HasHealthEnhancement()				{ return m_inventory.HaveItem (IT_ENHANCEMENT, IST_ENHANCEMENT_HEALTH); }
		DBOOL HasArmorEnhancement()					{ return m_inventory.HaveItem (IT_ENHANCEMENT, IST_ENHANCEMENT_ARMOR); }

		void SetOnGround(DBOOL bOn)		
		{
			m_bLastOnGround = m_bOnGround;
			m_bOnGround		= bOn;
		}

		CDestructable*	GetDestructable() { return &m_damage; }
		CharacterClass  GetCharacterClass() const { return m_cc; }

		void			UpdateInLiquid(VolumeBrush* pBrush, ContainerPhysics* pCPStruct);
		void			UpdateOnLadder(VolumeBrush* pBrush, ContainerPhysics* pCPStruct);

		DBOOL			AddTimedPowerup (TimedPowerup* pPowerup);
		DBOOL			HaveTimedPowerup (PickupItemType eType);
		void			SaveModelColor();
	
		char*			GetHandName() const { return m_pHandName; }

		virtual void	HandleWeaponChange();
		void			HandleBigGunsCheat();

		CharacterDeath	GetDeathType() const { return m_eDeathType; }
		DBYTE			GetModelId()   const { return m_nModelId; }

		NodeType		GetNodeType(DDWORD nNode);
		void			SetLastHitModelNode(DDWORD dwIndex)	{ m_dwLastHitNode = dwIndex; }
		DBOOL			UsingHitDetection() const			{ return m_bUsingHitDetection; }

		DBOOL			CanCarryWeapon(DBYTE nWeaponId);

		void			SetLastFireInfo(DVector* pvFiredPos, DVector* pvImpactPos, 
									    DBYTE nWeaponId, DBOOL bSilenced);
		void			GetLastFireInfo(DVector & vFiredPos, DVector & vImpactPos, 
										DBYTE & nWeaponId, DFLOAT & fTime, DBOOL & bSilenced);

		DBOOL			CanDamageBody()	const { return m_bCanDamageBody; }

		void	SetRunVel(float vel)		{ChangeSpeedsVar(m_fRunVel, vel);}
		void	SetWalkVel(float vel)		{ChangeSpeedsVar(m_fWalkVel, vel);}
		void	SetJumpVel(float vel)		{ChangeSpeedsVar(m_fJumpVel, vel);}
		void	SetSwimVel(float vel)		{ChangeSpeedsVar(m_fSwimVel, vel);}
		void	SetMoveMul(float vel)		{ChangeSpeedsVar(m_fMoveMultiplier, vel);}
		void	SetMoveAccelMul(float vel)	{ChangeSpeedsVar(m_fMoveAccelMultiplier, vel);}
		void	SetJumpVelMul(float vel)	{ChangeSpeedsVar(m_fJumpMultiplier, vel);}
		void	SetLadderVel(float vel)		{ChangeSpeedsVar(m_fLadderVel, vel);}



	protected :

		void ChangeSpeedsVar(float &var, float val)
		{
			if(var != val)
				m_PStateChangeFlags |= PSTATE_SPEEDS;
			var = val;
		}

		HMODELANIM		GetRecoilAni();

		void			ClearControlFlags() { m_dwControlFlags = 0; }
		void			SetControlFlag(DDWORD dwFlag)	{m_dwControlFlags |= dwFlag;}
		void			ClearControlFlag(DDWORD dwFlag) { m_dwControlFlags &= ~dwFlag; }
		DDWORD			GetControlFlags() const { return m_dwControlFlags; }

		virtual void	UpdateControlFlags();
		virtual void	UpdateAnimation();

		virtual DVector	GetHeadOffset();

		DDWORD			EngineMessageFn(DDWORD messageID, void *pData, DFLOAT fData);
		DDWORD			ObjectMessageFn(HOBJECT hSender, DDWORD messageID, HMESSAGEREAD hRead);

		void			AddAmmo(DBYTE nWeaponID, int nAmmo)		{ m_weapons.AddAmmo(nWeaponID, nAmmo); }

		virtual void	Reset();
		virtual DBOOL	SetDims(DVector* pvDims, DBOOL bSetLargest=DTRUE);

		virtual void	SetAnimationIndexes();

		virtual void	SetAnimation(HMODELANIM hAni, DBOOL bLoop=DTRUE);
		
		virtual void	SetStandAnimation();
		virtual void	SetWalkAnimation();
		virtual void	SetRunAnimation();
		virtual void	SetCrouchAnimation();
		virtual void	SetDeathAnimation();
		virtual void	SetTransformAnimation();
		virtual void	SetLandingAnimation();
		virtual void	SetSwimAnimation();
		virtual void	SetRecoilAnimation();
		virtual void	UpdateRecoilAnimation();
		virtual void	SetTearsAnimation();
		virtual void	UpdateTearsAnimation();

		virtual DBOOL	CheckForceJump(DVector *pvAccel, DVector *pvForward);
		virtual void	UpdateOnGround();
		virtual void	UpdateMovement(DBOOL bUpdatePhysics=DTRUE);
		virtual void	UpdateInAir();
		virtual void	UpdateSounds();
		virtual void	KillDlgSnd();

		virtual void	CheckPowerups();									// Check to see if any of our timed powerups have expired
		virtual void	ApplyPowerups();									// Apply any timed powerups we have
		virtual void	OnTimedPowerupExpiration (PickupItemType eType);	// Called when a timed powerup expires
		void RemoveAllPowerups();

		virtual void	PlayVehicleSound();
		virtual char*	GetDamageSound(DamageType eType);
		virtual char*	GetFootStepSound(SurfaceType eSurfaceType);
		virtual void	PlayFootStepSound(SurfaceType eSurfaceType);
		virtual char*	GetDeathSound();
		virtual void    PlayDialogSound(char* pSound, CharacterSoundType eType=CST_DIALOG, DBOOL bAtObjectPos=DFALSE);
		virtual void	PlayDamageSound(DamageType eType);
		virtual void	PlayDeathSound();
		virtual void	HandleDead(DBOOL bRemoveObj);

		virtual void	StartDeath();
		virtual void	SpawnWeapon();
		virtual void	SpawnItem(char* pItem, DVector & vPos, DRotation & rRot);
		virtual void	CreateBody();

		virtual DBOOL	ProcessTriggerMsg(char* pMsg);
		virtual void	ProcessDamageMsg(HMESSAGEREAD hRead);
		virtual DBOOL	ProcessCommand(char** pTokens, int nArgs, char* pNextCommand);

		virtual void	PlaySound( char *pSoundName, DBYTE nPriorityMod, DFLOAT fRadius=1500.0f, DBOOL bAttached = DTRUE);
		virtual void	PlaySound( HSTRING hstrSoundName, DBYTE nPriorityMod, DFLOAT fRadius=1500.0f, DBOOL bAttached = DTRUE);

		virtual void	CreateHandHeldWeapon(char* pFilename, char* pSkin);
		virtual DVector	HandHeldWeaponFirePos();

		virtual void	CreateDialogSprite();
		virtual char*	GetDialogSpriteFilename(DVector & vScale);

		virtual void	CreateBoundingBox();
		virtual void	UpdateBoundingBox();

		virtual	void	AdjustDamageAggregate() {}
		virtual	void	InitializeWeapons() {}
		virtual void	RemoveHandHeldWeapon();

		virtual void	RemoveObject();

		DDWORD			m_PStateChangeFlags;	// Which things need to get sent to client.
		DBOOL			m_bCreateBody;				// Create body prop when dead
		DBOOL			m_bMoveToFloor;				// Move character to floor
		ModelSize		m_eModelSize;				// Size of model
		DDWORD			m_dwControlFlags;			// Control (movement) flags
		DDWORD			m_dwLastFrameCtlFlgs;		// Control flags on the last frame
		DFLOAT			m_fTimeInAir;				// How long have we been in the air
		CharacterDeath	m_eDeathType;				// How did we die
		DBOOL			m_bStartedDeath;			// Did I start death ani?
		DBOOL			m_bOneHandedWeapon;			// Are we holding a one handed weapon?
		DBOOL			m_bTransforming;			// Are we transforming (Mecha only)
		DBOOL			m_bLanding;					// Are we in the process of landing
		DBOOL			m_bAllowRun;				// Can we run
		DBOOL			m_bAllowMovement;			// Can we currently move
		DBOOL			m_bOnGround;				// Are we on the ground
		DBOOL			m_bLastOnGround;			// Were we on the ground on the last update
		DBOOL			m_bBipedal;					// Are we bipedal (not in vehicle mode - mecha only)
		DBOOL			m_bBipedalLastFrame;		// Were we bipedal on the last frame
		DBOOL			m_bSpectatorMode;			// Are we in spectator mode (player only)
		ContainerCode	m_eContainerCode;			// Code of container our Head is in (if any)
		ContainerCode	m_eLastContainerCode;		// Code of container our Head was in on last update
		DBOOL			m_bBodyInLiquid;			// Is our body in liquid
		DBOOL			m_bBodyWasInLiquid;			// Was our body in liquid on the last frame
		DBOOL			m_bBodyOnLadder;			// Is our body on a ladder
		DBOOL			m_bJumping;					// Are we jumping
		DBOOL			m_bJumped;					// Did we jump
		DBOOL			m_bRecoiling;				// Play recoil ani
		DVector			m_vOldCharacterColor;		// Old color (for use with stealth powerup)
		DFLOAT			m_fOldCharacterAlpha;		// Old alpha value (for use with stealth powerup)
		DBOOL			m_bCharacterHadShadow;		// character had shadow before stealth powerup
		DDWORD			m_dwLastHitNode;			// What model node was last hit
		DBOOL			m_bLeftFoot;				// Left foot down?
		DBYTE			m_nModelId;					// Id of the model used by this character
		HSTRING			m_hstrSpawnItem;			// Object to spawn when dead

		DVector			m_vLastFiredPos;		// Position when we last fired
		DVector			m_vLastImpactPos;		// Impact position of last fired weapon
		DBYTE			m_nLastFiredWeapon;		// Id of last fired weapon
		DFLOAT			m_fLastFiredTime;		// Time weapon was last fired
		DBOOL			m_bLastFireSilenced;	// Was the weapon silenced

		DFLOAT			m_fDefaultHitPts;		// Default hit pts
		DFLOAT			m_fDefaultArmor;		// Default armor
		DFLOAT			m_fSoundRadius;			// Radius for sounds we play
		DBYTE			m_nBasePriority;		// Base sound priority to use when playing sounds

		DFLOAT			m_fBaseMoveAccel;			// The base (starting) move acceleration
		DFLOAT			m_fLadderVel;				// How fast we swim
		DFLOAT			m_fSwimVel;					// How fast we swim
		DFLOAT			m_fRunVel;					// How fast we run
		DFLOAT			m_fWalkVel;					// How fast we walk
		DFLOAT			m_fJumpVel;					// How fast we jump
		DFLOAT			m_fDimsScale[NUM_MODELSIZES];	// Normal, small, large scale value
		DBOOL			m_bCreateDialogSprite;		// Create the dialog sprite
		DBOOL			m_bCreateHandHeldWeapon;	// Should we create a hand-held weapon
		DBOOL			m_bSpawnWeapon;				// Spawn weapon when dead
		DBOOL			m_bIsMecha;					// Are we a Mecha
		DBOOL			m_bUsingHitDetection;		// Using hit detection
		CharacterClass	m_cc;						// Who am I anyway...
		DDWORD			m_dwFlags;					// Initial flags

		TimedPowerup*	m_powerups[MAX_TIMED_POWERUPS];	// Any time-limited powerups we currently have

		HOBJECT			m_hHandHeldWeapon;			// Hand held weapon
		HOBJECT			m_hDlgSprite;				// Dialog sprite

		DBOOL			m_bTakesSqueakyDamage;
		DBOOL			m_bCrying;
		DBYTE			m_nSqueakyCount;

		DBOOL			m_bCanPlayDialogSound;
		DBOOL			m_bCanDamageBody;

	// NOTE:  The following data members do not need to be saved / loaded
	// when saving games.  Any data members that don't need to be saved
	// should be added here (to keep them together)...


		HOBJECT				m_hBoundingBox;			// Debugging bounding box model
		HSOUNDDE			m_hVehicleSound;		// Handle to vehicle sound (so we can stop it)
		HSOUNDDE			m_hCurDlgSnd;			// Handle to current dialog sound
		CharacterSoundType	m_eCurDlgSndType;		// Type of sound playing
		char*				m_pHandName;			// Name of hand node used to hold gun
		char*				m_pVehicleSound;		// Sound played in vehicle mode

		CWeapons		m_weapons;					// Our weapons
		CDestructable	m_damage;					// Handle damage/healing
		Inventory		m_inventory;				// What we are carrying 
		CActivation		m_activation;				// Handle activation

		// Animation indexes...

		HMODELANIM			m_hWalkKnifeAni;
		HMODELANIM			m_hWalkKnifeAttackAni;
		HMODELANIM			m_hWalkBKnifeAni;
		HMODELANIM			m_hWalkBKnifeAttackAni;
		HMODELANIM			m_hWalkStrafeLKnifeAni;
		HMODELANIM			m_hWalkStrafeRKnifeAni;
		HMODELANIM			m_hWalkStrafeLKnifeAttackAni;
		HMODELANIM			m_hWalkStrafeRKnifeAttackAni;
		HMODELANIM			m_hRunKnifeAni;
		HMODELANIM			m_hRunKnifeAttackAni;
		HMODELANIM			m_hRunBKnifeAni;
		HMODELANIM			m_hRunBKnifeAttackAni;
		HMODELANIM			m_hRunStrafeLKnifeAni;
		HMODELANIM			m_hRunStrafeRKnifeAni;
		HMODELANIM			m_hRunStrafeLKnifeAttackAni;
		HMODELANIM			m_hRunStrafeRKnifeAttackAni;
		HMODELANIM			m_hCrouchKnifeAni;
		HMODELANIM			m_hCrouchKnifeAttackAni;
		HMODELANIM			m_hCrouchWalkKnifeAni;
		HMODELANIM			m_hCrouchWalkKnifeAttackAni;
		HMODELANIM			m_hCrouchWalkBKnifeAni;
		HMODELANIM			m_hCrouchWalkBKnifeAttackAni;
		HMODELANIM			m_hCrouchStrafeLKnifeAni;
		HMODELANIM			m_hCrouchStrafeRKnifeAni;
		HMODELANIM			m_hCrouchStrafeLKnifeAttackAni;
		HMODELANIM			m_hCrouchStrafeRKnifeAttackAni;
		HMODELANIM			m_hWalkRifleAni;
		HMODELANIM			m_hWalkRifleAttackAni;
		HMODELANIM			m_hWalkBRifleAni;
		HMODELANIM			m_hWalkBRifleAttackAni;
		HMODELANIM			m_hWalkStrafeLRifleAni;
		HMODELANIM			m_hWalkStrafeRRifleAni;
		HMODELANIM			m_hWalkStrafeLRifleAttackAni;
		HMODELANIM			m_hWalkStrafeRRifleAttackAni;
		HMODELANIM			m_hRunRifleAni;
		HMODELANIM			m_hRunRifleAttackAni;
		HMODELANIM			m_hRunBRifleAni;
		HMODELANIM			m_hRunBRifleAttackAni;
		HMODELANIM			m_hRunStrafeLRifleAni;
		HMODELANIM			m_hRunStrafeRRifleAni;
		HMODELANIM			m_hRunStrafeLRifleAttackAni;
		HMODELANIM			m_hRunStrafeRRifleAttackAni;
		HMODELANIM			m_hCrouchRifleAni;
		HMODELANIM			m_hCrouchRifleAttackAni;
		HMODELANIM			m_hCrouchWalkRifleAni;
		HMODELANIM			m_hCrouchWalkRifleAttackAni;
		HMODELANIM			m_hCrouchWalkBRifleAni;
		HMODELANIM			m_hCrouchWalkBRifleAttackAni;
		HMODELANIM			m_hCrouchStrafeLRifleAni;
		HMODELANIM			m_hCrouchStrafeRRifleAni;
		HMODELANIM			m_hCrouchStrafeLRifleAttackAni;
		HMODELANIM			m_hCrouchStrafeRRifleAttackAni;
		HMODELANIM			m_hPose1Ani;
		HMODELANIM			m_hPose2Ani;
		HMODELANIM			m_hRifleIdleAni[5];
		HMODELANIM			m_hKnifeIdle1Ani;
		HMODELANIM			m_hKnifeIdle2Ani;
		HMODELANIM			m_hJumpUpAni;
		HMODELANIM			m_hJumpDownAni;
		HMODELANIM			m_hLandingAni;
		HMODELANIM			m_hTransformAni;
		HMODELANIM			m_hInverseTransformAni;
		HMODELANIM			m_hVehicleAni;
		HMODELANIM			m_hSwimAni;
		HMODELANIM			m_hTearsAni;
		HMODELANIM			m_hStandKnifeAttackAni;
		HMODELANIM			m_hStandRifleAttackAni;

		DFLOAT				m_fMoveMultiplier;
		DFLOAT				m_fJumpMultiplier;
		DFLOAT				m_fMoveAccelMultiplier;

	private :

		void	InitialUpdate(int nInfo);	
		void	Update();
		DBOOL	ReadProp(ObjectCreateStruct *pStruct);
		void	HandleModelString(ArgList* pArgList);
		void	Save(HMESSAGEWRITE hWrite);
		void	Load(HMESSAGEREAD hRead);
		void	CacheFiles();

		void	UpdateContainerCode();
		void	UpdateCheatInfo();
};


#endif // __BASE_CHARACTER_H__
