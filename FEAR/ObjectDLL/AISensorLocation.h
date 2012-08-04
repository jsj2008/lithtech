// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorLocation.h
//
// PURPOSE : AISensorLocation class definition
//
// CREATED : 05/06/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_LOCATION_H__
#define __AISENSOR_LOCATION_H__

#include "AISensorAbstract.h"
#include "AIEnumNavMeshTypes.h"

class CAISensorLocation : public CAISensorAbstract
{
	typedef CAISensorAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorLocation, kSensor_Location );

		CAISensorLocation();

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);

		// CAISensorAbstract members.

		virtual bool	UpdateSensor();

	protected:

		ENUM_NMPolyID	m_eLastPoly;
};

#endif
