// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionFlushOutWithGrenade.h
//
// PURPOSE : AIActionFlushOutWithGrenade class definition
//
// CREATED : 01/30/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_FLUSH_OUT_WITH_GRENADE_H__
#define __AIACTION_FLUSH_OUT_WITH_GRENADE_H__

#include "AIActionAttackGrenade.h"

class CAIActionFlushOutWithGrenade : public CAIActionAttackGrenade
{
	typedef CAIActionAttackGrenade super;

public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionFlushOutWithGrenade, kAct_FlushOutWithGrenade );

		CAIActionFlushOutWithGrenade();

		// CAIActionAbstract members.

		virtual void	InitAction( AIDB_ActionRecord* pActionRecord );
};

// ----------------------------------------------------------------------- //

#endif
