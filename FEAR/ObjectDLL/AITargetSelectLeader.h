// ----------------------------------------------------------------------- //
//
// MODULE  : AITargetSelectLeader.h
//
// PURPOSE : AITargetSelectLeader class declaration
//
// CREATED : 07/20/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AITARGETSELECT_LEADER_H__
#define __AITARGETSELECT_LEADER_H__

#include "AITargetSelectAbstract.h"


// ----------------------------------------------------------------------- //

class CAITargetSelectLeader : public CAITargetSelectAbstract
{
	typedef CAITargetSelectAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectLeader, kTargetSelect_Leader );

		// CAIActionAbstract members.

		virtual bool	ValidatePreconditions( CAI* pAI );
		virtual void	Activate( CAI* pAI );
		virtual bool	Validate( CAI* pAI );
};

// ----------------------------------------------------------------------- //

#endif
