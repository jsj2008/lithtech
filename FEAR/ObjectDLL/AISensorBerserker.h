// ----------------------------------------------------------------------- //
//
// MODULE  : AISensorBerserker.h
//
// PURPOSE : 
//
// CREATED : 8/11/04
//
// (c) 2004 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef _AISENSORBERSERKER_H_
#define _AISENSORBERSERKER_H_

LINKTO_MODULE(AISensorBerserker);

#include "AISensorAbstract.h"

// ----------------------------------------------------------------------- //
//
//	CLASS:		CAISensorBerserker
//
//	PURPOSE:	
//
// ----------------------------------------------------------------------- //

class CAISensorBerserker : public CAISensorAbstract
{
	typedef CAISensorAbstract super;
	
public:
	DECLARE_AI_FACTORY_CLASS_SPECIFIC( Sensor, CAISensorBerserker, kSensor_Berserker );

	// Ctor/Dtor

	CAISensorBerserker();
	virtual ~CAISensorBerserker();

	virtual bool UpdateSensor();

	virtual void GetBerserkerSenseRange( float* pOutMinRange, float* pOutMaxRange );

private:
	PREVENT_OBJECT_COPYING(CAISensorBerserker);
};

#endif // _AISENSORBERSERKER_H_
