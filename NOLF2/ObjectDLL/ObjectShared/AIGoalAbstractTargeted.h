// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalAbstractTargeted.h
//
// PURPOSE : AIGoalAbstractTargeted abstract class definition
//
// CREATED : 8/16/01
//
// (c) 2001 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIGOAL_ABSTRACT_TARGETED_H__
#define __AIGOAL_ABSTRACT_TARGETED_H__

#include "AIGoalAbstract.h"


// Forward Declarations.

class CAIGoalAbstractTargeted : public CAIGoalAbstract, public ILTObjRefReceiver
{
	typedef CAIGoalAbstract super;

	public :

		DECLARE_AI_FACTORY_CLASS_ABSTRACT_SPECIFIC(Goal);

		// Save / Load

		virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		// ILTObjRefReceiver function.
		virtual void OnLinkBroken( LTObjRefNotifier *pRef, HOBJECT hObj );

		// Command Handling.

		virtual LTBOOL	HandleNameValuePair(const char *szName, const char *szValue);

	protected:

		LTObjRefNotifier	m_hTarget;
};


#endif
