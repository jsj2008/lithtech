
// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorCounterMelee.h
//
// PURPOSE : 
//
// CREATED : 9/08/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AISENSORCOUNTERMELEE_H_
#define _AISENSORCOUNTERMELEE_H_

#include "AISensorAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAISensorCounterMelee
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAISensorCounterMelee : public CAISensorAbstract
{
	typedef CAISensorAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorCounterMelee, kSensor_CounterMelee );

	// Ctor/Dtor

	CAISensorCounterMelee();
	virtual ~CAISensorCounterMelee();

	virtual void	Save(ILTMessage_Write *pMsg);
	virtual void	Load(ILTMessage_Read *pMsg);

	// CAISensorAbstract members.

	virtual bool	UpdateSensor();

private:
	PREVENT_OBJECT_COPYING(CAISensorCounterMelee);

	bool m_bBlockOpportunity;
	bool m_bDodgeOpportunity;
};

#endif // _AISENSORCOUNTERMELEE_H_
