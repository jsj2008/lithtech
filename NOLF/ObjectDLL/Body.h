// ----------------------------------------------------------------------- //
//
// MODULE  : Body.h
//
// PURPOSE : Body Prop - Definition
//
// CREATED : 1997 (was BodyProp)
//
// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved
//
// ----------------------------------------------------------------------- //

#ifndef __BODY_PROP_H__
#define __BODY_PROP_H__

class CAttachments;
class CBodyState;

#include "Character.h"
#include "Prop.h"
#include "ModelButeMgr.h"
#include "SFXMsgIDs.h"
#include "Animator.h"
#include "DeathScene.h"

struct BODYINITSTRUCT
{
	CCharacter*		pCharacter;
	BodyState		eBodyState;
	LTFLOAT			fLifetime;
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

class Body : public Prop
{
	public : // Public methods

		// Ctors/Dtors/etc

 		Body();
		~Body();

		void Init(const BODYINITSTRUCT& bi);

		void SetState(BodyState eBodyState, LTBOOL bLoad = LTFALSE);

		void FacePos(const LTVector& vTargetPos);
		void FaceDir(const LTVector& vTargetDir);

		ModelId			GetModelId() const { return m_eModelId; }
		ModelSkeleton	GetModelSkeleton() const { return m_eModelSkeleton; }

		void			AddSpear(HOBJECT hSpear, const LTRotation& rRot, ModelNode eModelNode=eModelNodeInvalid);
		void			SetModelNodeLastHit(ModelNode eModelNodeLastHit)	{ m_eModelNodeLastHit = eModelNodeLastHit; }
		ModelNode		GetModelNodeLastHit()	const	{ return m_eModelNodeLastHit; }

		CAttachments*	GetAttachments() { return m_pAttachments; }

		HOBJECT			GetHitBox()			const { return m_hHitBox; }

		LTFLOAT			GetLifetime() { return m_fLifetime; }
		LTFLOAT			GetStarttime() { return m_fStartTime; }

		HOBJECT			GetLastDamager() const { return m_damage.GetLastDamager(); }

		const LTVector&	GetDeathDir() const { return m_vDeathDir; }

		void			RemoveObject();

		void			ReleasePowerups(); 

		LTBOOL			CanCheckPulse();

		// Checker

		void SetChecker(HOBJECT hChecker);
		LTBOOL HasChecker() const { return !!m_hChecker; }
		HOBJECT GetChecker() const { return m_hChecker; }

	protected : // Protected methods

		friend class CCharacterHitBox;

		// Engine methods

        uint32 EngineMessageFn(uint32 messageID, void *pData, LTFLOAT fData);
        uint32 ObjectMessageFn(HOBJECT hSender, uint32 messageID, HMESSAGEREAD hRead);
		void TriggerMsg(HOBJECT hSender, const char* szMsg);

		// Hitbox message
		void CreateHitBox(const BODYINITSTRUCT& bi);
		void UpdateHitBox();

		// Handling methods

		void HandleDamage(const DamageStruct& damage);
		void HandleModelString(ArgList* pArgList);
		void HandleVectorImpact(IntersectInfo& iInfo, LTVector& vDir,	LTVector& vFrom, ModelNode& eModelNode) {}

		void ReadProp(ObjectCreateStruct *pData);

		void Update();
//		LTBOOL UpdateNormalDeath(LTFLOAT fTime);
//		LTBOOL UpdateFreezeDeath(LTFLOAT fTime);
//		LTBOOL UpdateVaporizeDeath(LTFLOAT fTime);

        void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);

		void CreateDeathScene(CCharacter *pChar);
//		void CreateGibs();

		void ReleasePowerup(HOBJECT hPowerup);

	protected :

		enum Constants
		{
			kMaxSpears = 32,
		};

	protected :

		CAttachments*		m_pAttachments;
		CharacterDeath		m_eDeathType;
		ModelId				m_eModelId;
		ModelSkeleton		m_eModelSkeleton;
		ModelNode			m_eModelNodeLastHit;
		ModelStyle			m_eModelStyle;
		LTBOOL				m_bFirstUpdate;
		LTFLOAT				m_fStartTime;
		LTFLOAT				m_fLifetime;
		LTVector			m_vColor;
		LTVector			m_vDeathDir;
		DamageType			m_eDamageType;
		HOBJECT				m_hHitBox;
		HOBJECT				m_hWeaponItem;
		HOBJECT				m_hChecker;

		BodyState			m_eBodyState;
		BodyState			m_eBodyStatePrevious;
		CBodyState*			m_pState;
		CAnimatorBody		m_Animator;

		uint32				m_cSpears;
		HOBJECT				m_ahSpears[kMaxSpears];

		CDeathScene			m_DeathScene;
};

#endif // __BODY_PROP_H__




