// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorEnemyDamaged.h
//
// PURPOSE : AISensorEnemyDamaged class definition
//
// CREATED : 04/23/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_ENEMY_DAMAGED_H__
#define __AISENSOR_ENEMY_DAMAGED_H__

#include "AISensorAbstract.h"


class CAISensorEnemyDamaged : public CAISensorAbstract
{
	typedef CAISensorAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorEnemyDamaged, kSensor_EnemyDamaged );

		CAISensorEnemyDamaged();

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);

		// CAISensorAbstract members.

		virtual void	InitSensor( EnumAISensorType eSensorType, CAI* pAI );
		virtual bool	UpdateSensor();

	private:

		LTObjRef		m_hTarget;
		double			m_fLastDamageTime;
		int32			m_nHitsBeforeRetreat;
		int32			m_nMaxHitsBeforeRetreat;
};

#endif
