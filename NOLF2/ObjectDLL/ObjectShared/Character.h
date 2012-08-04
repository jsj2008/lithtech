// ----------------------------------------------------------------------- //
//
// MODULE  : Character.h
//
// PURPOSE : Base class for player and AI
//
// CREATED : 10/6/97
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CHARACTER_H__
#define __CHARACTER_H__

#include "GameBase.h"
#include "Weapons.h"
#include "ServerUtilities.h"
#include "SurfaceMgr.h"
#include "Destructible.h"
#include "SoundTypes.h"
#include "ContainerCodes.h"
#include "GibTypes.h"
#include "ModelButeMgr.h"
#include "SharedMovement.h"
#include "Editable.h"
#include "SFXMsgIDs.h"
#include "LTObjRef.h"
#include "GeneralInventory.h"
#include "IDList.h"
#include "IHitBoxUser.h"
#include "Timer.h"
#include "MusicMgr.h"
#include "PlayerTracker.h"

LINKTO_MODULE( Character );


#define BC_DEFAULT_SOUND_RADIUS	 1500.0f

class AIVolume;
class AIInformationVolume;
class Body;
class CAttachments;
class CAnimator;
class TeleportPoint;
class RelationData;
class RelationSet;
class CObjectRelationMgr;
class CSearchable;

enum  EnumAIStateType;
enum  EnumAIStimulusID;
enum CharacterClass;

struct CHARCREATESTRUCT;

struct CharFireInfo
{
	LTObjRef	hObject;
    LTVector    vFiredPos;
    LTVector    vImpactPos;
    uint8       nWeaponId;
    uint8       nAmmoId;
    LTFLOAT     fTime;
    LTBOOL      bSilenced;
	SurfaceType	eSurface;

	CharFireInfo() { Clear(); }
	void Clear();

	void Save(ILTMessage_Write *pMsg);
	void Load(ILTMessage_Read *pMsg);
};

inline void CharFireInfo::Clear()
{
	hObject = 0;
    vFiredPos.Init();
    vImpactPos.Init();
    nAmmoId = 0;
    bSilenced = false;
	eSurface = ST_UNKNOWN;
	nWeaponId = WMGR_INVALID_ID;
    fTime = (float)-INT_MAX;
}

inline void CharFireInfo::Save(ILTMessage_Write *pMsg)
{
    if (!pMsg) return;

	SAVE_HOBJECT(hObject);
	SAVE_VECTOR(vFiredPos);
    SAVE_VECTOR(vImpactPos);
    SAVE_BYTE(nWeaponId);
    SAVE_BYTE(nAmmoId);
    SAVE_TIME(fTime);
    SAVE_BOOL(bSilenced);
    SAVE_BYTE(eSurface);
}

inline void CharFireInfo::Load(ILTMessage_Read *pMsg)
{
    if (!pMsg) return;

	LOAD_HOBJECT(hObject);
    LOAD_VECTOR(vFiredPos);
    LOAD_VECTOR(vImpactPos);
    LOAD_BYTE(nWeaponId);
    LOAD_BYTE(nAmmoId);
    LOAD_TIME(fTime);
    LOAD_BOOL(bSilenced);
    LOAD_BYTE_CAST(eSurface, SurfaceType);
}



// Other defines...

#define MAX_TIMED_POWERUPS				8


class  VolumeBrush;

enum CharacterDeath { CD_NORMAL=0, CD_GIB, CD_FREEZE, CD_VAPORIZE, CD_BURST };

class CCharacter : public GameBase, public IHitBoxUser, public IPlayerTrackerReceiver
{
	public :

		CCharacter();
		~CCharacter();

        LTBOOL          IsDead()            const { return m_damage.IsDead(); }
        LTBOOL          IsFlashLightOn()    const;
		virtual LTBOOL	IsHidden()			const { return LTFALSE; }
		ContainerCode	GetContainerCode()	const { return m_eContainerCode; }

		// GameBase Overrides...

		virtual void AddToObjectList( ObjectList *pObjList, eObjListControl eControl = eObjListNODuplicates );

		// damage filtering
		virtual bool        FilterDamage( DamageStruct *pDamageStruct );

		// SFX

		void SendStealthToClients();
		void SendDamageFlagsToClients();
		void SendAllFXToClients() { CreateSpecialFX( LTTRUE ); }

		void CreateCigarette(LTBOOL bSmoke);
		void DestroyCigarette();

		bool HasZzz() const;
		void CreateZzz();
		void DestroyZzz();

		LTBOOL IsArmored() { return m_bArmored; }
		virtual void CreateArmor();
		virtual void DestroyArmor();

		LTBOOL HasFlashLight();
		void CreateFlashLight();
		void DestroyFlashLight();

		// Check if character is deflecting shots.
		bool			IsDeflecting( );
		virtual void	SetDeflecting( float fDuration );

		// Attachments
        virtual void    CreateAttachments() { _ASSERT(LTFALSE); }
        void            DestroyAttachments();
		CAttachments*	GetAttachments() { return m_pAttachments; }
		void			HideAttachments(LTBOOL bHide);
		virtual CAttachments*	TransferAttachments( bool bRemove ); // Responsibility of call to delete attachments
		virtual void			TransferWeapons(Body* pBody, bool bRemove);
		void			TransferSpears(Body* pBody);
		virtual	void	RemoveAttachments( bool bDestroyAttachments );
		virtual void	RemoveWeapons();

		bool			AddSpear(HOBJECT hSpear, ModelNode eModelNode, const LTRotation& rRot, bool bCanWallStick );

        void SetOnGround(LTBOOL bOn) { m_bOnGround = bOn; }
        LTBOOL IsOnGround() const { return m_bOnGround; }

        virtual LTFLOAT ComputeDamageModifier(ModelNode eModelNode);

		CDestructible*	GetDestructible() { return &m_damage; }

		CSearchable*	GetSearchable() { return m_pSearch; }


		const RelationData&	GetRelationData() const;
		const RelationSet&	GetRelationSet() const;
		CObjectRelationMgr* GetRelationMgr();
		virtual void   UpdateRelationMgr();

		void			RegisterPersistentStimuli(void);
		void			RemovePersistentStimuli(void);

		HOBJECT			GetHitBox()			const { return m_hHitBox; }
		HOBJECT			GetLastDamager()	const { return m_damage.GetLastDamager(); }

		void			SetBlinking(LTBOOL bBlinking);

		LTBOOL			IsShortRecoiling()	const { return m_bShortRecoiling; }

		void			UpdateOnLadder(VolumeBrush* pBrush, ContainerPhysics* pCPStruct);
		void			UpdateInLiquid(VolumeBrush* pBrush, ContainerPhysics* pCPStruct);

		char*			GetHandName() const { return m_pHandName; }

		CharacterDeath	GetDeathType() const { return m_eDeathType; }
		ModelId			GetModelId()   const { return m_eModelId; }
		virtual ModelSkeleton	GetModelSkeleton()   const { return m_eModelSkeleton; }

		void			SetModelNodeLastHit(ModelNode eModelNodeLastHit)	{ m_eModelNodeLastHit = eModelNodeLastHit; }
		ModelNode		GetModelNodeLastHit()	const	{ return m_eModelNodeLastHit; }
        virtual LTBOOL  UsingHitDetection()     const   { return m_bUsingHitDetection; }
		virtual float GetNodeRadius( ModelSkeleton eModelSkeleton, ModelNode eModelNode ) { return ( g_pModelButeMgr->GetSkeletonNodeHitRadius(eModelSkeleton, eModelNode) ); }

		void			SetLastFireInfo(CharFireInfo* pInfo);
		void			GetLastFireInfo(CharFireInfo & info);

		void			RecalcLastVolumePos();

		AIVolume*		GetLastVolume();
        LTBOOL          HasLastVolume() { return !!GetLastVolume(); }
        const LTVector& GetLastVolumePos() { return m_vLastVolumePos; }

		AIVolume*		GetCurrentVolume();
        LTBOOL          HasCurrentVolume() { return !!GetCurrentVolume(); }

		AIInformationVolume*	GetLastInformationVolume();
        LTBOOL					HasLastInformationVolume() { return !!GetLastInformationVolume(); }
        const LTVector&			GetLastInformationVolumePos() { return m_vLastInformationVolumePos; }

		AIInformationVolume*	GetCurrentInformationVolume();
        LTBOOL					HasCurrentInformationVolume() { return !!GetCurrentInformationVolume(); }
		void					UpdateCurrentInformationVolume(bool bForce = false);

        LTBOOL           HitFromFront(const LTVector& vDir);
        LTBOOL           HitFromBack(const LTVector& vDir) { return !HitFromFront(vDir); }

		virtual LTBOOL	ShouldWallStick();

        LTFLOAT          GetLastPainTime() { return m_fLastPainTime; }
        LTFLOAT          GetLastPainVolume() { return m_fLastPainVolume; }

		void			AddAggregate(LPAGGREGATE pAggregate) { GameBase::AddAggregate(pAggregate); }

        LTFLOAT          GetHitPoints() { return m_damage.GetHitPoints(); }
        LTFLOAT          GetMaxHitPoints() { return m_damage.GetMaxHitPoints(); }

		LTFLOAT			GetEnergy() { return m_damage.GetEnergy(); }
		LTFLOAT			GetMaxEnergy() { return m_damage.GetMaxEnergy(); }

        virtual void    SpawnItem(char* pItem, LTVector & vPos, LTRotation & rRot);

        virtual LTBOOL  CanLipSync() { return LTTRUE; }
		virtual LTBOOL	DoDialogueSubtitles() { return LTFALSE; }

		virtual void    PlayDialogSound(const char* pSound, CharacterSoundType eType=CST_DIALOG);
        LTBOOL          IsPlayingDialogSound() const { return m_eCurDlgSndType != CST_NONE; }
        LTBOOL          IsPlayingDialogue() { return (m_bPlayingTextDialogue || IsPlayingDialogSound()); }

		virtual void	KillDlgSnd();
		void			KillDialogueSound() { KillDlgSnd(); }

        LTBOOL          PlayDialogue( char *szDialogue );
        LTBOOL          PlayDialogue( DWORD dwID );

        virtual void    StopDialogue(LTBOOL bCinematicDone = LTFALSE);

		CMusicMgr::Mood	GetMusicMoodMin() { return m_eMusicMoodMin; }
		CMusicMgr::Mood GetMusicMoodMax() { return m_eMusicMoodMax; }

        virtual LTBOOL  SetDims(LTVector* pvDims, LTBOOL bSetLargest=LTTRUE);
        virtual LTFLOAT GetRadius();

        virtual LTBOOL  HasDangerousWeapon() { return LTFALSE; }
        virtual LTBOOL  HasMeleeWeapon() { return LTFALSE; }
		bool			IsVisible(void);

		virtual void	HideCharacter(LTBOOL bHide);

		inline	DamageFlags	GetDamageFlags( ) const { return m_nDamageFlags; }
		void			SetDamageFlags( const DamageFlags nDmgFlags );
		void			SetInstantDamageFlags( const DamageFlags nDmgFlags );

		virtual void	PushCharacter(const LTVector &vPos, LTFLOAT fRadius, LTFLOAT fStartDelay, LTFLOAT fDuration, LTFLOAT fStrength);

		uint8			GetPermissionSet() const {return m_iPermissionSet;}

		// General inventory accessor
		GEN_INVENTORY_LIST&	GetInventory() { return m_lstInventory; }

		// Key item handling
		IDList*			GetKeyList() { return &m_Keys; }
		void			UpdateKeys(uint8 nType, uint16 nId);

		// Implementing classes will have this function called
		// when HOBJECT ref points to gets deleted.
		virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );

		virtual void	SetHitPointsMod( float fMod );
		virtual void	SetArmorPointsMod( float fMod );

		virtual	void	SetPitch( float fPitch );
		float			GetPitchPercent( ) const { return m_fPitch; }
		LTRotation		GetRotationWithPitch( ) const;

		DamageType		GetDeathDamageType() const { return m_eDeathDamageType; }

		//
		// Defense
		//

		// true means the character is performing some type of defense
		virtual bool	IsDefending() const { return false; }

		// range: 0.0f - 1.0f; Get the "amount" of the defense...the
		// amount of damage that is suppressed by the defense.
		//
		// Example: if hit for 10 points of damage and the percentage
		// is 0.7, the character will take 3 points of damage.
		virtual float	GetDefensePercentage( LTVector const *pIncomingProjectilePosition = 0 ) const { return 0.0f; }
		
		virtual CAnimator* GetAnimator() { return m_pAnimator; }

	protected :

		friend class CCharacterHitBox;
		friend class CAttachments;

		virtual void	UpdateAnimation();

        virtual LTVector GetHeadOffset();

        uint32          EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
        uint32          ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);

		virtual	bool	OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);

		virtual void	ResetAfterDeath();

		virtual void	UpdateOnGround();
        virtual void    UpdateMovement(LTBOOL bUpdatePhysics=LTTRUE);
		virtual void	UpdateSounds();

		virtual void		SetDeathAnimation();
        virtual HMODELANIM  GetDeathAni(LTBOOL bFront);
		virtual	HMODELANIM	GetAlternateDeathAnimation() { return INVALID_ANI; }

		virtual LTFLOAT	GetFootstepVolume() { return 1.0f; }

        virtual char*   GetDamageSound(DamageType eType) { return LTNULL; }
        virtual char*   GetDeathSound() { return LTNULL; }
		virtual void	PlayDamageSound(DamageType eType);
        virtual void    PlayDeathSound();
        virtual void    HandleDead(LTBOOL bRemoveObj);
		void			HandleShortRecoil();

		// Last Volume* tracking information

		virtual void	HandleVolumeEnter(AIVolume* pVolume) {}
		virtual void	HandleVolumeExit(AIVolume* pVolume) {} 
		virtual void	HandleVolumeEnter(AIInformationVolume* pVolume) {}
		virtual void	HandleVolumeExit(AIInformationVolume* pVolume) {} 
		virtual float	GetVerticalThreshold() const;
		virtual float	GetInfoVerticalThreshold() const;

        virtual void    HandleGadget(uint8 nAmmoID) {}

        virtual void    HandleVectorImpact(IntersectInfo& iInfo, LTVector& vDir, LTVector& vFrom, ModelNode& eModelNode);

		virtual void	StartDeath();
		virtual void	CreateBody();
		virtual void	SetupBody(Body* pProp);

		virtual void	HandleAttach() {}
		virtual void	HandleDetach() {}

		void			UpdateTeleport( );
		virtual void	HandleTeleport(const LTVector& vPos) { };
		virtual void	HandleTeleport(TeleportPoint* pTeleportPoint) {}

		// Body state methods

		EnumAIStateType GetPriorityBodyState(EnumAIStateType bs1, EnumAIStateType bs2);
		virtual EnumAIStateType	GetBodyState();

		virtual void	ProcessDamageMsg(ILTMessage_Read *pMsg);

        virtual void    PlaySound(char *pSoundName, LTFLOAT fRadius=BC_DEFAULT_SOUND_RADIUS, LTBOOL bAttached=LTTRUE);

        virtual LTVector HandHeldWeaponFirePos(CWeapon* pWeapon);

		virtual void	CreateHitBox();
		virtual void	UpdateHitBox();
		virtual void	UpdateClientHitBox();
		
		virtual void	CreateSpecialFX(LTBOOL bUpdateClients=LTFALSE);
		virtual void	PreCreateSpecialFX(CHARCREATESTRUCT& cs) {};

		// Get the User-Flag version of our surface type
		virtual uint32	GetUserFlagSurfaceType();

		// Check if we should be doing lipsyncing on the dialog.
		bool			DoLipSyncingOnDlgSound( HOBJECT hSpeaker );

		// Handles when all players have broken their links.
		virtual void	OnPlayerTrackerAbort( );

		void			HandleSfxMessage( HOBJECT hSender, ILTMessage_Read *pMsg );

		// Tell client about crosshair alignment change.
		void			ResetCrosshair( );

		// Sets whether character has been hit by a tracking dart
		void			SetTracking( bool bTracking );

		// Sets whether character shows up on radar.
		void			SetRadarVisible( bool bRadarVisible );

		// Sets what character is carrying somthing.
		void			SetCarrying( uint8 nCarrying );
		uint8			GetCarrying() {return m_nCarrying;}


	protected :

		enum Constants
		{
			kMaxSpears = 16,
			kMaxWeapons = 2,
			kMaxProps = 32,
		};

		struct SPEARSTRUCT
		{
			LTObjRefNotifier	hObject;
			ModelNode	eModelNode;
            LTRotation  rRot;

			SPEARSTRUCT()
			{
				hObject = LTNULL;
				eModelNode = eModelNodeInvalid;
				rRot.Init();
			}
		};

	protected :
		CHARCREATESTRUCT*	m_pcs;							// Our character specialfx struct

		SurfaceType			m_eStandingOnSurface;			// What surface we're currently standing on.
        bool				m_bMakeBody;					// Make body when dead.
		bool				m_bPermanentBody;				// Dead body is permanent.
        LTBOOL				m_bMoveToFloor;					// Move character to floor
		CharacterDeath		m_eDeathType;					// How did we die
        LTBOOL              m_bStartedDeath;                // Did I start death ani?
        LTBOOL				m_bRolling;						// Are we in the process of rolling
        LTBOOL				m_bPivoting;					// Are we in the process of pivoting
        LTBOOL				m_bAllowRun;					// Can we run
        LTBOOL				m_bAllowMovement;				// Can we currently move
        LTBOOL				m_bOnGround;					// Are we on the ground
		ContainerCode		m_eContainerCode;				// Code of container our Head is in (if any)
		ContainerCode		m_eLastContainerCode;			// Code of container our Head was in on last update
        LTBOOL              m_bBodyInLiquid;                // Is our body in liquid
        LTBOOL				m_bBodyWasInLiquid;				// Was our body in liquid on the last frame
        LTBOOL				m_bBodyOnLadder;				// Is our body on a ladder
        LTVector			m_vOldCharacterColor;			// Old color (for use with stealth powerup)
        LTFLOAT				m_fOldCharacterAlpha;			// Old alpha value (for use with stealth powerup)
        LTBOOL				m_bCharacterHadShadow;			// character had shadow before stealth powerup
		ModelNode			m_eModelNodeLastHit;			// The model node that was last hit
        LTBOOL              m_bLeftFoot;                    // Left foot down?
		ModelId				m_eModelId;						// Id of the model used by this character
		ModelSkeleton		m_eModelSkeleton;				// Skeleton of the model used by this character
        HSTRING             m_hstrSpawnItem;                // Object to spawn when dead

        uint8				m_byFXFlags;					// Our CharacterFX fx flags (zZZ, flashlight, etc)

        LTFLOAT				m_fLastPainTime;				// Last time we were hurt
        LTFLOAT				m_fLastPainVolume;				// Last volume of pain we made

		CharFireInfo		m_LastFireInfo;					// Info about last fired shot

        LTFLOAT				m_fDefaultHitPts;				// Default hit pts
        LTFLOAT				m_fDefaultArmor;				// Default armor
		LTFLOAT				m_fDefaultEnergy;				// Default energy
        LTFLOAT				m_fSoundRadius;					// Radius for sounds we play
		SoundPriority		m_eSoundPriority;				// Sound priority used when sound played

		CMusicMgr::Mood		m_eMusicMoodMin;				// Minimum music intensity this character requests.
		CMusicMgr::Mood		m_eMusicMoodMax;				// Maximum music intensity this character requests.

        LTFLOAT				m_fBaseMoveAccel;				// The base (starting) move acceleration
        LTFLOAT				m_fLadderVel;					// How fast we swim
        LTFLOAT				m_fRunVel;						// How fast we run
        LTFLOAT				m_fWalkVel;						// How fast we walk
        LTFLOAT				m_fSwimVel;						// How fast we swim
        LTFLOAT				m_fJumpVel;						// How fast we jump
		LTFLOAT				m_fSuperJumpVel;				// How fast we super-jump
        LTFLOAT				m_fFallVel;						// How fast we fall
        LTBOOL				m_bUsingHitDetection;			// Using hit detection

		CObjectRelationMgr*	m_pRelationMgr;					// Who am I anyway... 
		
		CharacterClass		m_ccCrosshair;					// How do I show up in crosshairs?
        uint32              m_dwFlags;                      // Initial flags

		AIInformationVolume* m_pLastInformationVolume;		// last volume I was in (often same as current)
		AIInformationVolume* m_pCurrentInformationVolume;	// volume I'm in right now
        LTVector			m_vLastInformationVolumePos;

		AIVolume*			m_pLastVolume;					// last volume I was in (often same as current)
		AIVolume*			m_pCurrentVolume;				// volume I'm in right now
        LTVector			m_vLastVolumePos;

		LTObjRef			m_hHitBox;						// Used to calculate weapon impacts

		LTBOOL				m_bShortRecoiling;				// Are we doing a short recoil?

		uint32				m_cSpears;						// How many spears do we have stuck in us
		SPEARSTRUCT			m_aSpears[kMaxSpears];			// Array of spear HOBJECTs

		EnumAIStimulusID	m_eEnemyVisibleStimID;			// Registration ID of visibility stimulus.
															// Saved in case we want to toggle visibility.

		EnumAIStimulusID	m_eUndeterminedVisibleStimID;	// Registration ID of visibility stimulus.
															// Saved in case we want to toggle visibility.

		DamageFlags			m_nDamageFlags;					// What types of damage (progressive) are cureently affecting us
		DamageFlags			m_nInstantDamageFlags;			// What types of damage (instant) are curently affecting us...

		// List of key items the player currently has
		IDList				m_Keys;

	// NOTE:  The following data members do not need to be saved / loaded
	// when saving games.  Any data members that don't need to be saved
	// should be added here (to keep them together)...

        HLTSOUND            m_hCurDlgSnd;					// Handle to current dialog sound
		CharacterSoundType	m_eCurDlgSndType;				// Type of sound playing
        char*               m_pHandName;                    // Name of hand node used to hold gun

		CDestructible		m_damage;						// Handle damage/healing
        CAttachments*       m_pAttachments;                 // Our attachments
		CEditable			m_editable;						// Handle editting

		HSTRING				m_hstrHeadExtension;			// The extension of the texture for our head skin
        uint8               m_nExtraChildModels;            // number of extra childmodels.
		HSTRING				m_hstrExtraChildModels[MAX_CHILD_MODELS];		// the child models as files. 
		

        LTBOOL              m_bPlayingTextDialogue;			// Are we displaying text

        LTFLOAT             m_fMoveMultiplier;
        LTFLOAT             m_fJumpMultiplier;

		CAnimator*			m_pAnimator;					// Our animator

		LTBOOL				m_bBlink;						// Do we blink?
        ANIMTRACKERID		m_BlinkAnimTracker;				// Our blinking animation tracker

		LTBOOL				m_bShortRecoil;					// Do we do short recoils?
        ANIMTRACKERID		m_RecoilAnimTracker;			// Our recoil anim tracker
 
		HMODELWEIGHTSET		m_hBlinkWeightset;				// The blink weightset
		HMODELWEIGHTSET		m_hTwitchWeightset;				// The twitch weightset
		HMODELWEIGHTSET		m_hNullWeightset;				// The null weightset

		LTBOOL				m_bInitializedAnimation;		// Did we initialize our animation stuff yet?

		LTBOOL				m_bWallStick;					// Should we wall stick when we die?
		LTBOOL				m_bStuckInFront;				// Were we stuck in the front or back?

		LTBOOL				m_bArmored;						// Do we have armor on?

		int32				m_cActive;						// Activation count
		static int32		sm_cAISnds;

		CSearchable*		m_pSearch;

		uint8				m_iPermissionSet;
		
		float				m_fPitch;						// Pitch we should apply to the nodes
		float				m_fLastPitch;					// How much the last pitch sent to the clients was.

		GEN_INVENTORY_LIST	m_lstInventory;					// General inventory items (see GeneralInventory.h)

		CTimer				m_tmrDeflecting;				// On while deflecting ammo.

		// Was sent trigger to teleport.
		enum TeleportTriggerStates
		{
			eTeleporTriggerStateNone,
			eTeleporTriggerStateVector,
			eTeleporTriggerStatePoint,
		};

		TeleportTriggerStates	m_eTeleportTriggerState;
		LTVector			m_vTeleportPos;
		LTObjRef			m_hTeleportPoint;

		DamageType			m_eDeathDamageType;

		float				m_fBodyLifetime;

	private :

		void	InitialUpdate(int nInfo);
		void	Update();
		
        LTBOOL  ReadProp(ObjectCreateStruct *pStruct);
		void	HandleModelString(ArgList* pArgList);
		void	Save(ILTMessage_Write *pMsg);
		void	Load(ILTMessage_Read *pMsg);

		void	UpdateContainerCode();
		void	UpdateCheatInfo();

		void	InitAnimation();

		virtual HATTACHMENT GetAttachment() const { return NULL; }

		// The unique id to identify dialogue sounds
		// told to the client.  This is used to make sure multiple
		// clients don't kill valid dialogues.  It's ok that it wraps,
		// since we won't have that many going within a short period of time.
		// Not saved.
		uint8	m_nUniqueDialogueId;

		// Tracks the players that were told about dialogue.  We have
		// to wait until everyone says they're done.
		PlayerTracker	m_PlayerTrackerDialogue;
		CTimer			m_tmrDialogue;

		// Indicates character is tracked by a tracking dart
		bool	m_bTracking;

		// Indicates character is visible on radar.
		bool	m_bRadarVisible;

		// Indicates what character is carrying
		uint8	m_nCarrying;

};

class CCharacterPlugin : public IObjectPlugin
{
	public:
       virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);
};

#endif // __CHARACTER_H__
