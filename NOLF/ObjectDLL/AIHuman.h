// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AI_HUMAN_H__
#define __AI_HUMAN_H__

#include "AI.h"
#include "AIHumanState.h"
#include "AIHumanReactions.h"
#include "AINudge.h"
#include "AIBrain.h"

class CAIHuman : public CAI
{
	public : // Public methods

		// Ctors/Dtors/etc

		CAIHuman();
		virtual ~CAIHuman();

		// Type

        virtual LTBOOL IsScuba() { return LTFALSE; }
        virtual LTBOOL IsParatrooper() { return LTFALSE; }

		// Attachments

		virtual void CreateAttachments();

		// Senses

        virtual LTBOOL IsObjectVisibleFromKnee(ObjectFilterFn fn, HOBJECT hObj, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV /* = LTTRUE */);
        virtual LTBOOL IsPositionVisibleFromKnee(ObjectFilterFn fn, const LTVector& vSourcePosition, LTFLOAT fVisibleDistanceSqr, LTBOOL bFOV /* = LTTRUE */);

		// Weapons

        virtual LTVector GetWeaponPosition(CWeapon *pWeapon);

		// Brain

		CAIBrain* GetBrain() { return m_pBrain; }

		// Attributes

		LTFLOAT GetCreepSpeed() { return m_fCreepVel; }
		void SetCreepSpeed(LTFLOAT fCreepSpeed) { m_fCreepVel = fCreepSpeed; }
		LTFLOAT GetWalkSpeed() { return m_fWalkVel; }
		void SetWalkSpeed(LTFLOAT fWalkSpeed) { m_fWalkVel = fWalkSpeed; }
		LTFLOAT GetRunSpeed() { return m_fRunVel; }
		void SetRunSpeed(LTFLOAT fRunSpeed) { m_fRunVel = fRunSpeed; }
		LTFLOAT GetSwimSpeed() { return m_fSwimVel; }
		void SetSwimSpeed(LTFLOAT fSwimSpeed) { m_fSwimVel = fSwimSpeed; }
		LTFLOAT GetRollSpeed() { return m_fRollVel; }
		void SetRollSpeed(LTFLOAT fRollSpeed) { m_fRollVel = fRollSpeed; }

		// Movement methods

		void SyncPosition()
		{
			m_bSyncPosition = LTTRUE;
		}

		LTVector GetKneePosition() const;

		void Move(const LTVector& vPos);

		void Turn(LTFLOAT fRadians);

		void OpenDoor(HOBJECT hDoor);
		void CloseDoor(HOBJECT hDoor);

		LTFLOAT	GetSpeed();
		void Walk() { m_fSpeed = GetWalkSpeed(); }
		void Run() { m_fSpeed = GetRunSpeed(); }
		void Swim() { m_fSpeed = GetSwimSpeed(); }
		void Stop() { m_fSpeed = 0.0f; }

		LTFLOAT GetRotationInterpolation(LTFLOAT fTime) { return (LTFLOAT)sqrt(fTime); }

		CNudge* GetNudge() { return &m_Nudge; }
/*
		// Sleeping stuff

		void SetSleeping(LTBOOL bSleeping);
*/
		// Death stuff

		virtual void PlayDeathSound();
		virtual HMODELANIM GetDeathAni(LTBOOL bFront);
		const char* GetCrouchDeathAni();
		const char* GetProneDeathAni();
		virtual LTBOOL WasSilentKill();
		virtual BodyState GetBodyState();

		// Aiming stuff

		LTBOOL Aim(const LTVector& vDir);
		LTBOOL AimAt(const LTVector& vPosition);
		LTBOOL AimAt(HOBJECT hObject);

		// Recoil

		void SetCanShortRecoil(LTBOOL bCanShortRecoil) { m_bCanShortRecoil = bCanShortRecoil; }
		LTBOOL CanShortRecoil() const { return m_bCanShortRecoil; }

		// Holster

		LTBOOL HasHolsterString() const { return !!m_hstrHolster; }
		const char* GetHolsterString() const { return g_pLTServer->GetStringData(m_hstrHolster); }

		// Hitpoints

		void BoostHitpoints(LTFLOAT fFactor)
		{
			m_damage.SetHitPoints(m_damage.GetHitPoints()*fFactor);
			m_damage.SetMaxHitPoints(m_damage.GetMaxHitPoints()*fFactor);
		}

		// Animation

		CAnimationContext* GetAnimationContext() { return m_pAnimationContext; }

	protected : // Protected member functions

		// Engine methods

		virtual LTBOOL ReadProp(ObjectCreateStruct *pInfo);
        virtual void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        virtual void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);

		// Character stuff

		virtual void PreCreateSpecialFX(CHARCREATESTRUCT& cs);

		// Handlers

        virtual void HandleGadget(uint8 nAmmoID);
		virtual void HandleDamage(const DamageStruct& damage);
		virtual LTBOOL HandleCommand(char **pTokens, int nArgs);
		virtual void HandleTeleport(const LTVector& vPos);
		virtual void HandleTeleport(TeleportPoint* pTeleportPoint);

		// Updates

		virtual void PreUpdate();
		virtual void UpdateAnimation();
		virtual void UpdateMovement();
		virtual void UpdateTarget();
		virtual void UpdateNodes();
		virtual void UpdateCharacterFx();

		// Reactions

		DEFINE_REACTIONS_AIHUMAN()

	protected : // Protected methods

		// Brain methods

		void SetBrain(HSTRING hstrBrain);

		// State methods

		void SetState(CAIHumanState::AIHumanStateType eState, LTBOOL bUnlockAnimation = LTTRUE);

	protected : // Member Variables

		// State

		CAIHumanState*	m_pHumanState;			// Current state

		// Brain

		HSTRING			m_hstrBrain;			// Name of our brain
		CAIBrain*		m_pBrain;				// Our brain (attributes and whatnot)

		// Movement

		LTBOOL		m_bForceGround;				// Force AI to ground?

		LTBOOL		m_bPosDirty;				// Is our position dirty? (do we need to refind the ground)

		LTVector	m_vMovePos;					// Position to move to
		LTBOOL		m_bMove;					// Are we moving or not this frame
		LTFLOAT		m_fSpeed;					// Current movement speed

		CNudge		m_Nudge;					// Nudge helper

		LTBOOL		m_bSyncPosition;			// Sync position

		// Node handle cache

		HMODELNODE	m_hHeadNode;

		// Attributes

		LTFLOAT		m_fCreepVel;				// Creep speed

		// Recoil stuff

		LTBOOL		m_bCanLongRecoil;			// Can we do a long recoil?
		LTBOOL		m_bLongRecoiling;			// Are we doing a long recoil?

		LTBOOL		m_bCanShortRecoil;			// Can we do short recoils?

		// Commands

		HSTRING		m_hstrCmdLighter;			// Our lighter activation message

		// Holster

		HSTRING		m_hstrHolster;				// Our holstered weapon

		// Animation stuff

		CAnimationContext*	m_pAnimationContext;	// Our animation context
};

// Plugin
class CAIHumanPlugin : public CAIPlugin
{
	public:

        virtual LTRESULT PreHook_EditStringList(const char* szRezPath, const char* szPropName, char** aszStrings, uint32* pcStrings, const uint32 cMaxStrings, const uint32 cMaxStringLength);
		virtual CAttachmentsPlugin* GetAttachmentsPlugin() { return &m_HumanAttachmentsPlugin; }
		virtual void GetReactions(REACTIONSTRUCT** ppReactions, int* pcReactions) { *ppReactions = g_aAIHumanReactions; *pcReactions = g_cAIHumanReactions; }

	private :

		CHumanAttachmentsPlugin	m_HumanAttachmentsPlugin;
};

#endif // __AI_HUMAN_H__