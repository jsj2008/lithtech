// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorCritter.h
//
// PURPOSE : AISensorCritter class definition
//
// CREATED : 02/04/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_CRITTER_H__
#define __AISENSOR_CRITTER_H__

#include "AISensorAbstractStimulatable.h"


class CAISensorCritter : public CAISensorAbstractStimulatable
{
	typedef CAISensorAbstractStimulatable super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorCritter, kSensor_Critter );

		CAISensorCritter();

		// CAISensorAbstract members.

	protected:

		// CAISensorAbstractStimulatable members.

		virtual float		GetSenseDistSqr( float fStimulusRadius );
		virtual bool		StimulateSensor( CAIStimulusRecord* pStimulusRecord );
		virtual CAIWMFact*	CreateWorkingMemoryFact( CAIStimulusRecord* pStimulusRecord );
};

#endif
