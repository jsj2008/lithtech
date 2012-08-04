// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionTargetDisturbance.h
//
// PURPOSE : AITargetSelectCharacter class declaration
//
// CREATED : 5/19/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AITARGETSELECT_CHARACTER_H__
#define __AITARGETSELECT_CHARACTER_H__

#include "AITargetSelectAbstract.h"

// Forward declarations.

class	CAIWMFact;


// ----------------------------------------------------------------------- //

class CAITargetSelectCharacter : public CAITargetSelectAbstract
{
	typedef CAITargetSelectAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectCharacter, kTargetSelect_Character );

		CAITargetSelectCharacter();

		void			TargetCharacter( CAI* pAI, CAIWMFact* pFact );
		void			TargetObject( CAI* pAI, CAIWMFact* pFact );

		// CAIActionAbstract members.

		virtual bool	ValidatePreconditions( CAI* pAI );
		virtual void	Activate( CAI* pAI );
		virtual bool	Validate( CAI* pAI );

	protected:

		virtual CAIWMFact*	FindValidTarget( CAI* pAI );

	protected:

		bool				m_bRecordFirstThreat;
};

// ----------------------------------------------------------------------- //

#endif
