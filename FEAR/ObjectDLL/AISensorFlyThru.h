// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorFlyThru.h
//
// PURPOSE : AISensorFlyThru class definition
//
// CREATED : 10/13/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_FLY_THRU_H__
#define __AISENSOR_FLY_THRU_H__


class CAISensorFlyThru : public CAISensorAbstract
{
	typedef CAISensorAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorFlyThru, kSensor_FlyThru );

		// CAISensorAbstract members.

		virtual bool	UpdateSensor();

	private:

		void			FindNextFlyThruLink();
};

#endif
