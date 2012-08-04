// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorFlamePot.h
//
// PURPOSE : 
//
// CREATED : 4/01/05
//
// (c) 2005 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AISENSORINFLAMEPOT_H_
#define _AISENSORINFLAMEPOT_H_

#include "AISensorAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAISensorFlamePot
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAISensorFlamePot : public CAISensorAbstract
{
	typedef CAISensorAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorFlamePot, kSensor_FlamePot );

	// Ctor/Dtor

	CAISensorFlamePot();
	virtual ~CAISensorFlamePot();

	// Save/Load

	virtual void Load(ILTMessage_Read *pMsg);
	virtual void Save(ILTMessage_Write *pMsg);

	// Handles updating the sensors state.  Return true if this sensor 
	// updated, and the SensorMgr should wait to update others.
	virtual bool	UpdateSensor();

private:
	PREVENT_OBJECT_COPYING(CAISensorFlamePot);
	bool	m_bWasInFlamePotLastFrame;
	double	m_flTimeEnteredFlamePot;
};

#endif // _AISENSORFLAMEPOT_H_
