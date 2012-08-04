// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionLongRecoil.h
//
// PURPOSE : AIActionLongRecoil class definition
//
// CREATED : 11/11/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_LONG_RECOIL_H__
#define __AIACTION_LONG_RECOIL_H__

#include "AIActionShortRecoil.h"


// Forward declarations.
class CAIWMFact;
class CAnimationProps;

class CAIActionLongRecoil : public CAIActionShortRecoil
{
	typedef CAIActionShortRecoil super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionLongRecoil, kAct_LongRecoil );

		// CAIActionAbstract members.

		virtual float	GetActionProbability( CAI* pAI );
};

// ----------------------------------------------------------------------- //

#endif
