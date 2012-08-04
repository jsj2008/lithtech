// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalBerserker.h
//
// PURPOSE : 
//
// CREATED : 8/04/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIGOALBERSERKER_H_
#define _AIGOALBERSERKER_H_

LINKTO_MODULE(AIGoalBerserker);

#include "AIGoalAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIGoalBerserker
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIGoalBerserker : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalBerserker, kGoal_Berserker );

	// Ctor/Dtor

	CAIGoalBerserker();
	virtual ~CAIGoalBerserker();

	// Save/Load

	virtual void Load(ILTMessage_Read *pMsg);
	virtual void Save(ILTMessage_Write *pMsg);

	virtual void ActivateGoal();
	virtual void DeactivateGoal();
	virtual void CalculateGoalRelevance( );
	virtual void SetWSSatisfaction( CAIWorldState& WorldState );

private:
	PREVENT_OBJECT_COPYING(CAIGoalBerserker);

	// Remember who this goal is targeting with Berserker, so that central
	// memory can be cleaned up.

	LTObjRef	m_hBerserkerTarget;

	float		m_flLastActivationTime;
};

#endif // _AIGOALBERSERKER_H_

