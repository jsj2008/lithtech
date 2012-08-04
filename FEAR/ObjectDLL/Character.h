// ----------------------------------------------------------------------- //
//
// MODULE  : Character.h
//
// PURPOSE : Base class for player and AI
//
// CREATED : 10/6/97
//
// (c) 1997-2004 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __CHARACTER_H__
#define __CHARACTER_H__

#include "GameBase.h"
#include "ServerUtilities.h"
#include "Destructible.h"
#include "SoundTypes.h"
#include "ContainerCodes.h"
#include "SharedMovement.h"
#include "SFXMsgIds.h"
#include "ltobjref.h"
#include "IDList.h"
#include "IHitBoxUser.h"
#include "PlayerTracker.h"
#include "Arsenal.h"
#include "EngineTimer.h"
#include "CharacterAlignment.h"
#include "AIEnumStateTypes.h"
#include "AIEnumStimulusTypes.h"
#include "AIEnumNavMeshTypes.h"
#include "AIEnumNavMeshLinkTypes.h"
#include "AIRegion.h"
#include "DamageTracker.h"
#include "PhysicsUtilities.h"
#include "iltphysicssim.h" // for HPHYSICSRIGIDBODY
#include "CharacterPhysics.h"
#include "SharedFXStructs.h"
#include "ServerMeleeCollisionController.h"

LINKTO_MODULE( Character );


#define BC_DEFAULT_SOUND_RADIUS	 1500.0f

class CAttachments;
class CAnimator;
class TeleportPoint;
struct CHARCREATESTRUCT;
struct AIDB_AttributesRecord;


// Other defines...

#define MAX_TIMED_POWERUPS				8


class  VolumeBrush;

enum CharacterDeath { CD_NORMAL=0, CD_GIB, CD_FREEZE, CD_VAPORIZE, CD_BURST };

// Was sent trigger to teleport.
enum TeleportTriggerStates
{
	eTeleporTriggerStateNone,
	eTeleporTriggerStateVector,
	eTeleporTriggerStatePoint,
};


class CCharacter : public GameBase, public IHitBoxUser, public IPlayerTrackerReceiver
{
	public :
		DEFINE_CAST( CCharacter );

		CCharacter();
		~CCharacter();

		virtual bool	IsDead() const { return m_damage.IsDead(); }
		virtual bool	IsAlive() const { return !m_damage.IsDead(); }

		virtual bool	IsCrouched() {return false;}
		virtual bool	IsKnockedDown() {return false;}

		ContainerCode	GetContainerCode()	const { return m_eContainerCode; }

		// GameBase Overrides...

		virtual void AddToObjectList( ObjectList *pObjList, eObjListControl eControl = eObjListNODuplicates );

		// damage filtering
		virtual bool        FilterDamage( DamageStruct *pDamageStruct );

		// SFX

		void SendStealthToClients();
		void SendDamageFlagsToClients();
		void SendAllFXToClients() { CreateSpecialFX( true ); }
		void SendWeaponRecordToClients( );

		void CreateFlashLight();
		void DestroyFlashLight();
		void UpdateFlashLight();
		bool IsFlashlightOn();

		// Attachments
		virtual void	CreateAttachments() { ASSERT(false); }
        void            DestroyAttachments();
		CAttachments*	GetAttachments() { return m_pAttachments; }
		void			HideAttachments(bool bHide);
		virtual CAttachments*	TransferAttachments( bool bRemove ); // Responsibility of call to delete attachments
		virtual	void	RemoveAttachments( bool bDestroyAttachments );
		virtual void	RemoveWeapon( CActiveWeapon* );

		virtual void	AttachToObject( HOBJECT hObject ) {}
		virtual void	DetachFromObject( HOBJECT hObject ) {}

		bool			AddSpear(HOBJECT hSpear, ModelsDB::HNODE hModelNode, const LTRotation& rRot, bool bCanWallStick );

		virtual void	AddRemoteCharge(HOBJECT hRemote);
		virtual bool	AttachRemoteCharge(HOBJECT hRemote, ModelsDB::HNODE hModelNode);
		virtual void	DetonateRemoteCharges();
		virtual void	RemoveRemoteCharges();


		void SetOnGround(bool bOn) { m_bOnGround = bOn; }
		bool IsOnGround() const { return m_bOnGround; }

		virtual float ComputeDamageModifier(ModelsDB::HNODE hModelNode);

		CDestructible*	GetDestructible() { return &m_damage; }

		// Tell client about crosshair alignment change.
		void			ResetCrosshair( EnumCharacterStance ccCrosshair );
		void			SetAlignment( EnumCharacterAlignment eAlignment ) { m_eAlignment = eAlignment; }
		EnumCharacterAlignment GetAlignment() const { return m_eAlignment; }

		void			RegisterPersistentStimuli(void);
		void			RemovePersistentStimuli(void);

		HOBJECT			GetHitBox()			const { return m_hHitBox; }

//		void			SetOnLadder(bool bOnLadder);
		void			SetLadderObject( HOBJECT hLadder );
		void			SetSpecialMove(bool bSpecialMove);
		void			UpdateInLiquid(VolumeBrush* pBrush, ContainerPhysics* pCPStruct);

		CharacterDeath	GetDeathType() const { return m_eDeathType; }
		virtual void	SetModel( ModelsDB::HMODEL hModel ) { m_hModel = hModel; }
		ModelsDB::HMODEL GetModel()   const { return m_hModel; }
		virtual ModelsDB::HSKELETON	GetModelSkeleton()   const { return m_hModelSkeleton; }
		virtual void	ResetModel( );
		virtual void	UpdateSurfaceFlags();


		void			SetModelNodeLastHit( ModelsDB::HNODE hModelNodeLastHit)	{ m_hModelNodeLastHit = hModelNodeLastHit; }
		ModelsDB::HNODE GetModelNodeLastHit()	const	{ return m_hModelNodeLastHit; }
		virtual float GetNodeRadius( ModelsDB::HNODE hModelNode ) { return ( g_pModelsDB->GetNodeRadius( hModelNode )); }

		ENUM_NMPolyID	GetLastNavMeshPoly() const { return m_eLastNavMeshPoly; }
		bool			HasLastNavMeshPoly() const;
		const LTVector& GetLastNavMeshPos()	const { return m_vLastNavMeshPos; }

		ENUM_NMPolyID	GetCurrentNavMeshPoly() const { return m_eCurrentNavMeshPoly; }
		ENUM_NMLinkID	GetCurrentNavMeshLink() const { return m_eCurrentNavMeshLink; }
		bool			HasCurrentNavMeshPoly()	const;
		bool			HasCurrentNavMeshLink()	const;

		TeleportTriggerStates GetTeleportTriggerState() const { return m_eTeleportTriggerState; }

		double			GetLastCoolMoveTime() const { return m_fLastCoolMoveTime; }

		bool			HitFromFront(const LTVector& vDir);

		void			AddAggregate(LPAGGREGATE pAggregate) { GameBase::AddAggregate(pAggregate); }

		virtual bool	CanLipSync() { return true; }
		virtual bool	DoDialogueSubtitles() { return false; }

		virtual void    PlayDialogSound(const char* pSound, CharacterSoundType eType=CST_DIALOG, const char*szIcon = NULL, bool bUseRadioVoice = false );
		bool			IsPlayingDialogSound() const { return m_eCurDlgSndType != CST_NONE; }
		bool			IsPlayingDialogue() { return (m_bPlayingTextDialogue || IsPlayingDialogSound()); }
		static int32	GetAISoundCount() { return sm_cAISnds; }

		virtual void	KillDlgSnd();
		void			KillDialogueSound() { KillDlgSnd(); }

		bool			PlayDialogue( char *szDialogue, const char*szIcon = NULL, bool bUseRadioVoice = false );

		virtual void	StopDialogue(bool bCinematicDone = false);


		virtual bool	SetDims(LTVector* pvDims, bool bSetLargest=true);
		virtual float	GetRadius();

		virtual void	GetViewTransform( LTRigidTransform &tfView ) const;

		bool			IsVisible(void);

		virtual void	HideCharacter(bool bHide);
		virtual void	HandleCharacterRemoval();

		inline	DamageFlags	GetDamageFlags( ) const { return m_nDamageFlags; }
		void			SetDamageFlags( const DamageFlags nDmgFlags );
		void			SetInstantDamageFlags( const DamageFlags nDmgFlags );
		void			HandleShortRecoil();

		void			SetLastPainVolume( float fVolume ) { m_fLastPainVolume = fVolume; }
		float			GetSoundOuterRadius() { return m_fSoundOuterRadius; }

		virtual void	PushCharacter(const LTVector &vPos, float fRadius, float fStartDelay, float fDuration, float fStrength);

		virtual void	SetMoveToFloor( bool bValue );
		bool			GetMoveToFloor( ) const { return m_bMoveToFloor; }

		// Key item handling
		IDList*			GetKeyList() { return &m_Keys; }

		// Implementing classes will have this function called
		// when HOBJECT ref points to gets deleted.
		virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );

		DamageType		GetDeathDamageType() const { return m_eDeathDamageType; }

		//
		// Defense
		//

		// true means the character is performing some type of defense
		virtual bool	IsDefending() const { return false; }

		virtual CArsenal*	GetArsenal()	{ return &m_Arsenal; }

		virtual float	GetStealthModifier() { return 1.0f;	}

		virtual bool	IsSpectating( ) const { return false; }



		// Get the complete list of all CCharacter's.
		typedef std::vector< CCharacter* > CharacterList;
		static CharacterList const& GetCharacterList( ) { return m_lstCharacters; }
		static CharacterList const& GetBodyList( ) { return m_lstBodies; }

		bool		GetBlockWindowOpen() const { return m_flBlockWindowEndTime > g_pLTServer->GetTime(); }
		bool		GetDodgeWindowOpen() const { return m_flDodgeWindowEndTime > g_pLTServer->GetTime(); }


		//Does character prefer WeaponA to WeaponB.
		virtual bool IsPreferredWeapon( HWEAPON hWeaponA, HWEAPON hWeaponB ) const;

		static void ClearDeathCaps() { sm_nGibCounter = 0; sm_nSeverCounter = 0; };
	

		// Attributes

		const AIDB_AttributesRecord* const GetAIAttributes() const;
		virtual uint32	GetCharTypeMask() const;

		// Drops all weapons of the character...
		virtual void DropWeapons() { ASSERT(false); }

		// Drops gear items when the character dies... 
		virtual void SpawnGearItemsOnDeath( ) {};

		bool IsRagdolling(){ return m_bIsRagdolling; }

		bool IsSolidToAI(){ return m_bIsSolidToAI; }
		void SetSolidToAI( bool bSolid ) { m_bIsSolidToAI = bSolid; }

		void SetDeathAnimation( bool bClampRagdollVelocity );

		EnumAIStimulusID	GetEnemyVisibleStimulusID();

		// Rigid body melee event response.

		virtual void HandleMeleeBlocked();
		virtual void HandleMeleeBlockedAttacker();

		virtual void ForceGib();

		bool IsOnLadder( ) const { return ( m_hLadderObject != INVALID_HOBJECT ); }

		// The teamid the player is on.
		virtual uint8	GetTeamID( ) const { return INVALID_TEAM; }

	protected :

		friend class CCharacterHitBox;
		friend class CAttachments;

		virtual void	UpdateAnimation();

        virtual LTVector GetHeadOffset();

		uint32			EngineMessageFn(uint32 messageID, void *pData, float fData);
        uint32          ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);

		virtual void	ResetAfterDeath();

		void			UpdateNavMeshPosition();
		virtual void	UpdateOnGround();
		virtual void	UpdateMovement(bool bUpdatePhysics=true);
		virtual void	UpdateSounds();
		virtual void	UpdateDead( bool bCanBeRemoved );
		void			UpdateFireWeaponDuringDeath();

		virtual HMODELANIM	GetDeathAni(bool bFront);
		virtual	HMODELANIM	GetAlternateDeathAnimation() { return INVALID_ANI; }

		virtual float	GetFootstepVolume() { return 1.0f; }

		virtual const char*	GetDamageSound(DamageType eType) { LTUNREFERENCED_PARAMETER( eType ); return NULL; }

		//death related functions
		virtual const char*	GetDeathSound() { return NULL; }
		virtual void	PlayDamageSound(DamageType eType);
        virtual void    PlayDeathSound();
		virtual void	HandleDead();
		virtual void	HandleGib();
		virtual void	PrepareToSever();
		virtual void	HandleSever(ModelsDB::HSEVERPIECE hPiece);
		virtual bool	HandleDeathFX();


		// Keeps body density down within a radius.
		void CapNumberOfBodies( );
		void StartFade();

		bool SetAIAttributes(const char* pszName);

		// Last NavMesh poly* tracking information

		virtual void	HandleNavMeshPolyEnter( ENUM_NMPolyID ePoly );
		virtual void	HandleNavMeshPolyExit( ENUM_NMPolyID ePoly ); 
		virtual void	HandleAIRegionEnter( ENUM_AIRegionID eAIRegion ) {}
		virtual void	HandleAIRegionExit( ENUM_AIRegionID eAIRegion ) {}
		virtual float	GetVerticalThreshold() const;
		
		void			FireWeaponDuringDeath( bool bFire ) { m_bFireWeaponDuringDeath = bFire; }
		virtual void	StartDeath();

		void			UpdateTeleport( );
		virtual void	HandleTeleport(const LTVector& vPos) { LTUNREFERENCED_PARAMETER( vPos ); };
		virtual void	HandleTeleport(TeleportPoint* pTeleportPoint) { LTUNREFERENCED_PARAMETER( pTeleportPoint ); }

		virtual void	ProcessDamageMsg( DamageStruct &rDamage );

		virtual void	PlaySound(const char *pSoundName, float fRadius=BC_DEFAULT_SOUND_RADIUS, bool bAttached=true);
		virtual void	PlayDBSound(const char *pSoundRecordName, float fRadius=BC_DEFAULT_SOUND_RADIUS, bool bAttached=true);
		virtual void	PlayDBSound( HRECORD hSoundRec, float fRadius=BC_DEFAULT_SOUND_RADIUS, bool bAttached=true);

        virtual LTVector HandHeldWeaponFirePos(CWeapon* pWeapon);

		virtual void	CreateHitBox();
		virtual void	UpdateHitBox();
		virtual void	UpdateClientHitBox();
		
		virtual void	CreateSpecialFX(bool bUpdateClients=false);
		virtual void	PreCreateSpecialFX(CHARCREATESTRUCT& cs ) { cs.hServerObj = m_hObject; }

		// Get the User-Flag version of our surface type
		virtual uint32	GetUserFlagSurfaceType() const;

		// Get the CollisionProperty.
		HRECORD			GetCollisionPropertyRecord( ) const;

		// Check if we should be doing lipsyncing on the dialog.
		bool			DoLipSyncingOnDlgSound( HOBJECT hSpeaker );

		// Handles when all players have broken their links.
		virtual void	OnPlayerTrackerAbort( );

		void			HandleSfxMessage( HOBJECT hSender, ILTMessage_Read *pMsg );

		// Initialize the tracker used for playing recoil animations...
		void AddRecoilTracker( );
		
		// Remove the recoil tracker.  This is needed to ensure the recoil tracker is 
		// always the last tracker added...
		void RemoveRecoilTracker( );

		// After loading a saved game activate the ladder if the character has one
		virtual void ActivateLadderOnLoad( ) { };


	protected :

		enum Constants
		{
			kMaxSpears = 16,
			kMaxProps = 32,
		};

		struct SPEARSTRUCT
		{
			LTObjRefNotifier	hObject;
			ModelsDB::HNODE		hModelNode;
            LTRotation			rRot;

			SPEARSTRUCT()
			{
				hObject = NULL;
				hModelNode = NULL;
				rRot.Init();
			}
		};


	protected :
		CHARCREATESTRUCT	m_cs;							// Our character specialfx struct

		SurfaceType			m_eStandingOnSurface;			// What surface we're currently standing on.
		SurfaceType			m_eContainerSurface;			// The surface of any container the character is within
        bool				m_bMakeBody;					// Make body when dead.
		bool				m_bPermanentBody;				// Dead body is permanent.
		bool				m_bIsOnBodiesList;				// Added to bodies list.
		bool				m_bIsOnSeveredList;				// Added to severed list.
        bool				m_bMoveToFloor;					// Move character to floor
		CharacterDeath		m_eDeathType;					// How did we die
		bool				m_bStartedDeath;				// Did I start death ani?
		bool				m_bOnGround;					// Are we on the ground
		ContainerCode		m_eContainerCode;				// Code of container our Head is in (if any)
		bool				m_bBodyInLiquid;				// Is our body in liquid
		bool				m_bBodyWasInLiquid;				// Was our body in liquid on the last frame
		bool				m_bBodySpecialMove;				// Is our body performing a special move (like climbing on a box)
		ModelsDB::HNODE		m_hModelNodeLastHit;			// The model node that was last hit
		ModelsDB::HMODEL	m_hModel;						// Id of the model used by this character
		ModelsDB::HSKELETON	m_hModelSkeleton;				// Skeleton of the model used by this character
		std::string			m_sSpawnItem;					// Object to spawn when dead
		bool				m_bIsSolidToAI;					// Character is solid to AIs for pathing

        uint8				m_byFXFlags;					// Our CharacterFX fx flags (zZZ, flashlight, etc)

		float				m_fLastPainVolume;				// Last volume of pain we made

		float				m_fSoundOuterRadius;			// Radius for sounds we play
		float				m_fSoundInnerRadius;

		SoundPriority		m_eSoundPriority;				// Sound priority used when sound played

		float				m_fBaseMoveAccel;				// The base (starting) move acceleration
		float				m_fLadderVel;					// How fast we swim
		float				m_fRunVel;						// How fast we run
		float				m_fWalkVel;						// How fast we walk
		float				m_fSwimVel;						// How fast we swim
		float				m_fJumpVel;						// How fast we jump
		float				m_fFallVel;						// How fast we fall
		float				m_fCrawlVel;					// How fast we crawl

		const AIDB_AttributesRecord* m_pAIAttributes;		// AI Attributes

		EnumCharacterAlignment	m_eAlignment;
		EnumCharacterStance		m_ccCrosshair;					// How do I show up in crosshairs?
        uint32              m_dwFlags;                      // Initial flags

		ENUM_NMPolyID		m_eLastNavMeshPoly;				// last NavMesh poly I was in (often same as current)
		ENUM_NMPolyID		m_eCurrentNavMeshPoly;			// NavMesh poly I'm in right now
		ENUM_NMLinkID		m_eCurrentNavMeshLink;			// NavMesh link associated with poly I'm in right now.
		LTVector			m_vLastNavMeshPos;

		double				m_fLastCoolMoveTime;			// Last time character played a cool fighting move.

		LTObjRef			m_hHitBox;						// Used to calculate weapon impacts

		bool				m_bShortRecoiling;				// Are we doing a short recoil?

		uint32				m_cSpears;						// How many spears do we have stuck in us
		SPEARSTRUCT			m_aSpears[kMaxSpears];			// Array of spear HOBJECTs

		EnumAIStimulusID	m_eEnemyVisibleStimID;			// Registration ID of visibility stimulus.
															// Saved in case we want to toggle visibility.

		EnumAIStimulusID	m_eFlashlightStimID;			// Registration ID of flashlight beam visible stimulus.
															// Saved to toggle visibility when the flashlight is on/off.
		double				m_fNextFlashlightBeamPosUpdateTime;	// Next time to update the flashlight stimulus pos.

		DamageFlags			m_nDamageFlags;					// What types of damage (progressive) are cureently affecting us
		DamageFlags			m_nInstantDamageFlags;			// What types of damage (instant) are curently affecting us...

		DamageTracker		m_DamageTracker;				// helper class to track what kinds of damage we receive

		// List of key items the player currently has
		IDList				m_Keys;

		std::string			m_sPhysicsWeightSet;			// Name of physics weight set to change to

		bool				m_bSevered;						//has this body lost parts?
		bool				m_bDecapitated;					//has this body lost its head?
		bool				m_bGibbed;						//has this been destroyed?
		bool				m_bDeathEffect;					//has this had a death effect applied?

		ObjRefNotifierList m_ActiveRemoteCharges;					//remote charges that we own
		ObjRefNotifierList m_AttachedRemoteCharges;				//remote charges that are stuck to us

	// NOTE:  The following data members do not need to be saved / loaded
	// when saving games.  Any data members that don't need to be saved
	// should be added here (to keep them together)...

		CharacterSoundType	m_eCurDlgSndType;				// Type of sound playing

		CDestructible		m_damage;						// Handle damage/healing
        CAttachments*       m_pAttachments;                 // Our attachments

		bool				m_bPlayingTextDialogue;			// Are we displaying text
		
		float				m_fMoveMultiplier;
		float				m_fJumpMultiplier;

		bool				m_bShortRecoil;					// Do we do short recoils?
        ANIMTRACKERID		m_RecoilAnimTracker;			// Our recoil anim tracker
 
		HMODELWEIGHTSET		m_hTwitchWeightset;				// The twitch weightset
		HMODELWEIGHTSET		m_hNullWeightset;				// The null weightset

		ANIMTRACKERID		m_BlendAnimTracker;				// Our blend anim tracker.

		bool				m_bInitializedAnimation;		// Did we initialize our animation stuff yet?

		int32				m_cActive;						// Activation count
		static int32		sm_cAISnds;

		static uint32		sm_nGibCounter;					// How many more kills before we're allowed to Gib
		static uint32		sm_nSeverCounter;				// How many more kills before we're allowed to sever

		TeleportTriggerStates	m_eTeleportTriggerState;
		LTVector			m_vTeleportPos;
		LTObjRef			m_hTeleportPoint;

		DamageType			m_eDeathDamageType;


		// Each character holds their own arsenal...
		CArsenal			m_Arsenal;

		// The character needs to know which tracker actually emits footsteps...
		ANIMTRACKERID		m_nFootstepTrackerId;

		double				m_flBlockWindowEndTime;
		double				m_flDodgeWindowEndTime;

		// last hit box dims and offset that we sent to the client
		LTVector			m_vLastHitBoxDims;
		LTVector			m_vLastHitBoxOffset;

		// Melee support...
		CServerMeleeCollisionController	m_MeleeCollisionController;

		// Ladder the Character is currently climbing.  Invalid if not climbing.
		LTObjRef			m_hLadderObject;

		// Ladder object to reset to loading a saved game.
		// We can't use the above object since that gets cleared after loading a saved game due 
		// to how the client handles ladders.
		LTObjRef			m_hLoadLadderObject;

		// Helper function
		void SetPhysicsWeightSet(const char* pWeightSet);
		virtual void	InitAnimation();


	private :

		void	InitialUpdate(int nInfo);
		void	Update();
		
		bool	ReadProp(const GenericPropList *pProps);
		void	HandleModelString( ArgList* pArgList, ANIMTRACKERID nTrackerId );
		void	Save(ILTMessage_Write *pMsg);
		void	Load(ILTMessage_Read *pMsg);

		void	UpdateContainerCode();
		void	UpdateCheatInfo();

		void	RagdollDeath( bool bClampVelocity );

		// The unique id to identify dialogue sounds
		// told to the client.  This is used to make sure multiple
		// clients don't kill valid dialogues.  It's ok that it wraps,
		// since we won't have that many going within a short period of time.
		// Not saved.
		uint8	m_nUniqueDialogueId;

		// Tracks the players that were told about dialogue.  We have
		// to wait until everyone says they're done.
		PlayerTracker	m_PlayerTrackerDialogue;
		StopWatchTimer	m_tmrDialogue;


		//death/body related members
		bool			m_bDelayRagdollDeath;
		bool			m_bFirstDeathUpdate;
		bool			m_bIsRagdolling;
		float			m_fBodyLifetime;
		StopWatchTimer	m_tmrLifetime;
		StopWatchTimer	m_tmrBodyFade;
		EnumAIStimulusID m_eDeathStimID;	// Registration ID of dead body stimulus.
		bool			m_bFireWeaponDuringDeath;
		float			m_fDropWeaponDuringDeathHeight;

		// Ragdoll damage callback parms

		enum RagdollCallbackConstants
		{
			kMaxRagdollCallbacks = 11,
		};

		struct RAGDOLLCALLBACKSTRUCT
		{
			float fVelocityThresold;
			int32 nNumImpacts;
			HOBJECT hObject;
			HMODELNODE hNode;
			HRECORD hDecalType;
			HPHYSICSCOLLISIONNOTIFIER hCollisionNotifier;
		};

		RAGDOLLCALLBACKSTRUCT m_aRagdollCollisionParms[kMaxRagdollCallbacks];

		static void RagdollCollisionNotifier(	HPHYSICSRIGIDBODY hBody1, HPHYSICSRIGIDBODY hBody2,
												const LTVector& vCollisionPt, const LTVector& vCollisionNormal,
												float fVelocity, bool& bIgnoreCollision, void* pUser );


		void AddRagdollCollisionNotifications();
		void RemoveRagdollCollisionNotifications();

		CCharacterPhysics		m_CharacterPhysics;

		static CharacterList	m_lstCharacters;
		static CharacterList	m_lstBodies;
		static CharacterList	m_lstSeverBodies;

		// Message Handlers...

		DECLARE_MSG_HANDLER( CCharacter, HandlePlaySoundMsg );
		DECLARE_MSG_HANDLER( CCharacter, HandleTeleportMsg );
		DECLARE_MSG_HANDLER( CCharacter, HandleAttachMsg );
		DECLARE_MSG_HANDLER( CCharacter, HandleDetachMsg );
		DECLARE_MSG_HANDLER( CCharacter, HandleCanDamageMsg );
		DECLARE_MSG_HANDLER( CCharacter, HandleCrosshairMsg );
		DECLARE_MSG_HANDLER( CCharacter, HandleHiddenMsg );
		DECLARE_MSG_HANDLER( CCharacter, HandleFindMsg );
		DECLARE_MSG_HANDLER( CCharacter, HandleRemoveMsg );
		DECLARE_MSG_HANDLER( CCharacter, HandleMoveToFloorMsg );
		DECLARE_MSG_HANDLER( CCharacter, HandleDetonatorMsg );
		DECLARE_MSG_HANDLER( CCharacter, HandleFXMsg );
		DECLARE_MSG_HANDLER( CCharacter, HandleShowAttachFXMsg );
		DECLARE_MSG_HANDLER( CCharacter, HandleHideAttachFXMsg );
};

#endif // __CHARACTER_H__


// EOF
