// ----------------------------------------------------------------------- //
//
// MODULE  : Character.h
//
// PURPOSE : Base class for player and AI
//
// CREATED : 10/6/97
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
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
#include "CharacterAlignment.h"
#include "ContainerCodes.h"
#include "GibTypes.h"
#include "ModelButeMgr.h"
#include "SharedMovement.h"
#include "Editable.h"
#include "SFXMsgIDs.h"
#include "SharedFXStructs.h"

#define INVALID_ANI				((HMODELANIM)-1)

#define BC_DEFAULT_SOUND_RADIUS	 1500.0f

class Body;
class CinematicTrigger;
class CAttachments;
class CAnimator;
class CAIVolume;
class TeleportPoint;

struct CharMoveInfo
{
    LTFLOAT          fTime;
    LTFLOAT          fVolume;
	SurfaceType		eSurfaceType;

	CharMoveInfo() { Clear(); }
	void Clear();

	void Save(HMESSAGEWRITE hWrite);
	void Load(HMESSAGEREAD hRead);
};

inline void CharMoveInfo::Clear()
{
	fTime = (float)-INT_MAX;
	fVolume = 0.0f;
	eSurfaceType = ST_UNKNOWN;
}

inline void CharMoveInfo::Save(HMESSAGEWRITE hWrite)
{
    if (!g_pLTServer || !hWrite) return;

    g_pLTServer->WriteToMessageFloat(hWrite, fTime);
    g_pLTServer->WriteToMessageFloat(hWrite, fVolume);
    g_pLTServer->WriteToMessageDWord(hWrite, eSurfaceType);
}

inline void CharMoveInfo::Load(HMESSAGEREAD hRead)
{
    if (!g_pLTServer || !hRead) return;

    fTime           = g_pLTServer->ReadFromMessageFloat(hRead);
    fVolume         = g_pLTServer->ReadFromMessageFloat(hRead);
    eSurfaceType    = (SurfaceType)g_pLTServer->ReadFromMessageDWord(hRead);
}

struct CharCoinInfo
{
    LTFLOAT          fTime;
    LTFLOAT          fVolume;
	SurfaceType		eSurfaceType;
    LTVector        vPosition;

	CharCoinInfo() { Clear(); }
	void Clear();

	void Save(HMESSAGEWRITE hWrite);
	void Load(HMESSAGEREAD hRead);
};

inline void CharCoinInfo::Clear()
{
	fTime = (float)-INT_MAX;
	fVolume = 0.0f;
	eSurfaceType = ST_UNKNOWN;
}

inline void CharCoinInfo::Save(HMESSAGEWRITE hWrite)
{
    if (!g_pLTServer || !hWrite) return;

	SAVE_FLOAT(fTime);
	SAVE_FLOAT(fVolume);
	SAVE_DWORD(eSurfaceType);
    SAVE_VECTOR(vPosition);
}

inline void CharCoinInfo::Load(HMESSAGEREAD hRead)
{
    if (!g_pLTServer || !hRead) return;

	LOAD_FLOAT(fTime);
	LOAD_FLOAT(fVolume);
	LOAD_DWORD_CAST(eSurfaceType, SurfaceType);
    LOAD_VECTOR(vPosition);
}

struct CharFootprintInfo
{
    LTVector     vPos;
    LTFLOAT      fDuration;
	SurfaceType	eSurface;
    LTFLOAT      fTimeStamp;

	CharFootprintInfo() { Clear(); }
	void Clear();

	void Save(HMESSAGEWRITE hWrite);
	void Load(HMESSAGEREAD hRead);
};

inline void CharFootprintInfo::Clear()
{
    vPos = LTVector(0,0,0);
	fDuration = 0.0f;
	eSurface = ST_UNKNOWN;
    fTimeStamp = (float)-INT_MAX;
}

inline void CharFootprintInfo::Save(HMESSAGEWRITE hWrite)
{
	SAVE_VECTOR(vPos);
	SAVE_FLOAT(fDuration);
	SAVE_DWORD(eSurface);
    SAVE_FLOAT(fTimeStamp);
}

inline void CharFootprintInfo::Load(HMESSAGEREAD hRead)
{
	LOAD_VECTOR(vPos);
	LOAD_FLOAT(fDuration);
	LOAD_DWORD_CAST(eSurface, SurfaceType);
    LOAD_FLOAT(fTimeStamp);
}

struct CharFireInfo
{
	HOBJECT		hObject;
    LTVector    vFiredPos;
    LTVector    vImpactPos;
    uint8       nWeaponId;
    uint8       nAmmoId;
    LTFLOAT     fTime;
    LTBOOL      bSilenced;
	SurfaceType	eSurface;

	CharFireInfo() { Clear(); }
	void Clear();

	void Save(HMESSAGEWRITE hWrite);
	void Load(HMESSAGEREAD hRead);
};

inline void CharFireInfo::Clear()
{
	memset(this, 0, sizeof(CharFireInfo));
	nWeaponId = WMGR_INVALID_ID;
    fTime = (float)-INT_MAX;
}

inline void CharFireInfo::Save(HMESSAGEWRITE hWrite)
{
    if (!g_pLTServer || !hWrite) return;

	SAVE_HOBJECT(hObject);
    g_pLTServer->WriteToMessageVector(hWrite, &vFiredPos);
    g_pLTServer->WriteToMessageVector(hWrite, &vImpactPos);
    g_pLTServer->WriteToMessageByte(hWrite, nWeaponId);
    g_pLTServer->WriteToMessageByte(hWrite, nAmmoId);
    g_pLTServer->WriteToMessageFloat(hWrite, fTime);
    g_pLTServer->WriteToMessageByte(hWrite, bSilenced);
    g_pLTServer->WriteToMessageByte(hWrite, eSurface);
}

inline void CharFireInfo::Load(HMESSAGEREAD hRead)
{
    if (!g_pLTServer || !hRead) return;

	LOAD_HOBJECT(hObject);
    g_pLTServer->ReadFromMessageVector(hRead, &vFiredPos);
    g_pLTServer->ReadFromMessageVector(hRead, &vImpactPos);
    nWeaponId   = g_pLTServer->ReadFromMessageByte(hRead);
    nAmmoId     = g_pLTServer->ReadFromMessageByte(hRead);
    fTime       = g_pLTServer->ReadFromMessageFloat(hRead);
    bSilenced   = (LTBOOL) g_pLTServer->ReadFromMessageByte(hRead);
    eSurface    = (SurfaceType) g_pLTServer->ReadFromMessageByte(hRead);
}



// Other defines...

#define MAX_TIMED_POWERUPS				8


class  VolumeBrush;

enum CharacterDeath { CD_NORMAL=0, CD_GIB, CD_FREEZE, CD_VAPORIZE, CD_BURST };

class CCharacter : public GameBase
{
	public :

		CCharacter();
		~CCharacter();

        LTBOOL          IsDead()            const { return m_damage.IsDead(); }
		ContainerCode	GetContainerCode()	const { return m_eContainerCode; }

		// SFX

		void SendStealthToClients();

		void CreateCigarette(LTBOOL bSmoke);
		void DestroyCigarette();

		void CreateSmokepuffs();
		void DestroySmokepuffs();

		void CreateZzz();
		void DestroyZzz();

		void CreateHearts();
		void DestroyHearts();

		// Attachments
        virtual void    CreateAttachments() { _ASSERT(LTFALSE); }
        void            DestroyAttachments();
		CAttachments*	GetAttachments() { return m_pAttachments; }
		virtual CAttachments*	TransferAttachments(); // Responsibility of call to delete attachments
		HOBJECT			TransferWeapon(HOBJECT hBody);
		void			TransferSpears(Body* pBody);

		void			AddSpear(HOBJECT hSpear, ModelNode eModelNode, const LTRotation& rRot);

        void SetOnGround(LTBOOL bOn) { m_bOnGround = bOn; }
        LTBOOL IsOnGround() const { return m_bOnGround; }

        virtual LTFLOAT ComputeDamageModifier(ModelNode eModelNode);

		CDestructible*	GetDestructible() { return &m_damage; }
		CharacterClass  GetCharacterClass() const { return m_cc; }
		ModelStyle		GetModelStyle()		const { return m_eModelStyle; }

		virtual const char* GetHeadSkinFilename() const { return g_pModelButeMgr->GetHeadSkinFilename(m_eModelId, m_eModelStyle, g_pLTServer->GetStringData(m_hstrHeadExtension)); }

		HOBJECT			GetHitBox()			const { return m_hHitBox; }
		HOBJECT			GetLastDamager()	const { return m_damage.GetLastDamager(); }

		void			SetBlinking(LTBOOL bBlinking);

		LTBOOL			IsShortRecoiling()	const { return m_bShortRecoiling; }

		void			UpdateOnLadder(VolumeBrush* pBrush, ContainerPhysics* pCPStruct);
		void			UpdateInLiquid(VolumeBrush* pBrush, ContainerPhysics* pCPStruct);

		char*			GetHandName() const { return m_pHandName; }

		CharacterDeath	GetDeathType() const { return m_eDeathType; }
		ModelId			GetModelId()   const { return m_eModelId; }
		ModelSkeleton	GetModelSkeleton()   const { return m_eModelSkeleton; }

		void			SetModelNodeLastHit(ModelNode eModelNodeLastHit)	{ m_eModelNodeLastHit = eModelNodeLastHit; }
		ModelNode		GetModelNodeLastHit()	const	{ return m_eModelNodeLastHit; }
        LTBOOL           UsingHitDetection()     const   { return m_bUsingHitDetection; }

		void			SetLastFireInfo(CharFireInfo* pInfo);
		void			GetLastFireInfo(CharFireInfo & info);

		void			SetLastMoveInfo(CharMoveInfo* pInfo);
		void			GetLastMoveInfo(CharMoveInfo & info);

		void			SetLastCoinInfo(CharCoinInfo* pInfo);
		void			GetLastCoinInfo(CharCoinInfo & info);

		CAIVolume*		GetLastVolume();
        LTBOOL           HasLastVolume() { return !!GetLastVolume(); }
        const LTVector&  GetLastVolumePos() { return m_vLastVolumePos; }

		CTList<CharFootprintInfo*>* GetFootprints() { return &m_listFootprints; }

        LTBOOL           HitFromFront(const LTVector& vDir);
        LTBOOL           HitFromBack(const LTVector& vDir) { return !HitFromFront(vDir); }

		virtual LTBOOL	ShouldWallStick();

        LTFLOAT          GetLastPainTime() { return m_fLastPainTime; }
        LTFLOAT          GetLastPainVolume() { return m_fLastPainVolume; }

        LTBOOL           CanDamageBody() const { return m_bCanDamageBody; }

		void			AddAggregate(LPAGGREGATE pAggregate) { GameBase::AddAggregate(pAggregate); }

        LTFLOAT          GetHitPoints() { return m_damage.GetHitPoints(); }
        LTFLOAT          GetMaxHitPoints() { return m_damage.GetMaxHitPoints(); }

		virtual void	RemoveObject();
        virtual void    SpawnItem(char* pItem, LTVector & vPos, LTRotation & rRot);

		enum CharacterSoundType { CST_NONE=0, CST_DAMAGE, CST_DEATH, CST_DIALOG, CST_EXCLAMATION, CST_AI_SOUND };

        virtual LTBOOL  CanLipSync() { return LTTRUE; }
		virtual LTBOOL	DoDialogueSubtitles() { return LTFALSE; }

		virtual void    PlayDialogSound(char* pSound, CharacterSoundType eType=CST_DIALOG);
        LTBOOL          IsPlayingDialogSound() const { return (LTBOOL)!!m_hCurDlgSnd; }
        LTBOOL          IsPlayingDialogue() { return (m_bPlayingTextDialogue || IsPlayingDialogSound()); }

		virtual void	KillDlgSnd();
		void			KillDialogueSound() { KillDlgSnd(); }

        LTBOOL          PlayDialogue(char *szDialogue, CinematicTrigger* pCinematic, BOOL bWindow = FALSE,
                            BOOL bStayOpen = FALSE, const char *szCharOverride = NULL, char *szDecisions = NULL, unsigned char byMood=0);
        LTBOOL          PlayDialogue(DWORD dwID, CinematicTrigger* pCinematic,  BOOL bWindow = FALSE, BOOL bStayOpen = FALSE,
                            const char *szCharOverride = NULL, char *szDecisions = NULL, unsigned char byMood=0);

        LTBOOL           DoDialogueWindow(CinematicTrigger* pCinematic,DWORD dwID, BOOL bStayOpen = FALSE,
							const char *szCharOverride = NULL, char *szDecisions = NULL);

        virtual void    StopDialogue(LTBOOL bCinematicDone = LTFALSE);

        virtual LTBOOL   SetDims(LTVector* pvDims, LTBOOL bSetLargest=LTTRUE);

        virtual LTBOOL   HasDangerousWeapon() { return LTFALSE; }

	protected :

		friend class CCharacterHitBox;
		friend class CAttachments;

		virtual void	UpdateAnimation();

        virtual LTVector GetHeadOffset();

        uint32          EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
        uint32          ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);

		virtual void	Reset();

		virtual void	UpdateOnGround();
        virtual void    UpdateMovement(LTBOOL bUpdatePhysics=LTTRUE);
		virtual void	UpdateSounds();
		virtual void	UpdateFootprints();

		virtual void		SetDeathAnimation();
        virtual HMODELANIM  GetDeathAni(LTBOOL bFront);

		virtual LTFLOAT	GetFootstepVolume() { return 1.0f; }

        virtual char*   GetDamageSound(DamageType eType) { return LTNULL; }
        virtual char*   GetDeathSound() { return LTNULL; }
		virtual void	PlayDamageSound(DamageType eType);
        virtual void    PlayDeathSound();
        virtual void    HandleDead(LTBOOL bRemoveObj);
		void			HandleShortRecoil();

        virtual void    HandleGadget(uint8 nAmmoID) {}

        virtual void    HandleVectorImpact(IntersectInfo& iInfo, LTVector& vDir, LTVector& vFrom, ModelNode& eModelNode);

		virtual void	StartDeath();
		virtual void	CreateBody();
		virtual void	SetupBody(Body* pProp);

		virtual void	HandleAttach() {}
		virtual void	HandleDetach() {}
		virtual void	HandleTeleport(TeleportPoint* pTeleportPoint) {}

		// Body state methods

		BodyState GetPriorityBodyState(BodyState bs1, BodyState bs2);
		virtual BodyState	GetBodyState();

        virtual LTBOOL   ProcessTriggerMsg(const char* pMsg);
		virtual void	ProcessDamageMsg(HMESSAGEREAD hRead);
        virtual LTBOOL   ProcessCommand(char** pTokens, int nArgs, char* pNextCommand);

        virtual void    PlaySound(char *pSoundName, LTFLOAT fRadius=BC_DEFAULT_SOUND_RADIUS, LTBOOL bAttached=LTTRUE);

        virtual LTVector HandHeldWeaponFirePos(CWeapon* pWeapon);

		virtual void	CreateHitBox();
		virtual void	UpdateHitBox();

		virtual void	CreateSpecialFX(LTBOOL bUpdateClients=LTFALSE);
		virtual void	PreCreateSpecialFX(CHARCREATESTRUCT& cs) {};

	protected :

		enum Constants
		{
			kMaxSpears = 16,
		};

		struct SPEARSTRUCT
		{
			HOBJECT		hObject;
			ModelNode	eModelNode;
            LTRotation  rRot;

			SPEARSTRUCT()
			{
				hObject = LTNULL;
				eModelNode = eModelNodeInvalid;
				rRot = LTRotation(0,0,0,1);
			}
		};

	protected :
		CHARCREATESTRUCT	m_cs;							// Our character specialfx struct

		SurfaceType			m_eStandingOnSurface;			// What surface we're currently standing on.
        LTFLOAT             m_fBodyLifetime;                // Create body prop when dead
        LTBOOL				m_bMoveToFloor;					// Move character to floor
		CharacterDeath		m_eDeathType;					// How did we die
        LTBOOL              m_bStartedDeath;                // Did I start death ani?
        LTBOOL				m_bRolling;						// Are we in the process of rolling
        LTBOOL				m_bPivoting;					// Are we in the process of pivoting
        LTBOOL				m_bAllowRun;					// Can we run
        LTBOOL				m_bAllowMovement;				// Can we currently move
        LTBOOL				m_bOnGround;					// Are we on the ground
        LTBOOL				m_bSpectatorMode;				// Are we in spectator mode (player only)
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
		ModelStyle			m_eModelStyle;					// Stylf of model
        HSTRING             m_hstrSpawnItem;                // Object to spawn when dead

        uint8				m_byFXFlags;					// Our CharacterFX fx flags (hearts, smokepuffs, etc)

        LTFLOAT				m_fLastPainTime;				// Last time we were hurt
        LTFLOAT				m_fLastPainVolume;				// Last volume of pain we made

		CharFireInfo		m_LastFireInfo;					// Info about last fired shot
		CharMoveInfo		m_LastMoveInfo;					// Info about last movement
		CharCoinInfo		m_LastCoinInfo;					// Info about last coin toss

        LTFLOAT				m_fDefaultHitPts;				// Default hit pts
        LTFLOAT				m_fDefaultArmor;				// Default armor
        LTFLOAT				m_fSoundRadius;					// Radius for sounds we play
		SoundPriority		m_eSoundPriority;				// Sound priority used when sound played

        LTFLOAT				m_fBaseMoveAccel;				// The base (starting) move acceleration
        LTFLOAT				m_fLadderVel;					// How fast we swim
        LTFLOAT				m_fRunVel;						// How fast we run
        LTFLOAT				m_fWalkVel;						// How fast we walk
        LTFLOAT				m_fSwimVel;						// How fast we swim
        LTFLOAT				m_fRollVel;						// How fast we roll
        LTFLOAT				m_fJumpVel;						// How fast we jump
        LTBOOL				m_bUsingHitDetection;			// Using hit detection
		CharacterClass		m_cc;							// Who am I anyway...
		CharacterClass		m_ccCrosshair;					// How do I show up in crosshairs?
        uint32              m_dwFlags;                      // Initial flags

        LTBOOL				m_bCanPlayDialogSound;
        LTBOOL				m_bCanDamageBody;

		int					m_iLastVolume;
        LTVector			m_vLastVolumePos;

		HOBJECT				m_hHitBox;						// Used to calculate weapon impacts

		LTBOOL				m_bShortRecoiling;				// Are we doing a short recoil?

		uint32				m_cSpears;						// How many spears do we have stuck in us
		SPEARSTRUCT			m_aSpears[kMaxSpears];			// Array of spear HOBJECTs

		CTList<CharFootprintInfo*>	m_listFootprints;		// List of our footprints

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

        LTBOOL              m_bPlayingTextDialogue;			// Are we displaying text

        LTFLOAT             m_fMoveMultiplier;
        LTFLOAT             m_fJumpMultiplier;

		CString				m_csName;						// Unique name for this character

		CAnimator*			m_pAnimator;					// Our animator

		LTBOOL				m_bBlink;						// Do we blink?
        LTAnimTracker       m_BlinkAnimTracker;				// Our blinking animation tracker
        uint32              m_iBlinkAnimTracker;			// Which anim tracker the blink one is

		LTBOOL				m_bShortRecoil;					// Do we do short recoils?
        LTAnimTracker       m_RecoilAnimTracker;			// Our recoil anim tracker
        uint32              m_iRecoilAnimTracker;			// Which anim tracker the recoil one is

		HMODELWEIGHTSET		m_hBlinkWeightset;				// The blink weightset
		HMODELWEIGHTSET		m_hTwitchWeightset;				// The twitch weightset
		HMODELWEIGHTSET		m_hNullWeightset;				// The null weightset

		LTBOOL				m_bInitializedAnimation;		// Did we initialize our animation stuff yet?

		LTBOOL				m_bWallStick;					// Should we wall stick when we die?

		int32				m_cActive;						// Activation count
		static int32		sm_cAISnds;

	private :

		void	InitialUpdate(int nInfo);
		void	Update();

        LTBOOL  ReadProp(ObjectCreateStruct *pStruct);
		void	HandleModelString(ArgList* pArgList);
		void	Save(HMESSAGEWRITE hWrite);
		void	Load(HMESSAGEREAD hRead);
		void	CacheFiles();

		void	UpdateContainerCode();
		void	UpdateCheatInfo();

		void	InitAnimation();
};

#endif // __CHARACTER_H__