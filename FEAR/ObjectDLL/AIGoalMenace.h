// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalMenace.h
//
// PURPOSE : 
//
// CREATED : 12/13/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIGOALMENACE_H_
#define _AIGOALMENACE_H_

#include "AIGoalWork.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIGoalMenace
//
//	PURPOSE:	This goal handles both scripted and unscripted use of 
//				the menace node.  It superceds use of AIGoalUseSmartObject
//				for all menace use.
//
// ----------------------------------------------------------------------- //

class CAIGoalMenace : public CAIGoalWork
{
	typedef CAIGoalWork super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalMenace, kGoal_Menace );

	// Ctor/Dtor

	CAIGoalMenace();
	virtual ~CAIGoalMenace();

	// 

	virtual void		CalculateGoalRelevance();
	virtual HOBJECT		FindBestNode( EnumAINodeType eNodeType );

private:
	PREVENT_OBJECT_COPYING(CAIGoalMenace);
};

#endif // _AIGOALMENACE_H_
