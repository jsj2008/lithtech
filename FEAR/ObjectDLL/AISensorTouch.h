// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorTouch.h
//
// PURPOSE : 
//
// CREATED : 8/05/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AISENSORTOUCH_H_
#define _AISENSORTOUCH_H_

LINKTO_MODULE(AISensorTouch);

#include "AISensorAbstractStimulatable.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAISensorTouch
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAISensorTouch : public CAISensorAbstractStimulatable
{
	typedef CAISensorAbstractStimulatable super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorTouch, kSensor_Touch );

	// Ctor/Dtor

	CAISensorTouch();
	virtual ~CAISensorTouch();

	// Save/Load

	virtual void Load(ILTMessage_Read *pMsg);
	virtual void Save(ILTMessage_Write *pMsg);

	virtual float		GetSenseDistSqr( float fStimulusRadius );
	virtual bool		DoComplexCheck( CAIStimulusRecord* pStimulusRecord, float* pfRateModifier );
	virtual CAIWMFact*	CreateWorkingMemoryFact( CAIStimulusRecord* pStimulusRecord );

private:
	PREVENT_OBJECT_COPYING(CAISensorTouch);
};

#endif // _AISENSORTOUCH_H_
