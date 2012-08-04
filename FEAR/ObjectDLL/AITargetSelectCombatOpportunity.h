// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionTargetCombatOpportunity.h
//
// PURPOSE : 
//
// CREATED : 6/24/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AITARGETSELECTCOMBATOPPORTUNITY_H_
#define _AIACTIONTARGETCOMBATOPPORTUNITY_H_

LINKTO_MODULE(AITargetSelectCombatOpportunity);

#include "AITargetSelectAbstract.h"

class CAIWMFact;
class CAI;

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAITargetSelectCombatOpportunity
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAITargetSelectCombatOpportunity : public CAITargetSelectAbstract
{
	typedef CAITargetSelectAbstract super;
	
	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectCombatOpportunity, kTargetSelect_CombatOpportunity );

		// Ctor/Dtor

		CAITargetSelectCombatOpportunity();
		virtual ~CAITargetSelectCombatOpportunity();

		virtual void	Activate( CAI* pAI );
		virtual void	Deactivate( CAI* pAI );
		virtual bool	ValidatePreconditions( CAI* pAI );
		virtual bool	Validate( CAI* pAI );

	private:

		void	SetTargetCombatOpportunity( CAI* pAI );

		PREVENT_OBJECT_COPYING(CAITargetSelectCombatOpportunity);
};

#endif // _AITARGETSELECTCOMBATOPPORTUNITY_H_
