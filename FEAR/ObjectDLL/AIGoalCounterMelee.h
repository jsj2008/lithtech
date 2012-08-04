// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalCounterMelee.h
//
// PURPOSE : 
//
// CREATED : 9/09/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIGOALCOUNTERMELEE_H_
#define _AIGOALCOUNTERMELEE_H_

#include "AIGoalAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIGoalCounterMelee
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIGoalCounterMelee : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalCounterMelee, kGoal_CounterMelee );

	// Ctor/Dtor

	CAIGoalCounterMelee();
	virtual ~CAIGoalCounterMelee();

	// Save/Load

	virtual void Load(ILTMessage_Read *pMsg);
	virtual void Save(ILTMessage_Write *pMsg);

	// CAIGoalAbstract overrides.

	virtual void		CalculateGoalRelevance();
	virtual void		SetWSSatisfaction( CAIWorldState& WorldState );
	virtual void		ActivateGoal();
	virtual void		DeactivateGoal();

private:
	PREVENT_OBJECT_COPYING(CAIGoalCounterMelee);

	// Store the time of the last melee attack we attempted to counter.  To 
	// avoid duplicate responses, we cannot counter any melee attacks this 
	// age or older.

	double m_flLastCounterMeleeTime;
};

#endif // _AIGOALCOUNTERMELEE_H_
