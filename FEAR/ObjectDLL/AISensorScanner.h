// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorScanner.h
//
// PURPOSE : AISensorScanner class definition
//
// CREATED : 09/15/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_SCANNER_H__
#define __AISENSOR_SCANNER_H__

#include "AISensorAbstract.h"


// Forward declarations.


class CAISensorScanner : public CAISensorAbstract
{
	typedef CAISensorAbstract super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorScanner, kSensor_Scanner );

		CAISensorScanner();

		virtual void	Save(ILTMessage_Write *pMsg);
		virtual void	Load(ILTMessage_Read *pMsg);

		// CAISensorAbstract members.

		virtual bool	UpdateSensor();
};

#endif
