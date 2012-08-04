// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalInvestigate.h
//
// PURPOSE : AIGoalInvestigate class definition
//
// CREATED : 2/25/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AIAMGOAL_INVESTIGATE_H__
#define __AIAMGOAL_INVESTIGATE_H__

#include "AIGoalAbstract.h"
#include "AIEnumStimulusTypes.h"

// Forward declarations.

class CAIWMFact;


class CAIGoalInvestigate : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalInvestigate, kGoal_Investigate );

				 CAIGoalInvestigate();
		virtual ~CAIGoalInvestigate();

        virtual void	Save(ILTMessage_Write *pMsg);
        virtual void	Load(ILTMessage_Read *pMsg);

		virtual void	CalculateGoalRelevance();

		virtual bool	ReplanRequired();
		virtual void	HandleBuildPlanFailure();

		virtual void	ActivateGoal();
		virtual void	DeactivateGoal();

		virtual void	SetWSSatisfaction( CAIWorldState& WorldState );
		virtual bool	IsWSSatisfied( CAIWorldState* pwsWorldState );


	protected:

		EnumAIStimulusID	m_eDisturbanceID;
		LTObjRef			m_hTargetObject;
};

#endif
