// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalSearch.h
//
// PURPOSE : AIGoalSearch class definition
//
// CREATED : 1/11/02
//
// (c) 2002 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_SEARCH_H__
#define __AIGOAL_SEARCH_H__

#include "AIGoalAbstractSearch.h"
#include "AISenseRecorderAbstract.h"


class CAIGoalSearch : public CAIGoalAbstractSearch
{
	typedef CAIGoalAbstractSearch super;

	public:

		DECLARE_AI_FACTORY_CLASS_SPECIFIC(Goal, CAIGoalSearch, kGoal_Search);

		CAIGoalSearch( );

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// Activation.

		virtual void ActivateGoal();

		// Updating.

		void UpdateGoal();

		// Command Handling.

		virtual LTBOOL HandleNameValuePair(const char *szName, const char *szValue);

	protected:

		// State Setting.

		virtual void SetStateSearch();
};


#endif
