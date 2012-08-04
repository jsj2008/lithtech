// ----------------------------------------------------------------------- //
//
// MODULE  : AIActivityAdvanceCover.h
//
// PURPOSE : AIActivityAdvanceCover class definition
//
// CREATED : 5/22/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTIVITY_ADVANCE_COVER_H__
#define __AIACTIVITY_ADVANCE_COVER_H__

#include "AIActivityGetToCover.h"

// Forward declarations.



// ----------------------------------------------------------------------- //

class CAIActivityAdvanceCover : public CAIActivityGetToCover
{
	typedef CAIActivityGetToCover super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Activity, CAIActivityAdvanceCover, kActivity_AdvanceCover );

		CAIActivityAdvanceCover();
		virtual ~CAIActivityAdvanceCover();

		// CAIActivityGetToCover overrides.

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);

		virtual bool	FindActivityParticipants();

	protected:

		virtual void	PlaySquadFailureAISound( HOBJECT hHasCover, HOBJECT hNeedsCover );
		virtual void	PlaySquadAISound( uint32 cCovering );
};


#endif
