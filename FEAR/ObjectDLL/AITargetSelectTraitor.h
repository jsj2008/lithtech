// ----------------------------------------------------------------------- //
//
// MODULE  : AITargetSelectTraitor.h
//
// PURPOSE : This sensor handles targeting ally AIs who the sensors 
//			 determined are traitors.
//
// CREATED : 3/16/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AITARGETSELECTTRAITOR_H_
#define _AITARGETSELECTTRAITOR_H_

#include "AITargetSelectAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAITargetSelectTraitor
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAITargetSelectTraitor : public CAITargetSelectAbstract
{
	typedef CAITargetSelectAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectTraitor, kTargetSelect_Traitor );

	// Ctor/Dtor

	CAITargetSelectTraitor();
	virtual ~CAITargetSelectTraitor();

	// CAIActionAbstract members.

	virtual bool	ValidatePreconditions( CAI* pAI );
	virtual void	Activate( CAI* pAI );
	virtual bool	Validate( CAI* pAI );

private:
	PREVENT_OBJECT_COPYING(CAITargetSelectTraitor);
};

#endif // _AITARGETSELECTTRAITOR_H_
