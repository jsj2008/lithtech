// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionMgr.h
//
// PURPOSE : AIActionMgr abstract class definition
//
// CREATED : 2/03/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTION_MGR_H__
#define __AIACTION_MGR_H__

#include "AIActionAbstract.h"
#include <bitset>
#include "AIActionAbstract.h"

// Forward declarations.

class	CAIActionMgr;
class	CAIActionAbstract;

typedef std::bitset<kAct_Count> AIActionBitSet;
enum	ENUM_AIActionSet { kAIActionSet_Invalid = -1, };

extern CAIActionMgr* g_pAIActionMgr;

// ----------------------------------------------------------------------- //

//
// CAIActionMgr
//

class CAIActionMgr
{
	public:

		CAIActionMgr();
		~CAIActionMgr();

		void				InitAIActionMgr();
		void				TermAIActionMgr();

		// Query.

		CAIActionAbstract*	GetAIAction( EnumAIActionType eAction );
		int					GetNumAIActions() const { return kAct_Count; }

		bool				IsActionInAIActionSet(ENUM_AIActionSet eSet, EnumAIActionType eAction);
		bool				ActionSupportsAbility(EnumAIActionType eAction, ENUM_ActionAbility eAbility);
		bool				ActionSetSupportsAbility(ENUM_AIActionSet eSet, ENUM_ActionAbility eAbility);

	protected:

		CAIActionAbstract*	AI_FACTORY_NEW_Action( EnumAIActionType eActionClass );

	protected:
		CAIActionAbstract*	m_pAIActions[kAct_Count];
};

// ----------------------------------------------------------------------- //

#endif
