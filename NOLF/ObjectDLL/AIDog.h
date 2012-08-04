// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AIDOG_H__
#define __AIDOG_H__

#include "AIAnimal.h"
#include "AISteering.h"
#include "AIDogReactions.h"

class AI_Dog : public CAIAnimal, public CSteerable
{
	public : // Public methods

		// Ctors/Dtors/etc

 		AI_Dog();
 		~AI_Dog();

		// Movement

		void Walk() { m_fSpeed = GetWalkSpeed(); }
		void Run() { m_fSpeed = GetRunSpeed(); }
		void Stop() { m_fSpeed = 0.0f; }
        LTFLOAT GetSpeed() { return m_fSpeed; }

		// Attributes

        LTFLOAT GetBarkDistance() { return m_fBarkDistance; }
        LTFLOAT GetBarkDistanceSqr() { return m_fBarkDistanceSqr; }
        LTFLOAT GetAttackDistance() { return m_fAttackDistance; }
        LTFLOAT GetAttackDistanceSqr() { return m_fAttackDistanceSqr; }
        LTFLOAT GetWalkSpeed() { return m_fWalkVel; }
        LTFLOAT GetRunSpeed() { return m_fRunVel; }
        LTFLOAT GetAttractingNoTargetTime() { return g_pAIButeMgr->GetAttract()->fNoTargetTime; }

		// Misc

        LTBOOL HasBarked() { return m_bBarked; }

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

		// Update methods

		void PostUpdate();
		void UpdateNodes();
		void UpdateMovement();
		void UpdateAnimator();

		// Handlers

        LTBOOL HandleCommand(char** pTokens, int nArgs);
        void DoReaction(HSTRING hstrReaction, CAISense* pAISense, LTBOOL bIndividual);

		// State methods

		void SetState(CAIDogState::AIDogStateType eState);

		// Precomputation stuff

		void ComputeSquares();

	protected : // Protected methods for CSteerable

        LTBOOL Steerable_PreUpdate();
        LTBOOL Steerable_Update(const LTVector& vSteeringDirection);
        LTBOOL Steerable_PostUpdate();

	protected : // Protected member variables

		// State

		CAIDogState*	m_pDogState;	// Our dog state

		// Movement

        LTFLOAT          m_fSpeed;       // Our current movement speed

		// Senses

        LTFLOAT          m_fBarkDistance;        // Distance at which we will only bark at enemies
        LTFLOAT          m_fBarkDistanceSqr;     // This value squared
        LTFLOAT          m_fAttackDistance;      // Distance at which we will attack enemies
        LTFLOAT          m_fAttackDistanceSqr;   // This value squared

		// Misc

        LTBOOL           m_bBarked;              // Did we bark last frame?

		// Steering

		CSteeringMgr	m_SteeringMgr;		// Our steering mgr
};

DEFINE_ALIGNMENTS(Dog);

class CAIDogPlugin : public CAIPlugin
{
	public:

		virtual CAttachmentsPlugin* GetAttachmentsPlugin() { return &m_AnimalAttachmentsPlugin; }
		virtual void GetReactions(REACTIONSTRUCT** ppReactions, int* pcReactions) { *ppReactions = g_aAIDogReactions; *pcReactions = g_cAIDogReactions; }

	private :

		CAnimalAttachmentsPlugin	m_AnimalAttachmentsPlugin;
};

#endif // __AIDOG_H__