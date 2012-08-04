// ----------------------------------------------------------------------- //
//
// MODULE  : AIActivityOrderlyAdvance.h
//
// PURPOSE : AIActivityOrderlyAdvance class definition
//
// CREATED : 4/28/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTIVITY_ORDERLY_ADVANCE_H__
#define __AIACTIVITY_ORDERLY_ADVANCE_H__

#include "AIActivityAbstract.h"
#include "AIWorkingMemory.h"


#define MAX_AI_FOLLOW	4

// ----------------------------------------------------------------------- //

struct SAI_FOLLOWER
{
	LTObjRef	hFollowAI;
	float		fDistSqr;
};

// ----------------------------------------------------------------------- //

class CAIActivityOrderlyAdvance : public CAIActivityAbstract
{
	typedef CAIActivityAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Activity, CAIActivityOrderlyAdvance, kActivity_OrderlyAdvance );

		CAIActivityOrderlyAdvance();
		virtual ~CAIActivityOrderlyAdvance();

		// CAIActivityAbstract overrides.

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);

		virtual bool	FindActivityParticipants();

		virtual bool	ActivateActivity();
		virtual void	DeactivateActivity();
		virtual bool	UpdateActivity();

		virtual void	ClearDeadAI();

	protected:

		virtual bool	FindLeader();
		void			SortFollowers();
		bool			SquadRegroup();
		virtual bool	BeginAdvance();
		virtual bool	HandleAdvanceNodeArrival();
		virtual void	ClearOrders();

	protected:

		LTObjRef			m_hAdvanceAI;
		LTObjRef			m_hAdvanceNode;

		bool				m_bAdvanceCovered;

		uint32				m_cFollowAI;
		SAI_FOLLOWER		m_aFollowAI[MAX_AI_FOLLOW];

		bool				m_bEnemySpotted;

		// Task type is constant, and does not need to get save/loaded.

		ENUM_AIWMTASK_TYPE	m_eTaskType;
};


#endif
