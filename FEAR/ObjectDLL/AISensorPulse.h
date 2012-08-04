// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorPulse.h
//
// PURPOSE : AISensorPulse class definition
//
// CREATED : 4/14/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_PULSE_H__
#define __AISENSOR_PULSE_H__


class CAISensorPulse : public CAISensorAbstract
{
	typedef CAISensorAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorPulse, kSensor_Pulse );

		// CAISensorAbstract members.

		virtual bool	UpdateSensor();
};

#endif
