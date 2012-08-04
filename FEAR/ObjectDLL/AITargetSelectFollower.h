// ----------------------------------------------------------------------- //
//
// MODULE  : AITargetSelectFollower.h
//
// PURPOSE : This class handles targeting a follower.  This is used when a
//			 follower is falling behind and needs to be told to keep up.  
//			 It is a little bit of a hack to allow head tracking/facing via 
//			 the AITarget functinality.
//
// CREATED : 4/11/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AITARGETSELECTFOLLOWER_H_
#define _AITARGETSELECTFOLLOWER_H_

#include "AITargetSelectAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAITargetSelectFollower
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAITargetSelectFollower : public CAITargetSelectAbstract
{
	typedef CAITargetSelectAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectFollower, kTargetSelect_Follower );

	// Ctor/Dtor

	CAITargetSelectFollower();
	virtual ~CAITargetSelectFollower();

	// CAIActionAbstract members.

	virtual bool	ValidatePreconditions( CAI* pAI );
	virtual void	Activate( CAI* pAI );
	virtual bool	Validate( CAI* pAI );

private:
	PREVENT_OBJECT_COPYING(CAITargetSelectFollower);
};

#endif // _AITARGETSELECTFOLLOWER_H_
