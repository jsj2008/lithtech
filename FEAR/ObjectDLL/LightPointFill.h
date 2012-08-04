// ----------------------------------------------------------------------- //
//
// MODULE  :	LightPointFill.h
//
// PURPOSE :	Provides the definition for the point fill light class which
//				represents a single color point emitter that cannot cast shadows
//				and has no specular.
//
// CREATED :	2/21/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __LIGHTPOINTFILL_H__
#define __LIGHTPOINTFILL_H__

LINKTO_MODULE( LightPointFill );

#ifndef __LIGHTBASE_H__
#	include "LightBase.h"
#endif

class LightPointFill :
	public LightBase
{
public:

	LightPointFill();
	~LightPointFill();
	
private:

	//virtual function that derived classes must override to handle loading in of
	//property data
	virtual void	ReadLightProperties(const GenericPropList *pProps);
};


#endif
