// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorInstantDeath.h
//
// PURPOSE : AISensorInstantDeath class definition
//
// CREATED : 6/10/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_INSTANT_DEATH_H__
#define __AISENSOR_INSTANT_DEATH_H__


class CAISensorInstantDeath : public CAISensorAbstract
{
	typedef CAISensorAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorInstantDeath, kSensor_InstantDeath );

		// CAISensorAbstract members.

		virtual bool	UpdateSensor();
};

#endif
