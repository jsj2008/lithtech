// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorSeekEnemy.h
//
// PURPOSE : AISensorSeekEnemy class definition
//
// CREATED : 12/16/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_SEEK_ENEMY_H__
#define __AISENSOR_SEEK_ENEMY_H__

#include "AISensorAbstract.h"


class CAISensorSeekEnemy : public CAISensorAbstract
{
	typedef CAISensorAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorSeekEnemy, kSensor_SeekEnemy );

		CAISensorSeekEnemy();

		// CAISensorAbstract members.

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);
		virtual bool	UpdateSensor();

		// Data access.

		void			SetEnemy( HOBJECT hEnemy, bool bSeekSquadEnemy ) { m_hEnemy = hEnemy; m_bEnemyReset = true; m_bSeekSquadEnemy = bSeekSquadEnemy; }

	protected:

		void			SeekEnemy( bool bSeek );

	protected:

		LTObjRef	m_hEnemy;
		bool		m_bEnemyReset;
		bool		m_bSeekSquadEnemy;
};

#endif
