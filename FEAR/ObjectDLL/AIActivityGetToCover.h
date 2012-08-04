// ----------------------------------------------------------------------- //
//
// MODULE  : AIActivityGetToCover.h
//
// PURPOSE : AIActivityGetToCover class definition
//
// CREATED : 6/05/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTIVITY_GET_TO_COVER_H__
#define __AIACTIVITY_GET_TO_COVER_H__

#include "AIActivityAbstract.h"

// Forward declarations.

class CAI;
class AINode;
class CAIWMFact;

#define MAX_AI_GET_COVER		5
#define CHECK_AMBUSH			true
#define VALID_STRICT			( kNodeStatus_All & ~kNodeStatus_ThreatOutsideBoundary )
#define VALID_LENIENT			( kNodeStatus_All & ~( kNodeStatus_ThreatOutsideBoundary | kNodeStatus_ThreatAimingAtNode | kNodeStatus_ThreatUnseen ) )
#define VISIBLE_STRICT			true
#define MAX_SUPPRESSION_TIME	2.5f

// ----------------------------------------------------------------------- //

class CAIActivityGetToCover : public CAIActivityAbstract
{
	typedef CAIActivityAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Activity, CAIActivityGetToCover, kActivity_GetToCover );

		CAIActivityGetToCover();
		virtual ~CAIActivityGetToCover();

		// CAIActivityAbstract overrides.

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);

		virtual void	InitActivity();

		virtual bool	FindActivityParticipants();

		virtual bool	ActivateActivity();
		virtual void	DeactivateActivity();
		virtual bool	UpdateActivity();

		virtual void	ClearDeadAI();

	protected:

		void			AlertSquadMembers();

		bool			WaitingForSuppressionFire();

		bool			ValidParticipantsStillExist();
		bool			DestinationsAreValid();

		uint32			IssueCoverOrders();
		uint32			ClearCompleted();

		virtual void	PlaySquadFailureAISound( HOBJECT hHasCover, HOBJECT hNeedsCover );
		virtual void	PlaySquadAISound( uint32 cCovering );

		CAIWMFact*		FindValidCoverNodeFact( CAI* pAI, HOBJECT hTarget, bool bCheckAmbush );
		AINode*			AtCoverNode( CAI* pAI, bool bCheckAmbush );
		AINode*			EnRouteToCoverNode( CAI* pAI, bool bCheckAmbush );
		bool			IsCoverNodeValid( CAI* pAI, AINode* pNode, HOBJECT hTarget, uint32 dwValidation );
		void			ClearCoverTasks( CAI* pAI );
		bool			SquadHasEverSeenThreat();
		bool			SquadHasSeenThreatRecently();
		HOBJECT			FindAllySeesThreat( HOBJECT hExclude, bool bVisibleStrict );

		void			BlockNode( HOBJECT hAI, AINode* pNode );
		void			UnblockNodes();

protected:

		LTObjRef		m_hTarget;
		LTObjRef		m_hSuppressAI;
		LTObjRef		m_hCoverAI[MAX_AI_GET_COVER];
		LTObjRef		m_hHeavyArmor;

		bool			m_bPlayedFailureSound;
		bool			m_bSquadReloaded;

		// Each AI can lock two nodes: cover and dependency.

		AINode*			m_aBlockedNodes[MAX_AI_GET_COVER * 2];
		uint32			m_cBlockedNodes;
};


#endif
