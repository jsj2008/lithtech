// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAbstractSearch.h
//
// PURPOSE : AIGoalAbstractSearch abstract class definition
//
// CREATED : 8/20/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_ABSTRACT_SEARCH_H__
#define __AIGOAL_ABSTRACT_SEARCH_H__

#include "AIGoalAbstractStimulated.h"

typedef std::vector< LTObjRef > AISEARCHREGION_LIST;


class CAIGoalAbstractSearch : public CAIGoalAbstractStimulated
{
	typedef CAIGoalAbstractStimulated super;

	public:

		DECLARE_AI_FACTORY_CLASS_ABSTRACT_SPECIFIC(Goal);

		CAIGoalAbstractSearch( );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void DeactivateGoal();

		// Command Handling.

		virtual LTBOOL HandleNameValuePair(const char *szName, const char *szValue);

	protected:

		// State Setting.

		virtual void SetStateSearch();

		// State Handling.

		virtual void HandleStateSearch();
		void HandleStateDraw();

	protected:

		LTBOOL			m_bSearch;
		LTBOOL			m_bFaceSearch;
		LTBOOL			m_bEngageSearch;

		AISEARCHREGION_LIST	m_lstRegions;

};


#endif
