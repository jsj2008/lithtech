// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorLostTarget.h
//
// PURPOSE : AISensorLostTarget class definition
//
// CREATED : 08/16/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_LOST_TARGET_H__
#define __AISENSOR_LOST_TARGET_H__

#include "AISensorAbstract.h"


// Forward declarations.


class CAISensorLostTarget : public CAISensorAbstract
{
	typedef CAISensorAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorLostTarget, kSensor_LostTarget );

		CAISensorLostTarget();

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);

		// CAISensorAbstract members.

		virtual bool	UpdateSensor();

	protected:

		void			LoseTarget( bool bLost );

	protected:

		bool	m_bLostTarget;
};

#endif
