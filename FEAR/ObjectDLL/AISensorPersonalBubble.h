// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorPersonalBubble.h
//
// PURPOSE : AISensorPersonalBubble class definition
//
// CREATED : 6/6/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_PERSONAL_BUBBLE_H__
#define __AISENSOR_PERSONAL_BUBBLE_H__

#include "AISensorSeeEnemy.h"


class CAISensorPersonalBubble : public CAISensorSeeEnemy
{
	typedef CAISensorSeeEnemy super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorPersonalBubble, kSensor_PersonalBubble );

		CAISensorPersonalBubble();

	protected:

		// CAISensorAbstractStimulatable members.

		virtual float		GetSenseDistSqr( float fStimulusRadius );
		virtual bool		DoComplexCheck( CAIStimulusRecord* pStimulusRecord, float* pfRateModifier );
};

#endif
