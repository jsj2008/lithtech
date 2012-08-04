// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AIShark_H__
#define __AIShark_H__

#include "AIAnimal.h"
#include "AISteering.h"

class AI_Shark : public CAIAnimal, public CSteerable
{
	public : // Public methods

		// Ctors/Dtors/etc

 		AI_Shark();
 		~AI_Shark();

		// Movement

		void Swim() { m_fSpeed = GetSwimSpeed(); }
		void Stop() { m_fSpeed = 0.0f; }
        LTFLOAT GetSpeed() { return m_fSpeed; }

		// Attributes

        LTFLOAT GetAttackDistance() { return m_fAttackDistance; }
        LTFLOAT GetAttackDistanceSqr() { return m_fAttackDistanceSqr; }
        LTFLOAT GetSwimSpeed() { return m_fSwimVel; }
        LTFLOAT GetAttractingNoTargetTime() { return g_pAIButeMgr->GetAttract()->fNoTargetTime; }

		// Simple accessors

		CSteeringMgr* GetSteeringMgr() { return &m_SteeringMgr; }

	protected : // Protected member functions

		// Engine methods

        LTBOOL ReadProp(ObjectCreateStruct *pData);
		void HandleModelString(ArgList* pArgList);
		void InitialUpdate();
        void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);

		// Character stuff

		void PreCreateSpecialFX(CHARCREATESTRUCT& cs);
        LTBOOL CanLipSync() { return LTFALSE; }
		BodyState GetBodyState();

		// Attachments

		void CreateAttachments();

		// Update methods

		void UpdateMovement();
		void UpdateAnimator();

		// Handlers

        LTBOOL HandleCommand(char** pTokens, int nArgs);

		// State methods

		void SetState(CAISharkState::AISharkStateType eState);

		// Precomputation stuff

		void ComputeSquares();

	protected : // Protected methods for CSteerable

        LTBOOL Steerable_PreUpdate();
        LTBOOL Steerable_Update(const LTVector& vSteeringDirection);
        LTBOOL Steerable_PostUpdate();

	protected : // Protected member variables

		// State

		CAISharkState*	m_pSharkState;	// Our Shark state

		// Movement

        LTFLOAT          m_fSpeed;       // Our current movement speed

		// Physical properties (hack)

		LTFLOAT			m_fTurnRadius;
		LTFLOAT			m_fMass;
		LTFLOAT			m_fMaxForce;
		LTFLOAT			m_fBrakeMultiplier;

		// Senses

        LTFLOAT          m_fAttackDistance;      // Distance at which we will attack enemies
        LTFLOAT          m_fAttackDistanceSqr;   // This value squared

		// Steering

		CSteeringMgr	m_SteeringMgr;		// Our steering mgr
};

DEFINE_ALIGNMENTS(Shark);

class CAISharkPlugin : public CAIAnimalPlugin
{
	public:

		virtual CAttachmentsPlugin* GetAttachmentsPlugin() { return &m_SharkAttachmentsPlugin; }

	private :

		CSharkAttachmentsPlugin	m_SharkAttachmentsPlugin;
};

#endif // __AIShark_H__