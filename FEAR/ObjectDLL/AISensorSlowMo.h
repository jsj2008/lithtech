// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorSlowMo.h
//
// PURPOSE : AISensorSlowMo class definition
//
// CREATED : 4/06/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_SLOW_MO_H__
#define __AISENSOR_SLOW_MO_H__


class CAISensorSlowMo : public CAISensorAbstract
{
	typedef CAISensorAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorSlowMo, kSensor_SlowMo );

		// CAISensorAbstract members.

		virtual bool	UpdateSensor();
};

#endif
