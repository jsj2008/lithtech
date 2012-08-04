// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionShortRecoil.h
//
// PURPOSE : AIActionShortRecoil class definition
//
// CREATED : 03/25/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_SHORT_RECOIL_H__
#define __AIACTION_SHORT_RECOIL_H__

#include "AIActionAbstract.h"


// Forward declarations.
class CAIWMFact;
class CAnimationProps;

class CAIActionShortRecoil : public CAIActionAbstract
{
	typedef CAIActionAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionShortRecoil, kAct_ShortRecoil );

		CAIActionShortRecoil();

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual void	DeactivateAction( CAI* pAI );
		virtual	bool	IsActionComplete( CAI* pAI );
		virtual void	ApplyContextEffect( CAI* pAI, CAIWorldState* pwsWorldStateCur, CAIWorldState* pwsWorldStateGoal );
		virtual bool	ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );

	protected:
	
		void SetFourQuadrantDirectionalRecoils( bool b );
		void SetApplyContextEffect( bool bApply );
		virtual bool GetRecoilProps(CAI* pAI, CAIWMFact* pFact, CAnimationProps& outAnimProps);

	private:
		bool IsStumbleRecoilValid( CAI* pAI, const LTVector& vDir, CAnimationProps& AnimProps );
		bool IsKnockDownValid( CAI* pAI, const LTVector& vDir );
		bool CanAIRecoil( CAI* pAI, AIDB_SmartObjectRecord* pSmartObjectRecord );
		void UpdateRecoilKnowledge( CAI* pAI, AIDB_SmartObjectRecord* pSmartObjectRecord );
		void ClearRecoilKnowledge( CAI* pAI, AIDB_SmartObjectRecord* pSmartObjectRecord );

		bool m_bFourQuadrantDirectionalRecoils;
		bool m_bApplyContextEffect;
};

// ----------------------------------------------------------------------- //

#endif
