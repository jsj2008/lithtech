// ----------------------------------------------------------------------- //
//
// MODULE  : AIGoalPickupWeapon.h
//
// PURPOSE : 
//
// CREATED : 7/07/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AIGOALPICKUPWEAPON_H_
#define _AIGOALPICKUPWEAPON_H_

LINKTO_MODULE(AIGoalPickupWeapon);


#include "AIGoalAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAIGoalPickupWeapon
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAIGoalPickupWeapon : public CAIGoalAbstract
{
	typedef CAIGoalAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Goal, CAIGoalPickupWeapon, kGoal_PickupWeapon );

	// Ctor/Dtor

	CAIGoalPickupWeapon();
	virtual ~CAIGoalPickupWeapon();

	// Save/Load

	virtual void Load(ILTMessage_Read *pMsg);
	virtual void Save(ILTMessage_Write *pMsg);

	// AIGoalAbstract

	virtual void	ActivateGoal();
	virtual void	DeactivateGoal();
	virtual void	CalculateGoalRelevance();
	virtual void	HandleBuildPlanFailure();
	virtual void	SetWSSatisfaction( CAIWorldState& /*WorldState*/ );
	virtual bool	IsWSSatisfied( CAIWorldState* /*pwsWorldState*/ );

private:
	PREVENT_OBJECT_COPYING(CAIGoalPickupWeapon);

	LTObjRef		m_hWeaponItem;
};

#endif // _AIGOALPICKUPWEAPON_H_
