// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionGotoAbstract.h
//
// PURPOSE : AIActionGotoAbstract abstract class definition
//
// CREATED : 10/07/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_GOTO_ABSTRACT_H__
#define __AIACTION_GOTO_ABSTRACT_H__

#include "Stdafx.h"
#include "AIActionAbstract.h"

class CAIActionGotoAbstract : public CAIActionAbstract
{
		typedef CAIActionAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_ABSTRACT_SPECIFIC( Action );

		CAIActionGotoAbstract();

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
		virtual void	ActivateAction( CAI* pAI, CAIWorldState& wsWorldStateGoal );
		virtual void	DeactivateAction( CAI* pAI );
		virtual	bool	ValidateAction( CAI* pAI );

	protected:

};

// ----------------------------------------------------------------------- //

#endif
