// ----------------------------------------------------------------------- //
//
// MODULE  : AITargetSelectDisturbance.h
//
// PURPOSE : AITargetSelectDisturbance class declaration
//
// CREATED : 5/19/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AITARGETSELECT_DISTURBANCE_H__
#define __AITARGETSELECT_DISTURBANCE_H__

#include "AITargetSelectAbstract.h"

// Forward declarations.

class	CAIWMFact;


// ----------------------------------------------------------------------- //

class CAITargetSelectDisturbance : public CAITargetSelectAbstract
{
	typedef CAITargetSelectAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectDisturbance, kTargetSelect_Disturbance );

		CAITargetSelectDisturbance();

		// CAIActionAbstract members.

		virtual bool	ValidatePreconditions( CAI* pAI );
		virtual void	Activate( CAI* pAI );
		virtual void	Deactivate( CAI* pAI );
		virtual bool	Validate( CAI* pAI );

	protected:

		void			TargetDisturbance( CAI* pAI, CAIWMFact* pFact );

	protected:

		// This value is static and does not need to be saved.

		float	m_fMinCharacterConfidence;
};

// ----------------------------------------------------------------------- //

#endif
