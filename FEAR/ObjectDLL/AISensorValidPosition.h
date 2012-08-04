// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorValidPosition.h
//
// PURPOSE : Defines implementation of sensor which detects if the AI is 
//				standing in a valid position.
//
// CREATED : 4/02/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSORVALIDPOSITION_H_
#define __AISENSORVALIDPOSITION_H_

#include "AISensorAbstract.h"

class CAISensorValidPosition : public CAISensorAbstract
{
	typedef CAISensorAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorValidPosition, kSensor_ValidPosition );

		CAISensorValidPosition();

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);

		// Updating.

		virtual bool	UpdateSensor();

private:

	bool	m_bPositionValid;
};

#endif // __AISENSORVALIDPOSITION_H_
