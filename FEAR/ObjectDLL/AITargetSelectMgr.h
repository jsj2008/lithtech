// ----------------------------------------------------------------------- //
//
// MODULE  : AITargetSelectMgr.h
//
// PURPOSE : AITargetSelectMgr abstract class definition
//
// CREATED : 2/03/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AITARGETSELECT_MGR_H__
#define __AITARGETSELECT_MGR_H__

#include <bitset>
#include "AITargetSelectAbstract.h"

// Forward declarations.

class	CAITargetSelectMgr;
class	CAITargetSelectAbstract;

typedef std::bitset<kTargetSelect_Count> AITargetSelectBitSet;
enum	ENUM_AITargetSelectSet { kAITargetSelectSet_Invalid = -1, };

extern CAITargetSelectMgr* g_pAITargetSelectMgr;

// ----------------------------------------------------------------------- //

//
// CAITargetSelectMgr
//

class CAITargetSelectMgr
{
	public:

		CAITargetSelectMgr();
		~CAITargetSelectMgr();

		void				InitAITargetSelectMgr();
		void				TermAITargetSelectMgr();

		// Query.

		CAITargetSelectAbstract*	GetAITargetSelect( EnumAITargetSelectType eTargetSelect );
		int							GetNumAITargetSelects() const { return kTargetSelect_Count; }

		bool				IsTargetSelectInAITargetSelectSet(ENUM_AITargetSelectSet eSet, EnumAITargetSelectType eTargetSelect);

	protected:

		CAITargetSelectAbstract*	AI_FACTORY_NEW_TargetSelect( EnumAITargetSelectType eTargetSelectClass );

	protected:
		CAITargetSelectAbstract*	m_pAITargetSelects[kTargetSelect_Count];
};

// ----------------------------------------------------------------------- //

#endif
