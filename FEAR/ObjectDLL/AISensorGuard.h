// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorGuard.h
//
// PURPOSE : AISensorGuard class definition
//
// CREATED : 4/07/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_GUARD_H__
#define __AISENSOR_GUARD_H__


class CAISensorGuard : public CAISensorAbstract
{
	typedef CAISensorAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorGuard, kSensor_Guard );

		// CAISensorAbstract members.

		virtual bool	UpdateSensor();
};

#endif
