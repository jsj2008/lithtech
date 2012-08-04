// ----------------------------------------------------------------------- //
//
// MODULE  :	LightPoint.h
//
// PURPOSE :	Provides the definition for the point light class which
//				represents a single color point emitter.
//
// CREATED :	2/21/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __LIGHTPOINT_H__
#define __LIGHTPOINT_H__

LINKTO_MODULE( LightPoint );

#ifndef __LIGHTBASE_H__
#	include "LightBase.h"
#endif

class LightPoint :
	public LightBase
{
public:

	LightPoint();
	~LightPoint();
	
private:

	//virtual function that derived classes must override to handle loading in of
	//property data
	virtual void	ReadLightProperties(const GenericPropList *pProps);
};


#endif
