// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorView.h
//
// PURPOSE : AISensorView class definition
//
// CREATED : 10/07/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __AISENSOR_VIEW_H__
#define __AISENSOR_VIEW_H__

#include "AISensorNode.h"


class CAISensorView : public CAISensorNode
{
	typedef CAISensorNode super;

	public:
		DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorView, kSensor_View );

		CAISensorView();

		// CAISensorAbstract members.

		virtual bool	UpdateSensor();
};

#endif
