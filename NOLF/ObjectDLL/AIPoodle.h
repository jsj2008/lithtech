// (c) 1997-2000 Monolith Productions, Inc.  All Rights Reserved

#ifndef __AIPOODLE_H__
#define __AIPOODLE_H__

#include "AIAnimal.h"

class AI_Poodle : public CAIAnimal
{
	public : // Public methods

		// Ctors/Dtors/etc

 		AI_Poodle();
 		~AI_Poodle();

		// Movement

		void Walk() { m_fSpeed = GetWalkSpeed(); }
		void Run() { m_fSpeed = GetRunSpeed(); }
		void Stop() { m_fSpeed = 0.0f; }
        LTFLOAT GetSpeed() { return m_fSpeed; }

        void Move(const LTVector& vPos);

		// Attributes

        LTFLOAT GetWalkSpeed() { return m_fWalkVel; }
        LTFLOAT GetRunSpeed() { return m_fRunVel; }

	protected : // Protected member functions

		// Engine methods

        LTBOOL ReadProp(ObjectCreateStruct *pData);
		void InitialUpdate();
        void Save(HMESSAGEWRITE hWrite, uint32 dwSaveFlags);
        void Load(HMESSAGEREAD hRead, uint32 dwLoadFlags);

		// Character stuff

		void PreCreateSpecialFX(CHARCREATESTRUCT& cs);

		// Update methods

		void UpdateMovement();
		void UpdateAnimator();

		// Handlers

        LTBOOL HandleCommand(char** pTokens, int nArgs);

		// State methods

		void SetState(CAIPoodleState::AIPoodleStateType eState);

	protected : // Protected member variables

		// State

		CAIPoodleState*	m_pPoodleState;	// Our Poodle state

		// Movement

        LTVector    m_vMovePos;			// Position to move to
        LTBOOL      m_bMove;			// Are we moving or not this frame
        LTFLOAT     m_fSpeed;			// Our current movement speed

		// Sound

		HLTSOUND	m_hRunningSound;	// The sound we make while in operation
};

DEFINE_ALIGNMENTS(Poodle);

#endif // __AIPOODLE_H__