// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorEnemyDamagedSuicide.h
//
// PURPOSE : AISensorEnemyDamagedSuicide class definition
//
// CREATED : 11/01/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_ENEMY_DAMAGED_SUICIDE_H__
#define __AISENSOR_ENEMY_DAMAGED_SUICIDE_H__

#include "AISensorAbstract.h"


class CAISensorEnemyDamagedSuicide : public CAISensorAbstract
{
	typedef CAISensorAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorEnemyDamagedSuicide, kSensor_EnemyDamagedSuicide );

		CAISensorEnemyDamagedSuicide();

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);

		// CAISensorAbstract members.

		virtual bool	UpdateSensor();
};

#endif
