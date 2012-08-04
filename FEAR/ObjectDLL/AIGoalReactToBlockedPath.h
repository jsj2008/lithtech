// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalReactToBlockedPath.h
//
// PURPOSE : 
//
// CREATED : 10/25/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIGOALREACTTOBLOCKEDPATH_H_
#define _AIGOALREACTTOBLOCKEDPATH_H_

#include "AIGoalAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIGoalReactToBlockedPath
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIGoalReactToBlockedPath : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalReactToBlockedPath, kGoal_ReactToBlockedPath );

	// Ctor/Dtor

	CAIGoalReactToBlockedPath();
	virtual ~CAIGoalReactToBlockedPath();

	// Save/Load

	virtual void	Load(ILTMessage_Read *pMsg);
	virtual void	Save(ILTMessage_Write *pMsg);

	virtual void	SetWSSatisfaction( CAIWorldState& WorldState );
	virtual void	CalculateGoalRelevance();

private:
	PREVENT_OBJECT_COPYING(CAIGoalReactToBlockedPath);
};

#endif // _AIGOALREACTTOBLOCKEDPATH_H_
