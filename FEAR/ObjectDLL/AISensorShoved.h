// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorShoved.h
//
// PURPOSE : 
//
// CREATED : 11/11/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AISENSORSHOVED_H_
#define _AISENSORSHOVED_H_

#include "AISensorAbstractStimulatable.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAISensorShoved
//
//	PURPOSE:	This sensor detects when an AI has been shoved, and 
//				translates this into a fact that the AI can respond to.
//
// ----------------------------------------------------------------------- //

class CAISensorShoved : public CAISensorAbstractStimulatable
{
	typedef CAISensorAbstractStimulatable super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorShoved, kSensor_Shoved );

	// Ctor/Dtor

	CAISensorShoved();
	virtual ~CAISensorShoved();

	// Save/Load

	virtual void	Load(ILTMessage_Read *pMsg);
	virtual void	Save(ILTMessage_Write *pMsg);

	virtual float	GetSenseDistSqr(float) { return FLT_MAX; }
	virtual CAIWMFact*	CreateWorkingMemoryFact( CAIStimulusRecord* pStimulusRecord );

private:
	PREVENT_OBJECT_COPYING(CAISensorShoved);
};

#endif // _AISENSORSHOVED_H_
