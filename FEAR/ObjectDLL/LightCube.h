// ----------------------------------------------------------------------- //
//
// MODULE  :	LightCube.h
//
// PURPOSE :	Provides the definition for the cube projector light class 
//				which represents cubic environment map point emitter.
//
// CREATED :	2/21/03
//
// (c) 2003 Monolith Productions, Inc.  All Rights Reserved
// ----------------------------------------------------------------------- //

#ifndef __LIGHTCUBE_H__
#define __LIGHTCUBE_H__

LINKTO_MODULE( LightCube );

#ifndef __LIGHTBASE_H__
#	include "LightBase.h"
#endif

class LightCube :
	public LightBase
{
public:

	LightCube();
	~LightCube();
	
private:

	//virtual function that derived classes must override to handle loading in of
	//property data
	virtual void	ReadLightProperties(const GenericPropList *pProps);
};


#endif
