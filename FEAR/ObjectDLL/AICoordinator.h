// ----------------------------------------------------------------------- //
//
// MODULE  : AICoordinator.h
//
// PURPOSE : AICoordinator class definition
//
// CREATED : 5/22/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AICOORDINATOR_H__
#define __AICOORDINATOR_H__

#include "AISquad.h"


// Forward declarations.

class	CAICoordinator;
class	CAIActivityAbstract;
class	CAISquad;

// ----------------------------------------------------------------------- //

extern CAICoordinator* g_pAICoordinator;

// ----------------------------------------------------------------------- //

class CAICoordinator
{
	public:

		CAICoordinator();
		~CAICoordinator();

		void		Save(ILTMessage_Write *pMsg);
		void		Load(ILTMessage_Read *pMsg);

		void		InitAICoordinator();
		void		TermAICoordinator();

		// Squads.

		void		GenerateSquads();
		CAISquad*	FindSquad( ENUM_AI_SQUAD_ID eSquadID );
		void		CompactSquads();

		// Activities.

		bool		IsActivityInAIActivitySet( ENUM_AIActivitySet eSet, EnumAIActivityType eActivity );

		// Update.

		void		UpdateAICoordinator();

		// Query.

		ENUM_AI_SQUAD_ID	GetSquadID( HOBJECT hAI ) const;
		HOBJECT				FindAlly( HOBJECT hAI, HOBJECT hVisibleTarget );

	protected:

		AI_SQUAD_LIST				m_lstSquads;
		uint32						m_cSquads;
		uint32						m_iSquadToUpdate;
		float						m_fSquadRegenRate;
		double						m_fNextSquadRegenTime;
};

// ----------------------------------------------------------------------- //

#endif
