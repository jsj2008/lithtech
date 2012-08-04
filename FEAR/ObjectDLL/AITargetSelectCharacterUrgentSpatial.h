// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionTargetCharacterUrgentSpatial.h
//
// PURPOSE : AITargetSelectCharacterUrgent class declaration
//
// CREATED : 5/19/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AITARGETSELECT_CHARACTER_URGENT_SPATIAL_H__
#define __AITARGETSELECT_CHARACTER_URGENT_SPATIAL_H__

#include "AITargetSelectCharacter.h"


// ----------------------------------------------------------------------- //

class CAITargetSelectCharacterUrgentSpatial : public CAITargetSelectCharacter
{
	typedef CAITargetSelectCharacter super;

	struct CharacterFact
	{
		CharacterFact() :
			m_flDistanceSqr(FLT_MAX)
			, m_pFact( NULL )
		{
		}

		bool operator<( const CharacterFact& OtherCharacterFact ) const
		{
			return ( m_flDistanceSqr < OtherCharacterFact.m_flDistanceSqr );
		}

		float		m_flDistanceSqr;
		CAIWMFact*	m_pFact;
	};

	typedef std::vector< CharacterFact, LTAllocator<CharacterFact, LT_MEM_TYPE_OBJECTSHELL> > TargetListType;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectCharacterUrgentSpatial, kTargetSelect_CharacterUrgent );

		// CAIActionAbstract members.

		virtual bool	ValidatePreconditions( CAI* pAI );

	protected:

		virtual CAIWMFact*	FindValidTarget( CAI* pAI );

	private:
		void CollectTargets( CAI* pAI, TargetListType& rOutTargetList ) const;
		TargetListType m_PotentialTargetList;
};

// ----------------------------------------------------------------------- //

#endif
