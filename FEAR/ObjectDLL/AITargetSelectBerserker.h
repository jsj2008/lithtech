// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionTargetBerserker.h
//
// PURPOSE : 
//
// CREATED : 8/11/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIONTARGETBERSERKER_H_
#define _AIACTIONTARGETBERSERKER_H_

LINKTO_MODULE(AITargetSelectBerserker);


#include "AITargetSelectAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAITargetSelectBerserker
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAITargetSelectBerserker : public CAITargetSelectAbstract
{
	typedef CAITargetSelectAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( TargetSelect, CAITargetSelectBerserker, kTargetSelect_Berserker );

	// Ctor/Dtor

	CAITargetSelectBerserker();
	virtual ~CAITargetSelectBerserker();

	// CAITargetSelectAbstract

	virtual bool ValidatePreconditions( CAI* pAI );
	virtual void Activate( CAI* pAI );
	virtual bool Validate( CAI* pAI );

private:
	PREVENT_OBJECT_COPYING(CAITargetSelectBerserker);
};

#endif // _AIACTIONTARGETBERSERKER_H_
