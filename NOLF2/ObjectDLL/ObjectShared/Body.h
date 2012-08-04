// ----------------------------------------------------------------------- //
//
// MODULE  : Body.h
//
// PURPOSE : Body Prop - Definition
//
// CREATED : 1997 (was BodyProp)
//
// (c) 1997-2002 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __BODY_PROP_H__
#define __BODY_PROP_H__

enum  EnumAIStateType;
class CAttachments;
class CBodyState;
class CSearchable;

#include "Character.h"
#include "Prop.h"
#include "ModelButeMgr.h"
#include "SFXMsgIDs.h"
#include "Animator.h"
#include "DeathScene.h"
#include "GeneralInventory.h"
#include "IHitBoxUser.h"
#include "SharedFXStructs.h"

LINKTO_MODULE( Body );

struct BODYINITSTRUCT
{
	CCharacter*		pCharacter;
	EnumAIStateType	eBodyState;
	bool			bPermanentBody;
	float			fBodyLifetime;
	bool			bCanRevive;
};

class CAnimatorBody : public CAnimator
{
	public :

		// Ctors/Dtors/etc

		void Init(HOBJECT hObject);

		// Updates

		void Update();

		// Methods

		void Twitch();

	protected :

		AniTracker			m_eAniTrackerTwitch;	// Our twitch ani tracker
		Ani					m_eAniTwitch;			// Our twitch ani
};

class Body : public Prop, public IHitBoxUser
{
	typedef Prop super;

	public : // Public methods

		// Ctors/Dtors/etc

 		Body();
		~Body();

		void Init(const BODYINITSTRUCT& bi);

		CBodyState* AI_FACTORY_NEW_State(EnumAIStateType eStateType);
		void SetState(EnumAIStateType eBodyState, LTBOOL bLoad = LTFALSE);
		EnumAIStateType GetBodyState() {return m_eBodyState;}

		void FacePos(const LTVector& vTargetPos);
		void FaceDir(const LTVector& vTargetDir);

		ModelId			GetModelId() const { return m_eModelId; }
		ModelSkeleton	GetModelSkeleton() const { return m_eModelSkeleton; }

		void			AddWeapon(HOBJECT hWeapon, char *pszPosition);
		bool			AddPickupItem(HOBJECT hItem, bool bForcePickup = false);

		void			AddSpear(HOBJECT hSpear, const LTRotation& rRot, ModelNode eModelNode=eModelNodeInvalid);
		void			SetModelNodeLastHit(ModelNode eModelNodeLastHit)	{ m_eModelNodeLastHit = eModelNodeLastHit; }
		ModelNode		GetModelNodeLastHit()	const	{ return m_eModelNodeLastHit; }

		CAttachments*	GetAttachments() { return m_pAttachments; }

		HOBJECT			GetHitBox()			const { return m_hHitBox; }

		LTFLOAT			GetLifetime() { return m_fLifetime; }
		LTFLOAT			GetStarttime() { return m_fStartTime; }
		bool			GetFadeAfterLifetime() { return m_bFadeAfterLifetime; }

		HOBJECT			GetLastDamager() const { return m_damage.GetLastDamager(); }

		const LTVector&	GetDeathDir() const { return m_vDeathDir; }

		void			ReleasePowerups(bool bWeaponsOnly = false); 
		void			RemovePowerupObjects(); 

		bool			IsPermanentBody() const { return m_bPermanentBody; }
		void			SetPermanentBody( bool bPerm );

		// for game types where a players body may sit aound indefinitely while waiting to respawn
		void			ResetLifetime(float fLifetime);

		virtual LTBOOL UsingHitDetection() const { return LTTRUE; }

		virtual float GetNodeRadius( ModelSkeleton eModelSkeleton, ModelNode eModelNode ) { return ( g_pModelButeMgr->GetSkeletonNodeHitRadius(eModelSkeleton, eModelNode) ); }

		HOBJECT			GetCharacter( ) const { return m_hCharacter; }


		// [kml] 3/19/02
		// For general inventory stuff
		void			DropInventoryObject();

		LTBOOL			CanCheckPulse();
		bool			CanBeSearched(); 

		void			AddToObjectList( ObjectList *pObjList, eObjListControl eControl );
		virtual HATTACHMENT GetAttachment() const { return NULL; }
		virtual LTFLOAT		ComputeDamageModifier(ModelNode eModelNode) { return 1.0f; }

		// Checker

		void SetChecker(HOBJECT hChecker);
		LTBOOL HasChecker() const { return !!m_hChecker; }
		HOBJECT GetChecker() const { return m_hChecker; }

		void	SetBodyResetTime( LTFLOAT fResetTime ) { m_fBodyResetTime = fResetTime; } 
		LTFLOAT GetBodyResetTime() { return m_fBodyResetTime; } 

		// General inventory accessor
		GEN_INVENTORY_LIST&	GetInventory() { return m_lstInventory; }
		float GetEnergy() { return m_fEnergy; }

		// Implementing classes will have this function called
		// when HOBJECT ref points to gets deleted.
		virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );

		// Carrying

		virtual void SetCanCarry( bool bCarry );
		virtual void SetBeingCarried( bool bBeingCarried, HOBJECT hCarrier );
		virtual bool BeingCarried() { return m_bBeingCarried; }

		void SetCanRevive( bool bCanRevive );
		bool GetCanRevive( ) const { return m_bCanRevive; }

		// Hide the body and it's associated objects.
		void		HideBody( bool bHide );

		virtual CAnimator* GetAnimator() { return &m_Animator; }

		// Get the complete list of all bodies.
		typedef std::vector< Body* > BodyList;
		static BodyList const& GetBodyList( ) { return m_lstBodies; }


	protected : // Protected methods


		// Engine methods

        uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
        uint32 ObjectMessageFn(HOBJECT hSender, ILTMessage_Read *pMsg);
		virtual bool OnTrigger(HOBJECT hSender, const CParsedMsg &cMsg);
		virtual uint32 OnDeactivate();

		// Hitbox message
		void CreateHitBox(const BODYINITSTRUCT& bi);
		void UpdateHitBox();
		void UpdateClientHitBox();

		// Handling methods

		void HandleDamage(const DamageStruct& damage);
		void HandleVectorImpact(IntersectInfo& iInfo, LTVector& vDir,	LTVector& vFrom, ModelNode& eModelNode) {}

		virtual void HandleDestroy(HOBJECT hDamager);

		void ReadProp(ObjectCreateStruct *pData);

		void Update();

        void Save(ILTMessage_Write *pMsg, uint32 dwSaveFlags);
        void Load(ILTMessage_Read *pMsg, uint32 dwLoadFlags);

		void CreateDeathScene(CCharacter *pChar);

		void ReleasePowerup(HOBJECT hPowerup);

		// Keeps body density down within a radius.
		void CapNumberOfBodies( );

		void CreateSpecialFX();

	protected :

		enum Constants
		{
			kMaxSpears = 32,
			kMaxWeapons = 2,
		};

	private:

		// Returns true when it shouldn't be passed to parent.
		bool HandleModelString(ArgList* pArgList);

	protected :

		CAttachments*		m_pAttachments;
		CharacterDeath		m_eDeathType;
		ModelId				m_eModelId;
		ModelSkeleton		m_eModelSkeleton;
		ModelNode			m_eModelNodeLastHit;
		LTBOOL				m_bFirstUpdate;
		LTFLOAT				m_fStartTime;
		LTFLOAT				m_fLifetime;
		bool				m_bFadeAfterLifetime;
		LTVector			m_vColor;
		LTVector			m_vDeathDir;
		DamageType			m_eDamageType;
		LTObjRef			m_hHitBox;

		uint32				m_cWeapons;
		LTObjRefNotifier	m_ahWeapons[kMaxWeapons];

		LTObjRef			m_hChecker;

		LTFLOAT				m_fBodyResetTime;

		EnumAIStateType		m_eBodyState;
		EnumAIStateType		m_eBodyStatePrevious;
		CBodyState*			m_pState;
		CAnimatorBody		m_Animator;

		uint32				m_cSpears;
		LTObjRefNotifier	m_ahSpears[kMaxSpears];

		CDeathScene			m_DeathScene;

		CSearchable*		m_pSearch;

		EnumAIStimulusID	m_eDeathStimID;	// Registration ID of dead body stimulus.

		bool				m_bCanBeCarried;

		bool				m_bUpdateHitBox;
		bool				m_bDimsUpdated;

		GEN_INVENTORY_LIST	m_lstInventory;					// General inventory items (see GeneralInventory.h)
		float				m_fEnergy;	
		
		LTObjRef			m_hCharacter;	// The character our body spawned from

		// Never remove this body through body density capping.
		bool				m_bPermanentBody;

		static BodyList		m_lstBodies;

		bool				m_bCanCarry;
		bool				m_bBeingCarried;
		LTObjRef			m_hCarrier;
		bool				m_bCanRevive;
		
		BODYCREATESTRUCT	m_BCS;
};

#endif // __BODY_PROP_H__




