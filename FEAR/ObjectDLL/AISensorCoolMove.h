// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorCoolMove.h
//
// PURPOSE : AISensorCoolMove class definition
//
// CREATED : 05/07/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_COOL_MOVE_H__
#define __AISENSOR_COOL_MOVE_H__

#include "AISensorAbstract.h"


// Forward declarations.


class CAISensorCoolMove : public CAISensorAbstract
{
	typedef CAISensorAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorCoolMove, kSensor_CoolMove );

		CAISensorCoolMove();

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);

		// CAISensorAbstract members.

		virtual bool	UpdateSensor();

	protected:

		bool			m_bEnteredSlowMo;
		double			m_fLastCoolMoveTime;
};

#endif
