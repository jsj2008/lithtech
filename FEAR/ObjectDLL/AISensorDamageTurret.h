// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorDamageTurret.h
//
// PURPOSE : AISensorDamageTurret class definition
//
// CREATED : 7/30/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_DAMAGE_TURRET_H__
#define __AISENSOR_DAMAGE_TURRET_H__

#include "AISensorDamage.h"


class CAISensorDamageTurret : public CAISensorDamage
{
	typedef CAISensorDamage super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorDamageTurret, kSensor_DamageTurret );

	protected:

		virtual bool		IsDamagerTarget( HOBJECT hDamager );

		// CAISensorAbstractStimulatable members.

		virtual void		SetFactTargetObject( CAIWMFact* pFact, CAIStimulusRecord* pStimulusRecord );
};

#endif
