// ----------------------------------------------------------------------- //
//
// MODULE  : AIActivitySearch.h
//
// PURPOSE : AIActivitySearch class definition
//
// CREATED : 8/18/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIACTIVITY_SEARCH_H__
#define __AIACTIVITY_SEARCH_H__

#include "AIActivityOrderlyAdvance.h"

#define MAX_AI_SEARCH	MAX_AI_FOLLOW + 1

class AINode;

// ----------------------------------------------------------------------- //

struct SAI_SEARCH_PARTY
{
	LTObjRef			hSearchers[2];
	ENUM_NMComponentID	eCurComponent;
	ENUM_NMComponentID	eLastComponent;
	uint32				cNodesSearchedInComponent;
	uint32				cComponentsSearched;
};

// ----------------------------------------------------------------------- //

class CAIActivitySearch : public CAIActivityOrderlyAdvance
{
	typedef CAIActivityOrderlyAdvance super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Activity, CAIActivitySearch, kActivity_Search );

		CAIActivitySearch();
		virtual ~CAIActivitySearch();

		// CAIActivityAbstract overrides.

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);

		virtual bool	ActivateActivity();
		virtual void	DeactivateActivity();
		virtual bool	UpdateActivity();
		virtual void	ClearDeadAI();

	protected:

		HOBJECT			FindLostAI();

		virtual bool	FindLeader();
		virtual bool	BeginAdvance();
		virtual bool	HandleAdvanceNodeArrival();
		virtual void	ClearOrders();

		void			PlaySearchLocationSound( SAI_SEARCH_PARTY& SearchParty, AINode* pNode );
		bool			ValidateSearchLocationSound( HOBJECT hAI, EnumAISoundType eLocationSound );
		void			ForceAISuspicious();
		void			ClearAIThreatKnowledge( CAI* pAI );
		bool			ThreatDetectedBySquad();

		void			CloseComponentOnExit();
		bool			UpdateSearchParty( SAI_SEARCH_PARTY& SearchParty );
		bool			WaitAtComponentBorder( HOBJECT hAI, ENUM_NMComponentID eComponent );
		bool			IsSearchInProgress( CAI* pAI );

		bool			IsComponentInList( ENUM_NMComponentID eComponent, NMCOMPONENT_LIST& lstComponents );
		void			AddComponentToList( ENUM_NMComponentID eComponent, NMCOMPONENT_LIST& lstComponents );
		void			RemoveComponentFromList( ENUM_NMComponentID eComponent, NMCOMPONENT_LIST& lstComponents );
		void			ExploreComponent( ENUM_NMComponentID eComponent );

	protected:

		NMCOMPONENT_LIST	m_lstOpen;
		NMCOMPONENT_LIST	m_lstClosed;

		bool				m_bScriptedSearch;
		ENUM_NMPolyID		m_eDynamicSearchOriginPoly;

		bool				m_bSearchFromLastKnownPos;

		ENUM_NMComponentID	m_eOriginComponent;
		ENUM_NMComponentID	m_eLastComponent;

		LTObjRef			m_hOriginGuard;

		SAI_SEARCH_PARTY	m_aSearchPartys[MAX_AI_SEARCH];
};


#endif
