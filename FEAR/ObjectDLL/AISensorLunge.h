// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorLunge.h
//
// PURPOSE : AISensorLunge class definition
//
// CREATED : 4/27/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_LUNGE_H__
#define __AISENSOR_LUNGE_H__

#include "AISensorAbstract.h"


class CAISensorLunge : public CAISensorAbstract
{
	typedef CAISensorAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorLunge, kSensor_Lunge );

		CAISensorLunge();

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);

		// CAISensorAbstract members.

		virtual bool	UpdateSensor();

	protected:

		float			GetLungeDistance();

	protected:

		float			m_fMaxLungeDist;
		bool			m_bCalculatedLungeDist;
};

#endif
