// ----------------------------------------------------------------------- //
//
// MODULE  : AIActionShortRecoilOnVehicle.h
//
// PURPOSE : This action specializes ShortRecoil to explicitly support 
//			 recoiling while on a vehicle.
//
// CREATED : 3/22/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIACTIONSHORTRECOILONVEHICLE_H_
#define _AIACTIONSHORTRECOILONVEHICLE_H_

#include "AIActionShortRecoil.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIActionShortRecoilOnVehicle
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIActionShortRecoilOnVehicle : public CAIActionShortRecoil
{
	typedef CAIActionShortRecoil super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Action, CAIActionShortRecoilOnVehicle, kAct_ShortRecoilOnVehicle );

	// Ctor/Dtor

	CAIActionShortRecoilOnVehicle();
	virtual ~CAIActionShortRecoilOnVehicle();

	virtual bool ValidateContextPreconditions( CAI* pAI, CAIWorldState& wsWorldStateGoal, bool bIsPlanning );

protected:

	virtual bool GetRecoilProps(CAI* pAI, CAIWMFact* pFact, CAnimationProps& outAnimProps);

private:

	PREVENT_OBJECT_COPYING(CAIActionShortRecoilOnVehicle);
};

#endif // _AIACTIONSHORTRECOILONVEHICLE_H_
